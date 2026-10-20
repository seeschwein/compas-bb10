# Project charter

## Mission

Compas aims to deliver a practical, privacy-respecting OpenStreetMap-based maps experience for BlackBerry 10 devices.

The project begins with toolchain and runtime validation, then progresses toward a Cascades-based OSM client. It does not assume that CoMaps can be compiled unchanged for BB10.

## Non-goals

- Do not treat a successful compile as runtime compatibility.
- Do not replace BB10 system libraries such as `libc.so.3`, `libm.so.2`, `libsocket.so.3`, or the QNX dynamic loader.
- Do not couple the project to CoMaps’ internal `.mwm` data format.
- Do not attempt full CoMaps offline search and routing before the underlying C++ runtime is proven.

## Guiding principles

- Validate on real BB10 hardware early.
- Keep compiler frontend work separate from QNX runtime compatibility.
- Preserve backwards compatibility with existing BB10 binaries where practical.
- Prefer small, testable milestones.
- Use controlled data formats for the first offline-map implementation.

## Initial technical findings

The GCC 9.3 toolchain produced ARM 32-bit EABI5 little-endian QNX executables requesting `/usr/lib/ldqnx.so.2`.

The probe binaries require:

- `libstdc++.so.6`
- `libm.so.2`
- `libgcc_s.so.1`
- `libsocket.so.3`
- `libc.so.3`

The staged private runtime contains only:

- `libstdc++.so.6`
- `libgcc_s.so.1`

The phone should continue providing `libc`, `libm`, `libsocket`, and its dynamic loader.
