// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
#pragma once
#include <uv.h>
#include "napi.h"
#include "sqlany_utils.h"
#include "stmt.h"
#include <vector>
#include <string>

class Connection : public Napi::ObjectWrap<Connection> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Connection(const Napi::CallbackInfo& info);
    ~Connection();

    // Public properties
    a_sqlany_connection *conn;
    std::vector<StmtObject*> statements;
    uv_mutex_t conn_mutex;
    unsigned int max_api_ver;
    bool sqlca_connection;
    std::string _arg;

    // Public methods
    void removeStmt(StmtObject *stmt);
    void cleanupStmts();

private:
    static Napi::FunctionReference constructor;

    // N-API Wrapped Methods
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value Disconnect(const Napi::CallbackInfo& info);
    Napi::Value Exec(const Napi::CallbackInfo& info);
    Napi::Value Prepare(const Napi::CallbackInfo& info);
    Napi::Value Commit(const Napi::CallbackInfo& info);
    Napi::Value Rollback(const Napi::CallbackInfo& info);
    Napi::Value Connected(const Napi::CallbackInfo& info);
};