# Toolchain status

## GCC 9.3 baseline

The initial toolchain is derived from [extrowerk/bb10-gcc9](https://github.com/extrowerk/bb10-gcc9).

A prebuilt Docker image is available at [sw7ft/bb10-gcc9](https://hub.docker.com/r/sw7ft/bb10-gcc9).

### Verified build results

The following targets compiled and linked successfully with `-std=c++17`:

- `filesystem_probe`
- `posix_probe`
- `thread_probe`
- `loader_probe`
- `libprobe.so`

The build used:

```sh
arm-blackberry-qnx8eabi-g++ \
  -O2 -g -Wall -Wextra -std=c++17 -pthread \
  -fPIC -fexceptions -frtti
```

### ELF verification

`filesystem_probe` was verified as:

- ELF32
- Little-endian
- ARM
- EABI5
- Soft-float ABI
- Dynamic executable
- Interpreter: `/usr/lib/ldqnx.so.2`

### Runtime dependencies

All probes and `libprobe.so` require:

- `libstdc++.so.6`
- `libm.so.2`
- `libgcc_s.so.1`
- `libsocket.so.3`
- `libc.so.3`

The staged replacement libraries have these SONAMEs:

- `libstdc++.so.6`
- `libgcc_s.so.1`

No `RPATH` or `RUNPATH` entries were observed in the inspected binaries.

## XIAZY Clang toolchain

[XIAZY/blackberry10-toolchain](https://github.com/XIAZY/blackberry10-toolchain) is the primary comparison toolchain.

It provides a Dockerized BlackBerry 10 ARMv7 environment using:

- Clang for compilation
- QNX `armnto` GNU linker
- QNX 6.5 startup files
- QNX libraries and runtime helpers

The next toolchain task is to build and run the same probe suite through this path, then compare ELF metadata, dependencies, runtime behavior, and on-device results.

## Additional toolchain available

GCC 9.3 prebuilt Docker image

A prebuilt GCC 9.3 Docker image is now available at `sw7ft/bb10-gcc9:latest` (Docker Hub). This provides:

- GCC 9.3 cross-compiler: `arm-blackberry-qnx8eabi-g++`
- QNX binutils and headers for BlackBerry 10 ARMv7 EABI5 little-endian

Verified build results with the Docker image match the existing toolchain baseline, producing ELF32 ARM EABI5 little-endian dynamic executables with interpreter `/usr/lib/ldqnx.so.2`.

## Known constraints

GCC 9 does not provide complete C++20/23 support. The longer-term objective is to evaluate newer GCC and Clang frontends while retaining a BB10-compatible QNX runtime.

The original GCC 9 project identifies at least two known gaps:

- `<cmath>` requires additional configuration work.
- C++17 `<execution>` is unavailable because Intel TBB depends on the removed/deprecated `getcontext()` interface.
