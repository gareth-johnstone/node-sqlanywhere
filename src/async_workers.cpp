#include "h/async_workers.h"
#include <cmath>

Napi::Value buildResult(Napi::Env env, a_sqlany_stmt* stmt) {
    int num_cols = api.sqlany_num_cols(stmt);
    if (num_cols <= 0) {
        return Napi::Number::New(env, api.sqlany_affected_rows(stmt));
    }
    Napi::Array results = Napi::Array::New(env);
    uint32_t row_num = 0;
    while (api.sqlany_fetch_next(stmt)) {
        Napi::Object row = Napi::Object::New(env);
        for (int i = 0; i < num_cols; i++) {
            a_sqlany_column_info info;
            api.sqlany_get_column_info(stmt, i, &info);
            a_sqlany_data_value val;
            api.sqlany_get_column(stmt, i, &val);
            if (*val.is_null) {
                row.Set(info.name, env.Null());
            } else {
                switch(val.type) {
                    case A_STRING: row.Set(info.name, Napi::String::New(env, val.buffer, *val.length)); break;
                    case A_DOUBLE: row.Set(info.name, Napi::Number::New(env, *(double*)val.buffer)); break;
                    case A_VAL32: row.Set(info.name, Napi::Number::New(env, *(int*)val.buffer)); break;
                    default: row.Set(info.name, Napi::String::New(env, "Unsupported Type"));
                }
            }
        }
        results[row_num++] = row;
    }
    return results;
}

// --- Helper: Prepare C++ bind parameters ---
void ExecWorker::prepareBindParams(Napi::Array params) {
    for (uint32_t i = 0; i < params.Length(); i++) {
        a_sqlany_bind_param p;
        memset(&p, 0, sizeof(p));
        p.direction = DD_INPUT; // Set the parameter direction
        Napi::Value val = params.Get(i);

        if (val.IsString()) {
            std::string str = val.ToString().Utf8Value();
            size_t* len = new size_t(str.length());
            char* buf = new char[*len + 1];
            memcpy(buf, str.c_str(), *len + 1);
            
            p.value.buffer = buf;
            p.value.type = A_STRING;
            p.value.length = len;
            param_data.addString(buf, len);
            bind_params.push_back(p);

        } else if (val.IsNumber()) {
            double num_val = val.ToNumber().DoubleValue();
            if (floor(num_val) == num_val) {
                int* int_val = new int((int)num_val);
                p.value.buffer = (char*)int_val;
                p.value.type = A_VAL32;
                param_data.addInt(int_val);
            } else {
                double* dbl_val = new double(num_val);
                p.value.buffer = (char*)dbl_val;
                p.value.type = A_DOUBLE;
                param_data.addDouble(dbl_val);
            }
            bind_params.push_back(p);
        }
    }
}

// --- Helper: Prepare C++ bind parameters ---
void ExecStmtWorker::prepareBindParams(Napi::Array params) {
    for (uint32_t i = 0; i < params.Length(); i++) {
        a_sqlany_bind_param p;
        memset(&p, 0, sizeof(p));
        p.direction = DD_INPUT; // Set the parameter direction
        Napi::Value val = params.Get(i);

        if (val.IsString()) {
            std::string str = val.ToString().Utf8Value();
            size_t* len = new size_t(str.length());
            char* buf = new char[*len + 1];
            memcpy(buf, str.c_str(), *len + 1);
            
            p.value.buffer = buf;
            p.value.type = A_STRING;
            p.value.length = len;
            param_data.addString(buf, len);
            bind_params.push_back(p);

        } else if (val.IsNumber()) {
            double num_val = val.ToNumber().DoubleValue();
            if (floor(num_val) == num_val) {
                int* int_val = new int((int)num_val);
                p.value.buffer = (char*)int_val;
                p.value.type = A_VAL32;
                param_data.addInt(int_val);
            } else {
                double* dbl_val = new double(num_val);
                p.value.buffer = (char*)dbl_val;
                p.value.type = A_DOUBLE;
                param_data.addDouble(dbl_val);
            }
            bind_params.push_back(p);
        }
    }
}


ConnectWorker::ConnectWorker(Connection* c, const Napi::Function& cb, std::string s)
    : Napi::AsyncWorker(cb), conn_obj(c), conn_str(s), error_msg("") {}
void ConnectWorker::Execute() {
    uv_mutex_lock(&conn_obj->conn_mutex);
    if (conn_obj->conn) { error_msg = "Connection already exists."; }
    else {
        conn_obj->conn = api.sqlany_new_connection();
        if (!api.sqlany_connect(conn_obj->conn, conn_str.c_str())) {
            getErrorMsg(conn_obj->conn, error_msg);
            api.sqlany_free_connection(conn_obj->conn);
            conn_obj->conn = NULL;
        } else { openConnections++; }
    }
    uv_mutex_unlock(&conn_obj->conn_mutex);
}
void ConnectWorker::OnOK() {
    Napi::HandleScope scope(Env());
    if (!error_msg.empty()) { Callback().Call({Napi::Error::New(Env(), error_msg).Value()}); }
    else { Callback().Call({Env().Null()}); }
}

NoParamsWorker::NoParamsWorker(Connection* c, const Napi::Function& cb, Task t)
    : Napi::AsyncWorker(cb), conn_obj(c), task(t), error_msg("") {}
void NoParamsWorker::Execute() {
    uv_mutex_lock(&conn_obj->conn_mutex);
    if (!conn_obj->conn && task != Task::Disconnect) { error_msg = "Not connected."; }
    else {
        bool success = false;
        switch(task) {
            case Task::Commit: success = api.sqlany_commit(conn_obj->conn); break;
            case Task::Rollback: success = api.sqlany_rollback(conn_obj->conn); break;
            case Task::Disconnect:
                if(conn_obj->conn) {
                    conn_obj->cleanupStmts();
                    api.sqlany_disconnect(conn_obj->conn);
                    api.sqlany_free_connection(conn_obj->conn);
                    conn_obj->conn = NULL;
                    openConnections--;
                }
                success = true;
                break;
        }
        if (!success) { getErrorMsg(conn_obj->conn, error_msg); }
    }
    uv_mutex_unlock(&conn_obj->conn_mutex);
}
void NoParamsWorker::OnOK() {
    Napi::HandleScope scope(Env());
    if (!error_msg.empty()) { Callback().Call({Napi::Error::New(Env(), error_msg).Value()}); }
    else { Callback().Call({Env().Null()}); }
}

ExecWorker::ExecWorker(Connection* c, const Napi::Function& cb, std::string s, Napi::Array p)
    : Napi::AsyncWorker(cb), conn_obj(c), sql(s), result(Env().Undefined()), error_msg("") {
    prepareBindParams(p);
}
void ExecWorker::Execute() {
    uv_mutex_lock(&conn_obj->conn_mutex);
    if (bind_params.empty()) {
        stmt_handle = api.sqlany_execute_direct(conn_obj->conn, sql.c_str());
    } else {
        stmt_handle = api.sqlany_prepare(conn_obj->conn, sql.c_str());
        if(stmt_handle) {
            for (size_t i = 0; i < bind_params.size(); i++) {
                if (!api.sqlany_bind_param(stmt_handle, i, &bind_params[i])) {
                    getErrorMsg(conn_obj->conn, error_msg);
                    break;
                }
            }
            if(error_msg.empty() && !api.sqlany_execute(stmt_handle)) {
                getErrorMsg(conn_obj->conn, error_msg);
            }
        }
    }
    if (!stmt_handle && error_msg.empty()) {
        getErrorMsg(conn_obj->conn, error_msg);
    }
    uv_mutex_unlock(&conn_obj->conn_mutex);
}
void ExecWorker::OnOK() {
    Napi::HandleScope scope(Env());
    if (stmt_handle) {
        if(error_msg.empty()) { result = buildResult(Env(), stmt_handle); }
        api.sqlany_free_stmt(stmt_handle);
    }
    if (!error_msg.empty()) { Callback().Call({Napi::Error::New(Env(), error_msg).Value()}); }
    else { Callback().Call({Env().Null(), result}); }
}

PrepareWorker::PrepareWorker(Connection* c, const Napi::Function& cb, std::string s)
    : Napi::AsyncWorker(cb), conn_obj(c), sql(s), error_msg("") {}
void PrepareWorker::Execute() {
    uv_mutex_lock(&conn_obj->conn_mutex);
    stmt_handle = api.sqlany_prepare(conn_obj->conn, sql.c_str());
    if (!stmt_handle) { getErrorMsg(conn_obj->conn, error_msg); }
    uv_mutex_unlock(&conn_obj->conn_mutex);
}
void PrepareWorker::OnOK() {
    Napi::HandleScope scope(Env());
    if (stmt_handle) {
        Napi::Object stmt_obj = StmtObject::constructor.New({});
        StmtObject* unwrapped = Napi::ObjectWrap<StmtObject>::Unwrap(stmt_obj);
        unwrapped->sqlany_stmt = stmt_handle;
        unwrapped->setConnection(conn_obj);
        Callback().Call({Env().Null(), stmt_obj});
    } else {
        Callback().Call({Napi::Error::New(Env(), error_msg).Value()});
    }
}

ExecStmtWorker::ExecStmtWorker(StmtObject* s, const Napi::Function& cb, Napi::Array p)
    : Napi::AsyncWorker(cb), stmt_obj(s), result(Env().Undefined()), error_msg("") {
    prepareBindParams(p);
}
void ExecStmtWorker::Execute() {
    uv_mutex_lock(&stmt_obj->connection->conn_mutex);
    for (size_t i = 0; i < bind_params.size(); i++) {
        if (!api.sqlany_bind_param(stmt_obj->sqlany_stmt, i, &bind_params[i])) {
            getErrorMsg(stmt_obj->connection->conn, error_msg);
            break;
        }
    }
    if(error_msg.empty() && !api.sqlany_execute(stmt_obj->sqlany_stmt)) {
        getErrorMsg(stmt_obj->connection->conn, error_msg);
    }
    uv_mutex_unlock(&stmt_obj->connection->conn_mutex);
}
void ExecStmtWorker::OnOK() {
    Napi::HandleScope scope(Env());
    if (error_msg.empty()) {
        result = buildResult(Env(), stmt_obj->sqlany_stmt);
        Callback().Call({Env().Null(), result});
    } else {
        Callback().Call({Napi::Error::New(Env(), error_msg).Value()});
    }
}

DropStmtWorker::DropStmtWorker(StmtObject* s, const Napi::Function& cb)
    : Napi::AsyncWorker(cb), stmt_obj(s) {}
void DropStmtWorker::Execute() {
    stmt_obj->cleanup();
}
void DropStmtWorker::OnOK() {
    Callback().Call({Env().Null()});
}

GetMoreResultsWorker::GetMoreResultsWorker(StmtObject* s, const Napi::Function& cb)
    : Napi::AsyncWorker(cb), stmt_obj(s), result(Env().Undefined()), error_msg(""), has_more_results(false) {}
void GetMoreResultsWorker::Execute() {
    uv_mutex_lock(&stmt_obj->connection->conn_mutex);
    if (!stmt_obj || !stmt_obj->sqlany_stmt) {
        error_msg = "Statement is not valid.";
        uv_mutex_unlock(&stmt_obj->connection->conn_mutex);
        return;
    }
    has_more_results = api.sqlany_get_next_result(stmt_obj->sqlany_stmt);
    if (!has_more_results) {
        char buffer[SACAPI_ERROR_SIZE];
        int rc = api.sqlany_error(stmt_obj->connection->conn, buffer, sizeof(buffer));
        if (rc != 0 && rc != 100) {
            error_msg = buffer;
        }
    }
    uv_mutex_unlock(&stmt_obj->connection->conn_mutex);
}
void GetMoreResultsWorker::OnOK() {
    Napi::HandleScope scope(Env());
    if (!error_msg.empty()) {
        Callback().Call({Napi::Error::New(Env(), error_msg).Value()});
    } else if (has_more_results) {
        result = buildResult(Env(), stmt_obj->sqlany_stmt);
        Callback().Call({Env().Null(), result});
    } else {
        Callback().Call({Env().Null(), Env().Undefined()});
    }
}