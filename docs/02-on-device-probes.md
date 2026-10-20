# On-device probes

## Purpose

The probe suite establishes whether a compiler-produced BB10 binary can execute correctly on a BlackBerry 10 device.

Compilation and linking are necessary but insufficient. The probes test runtime behavior that cannot be validated by `readelf` or a successful link.

## Probe suite

| Probe | Validates | Source |
|---|---|---|
| `filesystem_probe` | `std::filesystem`, paths, rename/remove, directory iteration | `probes/filesystem_probe.cpp` |
| `posix_probe` | Large files, `stat`, `mmap`, pipes, sockets, permissions | `probes/posix_probe.cpp` |
| `thread_probe` | Threads, mutexes, condition variables, atomics, timed waits | `probes/thread_probe.cpp` |
| `loader_probe` | `dlopen`, `dlsym`, C ABI, plugin-local exceptions, repeated load/unload | `probes/loader_probe.cpp` |

## Build status (2025-11-05)

### XIAZY Clang 19 (Primary)
All 4 probes + `libprobe.so` compile and link successfully:

```bash
bb10-c++ -std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit \
  -O2 -g -Wall -Wextra -pthread -fPIC \
  source.cpp -o build/output
```

**Key fix**: `-fno-use-cxa-atexit` required to resolve `__dso_handle` undefined reference.

### GCC 9.3 (Reference)
All 4 probes + `libprobe.so` compile and link successfully:

```bash
arm-blackberry-qnx8eabi-g++ -std=c++17 -O2 -g -Wall -Wextra \
  -pthread -fPIC -fexceptions -frtti \
  source.cpp -o build/output
```

## On-device execution status

⬜ **Not yet executed on hardware** — pending BAR packaging and deployment.

## Required result

A successful hardware run must produce four `PASS` results and exit status 0:

```text
PASS filesystem_probe
PASS posix_probe
PASS thread_probe
PASS loader_probe
```

Results written to `/accounts/1000/appdata/com.compas.probes/probe-results.txt` by `run-probes.sh`.

## Probe implementation notes

Current probes are minimal stubs that print a message and return 0. For hardware validation, each probe should be expanded to:

### filesystem_probe
- Create/write/read/delete files
- `std::filesystem::directory_iterator` traversal
- `rename()`, `remove()`, `exists()`, `is_directory()`

### posix_probe
- `open()`/`close()`/`read()`/`write()` with large files (>2GB)
- `stat()`/`fstat()`/`lstat()`
- `mmap()`/`munmap()` with `PROT_READ|PROT_WRITE`
- `pipe()`/`socketpair()` basic I/O
- `chmod()`/`chown()` permission checks

### thread_probe
- `std::thread` creation/join
- `std::mutex`/`std::condition_variable` wait/notify
- `std::atomic` load/store/compare_exchange
- `std::this_thread::sleep_for()` timed waits

### loader_probe
- `dlopen(nullptr, RTLD_NOW)` (self)
- `dlopen("libprobe.so", RTLD_NOW)` (bundled plugin)
- `dlsym()` function lookup
- Exception throw/catch across shared library boundary
- `dlclose()` cleanup

## Planned additions (post hardware validation)

- C++ exceptions across shared-library boundaries
- RTTI and `typeid`
- Virtual inheritance
- Locale and Unicode conversions (`std::wstring_convert`, ICU)
- C++20 coroutines (`co_await`, `co_yield`, `co_return`)
- Graphics primitives and OpenGL/EGL availability (`eglInitialize`, `glGetString`)

## Private runtime policy

Bundle only:

- `libstdc++.so.6` (from SDK GCC 4.8.3 — **available**)
- `libgcc_s.so.1` (from SDK GCC 4.8.3 — **pending**)

Do not bundle or replace:

- `libc.so.3`
- `libm.so.2`
- `libsocket.so.3`
- `/usr/lib/ldqnx.so.2`
