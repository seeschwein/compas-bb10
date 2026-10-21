# Cascades/QML starter

Classic BB10 layout: `src`, `assets`, `.pro`, and `bar-descriptor.xml`. Open it in Momentics for BlackBerry-specific wizards/GUI tooling, or try `make` in the container.

The probe executables are built by the XIAZY toolchain and placed in `build/compas-probes`.

If packaging reports a missing entry point, update the `bar-descriptor.xml` asset path:

```xml
<asset path="arm/o.le-v7-g/compas-probes">compas-probes</asset>
```
