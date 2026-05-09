// Simulates RegisterXPU_0.cpp -> compiled into b.dll (libtorch_xpu)
#include "a.h"
#include <iostream>

// StructuredXpu defined in a.cpp (libtorch_xpu_ops.a), linked into this dll.
struct StructuredXpuOp final : public StructuredXpu {
    void check() {
        std::cout << "call Op::check!" << std::endl;
    }
};

// Simulates structured_avg_pool3d_out_xpu_functional + wrapper_XPU_avg_pool3d
static void wrapper_XPU_avg_pool3d() {
    std::cout << "call wrapper_XPU_avg_pool3d" << std::endl;
    StructuredXpuOp op;
    op.check();
    op.meta();
    op.impl();
}

// Simulates TORCH_LIBRARY_IMPL(aten, XPU, m) { m.impl("avg_pool3d", ...) }
struct _XpuRegistrar {
    _XpuRegistrar() {
        std::cout << "[b.dll] _XpuRegistrar: registering avg_pool3d\n";
        Registry::instance().register_kernel("avg_pool3d", wrapper_XPU_avg_pool3d);
    }
} _xpu_registrar;
