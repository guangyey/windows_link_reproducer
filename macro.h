#pragma once

// MY_API simulates TORCH_API / TORCH_XPU_API as seen from libtorch_xpu_ops.a:
// neither CAFFE2_BUILD_MAIN_LIB nor TORCH_XPU_BUILD_MAIN_LIB is defined there,
#ifdef BUILD_MAIN_LIB
#define MY_API __declspec(dllexport)
#else
#define MY_API __declspec(dllimport)
#endif
