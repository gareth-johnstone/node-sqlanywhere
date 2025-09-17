#include "h/connection.h"
#include "h/async_workers.h"

Napi::FunctionReference Connection::constructor;

Napi::Object Connection::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(env, "Connection", {
        InstanceMethod("connect", &Connection::Connect),
        InstanceMethod("disconnect", &Connection::Disconnect),
        InstanceMethod("close", &Connection::Disconnect),
        InstanceMethod("exec", &Connection::Exec),
        InstanceMethod("prepare", &Connection::Prepare),
        InstanceMethod("commit", &Connection::Commit),
        InstanceMethod("rollback", &Connection::Rollback),
        InstanceMethod("connected", &Connection::Connected),
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Connection", func);
    return exports;
}

Connection::Connection(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Connection>(info) {
    this->conn = NULL;
    uv_mutex_init(&this->conn_mutex);
}

Connection::~Connection() {
    uv_mutex_lock(&this->conn_mutex);
    cleanupStmts();
    if (this->conn) {
        api.sqlany_disconnect(this->conn);
        api.sqlany_free_connection(this->conn);
        this->conn = NULL;
        openConnections--;
    }
    uv_mutex_unlock(&this->conn_mutex);
    uv_mutex_destroy(&this->conn_mutex);
}

void Connection::removeStmt(StmtObject* stmt) {
    for (size_t i = 0; i < statements.size(); ++i) {
        if (statements[i] == stmt) {
            statements.erase(statements.begin() + i);
            break;
        }
    }
}

void Connection::cleanupStmts() {
    for (auto const& stmt : statements) {
        stmt->cleanup();
    }
    statements.clear();
}

Napi::Value Connection::Connect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsObject() || !info[1].IsFunction()) {
        throwNapiError(env, "connect requires a connection parameters object and a callback function.");
        return env.Undefined();
    }
    Napi::Object params_obj = info[0].As<Napi::Object>();
    Napi::Function callback = info[1].As<Napi::Function>();
    std::string conn_str;
    Napi::Array props = params_obj.GetPropertyNames();
    for (uint32_t i = 0; i < props.Length(); i++) {
        Napi::Value key_val = props.Get(i);
        Napi::Value val_val = params_obj.Get(key_val);
        conn_str += key_val.ToString().Utf8Value() + "=" + val_val.ToString().Utf8Value() + ";";
    }
    conn_str.append("CHARSET=UTF-8");
    (new ConnectWorker(this, callback, conn_str))->Queue();
    return env.Undefined();
}

Napi::Value Connection::Disconnect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        throwNapiError(env, "disconnect requires a callback function.");
        return env.Undefined();
    }
    (new NoParamsWorker(this, info[0].As<Napi::Function>(), NoParamsWorker::Task::Disconnect))->Queue();
    return env.Undefined();
}

Napi::Value Connection::Exec(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int sql_idx = 0;
    int params_idx = -1;
    int callback_idx = 1;
    if (info.Length() < 2) {
        throwNapiError(env, "exec requires at least a SQL string and a callback.");
        return env.Undefined();
    }
    if (info.Length() > 2) {
        params_idx = 1;
        callback_idx = 2;
    }
    if (!info[sql_idx].IsString() || !info[callback_idx].IsFunction()) {
        throwNapiError(env, "Invalid arguments for exec: expecting (sql, [params], callback).");
        return env.Undefined();
    }
    if (params_idx != -1 && !info[params_idx].IsArray()) {
        throwNapiError(env, "Parameters for exec must be an array.");
        return env.Undefined();
    }
    std::string sql = info[sql_idx].ToString().Utf8Value();
    Napi::Array params = (params_idx != -1) ? info[params_idx].As<Napi::Array>() : Napi::Array::New(env);
    Napi::Function callback = info[callback_idx].As<Napi::Function>();
    (new ExecWorker(this, callback, sql, params))->Queue();
    return env.Undefined();
}

Napi::Value Connection::Prepare(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsFunction()) {
        throwNapiError(env, "prepare requires a SQL string and a callback function.");
        return env.Undefined();
    }
    std::string sql = info[0].ToString().Utf8Value();
    Napi::Function callback = info[1].As<Napi::Function>();
    (new PrepareWorker(this, callback, sql))->Queue();
    return env.Undefined();
}

Napi::Value Connection::Commit(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        throwNapiError(env, "commit requires a callback function.");
        return env.Undefined();
    }
    (new NoParamsWorker(this, info[0].As<Napi::Function>(), NoParamsWorker::Task::Commit))->Queue();
    return env.Undefined();
}

Napi::Value Connection::Rollback(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        throwNapiError(env, "rollback requires a callback function.");
        return env.Undefined();
    }
    (new NoParamsWorker(this, info[0].As<Napi::Function>(), NoParamsWorker::Task::Rollback))->Queue();
    return env.Undefined();
}

Napi::Value Connection::Connected(const Napi::CallbackInfo& info) {
    return Napi::Boolean::New(info.Env(), this->conn != NULL);
}