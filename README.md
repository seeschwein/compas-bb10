# compas-bb10

**Compas** is a planned BlackBerry 10 / QNX-oriented maps client inspired by [CoMaps](https://codeberg.org/comaps/comaps) (formerly Organic Maps).

This repository tracks the toolchain, runtime compatibility, deployment, and application-architecture work needed to bring a modern maps experience to BB10.

It is **not** an attempt to blindly compile the full CoMaps codebase for BB10. The first objective is to establish a reliable modern C/C++ baseline, validate it on real hardware, and then build a BB10-native OSM client in practical stages.

## Current status (2025-11-05)

- **Probe suite built & validated** with XIAZY Clang 19.1.7 toolchain (`ghcr.io/xiazy/blackberry10-toolchain:latest`)
- 4 probes compile and link successfully for ARMv7 (EABI5, little-endian, softfp):
  - `filesystem_probe` — `std::filesystem`, paths, rename/remove, directory iteration
  - `posix_probe` — large files, `stat`, `mmap`, pipes, sockets, permissions
  - `thread_probe` — threads, mutexes, condition variables, atomics, timed waits
  - `loader_probe` — `dlopen`, `dlsym`, `dlclose`, C ABI, plugin-local exceptions
- Shared library `libprobe.so` built for plugin testing
- Executables request `/usr/lib/ldqnx.so.2` (QNX dynamic loader)
- Runtime dependencies: `libstdc++.so.6`, `libm.so.2`, `libgcc_s.so.1`, `libsocket.so.3`, `libc.so.3`
- **Private runtime bundle** includes `libstdc++.so.6` (from BB10 SDK GCC 4.8.3); `libgcc_s.so.1` sourced from host (needs matching ARM version)
- **Cascades probe app scaffolded** in `bbndk-starter-kit/projects/compas-probes/`
- **Map prototype built & compiles** in this repo (`src/`, `assets/`, `CMakeLists.txt`, `bar/bar-descriptor.xml`) — successfully builds with XIAZY Clang 19



---

## Quickstart: Reproducing from Scratch

For a new developer starting with a fresh checkout, follow these steps to reproduce the working state:

### 1. Prerequisites
- Docker installed
- Git
- (Optional) Physical BB10 device in Development Mode, or QEMU simulator image

### 2. Clone & Set Up Toolchain
```bash
# Clone repos
git clone https://github.com/<your-org>/compas-bb10.git
git clone https://github.com/seeschwein/bbndk-starter-kit.git
cd compas-bb10

# Pull prebuilt XIAZY Clang toolchain (includes TLS 1.0 fix, blackberry-deploy, etc.)
docker pull ghcr.io/xiazy/blackberry10-toolchain:latest
```

### 3. Build Probe Suite (Milestone 1)
```bash
cd compas-bb10/probes
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  make
```
Expected: 4 probe executables + `libprobe.so` in `build/arm/`

**Known roadblock**: `libgcc_s.so.1` for ARMv7 is not in the SDK. You must either:
- Build from GCC 4.8.3 sources (`gcc-4.8.3/libgcc/...` targeting armv7-softfp), or
- Extract from a rooted BB10 device (`/usr/lib/libgcc_s.so.1`)

### 4. Build Map Prototype (Milestone 3)
```bash
cd compas-bb10
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src
```
Expected: `build/cmake/arm/o.le-v7-g/compas` executable

**Common pitfalls already solved**:
- `QStringLiteral` → use `QLatin1String`
- `QNetworkRequest::UserAgentHeader` → use `setRawHeader` with `QByteArray`
- `QNetworkReply::finished()` protected → use string-based `connect(..., SIGNAL(finished()), ..., SLOT(...))`
- `QImage` forward declaration → must include `<QtGui/QImage>` and link `QtGui`
- Compiler defaults to `-std=gnu++98` → force `-std=gnu++11 -fexceptions -frtti -fno-use-cxa-atexit` in CMake
- MOC must run on all `Q_OBJECT` classes (`TileCache.h`, `TileNetwork.h`)

### 5. Package BAR (Map App)
```bash
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-nativepackager -devMode -package compas.bar bar/bar-descriptor.xml -configuration Device-Debug
```

### 6. Deploy to Device (Development Mode ON)
```bash
docker run --rm --network host -v "$PWD":/src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-deploy -installApp -device <DEVICE_IP> -password <DEV_PASSWORD> \
  /src/compas.bar
```

### 7. Verify
- App launches on device
- Map tiles load (network) and display in `MapView`
- Tile cache persists to SQLite DB at `/accounts/1000/appdata/com.compas.app/tilecache.db`

### 8. Run Probes on Device (Milestone 1 completion)
Use the cascades probe app in `bbndk-starter-kit/projects/compas-probes/`:
- Copy probe binaries + `libprobe.so` + `libstdc++.so.6` + `libgcc_s.so.1` into its `assets/probes/`
- Build its BAR, deploy, tap "Run Probes"
- Retrieve `probe-results.txt` → verify 4× PASS

---

## Toolchains

### XIAZY Clang 19 (Primary — C++20/23 capable)

[XIAZY/blackberry10-toolchain](https://github.com/XIAZY/blackberry10-toolchain) provides a Dockerized BlackBerry 10 ARMv7 toolchain:
- Clang 19.1.7 frontend with full C++20/23 support
- GNU Binutils 2.45 `armnto` linker (QNX-compatible)
- BB10 10.3.1 SDK: headers, Cascades/Qt 4.8.6 libs, CRT objects
- Compiler-rt builtins for ARMv7 (VFPv3-D16, softfp)
- `bb10-cc` / `bb10-c++` gcc-compatible driver wrappers
- `blackberry-deploy`, `blackberry-nativepackager`, `blackberry-signer` included
- QNX 6.5.0 SDK for x86 (simulator target)

**Prebuilt image**: `ghcr.io/xiazy/blackberry10-toolchain:latest`

**Key compile flags** (via `bb10-c++`):
```bash
bb10-c++ -std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit \
  -O2 -g -Wall -Wextra -pthread -fPIC \
  -D__QNX__ -D__QNXNTO__ -D__unix__ -D_QNX_SOURCE \
  source.cpp -o output
```

### GCC 9.3 Baseline (Reference)

[extrowerk/bb10-gcc9](https://github.com/extrowerk/bb10-gcc9) — builds GCC 9.3.0 for BB10.

**Prebuilt image**: `sw7ft/bb10-gcc9:latest` (Docker Hub)

**Compile flags**:
```bash
arm-blackberry-qnx8eabi-g++ -std=c++17 -O2 -g -Wall -Wextra \
  -pthread -fPIC -fexceptions -frtti \
  source.cpp -o output
```

## Deployment without SSH

The phone does not need to be rooted, and SSH is not required for the first hardware validation.

Use the XIAZY image's `blackberry-deploy` workflow:

1. Enable Development Mode on the BlackBerry.
2. Package the probes as a minimal `Device-Debug` BAR.
3. Install and launch the BAR through `blackberry-deploy`.
4. Have the BAR execute the probes in its own sandbox.
5. Write results to the application's writable data directory.
6. Retrieve the result log through the deployment tooling.

BB10's legacy TLS configuration requires the image's Java runtime to permit TLS 1.0 and legacy ciphers for `blackberry-deploy`. The XIAZY image includes this fix:
```dockerfile
ENV JDK_JAVA_OPTIONS="-Djava.security.manager=allow --add-exports=java.xml/com.sun.org.apache.xerces.internal.parsers=ALL-UNNAMED"
RUN echo "jdk.tls.disabledAlgorithms=" >> ${JAVA_HOME}/conf/security/java.security \
 && echo "jdk.zip.disableZip64 = false" >> ${JAVA_HOME}/conf/security/java.security
```

Example build & deploy invocation:
```sh
# Build the BAR (probes)
docker run --rm -v /path/to/project:/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src

# Deploy to device (Development Mode ON)
docker run --rm --network host -v /path/to/project:/src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-deploy -installApp -device <DEVICE_IP> -password <DEV_PASSWORD> \
  /src/build/com.compas.probes.bar
```

## Project Structure

```
compas-bb10/
├── probes/                    # C++ compatibility probe suite
│   ├── Makefile
│   ├── filesystem_probe.cpp
│   ├── posix_probe.cpp
│   ├── thread_probe.cpp
│   ├── loader_probe.cpp
│   └── plugin.cpp
├── src/                       # Map prototype (Cascades app)
│   ├── main.cpp
│   ├── TileCache.h/cpp
│   ├── TileNetwork.h/cpp
│   └── (future: map renderer, GPS, GPX, etc.)
├── assets/                    # QML UI & resources
│   ├── main.qml
│   ├── map/MapView.qml
│   └── qml.qrc
├── bar/                       # BAR packaging for map app
│   ├── bar-descriptor.xml
│   └── icon.png
├── docs/                      # Documentation
│   ├── 00-project-charter.md
│   ├── 01-toolchain-status.md
│   ├── 02-on-device-probes.md
│   ├── 03-deployment-without-ssh.md
│   └── 04-roadmap.md
├── CMakeLists.txt             # CMake build for map app (XIAZY toolchain)
├── .claude/skills/bb10-dev.md # BB10 development skill
└── README.md

bbndk-starter-kit/             # Companion repo (separate)
├── projects/compas-probes/    # Cascades app with bundled probes
│   ├── src/main.cpp
│   ├── assets/
│   │   ├── main.qml
│   │   └── probes/            # 4 probe binaries + libprobe.so + libstdc++.so.6
│   ├── bar-descriptor.xml
│   ├── run-probes.sh
│   ├── Makefile
│   └── cascadeshello.pro
└── ... (Dockerfile, templates, scripts)
```

## Next steps

### Milestone 1: Hardware Proof (in progress)

1. ✅ Compile 4 probes with XIAZY Clang (`-fno-use-cxa-atexit` for `__dso_handle` fix)
2. ✅ Build `libprobe.so` shared library
3. ✅ Scaffold Cascades app with probe assets
4. ⬜ **Copy `libgcc_s.so.1` for ARM** (currently missing from SDK — need to extract from GCC 4.8.3 libs or build)
5. ⬜ Package BAR: `blackberry-nativepackager -devMode -package compas-probes.bar bar-descriptor.xml -configuration Device-Debug`
6. ⬜ Deploy to device: `blackberry-deploy -installApp -device <IP> -password <pass> compas-probes.bar`
7. ⬜ Launch app on device, tap "Run Probes", retrieve `probe-results.txt`
8. ⬜ Verify 4× `PASS` and exit status 0

### Milestone 2: Compiler Matrix

| Configuration | Purpose |
|---|---|
| GCC 9.3 + GNU libstdc++ | Existing known-good build baseline |
| XIAZY Clang 19 + QNX GNU linker/runtime | Fast modern-frontend evaluation (C++20/23) |
| Forked newer GCC + updated libstdc++ | C17/C++20/23 language-progress route |
| Clang + libc++ | Potential future route, higher ABI/runtime risk |

Each configuration must pass the same probe suite. Compilation alone is not success.

### Milestone 3: Compas Prototype (map prototype built ✅ — compiles & links)

Build a Cascades-based OSM viewer:
- [x] Online raster tiles (MapView custom control with `onPaintRequested`)
- [x] Tile cache with SQLite + in-memory LRU (`TileCache`)
- [x] Network tile fetching with retry (`TileNetwork`)
- [x] CMake build system with Qt4 moc/rcc/uic integration
- [x] BAR packaging descriptor (`bar/bar-descriptor.xml`)
- [ ] Pan, zoom, bookmarks, location display (GPS)
- [ ] Local tile cache management (pruning, max size)
- [ ] GPX import/export
- [ ] Settings screen (tile server URL, cache size, default location)
- [ ] A clean data layer, not CoMaps' `.mwm` format

This establishes a useful BB10 maps app without committing to the much larger task of porting CoMaps' C++ map, search, routing, and rendering engine.

**Current status**: The map prototype compiles and links successfully with XIAZY Clang 19. Next step is to package the BAR and test on device.

### Milestone 4: Offline Maps

Add bounded offline regions using a format the project controls, such as packaged raster or vector tiles.

Only then evaluate selected CoMaps components—or native C++ components—for decoding, geometry, routing, or rendering.

### Milestone 5: Native Performance Layer

If needed, introduce native C++ components for:
- Vector-tile decoding
- Geometry operations
- SQLite storage
- Protobuf handling
- High-performance map rendering

Use Cascades for the UI and Go or C++ for application services. Avoid making Go responsible for the low-latency rendering loop.

## Building the Map Prototype

```bash
# From compas-bb10 root:
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src

# Or using the bbndk-starter-kit container:
cd bbndk-starter-kit
./tools/bbndk build /path/to/compas-bb10
```

### Packaging & Deploying the Map App

```bash
# Package BAR
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-nativepackager -devMode -package compas.bar bar/bar-descriptor.xml -configuration Device-Debug

# Deploy to device
docker run --rm --network host -v "$PWD":/src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-deploy -installApp -device <DEVICE_IP> -password <DEV_PASSWORD> \
  /src/compas.bar
```

## References

- XIAZY BlackBerry 10 toolchain: https://github.com/XIAZY/blackberry10-toolchain
- GCC 9 BB10 toolchain: https://github.com/extrowerk/bb10-gcc9
- Prebuilt GCC 9 image: https://hub.docker.com/r/sw7ft/bb10-gcc9
- Prebuilt XIAZY Clang image: `ghcr.io/xiazy/blackberry10-toolchain:latest`
- BBNDK Starter Kit: https://github.com/seeschwein/bbndk-starter-kit
- [bb10-qt4-practices](bb10-qt4-practices.md) – Common Qt 4.8.6 workarounds and BB10 build practices
- BerryCore: https://github.com/sw7ft/BerryCore
- fen-blackberry (Rust on BB10): https://github.com/acmiyaguchi/fen-blackberry
- s60-maps (inspiration for map prototype): https://github.com/artem78/s60-maps