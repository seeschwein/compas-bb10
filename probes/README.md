# Compas BB10 Probe Suite

On-device compatibility probes for BlackBerry 10 / QNX.

## What this is for

The probes answer one question: *does the modern C/C++ stack we use to build
Compas actually work on a real BB10 device?*  Each probe is a tiny
standalone executable that exercises one area of the runtime.  A probe exits
with status 0 on success, non-zero on failure.

| Probe | Exercises |
|---|---|
| `filesystem_probe` | `std::filesystem`, paths, rename/remove, directory iteration |
| `posix_probe` | large files, `stat`, `mmap`, pipes, sockets, permissions |
| `thread_probe` | threads, mutexes, condition variables, atomics, timed waits |
| `loader_probe` | `dlopen`, `dlsym`, `dlclose`, C ABI, plugin-local exceptions |

`libprobe.so` is a small shared library used by `loader_probe` to exercise
the dynamic loader.

## Structure

```
probes/
├── CMakeLists.txt         # build for the runner + probes + libprobe.so
├── bar-descriptor.xml     # BAR manifest (entry point = compas-probes)
├── icon.png
├── probe_runner.cpp       # Cascades app: runs the probes, shows the report
├── filesystem_probe.cpp
├── posix_probe.cpp
├── thread_probe.cpp
├── loader_probe.cpp
├── plugin.cpp             # -> libprobe.so
└── assets/
    ├── main.qml           # UI showing the report
    ├── qml.qrc
    ├── libs/              # bundled private runtime libs
    │   ├── libgcc_s.so.1
    │   └── libstdc++.so.6
    └── probes/            # probe binaries + libprobe.so (filled by the build)
```

## Build

```bash
# From the repository root:
make build-probes
```

Or directly:

```bash
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src
```

Output lands in `probes/build/cmake/arm/o.le-v7-g/`.

## Package & deploy

```bash
# From the repository root:
make package-probes

# Deploy (device must be in Development Mode):
make deploy BAR=compas-probes.bar DEVICE_IP=<IP> DEVICE_PASSWORD=<pass>
```

## Reading the results

The runner writes a report to:

```
/accounts/1000/appdata/com.compas.probes/probe-results.txt
```

The same text is displayed in the app UI, so you can read it on the device
without pulling files off it.

## Important BB10 constraint

**The BAR entry point must be an ELF binary.**  A shell script cannot be
used as an application entry point — the packager rejects it with:

```
Error: Attribute Entry-Point: file cannot be found in the list of packaged files
```

That is why the probes are launched by a Cascades application
(`compas-probes`) rather than by a `run-probes.sh` script.

## Bundled runtime libraries

Only the private runtime libraries are bundled:

- `libstdc++.so.6` — from the BB10 SDK (`/opt/bb10-sdk/target/qnx6/armle-v7/lib/`)
- `libgcc_s.so.1` — not in the SDK; a copy is kept in `assets/libs/`

Never bundle `libc.so.3`, `libm.so.2`, `libsocket.so.3` or `ldqnx.so.2` —
the device provides those.
