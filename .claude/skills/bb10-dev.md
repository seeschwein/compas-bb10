---
name: bb10-dev
description: Compas BB10 project-specific references (see bbndk-starter-kit/.claude/skills/bb10-dev.md for universal skill)
metadata:
  type: reference
---

# Compas BB10 Project References

This file contains Compas-specific notes. For the universal BB10 development skill (toolchains, build workflows, deployment, MCP integration), see:

**`/Users/retroarch_builder/bbndk-starter-kit/.claude/skills/bb10-dev.md`**

---

## Compas-Specific Notes

### Current Probe Suite Status (2025-11-05)

Built with XIAZY Clang 19 (`ghcr.io/xiazy/blackberry10-toolchain:latest`):

| Probe | Source | Build Flags |
|---|---|---|
| `filesystem_probe` | `probes/filesystem_probe.cpp` | `-std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit` |
| `posix_probe` | `probes/posix_probe.cpp` | `-std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit` |
| `thread_probe` | `probes/thread_probe.cpp` | `-std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit` |
| `loader_probe` | `probes/loader_probe.cpp` | `-std=gnu++17 -fexceptions -frtti -fno-use-cxa-atexit` |
| `libprobe.so` | `probes/plugin.cpp` | Same + `-shared` |

**Key fix**: `-fno-use-cxa-atexit` required for `__dso_handle` linking.

### Cascades Probe App

Located at: `bbndk-starter-kit/projects/compas-probes/`

Assets bundled:
- 4 probe executables + `libprobe.so`
- `libstdc++.so.6` (from SDK GCC 4.8.3)
- `run-probes.sh` launcher

**Missing**: `libgcc_s.so.1` for ARMv7

### Next Actions for Compas

1. Obtain `libgcc_s.so.1` ARMv7 (build from GCC 4.8.3 or extract from device)
2. Package BAR: `blackberry-nativepackager -devMode -package compas-probes.bar bar-descriptor.xml -configuration Device-Debug`
3. Deploy: `blackberry-deploy -installApp -device <IP> -password <pass> compas-probes.bar`
4. Run probes, verify 4× PASS

### Roadmap Milestones

See `docs/04-roadmap.md` for detailed milestones:
- M1: Hardware proof (in progress)
- M2: Compiler matrix (GCC 9.3 vs XIAZY Clang)
- M3: Compas prototype (Cascades OSM viewer)
- M4: Offline maps
- M5: Native performance layer

---

## Universal Skill Location

**`/Users/retroarch_builder/bbndk-starter-kit/.claude/skills/bb10-dev.md`**

Contains:
- Complete toolchain reference (XIAZY Clang, GCC 9.3, BBNDK Starter Kit)
- Build systems (CMake, qmake, Make)
- Project templates (CLI, Cascades qmake, Cascades CMake)
- BAR packaging templates
- Deployment workflows (physical, rooted, simulator)
- Cascades UI development (C++, QML, moc)
- Runtime library policy & ABI notes
- Common issues & fixes table
- MCP integration with bbndk-starter-kit server
- bbndk-starter-kit specific commands
- Debugging strategies
