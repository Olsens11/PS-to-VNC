# TigerVNC 1.16.2 w0vncserver exploratory artifact

## Status

    CLASSIFICATION=EVALUATING
    ADOPTED_RUNTIME=NO
    ARCHITECTURE_DECISION=NO
    EXPERIMENT_DATE=2026-09-02
    EXPERIMENT_PURPOSE=FUTURE_FEASIBILITY_REFERENCE

This directory preserves the exact `w0vncserver` binary used during the
2026-09-02 Raspberry Pi OS labwc/Wayland exploratory hardware experiment.

Its presence in the repository does **not** replace or modify the adopted
Issue #5 PS2-facing `Xtigervnc` path.

The artifact is retained so future development can inspect the actual tested
binary and compare its capabilities, dependencies, protocol implementation,
Wayland frontend, and maintenance cost against the dedicated Xtigervnc design.

## Exact binary

    TigerVNC version = 1.16.2
    executable = w0vncserver
    architecture = Linux ARM64 / aarch64
    SHA256 = 76dd46e40a3b1d82da8a86b3bff622b2f6c9c703c2d1ed38dee81c660d0ef804

The preserved file is:

    reference/pi-experiments/tigervnc-w0vncserver-1.16.2/w0vncserver

## Source/build provenance

TigerVNC source archive used during the experiment:

    tigervnc-1.16.2.tar.gz
    SHA256=b107c0c8b8a962594281690366c24186e95c2ea4a169acbc0076aa62ed01f467

Disposable build environment:

    image=debian:13-slim
    image_digest=sha256:d7e12182ce18b85b93007c1dedf31f2d29e01ccf3182cc4017c709b6259bc132

The build occurred inside the disposable container rather than by installing
development packages onto the Raspberry Pi host.

Relevant CMake options used:

    BUILD_VIEWER=OFF
    BUILD_JAVA=OFF
    ENABLE_H264=OFF
    ENABLE_NLS=OFF
    ENABLE_GNUTLS=OFF
    ENABLE_NETTLE=OFF
    ENABLE_SELINUX=OFF
    ENABLE_SYSTEMD=OFF
    ENABLE_PWQUALITY=OFF
    ENABLE_WAYLAND=ON
    INSTALL_SYSTEMD_UNITS=OFF

The resulting binary resolved all required shared libraries on the experimental
Debian 13 / Raspberry Pi OS ARM64 host.

See:

- `w0vncserver.sha256`
- `file.txt`
- `ldd.txt`
- `readelf.txt`

for artifact-level inspection data.

## What the experiment established

The exact preserved binary was demonstrated to:

- attach directly to the Raspberry Pi OS `labwc` Wayland session;
- expose the real Pi desktop through RFB 3.8;
- serve an exact 704x462 Wayland output;
- accept SecurityType None on the private PS2 network;
- accept the PS2 client's requested 16-bpp/depth-15 pixel format;
- return complete Raw framebuffer coverage in an independent probe;
- display the real Pi desktop through the unchanged clean Issue #7 PS2 client;
- recover into a demand-start path after PS2 Ethernet carrier activated.

The final exploratory demand path was:

    PS2VNC launch
        -> Ethernet carrier
        -> NetworkManager ps2-link
        -> 192.168.50.1/24
        -> w0vncserver
        -> PS2 RFB connection
        -> real Raspberry Pi desktop

## Important unresolved findings

This experiment is intentionally not promoted because:

- the operator needed to launch the PS2 ELF twice during the final test;
- PS2-side input through w0vncserver was not qualified;
- the tested server is TigerVNC 1.16.2, while the adopted Debian Issue #5
  Xtigervnc package is TigerVNC 1.15.0;
- obtaining w0vncserver currently requires a project-controlled build rather
  than the adopted Debian package;
- direct attachment couples the PS2-facing VNC endpoint to the Pi graphical
  session;
- remote-computer/upstream-VNC routing was not tested;
- the existing architecture deliberately preserves a dedicated Xtigervnc
  endpoint as a stable presentation/compatibility boundary.

## Disposition

Continue the existing Issue #5 / Issue #7 plan.

Use this binary as preserved evidence for future feasibility analysis.
Reconsider adoption only if later inspection or experiments show a material
advantage over the dedicated Xtigervnc architecture.
