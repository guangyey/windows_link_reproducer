// Simulates aten/src/ATen/native/AveragePool3d.cpp TORCH_META_FUNC part -> libtorch_cpu
// dllexport here because this is the defining side (libtorch_cpu builds with CAFFE2_BUILD_MAIN_LIB)
#include "meta.h"
#include <iostream>

void StructuredBase::meta() {
    // shape inference / output allocation logic
    std::cout << "call Base::meta!" << std::endl;
}

// Registry implementation lives in b.dll
Registry& Registry::instance() {
    static Registry r;
    return r;
}

void Registry::register_kernel(const std::string& op, std::function<void()> fn) {
    std::cout << "[meta] Registry::register_kernel(" << op << ")\n";
    table_[op] = fn;
}

void Registry::dispatch(const std::string& op) {
    std::cout << "[meta] Registry::dispatch(" << op << ")\n";
    table_.at(op)();
}
