# Clean symbols — `src`

DIRECTORY=src
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This directory owns the application coordinator, authoritative framebuffer,
minimal diagnostics transport, display conversion, and RFB wire/session core.
The table is being expanded to every named symbol before enforcement is enabled.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| pstvnc_app_run | function | src/app.c | application coordinator | public | Runs ordered startup, the blocking live loop, and fatal cleanup convergence. | ISSUE7_MINIMAL_CORE: Complete application coordinator |
| pstvnc_app_run | function declaration | src/app.h | application coordinator | public | Declares the single top-level clean application lifecycle entry point. | CLEAN_ARCHITECTURE: Application coordinator |
| pstvnc_diagnostics_init | function | src/diagnostics.c | diagnostics transport | public | Opens and configures the optional UDP diagnostics transport to the Pi. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| pstvnc_diagnostics_send | function declaration | src/diagnostics.h | diagnostics transport | public | Declares best-effort transmission of one caller-owned diagnostic datagram. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| pstvnc_display_prepare_gs16 | function | src/display.c | display conversion | public | Converts an authoritative remote framebuffer into opaque GS CT16 pixels. | ISSUE7_MINIMAL_CORE: Fixed Standard 480p presentation |
| PSTVNC_DISPLAY_WIDTH | macro | src/display.h | display conversion | public | Names the fixed Issue #7 logical desktop width in pixels. | ISSUE7_MINIMAL_CORE: Fixed first target |
| pstvnc_framebuffer_init | function | src/framebuffer.c | desktop framebuffer | public | Initializes framebuffer metadata around caller-owned pixel storage. | ISSUE7_MINIMAL_CORE: Owned authoritative framebuffer |
| pstvnc_framebuffer_t | type | src/framebuffer.h | desktop framebuffer | public | Represents pixel storage, geometry, validity, and dirty-update state. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| main | function | src/main.c | process entry | process | Enters the coordinator and converges its return on the PS2 system menu. | CLEAN_ARCHITECTURE: Startup lifecycle |
| pstvnc_rfb_parse_protocol_version | function | src/rfb.c | RFB wire format | public | Validates and decodes the fixed 12-byte RFB protocol banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| PSTVNC_RFB_PROTOCOL_VERSION_SIZE | macro | src/rfb.h | RFB wire format | public | Names the exact byte length of an RFB version banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_io_read_exact | function declaration | src/rfb_io.h | RFB transport seam | platform | Requires the platform transport to deliver an exact protocol byte count. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_session_init | function | src/rfb_session.c | RFB session | public | Establishes the new, disconnected baseline for one synchronized session. | CLEAN_ARCHITECTURE: RFB client/session |
| pstvnc_rfb_session_t | type | src/rfb_session.h | RFB session | public | Holds handshake state, errors, parsed server data, and bounded row scratch. | ISSUE7_MINIMAL_CORE: Shared Raw server-message parser |
