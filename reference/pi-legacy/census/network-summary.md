# Historical network/runtime summary (sanitized)

Capture date: 2026-08-31.

## Product-significant PS2 link

- Pi interface: `eth0`
- Pi address: `192.168.50.1/24`
- PS2 peer convention: `192.168.50.2`
- The PS2-facing interface had no default route.
- VNC listener: `192.168.50.1:5900` (`Xtigervnc`)
- Management listener: `192.168.50.1:5959` (`python3`)
- Samba was bound to loopback + `eth0`.

## Other interfaces present on the exploratory Pi

The old Pi also had upstream Wi-Fi and Docker networking. Their household addresses, Wi-Fi identity, IPv6 addresses, connection UUIDs, and MAC addresses are intentionally omitted here because they are not part of the PS-to-VNC product contract.

The clean Pi must establish its own supported upstream-network configuration rather than inheriting these machine-specific values.
