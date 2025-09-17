#pragma once
#include <uv.h>
#include "napi.h"
#include "connection.h"
#include "stmt.h"
#include "execute_data.h"
#include <vector>
#include <string>

// --- Standalone Helper Function Declaration ---
Napi::Value buildResult(Napi::Env env, a_sqlany_stmt* stmt);

// --- Worker Classes ---
class ConnectWorker;
class NoParamsWorker;
class ExecWorker;
class PrepareWorker;
class ExecStmtWorker;
class DropStmtWorker;
class GetMoreResultsWorker;

class ExecWorker : public Napi::AsyncWorker {
public:
    ExecWorker(Connection* conn_obj, const Napi::Function& callback, std::string sql, Napi::Array params);
    void Execute();
    void OnOK();
private:
    // prepareBindParams is now a free function, so it's removed from here.
    Connection* conn_obj;
    std::string sql;
    Napi::Value result;
    std::string error_msg;
    a_sqlany_stmt* stmt_handle = nullptr;
    std::vector<a_sqlany_bind_param> bind_params;
    ExecuteData param_data;
};

class ExecStmtWorker : public Napi::AsyncWorker {
public:
    ExecStmtWorker(StmtObject* stmt_obj, const Napi::Function& callback, Napi::Array params);
    void Execute();
    void OnOK();
private:
    // prepareBindParams is now a free function, so it's removed from here.
    StmtObject* stmt_obj;
    Napi::Value result;
    std::string error_msg;
    std::vector<a_sqlany_bind_param> bind_params;
    ExecuteData param_data;
};

class ConnectWorker : public Napi::AsyncWorker {
public:
    ConnectWorker(Connection* conn_obj, const Napi::Function& callback, std::string conn_str);
    void Execute();
    void OnOK();
private:
    Connection* conn_obj;
    std::string conn_str;
    std::string error_msg;
};

class NoParamsWorker : public Napi::AsyncWorker {
public:
    enum class Task { Commit, Rollback, Disconnect };
    NoParamsWorker(Connection* conn_obj, const Napi::Function& callback, Task task);
    void Execute();
    void OnOK();
private:
    Connection* conn_obj;
    Task task;
    std::string error_msg;
};

class PrepareWorker : public Napi::AsyncWorker {
public:
    PrepareWorker(Connection* conn_obj, const Napi::Function& callback, std::string sql);
    void Execute();
    void OnOK();
private:
    Connection* conn_obj;
    std::string sql;
    a_sqlany_stmt* stmt_handle = nullptr;
    std::string error_msg;
};

class DropStmtWorker : public Napi::AsyncWorker {
public:
    DropStmtWorker(StmtObject* stmt_obj, const Napi::Function& callback);
    void Execute();
    void OnOK();
private:
    StmtObject* stmt_obj;
};

class GetMoreResultsWorker : public Napi::AsyncWorker {
public:
    GetMoreResultsWorker(StmtObject* stmt_obj, const Napi::Function& callback);
    void Execute();
    void OnOK();
private:
    StmtObject* stmt_obj;
    Napi::Value result;
    std::string error_msg;
    bool has_more_results = false;
};