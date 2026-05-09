// Simulates user code calling at::avg_pool3d(xpu_tensor).
// LoadLibraryA("b.dll") simulates explicit loading of libtorch_xpu,
// which triggers b.dll static init (_XpuRegistrar) and registers XPU kernels.
#include "meta.h"
#include <iostream>
#include <stdexcept>
#include <windows.h>

int main() {
    // Simulates loading libtorch_xpu (e.g. triggered by `import torch` in Python).
    // This causes _XpuRegistrar in b.dll to run and register the XPU kernels.
    if (!LoadLibraryA("b.dll")) {
        std::cout << "[c.exe] ERROR: failed to load b.dll: " << GetLastError() << "\n";
        return 1;
    }

    std::cout << "[c.exe] dispatch avg_pool3d\n";
    try {
        Registry::instance().dispatch("avg_pool3d");
    } catch (const std::exception& e) {
        std::cout << "[c.exe] ERROR: " << e.what()
                  << " -- _XpuRegistrar did not run\n";
        return 1;
    }
    std::cout << "[c.exe] done\n";
    return 0;
}
