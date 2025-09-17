// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
#pragma once
#include <uv.h>
#include "napi.h"
#include "sqlany_utils.h"

// Forward declare Connection to avoid circular dependency
class Connection;

class StmtObject : public Napi::ObjectWrap<StmtObject> {
public:
    static Napi::FunctionReference constructor;

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    StmtObject(const Napi::CallbackInfo& info);
    ~StmtObject();

    // Public methods
    void cleanup();
    void setConnection(Connection *conn_obj);

    // Public properties
    Connection *connection;
    a_sqlany_stmt *sqlany_stmt;

private:
    // N-API Wrapped Methods
    Napi::Value Exec(const Napi::CallbackInfo& info);
    Napi::Value Drop(const Napi::CallbackInfo& info);
    Napi::Value GetMoreResults(const Napi::CallbackInfo& info);
};