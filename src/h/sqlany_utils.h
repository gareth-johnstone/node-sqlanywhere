// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
#pragma once
#include <uv.h>
#include "napi.h"
#include "sacapidll.h"
#include "errors.h"
#include <string>
#include <vector>
#include <sstream>

// Forward declarations
class Connection;
class StmtObject;

// Global API interface, defined in sqlanywhere.cpp
extern SQLAnywhereInterface api;
extern unsigned openConnections;
extern uv_mutex_t api_mutex;

// Utility functions
void getErrorMsg(int code, std::string &str);
void getErrorMsg(a_sqlany_connection *conn, std::string &str);
void throwNapiError(Napi::Env env, const std::string& message);
void throwNapiError(Napi::Env env, int code);
void throwNapiError(Napi::Env env, a_sqlany_connection *conn);