# compas-bb10

**Compas** is a planned BlackBerry 10 / QNX-oriented maps client inspired by [CoMaps](https://codeberg.org/comaps/comaps) (formerly Organic Maps).

This repository tracks the toolchain, runtime compatibility, deployment, and application-architecture work needed to bring a modern maps experience to BB10.

It is **not** an attempt to blindly compile the full CoMaps codebase for BB10. The first objective is to establish a reliable modern C/C++ baseline, validate it on real hardware, and then build a BB10-native OSM client in practical stages.

## Current status

- A GCC 9.3-based BB10 cross-toolchain successfully compiles and links C++17 probe programs.
- The probes target 32-bit ARM, EABI5, little-endian QNX.
- The resulting executables request `/usr/lib/ldqnx.so.2`.
- The probes depend on `libstdc++.so.6`, `libm.so.2`, `libgcc_s.so.1`, `libsocket.so.3`, and `libc.so.3`.
- A private test bundle contains replacement `libstdc++.so.6` and `libgcc_s.so.1`, with matching SONAMEs.
- The probes have not yet been executed on a BlackBerry 10 device.

## Toolchains

### GCC 9 baseline

The starting point is [extrowerk/bb10-gcc9](https://github.com/extrowerk/bb10-gcc9), which builds GCC 9.3.0 for BlackBerry 10 and includes QNX header and binutils patches.

A prebuilt image is available at:

https://hub.docker.com/r/sw7ft/bb10-gcc9

This path is valuable because it preserves a known GCC/libstdc++ route for BB10 and provides a concrete baseline for C++17 runtime validation.

### Modern Clang path

[XIAZY/blackberry10-toolchain](https://github.com/XIAZY/blackberry10-toolchain) provides a Dockerized BlackBerry 10 ARMv7 toolchain. It uses Clang for compilation and QNX’s `armnto` GNU linker, startup files, libraries, and runtime helpers.

This is the most promising route for evaluating a newer C/C++ frontend without immediately replacing the entire BB10 runtime.

## Deployment without SSH

The phone does not need to be rooted, and SSH is not required for the first hardware validation.

Use the Docker image’s `blackberry-deploy` workflow:

1. Enable Development Mode on the BlackBerry.
2. Package the probes as a minimal `Device-Debug` BAR.
3. Install and launch the BAR through `blackberry-deploy`.
4. Have the BAR execute the probes in its own sandbox.
5. Write results to the application’s writable data directory.
6. Retrieve the result log through the deployment tooling.

BB10’s legacy TLS configuration may require the image’s Java runtime to permit TLS 1.0 and legacy ciphers for `blackberry-deploy`. Keep that change scoped to the Docker image rather than modifying system-wide Java settings.

Example build invocation:

```sh
BB10_CONFIGURATION=Device-Debug \
  ./tools/docker-builder.sh build /path/to/bb10-app
```

## Next steps

1. Create a minimal BAR project that bundles and runs the existing probes.
2. Add a launcher that records stdout, stderr, exit codes, and timing.
3. Build the BAR with `BB10_CONFIGURATION=Device-Debug`.
4. Install and launch it through `blackberry-deploy`.
5. Retrieve and archive `probe-results.txt`.
6. Compare GCC 9 and XIAZY Clang results.
7. Expand probes to cover C++ ABI, locales, coroutines, and graphics primitives.
8. Build a Cascades-based online OSM map viewer.
9. Add offline tile caching, GPX support, and a controlled map-data format.
10. Evaluate CoMaps core components only after the BB10 runtime baseline is proven.

## References

- XIAZY BlackBerry 10 toolchain: https://github.com/XIAZY/blackberry10-toolchain
- GCC 9 BB10 toolchain: https://github.com/extrowerk/bb10-gcc9
- Prebuilt GCC 9 image: https://hub.docker.com/r/sw7ft/bb10-gcc9
- CoMaps: https://codeberg.org/comaps/comaps
- BerryCore: https://github.com/sw7ft/BerryCore
