# Deployment without SSH

## Approach

The first hardware validation does not require root access or SSH.

Use BlackBerry Development Mode and the XIAZY Clang Docker image’s `blackberry-deploy` workflow to install and launch the probe BAR.

## Current BAR layout (bbndk-starter-kit/projects/compas-probes/)

```text
compas-probes/
├── bar-descriptor.xml
├── src/main.cpp
├── assets/
│   ├── main.qml
│   └── probes/
│       ├── filesystem_probe
│       ├── posix_probe
│       ├── thread_probe
│       ├── loader_probe
│       ├── libprobe.so
│       ├── libstdc++.so.6
│       └── run-probes.sh
├── run-probes.sh (launcher script at project root)
├── Makefile
└── cascadeshello.pro
```

The BAR launcher (`run-probes.sh`) must:

1. Set `LD_LIBRARY_PATH` to `/app/native/probes` (private bundled library directory).
2. Run each probe.
3. Capture stdout, stderr, and exit status.
4. Write results to the application’s writable data directory (`/accounts/1000/appdata/com.compas.probes/probe-results.txt`).
5. Flush all output before exiting.

## Build command (XIAZY Clang)

```bash
# From bbndk-starter-kit directory:
docker run --rm -v "$PWD/projects/compas-probes":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-nativepackager -devMode -package compas-probes.bar bar-descriptor.xml -configuration Device-Debug
```

Or using the bb10-build wrapper:
```bash
docker run --rm -v "$PWD/projects/compas-probes":/src -w /src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  bb10-build /src
```

## Deploy to device (Physical, non-rooted)

Prerequisites:
1. Enable Development Mode on BlackBerry (Settings → Security → Development Mode)
2. Note device IP address (shown on screen)
3. Set development password

```bash
docker run --rm --network host -v "$PWD/projects/compas-probes":/src \
  ghcr.io/xiazy/blackberry10-toolchain:latest \
  blackberry-deploy -installApp -device <DEVICE_IP> -password <DEV_PASSWORD> \
  /src/compas-probes.bar
```

## TLS requirement

BB10 only negotiates TLS 1.0 with legacy ciphers for the deployment channel.

The XIAZY Docker image includes the Java security policy fix:
```dockerfile
ENV JDK_JAVA_OPTIONS="-Djava.security.manager=allow --add-exports=java.xml/com.sun.org.apache.xerces.internal.parsers=ALL-UNNAMED"
RUN echo "jdk.tls.disabledAlgorithms=" >> ${JAVA_HOME}/conf/security/java.security \
 && echo "jdk.zip.disableZip64 = false" >> ${JAVA_HOME}/conf/security/java.security
```

This is scoped to the Docker image; no host-side Java modifications needed.

## Success criterion

The retrieved `probe-results.txt` (from `/accounts/1000/appdata/com.compas.probes/`) must contain four `PASS` entries and exit status 0 for each probe:

```
PASS filesystem_probe
PASS posix_probe
PASS thread_probe
PASS loader_probe
```

## References

- XIAZY BlackBerry 10 toolchain: https://github.com/XIAZY/blackberry10-toolchain
- Prebuilt XIAZY Clang image: `ghcr.io/xiazy/blackberry10-toolchain:latest`
- GCC 9 BB10 toolchain: https://github.com/extrowerk/bb10-gcc9
- Prebuilt GCC 9 image: https://hub.docker.com/r/sw7ft/bb10-gcc9
- BBNDK Starter Kit: https://github.com/seeschwein/bbndk-starter-kit
