# M3F management-service startup incident

During the first M3F hardware attempt, the exact M3E DUT appeared to stop
after the visible `Ethernet link is UP` startup boundary.

This was investigated before any M3E source repair was attempted.

Observed facts:

- The PS2 Ethernet/IP stack remained alive and answered ICMP.
- TigerVNC remained listening on TCP 5900.
- A packet capture observed no PS2 TCP/5900 SYN while the startup screen
  remained at the post-link boundary.
- Source inspection proved that startup enters
  `ps2vnc_config_load_from_pi()` before opening the initial RFB session.
- A detached diagnostic build placed visible stage markers through the
  configuration and management GET path.
- The diagnostic reached `M3F G6 recv`.
- Therefore the PS2 had successfully entered the detached services TU,
  allocated its buffers, created the management socket, connected to TCP
  5959, and transmitted the management HTTP request.
- The Pi management service had stale accepted/CLOSE-WAIT connections and
  its listen queue became saturated.
- A local Pi curl request to its own management endpoint timed out with
  return code 124.
- Only `ps2vnc-management.service` was restarted. TigerVNC was not
  restarted.
- After restart the management service immediately passed its local
  `/ps2vnc.conf` self-probe.
- The exact uninstrumented M3E ELF was then restored to `PS2VNC.ELF`.
- That exact M3E DUT booted successfully to the desktop.
- The subsequent five-mode M3F hardware matrix passed 5/5.

Conclusion:

The failed startup attempt was a Pi management-service availability
incident, not a demonstrated regression caused by the M3E
translation-unit split.

The management-service stale-connection/backlog behavior remains a
separate robustness issue for later repair. It does not block continued
M3 real translation-unit decomposition.
