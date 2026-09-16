# Clean symbols — `src/platform`

DIRECTORY=src/platform
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns genuinely PS2-specific system, controller-service
bootstrap, Ethernet, and GS mechanisms, including desktop-plus-local-overlay
presentation.
The clean Issue #7 platform surface is limited to the six `ps2_*.[ch]` files
listed below; no retained historical platform module is silently included.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| display | variable | src/platform/ps2_graphics.c | PS2 graphics | file static | Owns the gsKit global display instance used by the fixed Standard 480p presentation path. | fixed Standard 480p presentation |
| desktop_texture | variable | src/platform/ps2_graphics.c | PS2 graphics | file static | Owns the reusable gsKit texture descriptor for the complete 704x462 desktop upload. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_init | function | src/platform/ps2_graphics.c | PS2 graphics | public | Initializes dmaKit, gsKit, and the fixed Standard 480p presentation state exactly once. | ISSUE7_MINIMAL_CORE: Fixed Standard 480p presentation |
| pstvnc_ps2_graphics_present | function | src/platform/ps2_graphics.c | PS2 graphics | public | Presents one complete coherent logical desktop plus an optional generic caller-owned local overlay, mapping both project surfaces into private gsKit and VRAM mechanisms before synchronized execution and flip. | fixed Standard 480p presentation |
| pixel_count | parameter | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | function | Supplies the caller pixel count, which must exactly equal the fixed display pixel count. | fixed Standard 480p presentation |
| clear_color | constant | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Holds the opaque black GS clear color used before drawing the desktop texture. | fixed Standard 480p presentation |
| texture_color | constant | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Holds the neutral GS texture modulation color used to preserve uploaded desktop pixels. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_shutdown | function | src/platform/ps2_graphics.c | PS2 graphics | public | Releases the owned gsKit global and resets local texture ownership state. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| PSTVNC_PS2_GRAPHICS_H | include-guard macro | src/platform/ps2_graphics.h | PS2 graphics interface | header | Prevents duplicate inclusion of the narrow clean PS2 graphics interface. | PS2 platform mechanisms |
| pstvnc_ps2_graphics_init | function declaration | src/platform/ps2_graphics.h | PS2 graphics interface | public | Declares initialization of dmaKit and the fixed hardware-proven Standard 480p GS instance. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_present | function declaration | src/platform/ps2_graphics.h | PS2 graphics interface | public | Presents one complete coherent logical desktop plus an optional generic caller-owned local overlay, mapping both project surfaces into private gsKit and VRAM mechanisms before synchronized execution and flip. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| pixel_count | prototype parameter | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_present | prototype | Names the exact desktop pixel-count argument in the public presentation contract. | fixed Standard 480p presentation |
| pstvnc_ps2_graphics_shutdown | function declaration | src/platform/ps2_graphics.h | PS2 graphics interface | public | Declares release and reset of clean PS2 graphics ownership. | PS2 platform mechanisms |
| DEV9_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked embedded DEV9 IOP module image loaded first during Ethernet startup. | private-Ethernet platform seam |
| size_DEV9_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked byte size of the embedded DEV9 IOP module image. | private-Ethernet platform seam |
| NETMAN_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked embedded NETMAN IOP module image loaded after DEV9. | private-Ethernet platform seam |
| size_NETMAN_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked byte size of the embedded NETMAN IOP module image. | private-Ethernet platform seam |
| SMAP_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked embedded SMAP Ethernet-driver IOP module image loaded after NETMAN. | private-Ethernet platform seam |
| size_SMAP_irx | external symbol declaration | src/platform/ps2_network.c | embedded network modules | linker seam | Names the linked byte size of the embedded SMAP IOP module image. | private-Ethernet platform seam |
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
| socket_fd | variable | src/platform/ps2_network.c | pstvnc_ps2_network_connect_vnc | local | Holds the newly created TCP socket descriptor through connect and return. | private-Ethernet platform seam |
| server | variable | src/platform/ps2_network.c | pstvnc_ps2_network_connect_vnc | local | Holds the fixed IPv4 VNC endpoint address used by the blocking connect call. | private-Ethernet platform seam |
| pstvnc_ps2_network_close | function | src/platform/ps2_network.c | PS2 network | public | Closes a valid socket and discards buffered RFB bytes when that socket owns them. | private-Ethernet platform seam |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_ps2_network_close | function | Supplies the socket descriptor whose ownership is being closed and invalidated. | private-Ethernet platform seam |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_read_exact | function | Supplies the synchronized session socket from which exact protocol bytes are read. | RFB exact I/O seam |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_write_exact | function | Supplies the synchronized session socket that owns the outbound protocol message. | RFB exact I/O seam |
| socket_fd | parameter | src/platform/ps2_network.c | pstvnc_rfb_io_poll_receive | function | Supplies the synchronized session socket whose buffered or socket receive readiness is checked. | RFB exact I/O seam |
| PSTVNC_PS2_NETWORK_H | include-guard macro | src/platform/ps2_network.h | PS2 network interface | header | Prevents duplicate inclusion of the clean PS2 private-link interface. | private-Ethernet platform seam |
| PSTVNC_PS2_LOCAL_IP | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed Issue #7 PS2 private-link IPv4 address 192.168.50.2. | private-Ethernet platform seam |
| PSTVNC_PS2_NETMASK | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed Issue #7 private-link IPv4 netmask 255.255.255.0. | private-Ethernet platform seam |
| PSTVNC_PS2_GATEWAY_IP | macro | src/platform/ps2_network.h | PS2 network interface | public | Names the fixed Pi peer/gateway IPv4 address 192.168.50.1. | private-Ethernet platform seam |
| pstvnc_ps2_network_init | function declaration | src/platform/ps2_network.h | PS2 network interface | public | Declares loading and initialization of the fixed clean private-link network stack. | private-Ethernet platform seam |
| pstvnc_ps2_network_wait_link | function declaration | src/platform/ps2_network.h | PS2 network interface | public | Declares the bounded qualified Ethernet-carrier startup wait. | private-Ethernet platform seam |
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
| pstvnc_ps2_system_delay_us | function | src/platform/ps2_system.c | PS2 system | public | Delays the current EE thread for the caller-supplied microsecond interval without transferring ownership to another subsystem. | Issue #38 responsive RFB scheduling |
| microseconds | parameter | src/platform/ps2_system.c | pstvnc_ps2_system_delay_us | function | Supplies the requested EE-thread delay interval in microseconds. | Issue #38 responsive RFB scheduling |
| pstvnc_ps2_system_delay_us | function declaration | src/platform/ps2_system.h | PS2 system interface | public | Declares the narrow PS2 thread-delay platform seam used by responsive application polling. | Issue #38 responsive RFB scheduling |
| microseconds | prototype parameter | src/platform/ps2_system.h | pstvnc_ps2_system_delay_us | prototype | Names the requested microsecond delay in the public PS2 system interface. | Issue #38 responsive RFB scheduling |
| desktop_texture_configured | variable | src/platform/ps2_graphics.c | PS2 graphics | file | Records whether the fixed desktop texture already owns its persistent GS VRAM allocation. | Issue #39: local overlay presentation |
| local_overlay_texture | variable | src/platform/ps2_graphics.c | PS2 graphics | file | Owns the reusable gsKit texture descriptor used only for the optional generic local overlay. | Issue #39: local overlay presentation |
| local_overlay_texture_configured | variable | src/platform/ps2_graphics.c | PS2 graphics | file | Records whether the reusable local-overlay texture currently owns valid GS VRAM backing. | Issue #39: local overlay presentation |
| local_overlay_width | variable | src/platform/ps2_graphics.c | PS2 graphics | file | Records the width associated with the current reusable local-overlay texture allocation. | Issue #39: local overlay presentation |
| local_overlay_height | variable | src/platform/ps2_graphics.c | PS2 graphics | file | Records the height associated with the current reusable local-overlay texture allocation. | Issue #39: local overlay presentation |
| configure_desktop_texture | function | src/platform/ps2_graphics.c | PS2 graphics | file | Initializes the fixed desktop gsKit texture descriptor and allocates its persistent GS VRAM backing. | Issue #39: local overlay presentation |
| desktop_pixels | parameter | src/platform/ps2_graphics.c | configure_desktop_texture | local | Supplies desktop pixels to configure_desktop_texture. | Issue #39: local overlay presentation |
| local_overlay_is_valid | function | src/platform/ps2_graphics.c | PS2 graphics | file | Validates optional local-overlay storage, geometry, pixel count, and placement within the logical desktop before any GS presentation work. | Issue #39: local overlay presentation |
| overlay | parameter | src/platform/ps2_graphics.c | local_overlay_is_valid | local | Supplies overlay to local_overlay_is_valid. | Issue #39: local overlay presentation |
| expected_pixels | variable | src/platform/ps2_graphics.c | local_overlay_is_valid | local | Stores expected pixels while local_overlay_is_valid runs. | Issue #39: local overlay presentation |
| configure_local_overlay_texture | function | src/platform/ps2_graphics.c | PS2 graphics | file | Configures or reconfigures the reusable gsKit local-overlay texture and its VRAM backing for the current overlay dimensions. | Issue #39: local overlay presentation |
| overlay | parameter | src/platform/ps2_graphics.c | configure_local_overlay_texture | local | Supplies overlay to configure_local_overlay_texture. | Issue #39: local overlay presentation |
| desktop_pixels | parameter | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Supplies desktop pixels to pstvnc_ps2_graphics_present. | Issue #39: local overlay presentation |
| desktop_pixel_count | parameter | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Supplies desktop pixel count to pstvnc_ps2_graphics_present. | Issue #39: local overlay presentation |
| local_overlay | parameter | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Supplies local overlay to pstvnc_ps2_graphics_present. | Issue #39: local overlay presentation |
| x0 | variable | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Stores x0 while pstvnc_ps2_graphics_present runs. | Issue #39: local overlay presentation |
| y0 | variable | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Stores y0 while pstvnc_ps2_graphics_present runs. | Issue #39: local overlay presentation |
| x1 | variable | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Stores x1 while pstvnc_ps2_graphics_present runs. | Issue #39: local overlay presentation |
| y1 | variable | src/platform/ps2_graphics.c | pstvnc_ps2_graphics_present | local | Stores y1 while pstvnc_ps2_graphics_present runs. | Issue #39: local overlay presentation |
| pstvnc_ps2_graphics_overlay | structure | src/platform/ps2_graphics.h | PS2 graphics interface | public | Describes one generic caller-owned 16-bit local overlay surface plus logical dimensions and placement while keeping product meaning outside PS2 graphics. | Issue #39: local overlay presentation |
| pixels | field | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_overlay | public | Points to caller-owned opaque 16-bit project presentation pixels for the optional generic local overlay. | Issue #39: local overlay presentation |
| pixel_count | field | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_overlay | public | Records the exact available pixel count used to validate the generic local overlay surface. | Issue #39: local overlay presentation |
| width | field | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_overlay | public | Records the logical width of the generic local overlay surface. | Issue #39: local overlay presentation |
| height | field | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_overlay | public | Records the logical height of the generic local overlay surface. | Issue #39: local overlay presentation |
| x | field | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_overlay | public | Records the logical desktop X coordinate at which the platform presents the generic local overlay. | Issue #39: local overlay presentation |
| y | field | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_overlay | public | Records the logical desktop Y coordinate at which the platform presents the generic local overlay. | Issue #39: local overlay presentation |
| pstvnc_ps2_graphics_overlay_t | type | src/platform/ps2_graphics.h | PS2 graphics interface | public | Provides the generic project-overlay descriptor accepted by the narrow PS2 graphics seam. | Issue #39: local overlay presentation |
| desktop_pixels | prototype parameter | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_present | prototype | Declares the desktop pixels argument accepted by pstvnc_ps2_graphics_present. | Issue #39: local overlay presentation |
| desktop_pixel_count | prototype parameter | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_present | prototype | Declares the desktop pixel count argument accepted by pstvnc_ps2_graphics_present. | Issue #39: local overlay presentation |
| local_overlay | prototype parameter | src/platform/ps2_graphics.h | pstvnc_ps2_graphics_present | prototype | Declares the local overlay argument accepted by pstvnc_ps2_graphics_present. | Issue #39: local overlay presentation |
| pstvnc_ps2_network_connect_pstv | function | src/platform/ps2_network.c | ps2_network | file | Defines pstvnc_ps2_network_connect_pstv as a current clean-source function. | mechanically reconciled current clean source |
| server | variable | src/platform/ps2_network.c | pstvnc_ps2_network_connect_pstv | local | Defines server as a current clean-source variable. | mechanically reconciled current clean source |
| socket_fd | variable | src/platform/ps2_network.c | pstvnc_ps2_network_connect_pstv | local | Defines socket_fd as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_ps2_network_connect_pstv | function declaration | src/platform/ps2_network.h | ps2_network interface | public | Defines pstvnc_ps2_network_connect_pstv as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_PS2_PSTV_SERVER_IP | macro | src/platform/ps2_network.h | ps2_network interface | public | Defines PSTVNC_PS2_PSTV_SERVER_IP as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_PS2_PSTV_SERVER_PORT | macro | src/platform/ps2_network.h | ps2_network interface | public | Defines PSTVNC_PS2_PSTV_SERVER_PORT as a current clean-source macro. | mechanically reconciled current clean source |
