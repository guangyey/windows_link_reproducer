#pragma once
// Simulates AveragePool3d.cpp -> libtorch_xpu_ops.a (linked into b.dll)
#include "macro.h"
#include "meta.h"

// BUILD_MAIN_LIB is NOT defined -> MY_API = dllimport
struct MY_API StructuredXpu : public StructuredBase {
    void impl();
};
