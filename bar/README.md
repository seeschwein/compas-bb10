# BAR deployment

This directory will contain the BlackBerry 10 BAR packaging project for the on-device compatibility probes.

The initial BAR must:

- Bundle the compiled probe executables and `libprobe.so`.
- Bundle private `libstdc++.so.6` and `libgcc_s.so.1` runtime libraries.
- Run every probe in the application sandbox.
- Capture stdout, stderr, exit codes, and elapsed time.
- Write `probe-results.txt` to a writable application-data location.
- Be deployable through `blackberry-deploy` in `Device-Debug` configuration.

SSH is not required for this workflow.

## Build

```sh
BB10_CONFIGURATION=Device-Debug \
  ./tools/docker-builder.sh build /path/to/this-bar-project
```

## References

- XIAZY BlackBerry 10 toolchain: https://github.com/XIAZY/blackberry10-toolchain
- GCC 9 BB10 toolchain: https://github.com/extrowerk/bb10-gcc9
- Prebuilt GCC 9 image: https://hub.docker.com/r/sw7ft/bb10-gcc9
