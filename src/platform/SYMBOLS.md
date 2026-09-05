# Clean symbols — `src/platform`

DIRECTORY=src/platform
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns genuinely PS2-specific system, controller-service
bootstrap, Ethernet, and GS mechanisms.
The clean Issue #7 platform surface is limited to the six `ps2_*.[ch]` files
listed below; no retained historical platform module is silently included.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| display | variable | src/platform/ps2_graphics.c | PS2 graphics | file static | Owns the gsKit global display instance used by the fixed Standard 480p presentation path. | fixed Standard 480p presentation |
| desktop_texture | variable | src/platform/ps2_graphics.c | PS2 graphics | file static | Owns the reusable gsKit texture descriptor for the complete 704x462 desktop upload. | fixed Standard 480p presentation |
| texture_configured | variable | src/platform/ps2_graphics.c | PS2 graphics | file static | Records whether the reusable desktop texture has received its one-time VRAM allocation. | fixed Standard 480p presentation |
| configure_texture | function | src/platform/ps2_graphics.c | PS2 graphics | file | Initializes the desktop texture descriptor and allocates its persistent GS VRAM address. | fixed Standard 480p presentation |
| gs_pixels | parameter | src/platform/ps2_graphics.c | configure_texture | function | Points at the caller-owned GS16 pixel buffer initially attached to the texture descriptor. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_init | function | src/platform/ps2_graphics.c | PS2 graphics | public | Initializes dmaKit, gsKit, and the fixed Standard 480p presentation state exactly once. | ISSUE7_MINIMAL_CORE: Fixed Standard 480p presentation |
| pstvnc_ps2_graphics_present | function | src/platform/ps2_graphics.c | PS2 graphics | public | Uploads and presents one complete coherent 704x462 GS16 desktop, then synchronizes the flip. | fixed Standard 480p presentation |
| gs_pixels | parameter | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | function | Points at the complete prepared GS16 desktop that must remain stable through synchronized presentation. | fixed Standard 480p presentation |
| pixel_count | parameter | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | function | Supplies the caller pixel count, which must exactly equal the fixed display pixel count. | fixed Standard 480p presentation |
| clear_color | constant | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Holds the opaque black GS clear color used before drawing the desktop texture. | fixed Standard 480p presentation |
| texture_color | constant | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Holds the neutral GS texture modulation color used to preserve uploaded desktop pixels. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_shutdown | function | src/platform/ps2_graphics.c | PS2 graphics | public | Releases the owned gsKit global and resets local texture ownership state. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| PSTVNC_PS2_GRAPHICS_H | include-guard macro | src/platform/ps2_graphics.h | PS2 graphics interface | header | Prevents duplicate inclusion of the narrow clean PS2 graphics interface. | PS2 platform mechanisms |
| pstvnc_ps2_graphics_init | function declaration | src/platform/ps2_graphics.h | PS2 graphics interface | public | Declares initialization of dmaKit and the fixed hardware-proven Standard 480p GS instance. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_present | function declaration | src/platform/ps2_graphics.h | PS2 graphics interface | public | Declares complete upload, draw, execution, and synchronized flip of GS pixels. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| gs_pixels | prototype parameter | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_present | prototype | Names the caller-owned GS16 desktop pointer in the public presentation contract. | fixed Standard 480p presentation |
| pixel_count | prototype parameter | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_present | prototype | Names the exact desktop pixel-count argument in the public presentation contract. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_shutdown | function declaration | src/platform/ps2_graphics.h | PS2 graphics interface | public | Declares release and reset of clean PS2 graphics ownership. | PS2 platform mechanisms |
| DEV9_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked embedded DEV9 IOP module image loaded first during Ethernet startup. | private-Ethernet platform seam |
| size_DEV9_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked byte size of the embedded DEV9 IOP module image. | private-Ethernet platform seam |
| NETMAN_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked embedded NETMAN IOP module image loaded after DEV9. | private-Ethernet platform seam |
| size_NETMAN_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked byte size of the embedded NETMAN IOP module image. | private-Ethernet platform seam |
| SMAP_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked embedded SMAP Ethernet-driver IOP module image loaded after NETMAN. | private-Ethernet platform seam |
| size_SMAP_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked byte size of the embedded SMAP IOP module image. | private-Ethernet platform seam |
| PSTVNC_PS2_RFB_RX_BUFFER_SIZE | macro | src/platform/ps2_network.c | exact RFB transport | file | Sets the private 32768-byte TCP receive buffer used to decouple RFB framing from recv boundaries. | RFB exact I/O seam |
| rfb_rx_buffer | variable | src/platform/ps2_network.c | exact RFB transport | file static | Stores TCP bytes received ahead of the protocol layer's current exact-read request. | RFB exact I/O seam |
| rfb_rx_pos | variable | src/platform/ps2_network.c | exact RFB transport | file static | Tracks the next unread byte in the private buffered TCP receive span. | RFB exact I/O seam |
| rfb_rx_end | variable | src/platform/ps2_network.c | exact RFB transport | file static | Tracks the exclusive end of valid unread bytes in the private receive buffer. | RFB exact I/O seam |
| rfb_rx_socket | variable | src/platform/ps2_network.c | exact RFB transport | file static | Records which socket identity owns the currently buffered RFB bytes. | RFB exact I/O seam |
| reset_rfb_rx | function | src/platform/ps2_network.c | exact RFB transport | file | Discards buffered receive state and binds the empty buffer to a new socket identity. | RFB exact I/O seam |
| socket_fd | parameter | src/platform/ps2_network.c | reset_rfb_rx | function | Supplies the socket identity that will own subsequent buffered RFB bytes. | RFB exact I/O seam |
| link_wait_alarm | function | src/platform/ps2_network.c | Ethernet link wait | file callback | Wakes the sleeping startup thread when one bounded Ethernet-link wait interval expires. | private-Ethernet platform seam |
| alarm_id | parameter | src/platform/ps2_network.c | link_wait_alarm | callback | Receives the PS2 alarm identifier required by the alarm callback signature. | private-Ethernet platform seam |
| time | parameter | src/platform/ps2_network.c | link_wait_alarm | callback | Receives the alarm time value required by the PS2 alarm callback signature. | private-Ethernet platform seam |
| common | parameter | src/platform/ps2_network.c | link_wait_alarm | callback | Points at the sleeping thread identifier that must be awakened. | private-Ethernet platform seam |
| link_is_up | function | src/platform/ps2_network.c | Ethernet link wait | file | Queries NETMAN and reports whether the Ethernet carrier is currently up. | private-Ethernet platform seam |
| pstvnc_ps2_network_init | function | src/platform/ps2_network.c | PS2 network | public | Loads the qualified IOP network stack and configures the fixed 192.168.50.0/24 private link. | ISSUE7_MINIMAL_CORE: private-Ethernet platform seam |
| local_ip | variable | src/platform/ps2_network.c | pstvnc_ps2_network_init | local | Holds the fixed PS2 private-link IPv4 address passed to PS2IP initialization. | private-Ethernet platform seam |
| netmask | variable | src/platform/ps2_network.c | pstvnc_ps2_network_init | local | Holds the fixed private-link IPv4 netmask passed to PS2IP initialization. | private-Ethernet platform seam |
| gateway | variable | src/platform/ps2_network.c | pstvnc_ps2_network_init | local | Holds the fixed Pi peer/gateway IPv4 address passed to PS2IP initialization. | private-Ethernet platform seam |
| pstvnc_ps2_network_wait_link | function | src/platform/ps2_network.c | PS2 network | public | Waits for Ethernet carrier using bounded alarm-driven sleeps and fails when startup allowance is exhausted. | ISSUE7_MINIMAL_CORE: private-Ethernet platform seam |
| thread_id | variable | src/platform/ps2_network.c | pstvnc_ps2_network_wait_link | local | Stores the current EE thread identifier supplied to the alarm callback for wakeup. | private-Ethernet platform seam |
| retry_cycles | variable | src/platform/ps2_network.c | pstvnc_ps2_network_wait_link | local | Counts bounded one-second link-wait intervals before startup fails. | private-Ethernet platform seam |
| pstvnc_ps2_network_connect_vnc | function | src/platform/ps2_network.c | PS2 network | public | Opens one blocking TCP connection to the fixed PS2-facing VNC endpoint and binds receive buffering to it. | ISSUE7_MINIMAL_CORE: private-Ethernet platform seam |
| socket_fd | variable | src/platform/ps2_network.c | pstvnc_ps2_network_connect_vnc | local | Holds the newly created TCP socket descriptor through connect and return. | private-Ethernet platform seam |
| server | variable | src/platform/ps2_network.c | pstvnc_ps2_network_connect_vnc | local | Holds the fixed IPv4 VNC endpoint address used by the blocking connect call. | private-Ethernet platform seam |
| pstvnc_ps2_network_close | function | src/platform/ps2_network.c | PS2 network | public | Closes a valid socket and discards buffered RFB bytes when that socket owns them. | private-Ethernet platform seam |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_ps2_network_close | function | Supplies the socket descriptor whose ownership is being closed and invalidated. | private-Ethernet platform seam |
| pstvnc_rfb_io_read_exact | function | src/platform/ps2_network.c | exact RFB transport | platform seam | Delivers exactly the requested RFB byte count across arbitrary TCP recv boundaries or fails. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | function | Supplies the synchronized session socket from which exact protocol bytes are read. | RFB exact I/O seam |
| buffer | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | function | Points at caller-owned destination storage for the exact requested protocol bytes. | RFB exact I/O seam |
| count | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | function | Gives the exact number of protocol bytes the transport must deliver before returning success. | RFB exact I/O seam |
| destination | variable | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | local | Provides byte-addressable access to the caller's exact-read destination buffer. | RFB exact I/O seam |
| done | variable | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | local | Counts protocol bytes already copied into the caller's destination. | RFB exact I/O seam |
| available | variable | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | local | Measures currently buffered unread TCP bytes available to satisfy the exact read. | RFB exact I/O seam |
| need | variable | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | local | Measures how many protocol bytes remain before the caller's exact request is complete. | RFB exact I/O seam |
| take | variable | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | local | Selects the bounded number of buffered bytes copied during the current iteration. | RFB exact I/O seam |
| received | variable | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | local | Captures the next recv result before newly received bytes become buffered transport state. | RFB exact I/O seam |
| pstvnc_rfb_io_write_exact | function | src/platform/ps2_network.c | exact RFB transport | platform seam | Sends every byte of one protocol-sized client message across partial send results or fails. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | function | Supplies the synchronized session socket that owns the outbound protocol message. | RFB exact I/O seam |
| buffer | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | function | Points at the caller-owned outbound protocol bytes that must all be accepted by the socket. | RFB exact I/O seam |
| count | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | function | Gives the exact outbound byte count required for transport success. | RFB exact I/O seam |
| source | variable | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | local | Provides byte-addressable access to the caller's outbound protocol buffer. | RFB exact I/O seam |
| done | variable | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | local | Counts outbound protocol bytes already accepted by successful send calls. | RFB exact I/O seam |
| sent | variable | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | local | Captures each partial send result before advancing the exact-write progress count. | RFB exact I/O seam |
| pstvnc_rfb_io_poll_receive | function | src/platform/ps2_network.c | exact RFB transport | platform seam | Checks for server bytes with MSG_DONTWAIT and safely prefills the private exact-read buffer without advancing protocol parsing. | Issue #38 responsive RFB scheduling |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_poll_receive | function | Supplies the synchronized session socket whose buffered or socket receive readiness is checked. | RFB exact I/O seam |
| received | variable | src/platform/ps2_network.c | pstvnc_rfb_io_poll_receive | function | Stores the nonblocking recv result used to distinguish prefetched bytes, idle EAGAIN, closure, and transport failure. | Issue #38 responsive RFB scheduling |
| PSTVNC_PS2_NETWORK_H | include-guard macro | src/platform/ps2_network.h | PS2 network interface | header | Prevents duplicate inclusion of the clean PS2 private-link interface. | private-Ethernet platform seam |
| PSTVNC_PS2_LOCAL_IP | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed Issue #7 PS2 private-link IPv4 address 192.168.50.2. | private-Ethernet platform seam |
| PSTVNC_PS2_NETMASK | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed Issue #7 private-link IPv4 netmask 255.255.255.0. | private-Ethernet platform seam |
| PSTVNC_PS2_GATEWAY_IP | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed Pi peer/gateway IPv4 address 192.168.50.1. | private-Ethernet platform seam |
| PSTVNC_PS2_VNC_SERVER_IP | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed private-link address of the Pi VNC endpoint. | ISSUE7_MINIMAL_CORE: private-Ethernet platform seam |
| PSTVNC_PS2_VNC_SERVER_PORT | macro | src/platform/ps2_network.h | PS2 network interface | public | Names TCP port 5900 for the fixed PS2-facing VNC endpoint. | private-Ethernet platform seam |
| pstvnc_ps2_network_init | function declaration | src/platform/ps2_network.h | PS2 network interface | public | Declares loading and initialization of the fixed clean private-link network stack. | private-Ethernet platform seam |
| pstvnc_ps2_network_wait_link | function declaration | src/platform/ps2_network.h | PS2 network interface | public | Declares the bounded qualified Ethernet-carrier startup wait. | private-Ethernet platform seam |
| pstvnc_ps2_network_connect_vnc | function declaration | src/platform/ps2_network.h | PS2 network interface | public | Declares connection to the fixed PS2-facing VNC endpoint. | private-Ethernet platform seam |
| pstvnc_ps2_network_close | function declaration | src/platform/ps2_network.h | PS2 network interface | public | Declares socket close plus invalidation of any transport buffer owned by that socket. | private-Ethernet platform seam |
| socket_fd | prototype parameter | src/platform/ps2_network.h | pstvnc_ps2_network_close | prototype | Names the socket descriptor being closed in the public network contract. | private-Ethernet platform seam |
| SIO2MAN_irx | external symbol declaration | src/platform/ps2_system.c | embedded controller modules | linker seam | Names the linked embedded SIO2MAN IOP image loaded before PADMAN during deterministic system bootstrap. | PS2 controller-service foundation |
| size_SIO2MAN_irx | external symbol declaration | src/platform/ps2_system.c | embedded controller modules | linker seam | Names the linked byte size of the embedded SIO2MAN IOP image. | PS2 controller-service foundation |
| PADMAN_irx | external symbol declaration | src/platform/ps2_system.c | embedded controller modules | linker seam | Names the linked embedded PADMAN IOP image loaded after SIO2MAN during deterministic system bootstrap. | PS2 controller-service foundation |
| size_PADMAN_irx | external symbol declaration | src/platform/ps2_system.c | embedded controller modules | linker seam | Names the linked byte size of the embedded PADMAN IOP image. | PS2 controller-service foundation |
| pstvnc_ps2_system_prepare_iop | function | src/platform/ps2_system.c | PS2 system | public | Resets and synchronizes the IOP, initializes RPC/loadfile/heap support, enables memory module loading, and establishes embedded SIO2MAN/PADMAN controller services. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| pstvnc_ps2_system_exit_to_menu | function | src/platform/ps2_system.c | PS2 system | public | Transfers control to OSDSYS and parks the thread if that transfer unexpectedly returns. | CLEAN_ARCHITECTURE: Startup lifecycle |
| PSTVNC_PS2_SYSTEM_H | include-guard macro | src/platform/ps2_system.h | PS2 system interface | header | Prevents duplicate inclusion of the clean PS2 system-lifecycle interface. | PS2 platform mechanisms |
| pstvnc_ps2_system_prepare_iop | function declaration | src/platform/ps2_system.h | PS2 system interface | public | Declares deterministic IOP preparation and embedded SIO2MAN/PADMAN controller-service establishment before feature initialization. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| pstvnc_ps2_system_exit_to_menu | function declaration | src/platform/ps2_system.h | PS2 system interface | public | Declares final convergence on OSDSYS with no valid product continuation. | CLEAN_ARCHITECTURE: Startup lifecycle |
