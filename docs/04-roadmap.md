# Roadmap

## Milestone 1: hardware proof (In Progress)

**Goal:** prove that a modern compiler-produced binary starts and runs on a BlackBerry 10 device.

### Completed ✅
- [x] Scaffold Cascades probe app (`bbndk-starter-kit/projects/compas-probes/`)
- [x] Compile 4 probes with XIAZY Clang 19 (`-fno-use-cxa-atexit` fix for `__dso_handle`)
- [x] Build `libprobe.so` shared library
- [x] Bundle `libstdc++.so.6` (from SDK GCC 4.8.3)
- [x] Create `run-probes.sh` launcher with result capture
- [x] Configure `bar-descriptor.xml` with `LD_LIBRARY_PATH=/app/native/probes`
- [x] Apply TLS 1.0 Java fix in XIAZY Dockerfile

### Remaining ⬜
- [ ] **Obtain `libgcc_s.so.1` for ARMv7** (missing from SDK — extract from GCC 4.8.3 build or device)
- [ ] Package BAR: `blackberry-nativepackager -devMode -package compas-probes.bar bar-descriptor.xml -configuration Device-Debug`
- [ ] Deploy to device: `blackberry-deploy -installApp -device <IP> -password <pass> compas-probes.bar`
- [ ] Launch app, run probes, retrieve `probe-results.txt`
- [ ] Verify 4× `PASS` and exit status 0

**Definition of done:** all four probes report `PASS` and exit status 0 in the retrieved log.

## Milestone 2: compiler matrix

**Goal:** establish which frontend/runtime combination is most viable.

| Configuration | Purpose | Status |
|---|---|---|
| GCC 9.3 + GNU libstdc++ | Existing known-good build baseline | ✅ Compiles, not tested on-device |
| XIAZY Clang 19 + QNX GNU linker/runtime | Fast modern-frontend evaluation (C++20/23) | ✅ Compiles, pending device test |
| Forked newer GCC + updated libstdc++ | C17/C++20/23 language-progress route | ⬜ Not started |
| Clang + libc++ | Potential future route, higher ABI/runtime risk | ⬜ Not started |

Each configuration must pass the same probe suite. Compilation alone is not success.

## Milestone 3: Compas prototype

Build a Cascades-based OSM viewer:

- Online raster tiles (MapView / WebView)
- Pan, zoom, bookmarks, location display
- Local tile cache (SQLite)
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

## Immediate Next Steps (This Week)

1. **Get `libgcc_s.so.1` for ARMv7**
   - Option A: Build from GCC 4.8.3 sources in XIAZY image
   - Option B: Extract from physical device via `blackberry-deploy` + shell
   - Option C: Cross-compile GCC 4.8.3 libgcc for armv7-none-nto-qnx6.5.0eabi

2. **Package and deploy the BAR**
   ```bash
   # In bbndk-starter-kit:
   docker run --rm -v "$PWD/projects/compas-probes":/src -w /src \
     ghcr.io/xiazy/blackberry10-toolchain:latest \
     blackberry-nativepackager -devMode -package compas-probes.bar bar-descriptor.xml -configuration Device-Debug
   
   docker run --rm --network host -v "$PWD/projects/compas-probes":/src \
     ghcr.io/xiazy/blackberry10-toolchain:latest \
     blackberry-deploy -installApp -device <IP> -password <pass> /src/compas-probes.bar
   ```

3. **Run on-device validation** and collect `probe-results.txt`

4. **Compare GCC 9.3 vs XIAZY Clang** results (Milestone 2)
