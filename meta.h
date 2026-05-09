#pragma once
#include "macro.h"
#include <map>
#include <string>
#include <functional>

struct MY_API StructuredBase {
    void meta();
};

struct MY_API Registry {
    static Registry& instance();
    void register_kernel(const std::string& op, std::function<void()> fn);
    void dispatch(const std::string& op);
private:
    std::map<std::string, std::function<void()>> table_;
};
