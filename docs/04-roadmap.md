# Roadmap

## Milestone 1: hardware proof

**Goal:** prove that a modern compiler-produced binary starts and runs on a BlackBerry 10 device.

- Package the four existing probes and `libprobe.so` into a BAR.
- Bundle only `libstdc++.so.6` and `libgcc_s.so.1`.
- Capture results in the application sandbox.
- Deploy through `blackberry-deploy`; no SSH required.

**Definition of done:** all four probes report `PASS` and exit status 0 in the retrieved log.

## Milestone 2: compiler matrix

**Goal:** establish which frontend/runtime combination is most viable.

| Configuration | Purpose |
|---|---|
| GCC 9.3 + GNU libstdc++ | Existing known-good build baseline |
| Forked newer GCC + updated libstdc++ | C17/C++20/23 language-progress route |
| XIAZY Clang + QNX GNU linker/runtime | Fast modern-frontend evaluation |
| Clang + libc++ | Potential future route, but higher ABI/runtime risk |

Each configuration must pass the same probe suite. Compilation alone is not success.

## Milestone 3: Compas prototype

Build a Cascades-based OSM viewer:

- Online raster tiles
- Pan, zoom, bookmarks, location display
- Local tile cache
- GPX import/export
- A clean data layer, not CoMaps’ `.mwm` format

This establishes a useful BB10 maps app without committing to the much larger task of porting CoMaps’ C++ map, search, routing, and rendering engine.

## Milestone 4: offline maps

Add bounded offline regions using a format the project controls, such as packaged raster or vector tiles.

Only then evaluate selected CoMaps components—or native C++ components—for decoding, geometry, routing, or rendering.

## Milestone 5: native performance layer

If needed, introduce native C++ components for:

- Vector-tile decoding
- Geometry operations
- SQLite storage
- Protobuf handling
- High-performance map rendering

Use Cascades for the UI and Go or C++ for application services. Avoid making Go responsible for the low-latency rendering loop.
