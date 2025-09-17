#pragma once
#include <napi.h>
#include "sacapi.h"
#include <vector>
#include <string>

// A helper class to manage the lifetime of allocated memory for bind parameters,
// mimicking the original driver's strategy. This prevents pointers
// from becoming invalid when passed to the database client.
class ExecuteData {
public:
    ~ExecuteData() {
        for (auto p : int_vals) delete p;
        for (auto p : ll_vals) delete p; // For 64-bit integers
        for (auto p : double_vals) delete p;
        for (auto p : string_vals) delete[] p;
        for (auto p : len_vals) delete p;
    }

    void addInt(int* val) { int_vals.push_back(val); }
    void addLongLong(long long* val) { ll_vals.push_back(val); } // For 64-bit integers
    void addDouble(double* val) { double_vals.push_back(val); }
    void addString(char* str, size_t* len) {
        string_vals.push_back(str);
        len_vals.push_back(len);
    }

private:
    std::vector<int*> int_vals;
    std::vector<long long*> ll_vals; // For 64-bit integers
    std::vector<double*> double_vals;
    std::vector<char*> string_vals;
    std::vector<size_t*> len_vals;
};