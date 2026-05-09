# windows_link_reproducer

A minimal C++ reproducer for a Windows DLL linking issue in PyTorch's XPU backend.

## Background

This reproducer isolates a Windows-specific linker problem that occurs when:

1. A static library (`a.lib`, simulating `libtorch_xpu_ops.a`) defines a class (`StructuredXpu`) decorated with `__declspec(dllimport)` — because `BUILD_MAIN_LIB` is not defined at compile time.
2. That static library is linked into a DLL (`b.dll`, simulating `libtorch_xpu` / `libtorch_cpu`) via `/WHOLEARCHIVE`.
3. A static initializer (`_XpuRegistrar`) in `b.dll` registers an op kernel at load time.
4. An executable (`c.exe`, simulating `libtorch_python`) loads `b.dll` via `LoadLibraryA` and then dispatches the registered op.

The core issue is that `MY_API` on `StructuredXpu` always resolves to `__declspec(dllimport)` from the perspective of `libtorch_xpu_ops.a`, which mirrors how `TORCH_XPU_API` behaves in the real PyTorch build — neither `CAFFE2_BUILD_MAIN_LIB` nor `TORCH_XPU_BUILD_MAIN_LIB` is defined when compiling `libtorch_xpu_ops.a`.

## File Structure

| File | Simulates | Role |
|------|-----------|------|
| `meta.h` / `meta.cpp` | `libtorch_cpu` | Defines `StructuredBase` and `Registry` (kernel dispatch table), exported as `meta.dll` |
| `macro.h` | `TORCH_API` / `TORCH_XPU_API` | `MY_API` = `dllexport` if `BUILD_MAIN_LIB` is set, otherwise `dllimport` |
| `a.h` / `a.cpp` | `AveragePool3d.cpp` → `libtorch_xpu_ops.a` | Defines `StructuredXpu` as a static lib; `MY_API` is always `dllimport` here |
| `b.cpp` | `RegisterXPU_0.cpp` → `libtorch_xpu` (b.dll) | Defines `_XpuRegistrar` static init that registers `avg_pool3d`; links `a.lib` via `/WHOLEARCHIVE` |
| `c.cpp` | `libtorch_python` (c.exe) | Loads `b.dll` via `LoadLibraryA`, then dispatches `avg_pool3d` |
| `CMakeLists.txt` | Build system | Supports both XPU path (static `a.lib` + `b.dll`) and CUDA path (all-in-one `b.dll`) |

## Build

### Prerequisites

- Windows
- CMake >= 3.15
- MSVC (Visual Studio)

### Steps

```bat
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

The `FOR_CUDA` CMake option switches between two linking modes:

- `FOR_CUDA=FALSE` (default) — XPU path: `a.cpp` is compiled into `a.lib` (static), then linked into `b.dll` via `WHOLE_ARCHIVE`. This is the path that triggers the issue.
- `FOR_CUDA=TRUE` — CUDA path: `a.cpp` and `b.cpp` are compiled directly into `b.dll`. No static lib involved; no issue.

## Running

```bat
cd build/Release
c.exe
```

### Expected output (success)

```
[b.dll] _XpuRegistrar: registering avg_pool3d
[c.exe] dispatch avg_pool3d
[meta] Registry::dispatch(avg_pool3d)
call wrapper_XPU_avg_pool3d
call Op::check!
call Base::meta!
[c.exe] done
```

### Failure symptom

`_XpuRegistrar` does not run (static initializer is dead-stripped or the vtable is not correctly resolved due to the `dllimport` mismatch), leading to:

```
[c.exe] ERROR: map::at -- _XpuRegistrar did not run
```

## Root Cause

When `a.cpp` is compiled into a **static library**, `MY_API` expands to `__declspec(dllimport)` because `BUILD_MAIN_LIB` is not defined. This makes the vtable pointer of `StructuredXpu` point to an import thunk rather than the actual vtable in `b.dll`. When `b.dll` is built by linking `a.lib` via `/WHOLEARCHIVE`, the linker emits warnings `LNK4049`/`LNK4217` (locally defined symbol imported) and may silently drop the static initializer or produce incorrect vtable resolution.

The CUDA build avoids this by compiling `a.cpp` directly into `b.dll` alongside `b.cpp`, so `BUILD_MAIN_LIB` is defined (or the export/import attribute is consistent), and no import thunk is involved.
