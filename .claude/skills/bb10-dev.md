---
name: bb10-dev
description: Skill for BlackBerry 10 / QNX native C/C++/Qt development with Cascades
metadata:
  type: reference
---

# BlackBerry 10 Development Skill

This skill covers development for BlackBerry 10 (BB10) using the native C/C++ toolchain with Qt 4.8.6 / Cascades UI framework. It includes toolchain setup, build workflows, deployment strategies, and debugging tips.

## Prerequisites

- Docker (with BuildKit enabled for faster builds)
- A BlackBerry 10 device (physical) or QEMU simulator image
- USB debugging / Development Mode enabled on device
- Basic knowledge of C++17/20, Qt 4, QML, and Cascades

## Toolchains

### 1. XIAZY Clang Toolchain (Recommended for C++20/23)

**Image**: `ghcr.io/xiazy/blackberry10-toolchain:latest`
**Source**: https://github.com/XIAZY/blackberry10-toolchain

**Features**:
- Clang 19.1.7 frontend with C++20/23 support
- GNU Binutils 2.45 `armnto` linker (QNX-compatible)
- Full BB10 10.3.1 SDK (headers, Cascades/Qt libs, CRT)
- Compiler-rt builtins for ARMv7 (VFPv3-D16, softfp)
- `bb10-cc` / `bb10-c++` gcc-compatible driver wrappers
- `blackberry-deploy` and `blackberry-nativepackager` included
- QNX 6.5.0 SDK for x86 (simulator target)

**Usage**:
```bash
# Build a CMake project
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src

# Build with Make
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  make CC=bb10-cc CXX=bb10-c++

# Compile single file
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-c++ -std=gnu++20 -O2 -Wall -Wextra file.cpp -o output
```

**Compiler Flags** (via `bb10-c++`):
- Target: `--target=armv7-none-eabi`
- Architecture: `-marm -mcpu=cortex-a9 -mfpu=vfpv3-d16 -mfloat-abi=softfp`
- QNX macros: `-D__QNX__ -D__QNXNTO__ -D__unix__ -D__unix -D_QNX_SOURCE`
- Qt 4 includes: `-isystem /opt/bb10-sdk/target/qnx6/usr/include/qt4/...`
- C++ defaults: `-std=gnu++98 -fno-exceptions -fno-rtti -DQT_NO_EXCEPTIONS`
- **Override with**: `-std=gnu++17 -fexceptions -frtti` for modern C++

### 2. GCC 9.3 Toolchain (Legacy Baseline)

**Image**: `sw7ft/bb10-gcc9:latest` (Docker Hub)
**Source**: https://github.com/extrowerk/bb10-gcc9

**Features**:
- GCC 9.3.0 with C++17 support
- QNX headers and binutils patches
- Simpler, older toolchain

**Usage**:
```bash
docker run --rm -v "$PWD":/src -w /src \
  sw7ft/bb10-gcc9:latest \
  arm-blackberry-qnx8eabi-g++ -std=c++17 -O2 -g -Wall -Wextra \
  -pthread -fPIC -fexceptions -frtti file.cpp -o output
```

## Build Systems

### CMake (Recommended)

The XIAZY toolchain provides a CMake toolchain file: `/opt/bb10-toolchain.cmake`

**Basic CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.20)
project(my_app LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arm/o.le-v7-g")

include(BB10Qt4)
qt4_wrap_cpp(MOC_SOURCES src/controller.h)

add_executable(my_app src/main.cpp src/controller.cpp ${MOC_SOURCES})
target_link_libraries(my_app PRIVATE bbcascades QtDeclarative QtCore)
```

**Build**:
```bash
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src
```

### qmake (Legacy)

The starter-kit templates use `.pro` files with qmake.

```bash
mkdir -p build/arm
cd build/arm
qmake ../../my_app.pro CONFIG+=device
make
```

## Deployment

### Physical Device (Non-Rooted)

**Prerequisites**:
1. Enable Development Mode on device (Settings → Security → Development Mode)
2. Note device IP address (shown on screen)
3. Set development password

**Deploy**:
```bash
# Using XIAZY toolchain container
docker run --rm -v "$PWD":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-deploy -installApp -device <IP> -password <pass> \
  /src/build/my_app.bar
```

**TLS 1.0 Workaround**:
BB10 only speaks TLS 1.0 with legacy ciphers. The XIAZY image has Java configured with:
```dockerfile
ENV JDK_JAVA_OPTIONS="-Djava.security.manager=allow --add-exports=java.xml/com.sun.org.apache.xerces.internal.parsers=ALL-UNNAMED"
```
And in `java.security`:
```
jdk.tls.disabledAlgorithms=
jdk.zip.disableZip64 = false
```

### Rooted Device (SSH)

If device is rooted with `bb10mt`:
```bash
./tools/install-rooted.sh root@<phone-ip> /path/to/app.bar
```

### Simulator (QEMU)

The BB10 simulator uses x86 architecture (`QNX_ARCH=x86 bb10-cc`).

```bash
# Start simulator (requires qcow2 image)
# Then deploy to 10.0.2.2 (QEMU default gateway)
blackberry-deploy -installApp -device 10.0.2.2 -password <pass> app.bar
```

## BAR Packaging

### bar-descriptor.xml Structure

```xml
<?xml version="1.0" encoding="utf-8"?>
<qnx>
  <id>com.example.app</id>
  <filename>appname</filename>
  <name>App Name</name>
  <versionNumber>1.0.0</versionNumber>
  <description>Description</description>
  <configuration>Device-Debug</configuration>
  <icon><image>icon.png</image></icon>
  <action>
    <type>application</type>
    <name>appname</name>
    <arg>appname</arg>
  </action>
  <env>
    <var name="LD_LIBRARY_PATH">/app/native/probes</var>
  </env>
  <assets>
    <asset path="arm/o.le-v7-g/appname">appname</asset>
    <asset path="assets">assets</asset>
    <!-- Additional assets -->
  </assets>
  <permissions>
    <permission>access_shared</permission>
  </permissions>
</qnx>
```

### Packaging Command

```bash
blackberry-nativepackager -devMode -package app.bar bar-descriptor.xml -configuration Device-Debug
```

## Cascades UI Development

### Main.cpp Boilerplate

```cpp
#include <bb/cascades/AbstractPane>
#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>

using namespace bb::cascades;

Q_DECL_EXPORT int main(int argc, char **argv)
{
    Application app(argc, argv);
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(&app);
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    app.setScene(root);
    return Application::exec();
}
```

### QML Basics (main.qml)

```qml
import bb.cascades 1.0

Page {
    Container {
        layout: DockLayout {}
        Label {
            text: "Hello BB10"
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
        }
    }
}
```

### Signals/Slots & Q_OBJECT

Classes called from QML need `Q_OBJECT` macro and moc:
```cmake
include(BB10Qt4)
qt4_wrap_cpp(MOC_SOURCES src/controller.h)
add_executable(app src/main.cpp src/controller.cpp ${MOC_SOURCES})
```

## Runtime Libraries & ABI

### Private Runtime Strategy

BB10 system provides: `libc.so.3`, `libm.so.2`, `libsocket.so.3`, `/usr/lib/ldqnx.so.2`
**Bundle only**: `libstdc++.so.6`, `libgcc_s.so.1` (matching SONAMEs)

### XIAZY Toolchain Builtins

The XIAZY image builds compiler-rt builtins providing:
- `__aeabi_memcpy`, `__aeabi_memset`, `__aeabi_memclr`
- `__aeabi_d2lz`, `__aeabi_l2d`, `__aeabi_d2ulz`, etc.
- Linked after `-lc` so they only supply what libc lacks

### C++ Standard Library

- BB10 ships with Dinkumware STL (GCC 4.x era)
- XIAZY toolchain uses its own `libstdc++.so` from GCC 4.8.3
- **Do not mix** libstdc++ versions across shared library boundaries

## Debugging

### On-Device Logs

```bash
# View app logs
blackberry-deploy -getDeviceLog <IP> -password <pass>

# Or use Momentics IDE (if available)
```

### GDB (Limited)

```bash
# On device (requires SSH/root)
gdbserver :1234 /app/native/appname

# From host
gdb-multiarch -ex "target remote <IP>:1234" /path/to/appname
```

### Common Issues

| Issue | Cause | Fix |
|-------|-------|-----|
| `__dso_handle` undefined | Missing `-fno-use-cxa-atexit` or wrong C++ flags | Add `-fno-use-cxa-atexit` to link line |
| `libstdc++.so.6` version mismatch | Bundled lib doesn't match system | Use SDK's libstdc++ from same GCC version |
| TLS handshake fails | Java security policy blocks TLS 1.0 | Set `jdk.tls.disabledAlgorithms=` in java.security |
| `blackberry-deploy` connection timeout | Device not in Dev Mode / wrong IP | Verify Dev Mode enabled, check IP |
| moc fails on headers | Missing Q_OBJECT / Qt includes | Ensure `qt4_wrap_cpp` in CMake, Qt4 includes |

## MCP (Model Context Protocol) Instructions

When working with this codebase via MCP, the following patterns apply:

### File Structure Conventions

```
/workspace/
├── src/              # C++ source files
├── assets/           # QML, images, bundled libs
├── CMakeLists.txt    # CMake build definition
├── bar-descriptor.xml # BAR manifest
├── Makefile          # Optional qmake/Make alternative
└── run-probes.sh     # Device-side test runner
```

### Common MCP Tool Calls

```python
# Build with bb10-c++
result = run_bash({
    "command": "docker run --rm -v /workspace:/src -w /src/probes "
               "ghcr.io/xiazy/blackberry10-toolchain:latest "
               "bb10-c++ -std=gnu++17 -fexceptions -frtti "
               "-fno-use-cxa-atexit file.cpp -o build/file",
    "description": "Compile probe with XIAZY toolchain"
})

# Package BAR
result = run_bash({
    "command": "docker run --rm -v /workspace:/src -w /src "
               "ghcr.io/xiazy/blackberry10-toolchain:latest "
               "blackberry-nativepackager -devMode -package app.bar bar-descriptor.xml",
    "description": "Package BAR for deployment"
})

# Deploy to device
result = run_bash({
    "command": "docker run --rm --network host -v /workspace:/src "
               "ghcr.io/xiazy/blackberry10-toolchain:latest "
               "blackberry-deploy -installApp -device 192.168.x.x -password pass "
               "/src/app.bar",
    "description": "Install BAR on BB10 device"
})
```

## Next Steps for Compas BB10

### Milestone 1: Hardware Proof (Current)

1. ✅ Scaffold Cascades project with probe suite
2. ✅ Compile 4 probes with XIAZY Clang (`-fno-use-cxa-atexit` fix)
3. ✅ Bundle `libstdc++.so.6` (missing `libgcc_s.so.1` - needs fix)
4. ⬜ Package BAR with updated `bar-descriptor.xml`
5. ⬜ Deploy to device via `blackberry-deploy`
6. ⬜ Run probes, verify 4x `PASS` in `probe-results.txt`

### Milestone 2: Compiler Matrix

1. Compare GCC 9.3 vs XIAZY Clang probe results
2. Test C++20 features (coroutines, concepts, modules)
3. Evaluate libc++ vs libstdc++ ABI compatibility

### Milestone 3: Compas Prototype

1. Online raster tile viewer with Cascades MapView
2. Pan/zoom/bookmarks/location display
3. Local tile cache (SQLite)
4. GPX import/export
5. Custom map data format (not .mwm)

## References

- [XIAZY blackberry10-toolchain](https://github.com/XIAZY/blackberry10-toolchain)
- [extrowerk/bb10-gcc9](https://github.com/extrowerk/bb10-gcc9)
- [BBNDK Starter Kit](https://github.com/seeschwein/bbndk-starter-kit)
- [BlackBerry 10 Native SDK Docs](https://developer.blackberry.com/native/)
- [Qt 4.8 Documentation](https://doc.qt.io/archives/qt-4.8/)
- [Cascades API Reference](https://developer.blackberry.com/native/reference/cascades/)