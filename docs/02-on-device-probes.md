# On-device probes

## Purpose

The probe suite establishes whether a compiler-produced BB10 binary can execute correctly on a BlackBerry 10 device.

Compilation and linking are necessary but insufficient. The probes test runtime behavior that cannot be validated by `readelf` or a successful link.

## Probe suite

| Probe | Validates |
|---|---|
| `filesystem_probe` | `std::filesystem`, paths, rename/remove, directory iteration |
| `posix_probe` | Large files, `stat`, `mmap`, pipes, sockets, permissions |
| `thread_probe` | Threads, mutexes, condition variables, atomics, timed waits |
| `loader_probe` | `dlopen`, `dlsym`, C ABI, plugin-local exceptions, repeated load/unload |

## Current result

All probes compile and link successfully with the GCC 9.3 toolchain.

They have not yet been executed on a BlackBerry 10 device.

## Required result

A successful hardware run must produce four `PASS` results and exit status 0:

```text
PASS filesystem
PASS POSIX I/O
PASS threads/mutex/CV/atomics
PASS dlopen/dlsym/dlclose
```

## Planned additions

After the initial hardware run, add probes for:

- C++ exceptions across shared-library boundaries
- RTTI and `typeid`
- Virtual inheritance
- Locale and Unicode conversions
- C++20 coroutines
- Graphics primitives and OpenGL/EGL availability

## Private runtime policy

Bundle only:

- `libstdc++.so.6`
- `libgcc_s.so.1`

Do not bundle or replace:

- `libc.so.3`
- `libm.so.2`
- `libsocket.so.3`
- `/usr/lib/ldqnx.so.2`
