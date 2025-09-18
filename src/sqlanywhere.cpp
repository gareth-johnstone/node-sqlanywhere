// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
#include "napi.h"
#include "h/sqlany_utils.h"
#include "h/connection.h"
#include "h/stmt.h"

// Global variables
const char* const volatile BUILD_TIMESTAMP = "2025-09-18_18:00:00";
SQLAnywhereInterface api;
unsigned openConnections = 0;
uv_mutex_t api_mutex;

// Addon entry point
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    uv_mutex_init(&api_mutex);

    if (!sqlany_initialize_interface(&api, NULL)) {
        Napi::Error::New(env, "Could not initialize the SQL Anywhere C API interface.")
            .ThrowAsJavaScriptException();
        return exports;
    }
    if (!api.sqlany_init("node-sqlanywhere", SQLANY_API_VERSION_4, NULL)) {
         Napi::Error::New(env, "Failed to initialize the SQL Anywhere C API.")
            .ThrowAsJavaScriptException();
        return exports;
    }

    Connection::Init(env, exports);
    StmtObject::Init(env, exports);
    
    // Create a top-level createConnection function for convenience
    Napi::Function conn_constructor = exports.Get("Connection").As<Napi::Function>();
    exports.Set("createConnection", conn_constructor);

    return exports;
}

NODE_API_MODULE(sqlanywhere, Init)