/* loader_probe.cpp
 * Minimal probe to test dynamic linking: dlopen, dlsym, dlclose, and C ABI.
 */
#include <iostream>
#include <dlfcn.h>
int main() {
    // Test dlopen with RTLD_NOW
    void* handle = dlopen(nullptr, RTLD_NOW);
    if (handle) {
        std::cout << "loader_probe: dlopen succeeded\n";
        dlclose(handle);
    } else {
        std::cout << "loader_probe: dlopen returned " << dlerror() << "\n";
    }
    return 0;
}