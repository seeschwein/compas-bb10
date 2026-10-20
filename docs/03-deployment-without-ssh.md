# Deployment without SSH

## Approach

The first hardware validation does not require root access or SSH.

Use BlackBerry Development Mode and the Docker image’s `blackberry-deploy` workflow to install and launch a minimal probe BAR.

## BAR layout

```text
probe-bar/
├── bar-descriptor.xml
├── icon.png
├── probes/
│   ├── filesystem_probe
│   ├── posix_probe
│   ├── thread_probe
│   ├── loader_probe
│   ├── libprobe.so
│   ├── libstdc++.so.6
│   └── libgcc_s.so.1
└── run-probes.sh
```

The BAR launcher must:

1. Set `LD_LIBRARY_PATH` to its private bundled library directory.
2. Run each probe.
3. Capture stdout, stderr, and exit status.
4. Write results to the application’s writable data directory.
5. Flush all output before exiting.

## Build command

Use the image’s builder with the default `Device-Debug` configuration:

```sh
BB10_CONFIGURATION=Device-Debug \
  ./tools/docker-builder.sh build /path/to/probe-bar
```

## TLS requirement

BB10 may negotiate only TLS 1.0 with legacy ciphers for the deployment channel.

The Docker image’s Java runtime may need its TLS restrictions lifted specifically for `blackberry-deploy`. Keep this configuration inside the image; do not alter system-wide Java settings on the host.

## Success criterion

The retrieved `probe-results.txt` must contain four `PASS` entries and exit status 0 for each probe.

## References

- XIAZY BlackBerry 10 toolchain: https://github.com/XIAZY/blackberry10-toolchain
- GCC 9 BB10 toolchain: https://github.com/extrowerk/bb10-gcc9
- Prebuilt GCC 9 image: https://hub.docker.com/r/sw7ft/bb10-gcc9
