# Clean Raspberry Pi Virgin Baseline

## Status

    WORKSTREAM=GITHUB_ISSUE_5
    CAPTURE_STATE=VIRGIN_FIRST_BOOT
    PRODUCT_MUTATIONS=NONE
    CAPTURE_RESULT=PASS

This file records the first-boot baseline of the new 1 TB Raspberry Pi companion before any PS-to-VNC-specific package installation, Ethernet configuration, service installation, runtime-state creation, or import of historical configuration.

The baseline is intentionally non-sensitive. Household WLAN address details, machine ID, boot ID, MAC addresses, credentials, and other host-specific secret/private values are not recorded here.

## Hardware

- Raspberry Pi 4 Model B Rev 1.5
- approximately 8 GiB RAM (`7.6 GiB` usable reported)
- 1 TB-class SD card (`mmcblk0` reported as `921.9G`)
- root filesystem: ext4, approximately `908G`, `5.2G` used, `865G` available at capture
- boot filesystem: FAT32, 512 MiB
- zram swap: 2 GiB

## Operating system

Captured identity:

    PRETTY_NAME=Debian GNU/Linux 13 (trixie)
    VERSION_ID=13
    VERSION_CODENAME=trixie
    DEBIAN_VERSION_FULL=13.5
    DPKG_ARCH=arm64
    KERNEL=6.18.34+rpt-rpi-v8
    MACHINE=aarch64

Hostname at capture:

    ps-to-vnc-dev

This is materially different from the exploratory companion, which used the older Bookworm/armhf environment. The clean dependency ledger must therefore evaluate historical package/configuration choices rather than assuming they transfer unchanged.

## Network state before product configuration

At capture:

- loopback was up;
- `wlan0` was up through the ordinary household DHCP network and supplied the default route;
- `eth0` was `DOWN` and had no PS-to-VNC private address;
- there was no `192.168.50.1/24` PS2-facing Ethernet configuration yet.

This proves the private PS2 link had not been configured before the baseline capture.

## Relevant package state in the virgin image

The baseline and targeted follow-up query establish:

| Package | Virgin status | Version at capture |
|---|---|---|
| `network-manager` | installed | `1.52.1-1+rpt4` |
| `labwc` | installed | `0.9.7-1+rpt1` |
| `wayvnc` | installed | `0.9.1-1+rpt5` |
| `openbox` | installed | `3.6.1-12+rpt1` |
| `python3` | installed | `3.13.5-1` |
| `curl` | installed | `8.14.1-2+deb13u3` |
| `git` | installed | `1:2.47.3-0+deb13u1` |
| `samba` | not installed | none |
| `lxpanel` | not installed | none |
| `tigervnc-standalone-server` | not installed / no installed version | none |

The follow-up used `dpkg-query` only and reported `PRODUCT_MUTATIONS=NONE`.

`wayfire` and `docker.io` did not produce an installed-version line in the first capture and are not classified as installed from that evidence alone.

## Enabled service baseline

The virgin image reported 33 enabled service unit files. Product-relevant observations include:

- `NetworkManager.service` enabled;
- `NetworkManager-wait-online.service` enabled;
- `NetworkManager-dispatcher.service` enabled;
- `lightdm.service` enabled;
- `ssh.service` enabled;
- `wayvnc-control.service` enabled;
- `avahi-daemon.service` enabled;
- no PS-to-VNC-specific service unit existed.

Other enabled image services are OS baseline state and are not automatically product dependencies.

## Active graphical/session baseline

At capture the visible process census showed:

    labwc /usr/bin/labwc -m

No `wayvnc`, `Xtigervnc`, `Xorg`, `Xwayland`, `openbox`, or `lxpanel` process appeared in the targeted active-process sample.

This means package presence and enabled service files must not be confused with an already-running dedicated PS-to-VNC VNC desktop.

## Comparison with historical companion

Historical reference is preserved under `reference/pi-legacy/`.

Important initial differences:

- clean baseline: Debian 13 / arm64 / kernel 6.18 / labwc/Wayland-oriented desktop image;
- historical companion: older Bookworm/armhf runtime with a dedicated TigerVNC/Xtigervnc `:1` desktop and Openbox/LXPanel service graph;
- clean `eth0`: unconfigured/down at baseline;
- historical `eth0`: product-configured `192.168.50.1/24` private PS2 link;
- clean image already includes WayVNC/labwc/Openbox but not TigerVNC, Samba, or LXPanel.

The historical system remains evidence of required behavior, not an instruction to reproduce the old package stack blindly.

## Baseline exit condition

The virgin-baseline requirement from Issue #5 is satisfied for hardware, OS, storage, network, relevant package surface, enabled service surface, and active graphical-session surface.

No product mutation occurred before or during capture/follow-up package clarification.

Next: evaluate the smallest reproducible private-Ethernet definition and compare the OS-base WayVNC/labwc path against the historical dedicated TigerVNC contract before adopting a PS2-facing VNC runtime.
