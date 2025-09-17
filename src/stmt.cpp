#include "h/stmt.h"
#include "h/connection.h"
#include "h/async_workers.h"

Napi::FunctionReference StmtObject::constructor;

Napi::Object StmtObject::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(env, "Statement", {
        InstanceMethod("exec", &StmtObject::Exec),
        InstanceMethod("drop", &StmtObject::Drop),
        InstanceMethod("getMoreResults", &StmtObject::GetMoreResults),
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Statement", func);
    return exports;
}

StmtObject::StmtObject(const Napi::CallbackInfo& info) : Napi::ObjectWrap<StmtObject>(info) {
    this->sqlany_stmt = NULL;
    this->connection = NULL;
}

StmtObject::~StmtObject() {
    cleanup();
}

void StmtObject::cleanup() {
    if (this->sqlany_stmt) {
        api.sqlany_free_stmt(this->sqlany_stmt);
        this->sqlany_stmt = NULL;
    }
    if (this->connection) {
        this->connection->removeStmt(this);
        this->connection = NULL;
    }
}

void StmtObject::setConnection(Connection *conn_obj) {
    this->connection = conn_obj;
    this->connection->statements.push_back(this);
}

Napi::Value StmtObject::Exec(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    int params_idx = -1;
    int callback_idx = 0;
    if (info.Length() < 1) {
        throwNapiError(env, "Statement.exec requires a callback function.");
        return env.Undefined();
    }
    if (info.Length() > 1) {
        params_idx = 0;
        callback_idx = 1;
    }
    if (!info[callback_idx].IsFunction()) {
        throwNapiError(env, "The last argument to Statement.exec must be a callback function.");
        return env.Undefined();
    }
    if (params_idx != -1 && !info[params_idx].IsArray()) {
        throwNapiError(env, "Parameters for Statement.exec must be an array.");
        return env.Undefined();
    }
    Napi::Array params = (params_idx != -1) ? info[params_idx].As<Napi::Array>() : Napi::Array::New(env);
    Napi::Function callback = info[callback_idx].As<Napi::Function>();
    (new ExecStmtWorker(this, callback, params))->Queue();
    return env.Undefined();
}

Napi::Value StmtObject::Drop(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        throwNapiError(env, "Statement.drop requires a callback function.");
        return env.Undefined();
    }
    (new DropStmtWorker(this, info[0].As<Napi::Function>()))->Queue();
    return env.Undefined();
}

Napi::Value StmtObject::GetMoreResults(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsFunction()) {
        throwNapiError(env, "getMoreResults requires a callback function.");
        return env.Undefined();
    }
    Napi::Function callback = info[0].As<Napi::Function>();
    (new GetMoreResultsWorker(this, callback))->Queue();
    return env.Undefined();
}