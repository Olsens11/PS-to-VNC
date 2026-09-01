# Clean symbols — `src/platform`

DIRECTORY=src/platform
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This directory owns genuinely PS2-specific system, Ethernet, and GS mechanisms.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| pstvnc_ps2_graphics_init | function | src/platform/ps2_graphics.c | PS2 graphics | public | Initializes dmaKit, gsKit, and the fixed Standard 480p presentation state. | ISSUE7_MINIMAL_CORE: Fixed Standard 480p presentation |
| pstvnc_ps2_graphics_present | function declaration | src/platform/ps2_graphics.h | PS2 graphics | public | Declares complete upload, draw, execution, and synchronized flip of GS pixels. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| pstvnc_ps2_network_init | function | src/platform/ps2_network.c | PS2 network | public | Loads the qualified IOP network stack and configures the fixed private link. | ISSUE7_MINIMAL_CORE: private-Ethernet platform seam |
| PSTVNC_PS2_VNC_SERVER_IP | macro | src/platform/ps2_network.h | PS2 network | public | Names the fixed private-link address of the Pi VNC endpoint. | ISSUE7_MINIMAL_CORE: private-Ethernet platform seam |
| pstvnc_ps2_system_prepare_iop | function | src/platform/ps2_system.c | PS2 system | public | Resets and synchronizes the IOP before product module loading. | CLEAN_ARCHITECTURE: PS2 platform mechanisms |
| pstvnc_ps2_system_exit_to_menu | function declaration | src/platform/ps2_system.h | PS2 system | public | Declares final convergence on OSDSYS with no valid product continuation. | CLEAN_ARCHITECTURE: Startup lifecycle |
