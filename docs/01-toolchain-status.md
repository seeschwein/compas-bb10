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

## XIAZY Clang 19 toolchain (Primary — Verified)

[XIAZY/blackberry10-toolchain](https://github.com/XIAZY/blackberry10-toolchain) provides a Dockerized BlackBerry 10 ARMv7 toolchain with Clang 19.1.7.

**Prebuilt image**: `ghcr.io/xiazy/blackberry10-toolchain:latest`

**Components**:
- Clang 19.1.7 frontend (C++20/23 capable)
- GNU Binutils 2.45 `armnto` linker (QNX Neutrino output)
- BB10 10.3.1 SDK: headers, Cascades/Qt 4.8.6 libraries, CRT objects
- Compiler-rt builtins for ARMv7 (VFPv3-D16, softfp)
- `bb10-cc` / `bb10-c++` gcc-compatible driver wrappers
- `blackberry-deploy`, `blackberry-nativepackager`, `blackberry-signer`
- QNX 6.5.0 SDK for x86 (simulator target)
- Java 21 with TLS 1.0 workaround for `blackberry-deploy`

### Verified build results (2025-11-05)

All 4 probes + `libprobe.so` compiled and linked with:

```bash
bb10-c++ -std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit \
  -O2 -g -Wall -Wextra -pthread -fPIC \
  source.cpp -o build/output
```

**Key fix**: `-fno-use-cxa-atexit` required to resolve `__dso_handle` undefined reference during linking.

### ELF verification (XIAZY build)

`filesystem_probe` verified as:
- ELF32
- Little-endian
- ARM
- EABI5
- Soft-float ABI
- Dynamic executable
- Interpreter: `/usr/lib/ldqnx.so.2`
- NEEDED: `libstdc++.so.6`, `libm.so.2`, `libgcc_s.so.1`, `libsocket.so.3`, `libc.so.3`

### Runtime dependencies (XIAZY build)

All probes and `libprobe.so` require:
- `libstdc++.so.6` (bundled from SDK GCC 4.8.3)
- `libm.so.2` (system)
- `libgcc_s.so.1` (**pending** — needs ARM build from GCC 4.8.3)
- `libsocket.so.3` (system)
- `libc.so.3` (system)

### Cascades app scaffolded

A complete Cascades probe runner app created at `bbndk-starter-kit/projects/compas-probes/` with:
- `bar-descriptor.xml` with `LD_LIBRARY_PATH=/app/native/probes`
- 4 probe binaries + `libprobe.so` + `libstdc++.so.6` in `assets/probes/`
- `run-probes.sh` launcher capturing stdout/stderr/exit codes
- Ready for `blackberry-nativepackager` → `blackberry-deploy`

## GCC 9.3 baseline (Reference)

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

## Known constraints

GCC 9 does not provide complete C++20/23 support. The longer-term objective is to evaluate newer GCC and Clang frontends while retaining a BB10-compatible QNX runtime.

The original GCC 9 project identifies at least two known gaps:

- `<cmath>` requires additional configuration work.
- C++17 `<execution>` is unavailable because Intel TBB depends on the removed/deprecated `getcontext()` interface.

**XIAZY Clang additional notes**:
- Default C++ standard is `gnu++98` with exceptions/RTTI **disabled** — must explicitly enable
- `libgcc_s.so.1` for ARM not present in SDK; must be built from GCC 4.8.3 sources or extracted from device
- BB10 simulator (`bb10-x86`) builds but untested (no simulator available)
