// ***************************************************************************
// Copyright (c) 2019 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
#include "h/sqlany_utils.h"

void getErrorMsg(int code, std::string &str) {
    std::ostringstream message;
    message << "Code: " << code << " Msg: ";
    switch( code ) {
        case JS_ERR_INVALID_OBJECT: message << "Invalid Object"; break;
        case JS_ERR_INVALID_ARGUMENTS: message << "Invalid Arguments"; break;
        default: message << "Unknown JS Error";
    }
    str = message.str();
}

void getErrorMsg(a_sqlany_connection *conn, std::string &str) {
    char buffer[SACAPI_ERROR_SIZE];
    int sqlcode = api.sqlany_error(conn, buffer, sizeof(buffer));
    std::ostringstream message;
    message << "Code: " << sqlcode << " Msg: " << buffer;
    str = message.str();
}

void throwNapiError(Napi::Env env, const std::string& msg) {
    Napi::Error::New(env, msg).ThrowAsJavaScriptException();
}

void throwNapiError(Napi::Env env, int code) {
    std::string message;
    getErrorMsg(code, message);
    throwNapiError(env, message);
}

void throwNapiError(Napi::Env env, a_sqlany_connection *conn) {
    std::string message;
    getErrorMsg(conn, message);
    throwNapiError(env, message);
}