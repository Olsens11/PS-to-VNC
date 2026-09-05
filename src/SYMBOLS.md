# Clean symbols — `src`

DIRECTORY=src
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the application coordinator, authoritative framebuffer,
minimal diagnostics transport, display conversion, direct libpad pad
acquisition, controller/input runtime ownership, ordinary semantic input-event
FIFO, pure remote-mouse response policy, semantic keyboard events, the
controller-driven on-screen keyboard, local foreground ownership/presentation,
and the RFB wire/session core.
The inventory below covers the clean-generation symbols defined directly in
this directory. Historical/adopted subdirectories retain their own dictionaries.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| remote_pixels | variable | src/app.c | application coordinator | file | Stores the single authoritative CPU-side remote desktop image. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| gs_pixels | variable | src/app.c | application coordinator | file | Stores disposable GS-ready presentation pixels derived from remote authority. | CLEAN_ARCHITECTURE: Display model and presentation |
| send_diagnostic_literal | function | src/app.c | application coordinator | file | Sends one fixed diagnostic record only when optional diagnostics is available. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| diagnostics_ready | parameter | src/app.c | send_diagnostic_literal | local | States whether the optional diagnostics transport is owned and usable. | application coordinator cleanup |
| text | parameter | src/app.c | send_diagnostic_literal | local | Points to the caller-owned diagnostic bytes to transmit. | clean diagnostics |
| length | parameter | src/app.c | send_diagnostic_literal | local | Gives the exact diagnostic byte count to transmit. | clean diagnostics |
| pstvnc_app_run | function | src/app.c | application coordinator | public | Runs ordered startup, the blocking live loop, and fatal cleanup convergence. | ISSUE7_MINIMAL_CORE: Complete application coordinator |
| net_ready | variable | src/app.c | pstvnc_app_run | local | Holds the fixed diagnostic stage record emitted after the private link and required VNC endpoint are connected. | clean diagnostics |
| gs_ready | variable | src/app.c | pstvnc_app_run | local | Holds the fixed diagnostic stage record emitted after graphics initialization. | clean diagnostics |
| desktop_ready | variable | src/app.c | pstvnc_app_run | local | Holds the fixed diagnostic stage record emitted after first authoritative presentation. | clean diagnostics |
| fatal | variable | src/app.c | pstvnc_app_run | local | Holds the fixed diagnostic record emitted when the coordinator enters fatal convergence. | failure policy |
| framebuffer | variable | src/app.c | pstvnc_app_run | local | Holds metadata and authority state for the caller-owned remote pixel storage. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| session | variable | src/app.c | pstvnc_app_run | local | Holds the synchronized RFB protocol/session state owned by the main loop. | CLEAN_ARCHITECTURE: RFB client/session |
| socket_fd | variable | src/app.c | pstvnc_app_run | local | Tracks ownership of the connected VNC socket for fail-path cleanup. | application coordinator cleanup |
| graphics_ready | variable | src/app.c | pstvnc_app_run | local | Tracks whether GS resources were acquired and therefore require shutdown. | application coordinator cleanup |
| diagnostics_ready | variable | src/app.c | pstvnc_app_run | local | Tracks whether optional diagnostics resources require shutdown. | application coordinator cleanup |
| PSTVNC_APP_H | include guard | src/app.h | application coordinator interface | file | Prevents repeated inclusion of the coordinator public declaration. | clean source interface |
| pstvnc_app_run | function declaration | src/app.h | application coordinator interface | public | Declares the single top-level clean application lifecycle entry point. | CLEAN_ARCHITECTURE: Application coordinator |
| PSTVNC_DIAGNOSTICS_HOST_IPV4 | macro | src/diagnostics.c | diagnostics transport | file | Names the fixed private-link Pi address that receives observer datagrams. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| PSTVNC_DIAGNOSTICS_UDP_PORT | macro | src/diagnostics.c | diagnostics transport | file | Names the fixed UDP port used by the qualification observer stream. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| diagnostics_socket | variable | src/diagnostics.c | diagnostics transport | file | Owns the optional UDP socket or records that no diagnostics socket exists. | clean diagnostics ownership |
| diagnostics_address | variable | src/diagnostics.c | diagnostics transport | file | Stores the fixed sockaddr used for best-effort diagnostic datagrams. | clean diagnostics ownership |
| pstvnc_diagnostics_init | function | src/diagnostics.c | diagnostics transport | public | Opens and configures the optional UDP diagnostics transport to the Pi. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| pstvnc_diagnostics_is_ready | function | src/diagnostics.c | diagnostics transport | public | Reports whether the optional diagnostics socket is currently owned. | clean diagnostics ownership |
| pstvnc_diagnostics_send | function | src/diagnostics.c | diagnostics transport | public | Sends one caller-owned diagnostic payload as one UDP observer datagram. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| data | parameter | src/diagnostics.c | pstvnc_diagnostics_send | local | Points to caller-owned diagnostic bytes without transferring state ownership. | clean diagnostics |
| length | parameter | src/diagnostics.c | pstvnc_diagnostics_send | local | Gives the exact payload byte count for one diagnostic datagram. | clean diagnostics |
| pstvnc_diagnostics_shutdown | function | src/diagnostics.c | diagnostics transport | public | Releases optional diagnostics resources and restores the uninitialized baseline. | application coordinator cleanup |
| PSTVNC_DIAGNOSTICS_H | include guard | src/diagnostics.h | diagnostics transport interface | file | Prevents repeated inclusion of the diagnostics transport declarations. | clean source interface |
| pstvnc_diagnostics_init | function declaration | src/diagnostics.h | diagnostics transport interface | public | Declares optional diagnostics transport initialization. | clean diagnostics |
| pstvnc_diagnostics_is_ready | function declaration | src/diagnostics.h | diagnostics transport interface | public | Declares readiness inspection for the optional diagnostics transport. | clean diagnostics |
| pstvnc_diagnostics_send | function declaration | src/diagnostics.h | diagnostics transport interface | public | Declares best-effort transmission of one caller-owned diagnostic datagram. | clean diagnostics |
| data | parameter | src/diagnostics.h | pstvnc_diagnostics_send | public | Declares the diagnostic payload pointer accepted by the transport seam. | clean diagnostics |
| length | parameter | src/diagnostics.h | pstvnc_diagnostics_send | public | Declares the diagnostic payload byte count accepted by the transport seam. | clean diagnostics |
| pstvnc_diagnostics_shutdown | function declaration | src/diagnostics.h | diagnostics transport interface | public | Declares release of optional diagnostics transport resources. | application coordinator cleanup |
| pstvnc_display_prepare_gs16 | function | src/display.c | display conversion | public | Converts an authoritative remote framebuffer into opaque GS CT16 pixels. | ISSUE7_MINIMAL_CORE: Fixed Standard 480p presentation |
| framebuffer | parameter | src/display.c | pstvnc_display_prepare_gs16 | local | Supplies the valid authoritative remote image without permitting mutation. | authoritative framebuffer semantics |
| gs_pixels | parameter | src/display.c | pstvnc_display_prepare_gs16 | local | Supplies destination storage for disposable GS presentation pixels. | display presentation seam |
| gs_capacity_pixels | parameter | src/display.c | pstvnc_display_prepare_gs16 | local | Gives the available presentation-buffer capacity for bounds validation. | display presentation seam |
| i | variable | src/display.c | pstvnc_display_prepare_gs16 | local | Iterates exactly once over every pixel in the fixed logical desktop. | fixed 480p presentation |
| PSTVNC_DISPLAY_H | include guard | src/display.h | display conversion interface | file | Prevents repeated inclusion of fixed display-contract declarations. | clean source interface |
| PSTVNC_DISPLAY_PHYSICAL_WIDTH | macro | src/display.h | display contract | public | Names the 720-pixel physical width of Standard 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_PHYSICAL_HEIGHT | macro | src/display.h | display contract | public | Names the 480-line physical height of Standard 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_WIDTH | macro | src/display.h | display contract | public | Names the fixed 704-pixel Issue #7 logical desktop width. | ISSUE7_MINIMAL_CORE: Fixed first target |
| PSTVNC_DISPLAY_HEIGHT | macro | src/display.h | display contract | public | Names the fixed 462-line Issue #7 logical desktop height. | ISSUE7_MINIMAL_CORE: Fixed first target |
| PSTVNC_DISPLAY_OFFSET_X | macro | src/display.h | display contract | public | Names the fixed horizontal placement offset within physical 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_OFFSET_Y | macro | src/display.h | display contract | public | Names the fixed vertical placement offset within physical 480p output. | fixed Standard 480p presentation |
| PSTVNC_DISPLAY_PIXEL_COUNT | macro | src/display.h | display contract | public | Computes the exact logical desktop pixel count used for storage and presentation. | fixed Standard 480p presentation |
| pstvnc_display_prepare_gs16 | function declaration | src/display.h | display conversion interface | public | Declares non-destructive conversion from authoritative RFB pixels to GS CT16. | CLEAN_ARCHITECTURE: Display model and presentation |
| framebuffer | parameter | src/display.h | pstvnc_display_prepare_gs16 | public | Declares the authoritative framebuffer input to display conversion. | display presentation seam |
| gs_pixels | parameter | src/display.h | pstvnc_display_prepare_gs16 | public | Declares presentation destination storage for display conversion. | display presentation seam |
| gs_capacity_pixels | parameter | src/display.h | pstvnc_display_prepare_gs16 | public | Declares destination capacity used to reject undersized presentation storage. | display presentation seam |
| rect_fits | function | src/framebuffer.c | desktop framebuffer | file | Validates a nonempty rectangle using widened arithmetic before any pixel write. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | rect_fits | local | Supplies geometry against which rectangle bounds are validated. | authoritative framebuffer semantics |
| x | parameter | src/framebuffer.c | rect_fits | local | Gives the rectangle left coordinate. | framebuffer rectangle bounds |
| y | parameter | src/framebuffer.c | rect_fits | local | Gives the rectangle top coordinate. | framebuffer rectangle bounds |
| width | parameter | src/framebuffer.c | rect_fits | local | Gives the nonzero rectangle width to validate. | framebuffer rectangle bounds |
| height | parameter | src/framebuffer.c | rect_fits | local | Gives the nonzero rectangle height to validate. | framebuffer rectangle bounds |
| right | variable | src/framebuffer.c | rect_fits | local | Stores the widened exclusive right edge used to prevent 16-bit wraparound. | framebuffer rectangle bounds |
| bottom | variable | src/framebuffer.c | rect_fits | local | Stores the widened exclusive bottom edge used to prevent 16-bit wraparound. | framebuffer rectangle bounds |
| include_dirty | function | src/framebuffer.c | desktop framebuffer | file | Expands the conservative dirty bounding rectangle around one successful write. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | include_dirty | local | Supplies mutable dirty-state ownership for the authoritative image. | framebuffer dirty state |
| x | parameter | src/framebuffer.c | include_dirty | local | Gives the newly changed rectangle left coordinate. | framebuffer dirty state |
| y | parameter | src/framebuffer.c | include_dirty | local | Gives the newly changed rectangle top coordinate. | framebuffer dirty state |
| width | parameter | src/framebuffer.c | include_dirty | local | Gives the newly changed rectangle width. | framebuffer dirty state |
| height | parameter | src/framebuffer.c | include_dirty | local | Gives the newly changed rectangle height. | framebuffer dirty state |
| left | variable | src/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle left edge. | framebuffer dirty state |
| top | variable | src/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle top edge. | framebuffer dirty state |
| right | variable | src/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle exclusive right edge. | framebuffer dirty state |
| bottom | variable | src/framebuffer.c | include_dirty | local | Stores the merged dirty rectangle exclusive bottom edge. | framebuffer dirty state |
| current_right | variable | src/framebuffer.c | include_dirty | local | Stores the prior dirty rectangle exclusive right edge during union. | framebuffer dirty state |
| current_bottom | variable | src/framebuffer.c | include_dirty | local | Stores the prior dirty rectangle exclusive bottom edge during union. | framebuffer dirty state |
| pstvnc_framebuffer_init | function | src/framebuffer.c | desktop framebuffer | public | Initializes framebuffer metadata around caller-owned pixel storage. | ISSUE7_MINIMAL_CORE: Owned authoritative framebuffer |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_init | local | Supplies the framebuffer value whose metadata is initialized. | desktop framebuffer lifecycle |
| pixels | parameter | src/framebuffer.c | pstvnc_framebuffer_init | local | Supplies caller-owned CPU pixel storage without asserting content authority. | desktop framebuffer lifecycle |
| pixel_capacity | parameter | src/framebuffer.c | pstvnc_framebuffer_init | local | Gives available pixel storage capacity independent of logical geometry. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_set_geometry | function | src/framebuffer.c | desktop framebuffer | public | Sets logical geometry only when storage is sufficient and revokes prior authority. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Supplies the framebuffer whose logical geometry is changed. | desktop framebuffer lifecycle |
| width | parameter | src/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Gives the requested nonzero logical width. | desktop framebuffer lifecycle |
| height | parameter | src/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Gives the requested nonzero logical height. | desktop framebuffer lifecycle |
| required | variable | src/framebuffer.c | pstvnc_framebuffer_set_geometry | local | Stores required pixel capacity for the requested geometry. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_pixel_count | function | src/framebuffer.c | desktop framebuffer | public | Returns the logical pixel count represented by current framebuffer geometry. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_pixel_count | local | Supplies geometry used to compute the logical pixel count. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_invalidate | function | src/framebuffer.c | desktop framebuffer | public | Revokes whole-frame authority and clears presentation dirty state. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_invalidate | local | Supplies the framebuffer whose authority is revoked. | failure semantics |
| pstvnc_framebuffer_mark_valid | function | src/framebuffer.c | desktop framebuffer | public | Records an external complete-frame proof by publishing framebuffer authority. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_mark_valid | local | Supplies the initialized framebuffer to promote after session proof. | authoritative framebuffer semantics |
| pstvnc_framebuffer_write_rect | function | src/framebuffer.c | desktop framebuffer | public | Copies a bounded source rectangle into CPU authority storage and records dirty bounds. | authoritative framebuffer semantics |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Supplies destination framebuffer storage and geometry. | framebuffer rectangle writes |
| x | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle left coordinate. | framebuffer rectangle writes |
| y | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle top coordinate. | framebuffer rectangle writes |
| width | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle width. | framebuffer rectangle writes |
| height | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives destination rectangle height. | framebuffer rectangle writes |
| source | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Points to source pixels that will be copied into framebuffer storage. | framebuffer rectangle writes |
| source_stride | parameter | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Gives source row stride and must cover the requested width. | framebuffer rectangle writes |
| row | variable | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Iterates over source and destination rows during rectangle copy. | framebuffer rectangle writes |
| destination | variable | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Points at the current destination row within authoritative storage. | framebuffer rectangle writes |
| source_row | variable | src/framebuffer.c | pstvnc_framebuffer_write_rect | local | Points at the current source row selected by source stride. | framebuffer rectangle writes |
| pstvnc_framebuffer_clear_dirty | function | src/framebuffer.c | desktop framebuffer | public | Clears dirty state without changing whole-frame validity or pixel contents. | framebuffer dirty state |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_clear_dirty | local | Supplies mutable framebuffer dirty-state ownership. | framebuffer dirty state |
| pstvnc_framebuffer_get_dirty | function | src/framebuffer.c | desktop framebuffer | public | Copies the current conservative dirty rectangle when one exists. | framebuffer dirty state |
| framebuffer | parameter | src/framebuffer.c | pstvnc_framebuffer_get_dirty | local | Supplies the framebuffer whose dirty state is inspected. | framebuffer dirty state |
| rect | parameter | src/framebuffer.c | pstvnc_framebuffer_get_dirty | local | Receives a copy of the current dirty bounding rectangle. | framebuffer dirty state |
| PSTVNC_FRAMEBUFFER_H | include guard | src/framebuffer.h | desktop framebuffer interface | file | Prevents repeated inclusion of framebuffer types and declarations. | clean source interface |
| pstvnc_framebuffer_rect | structure | src/framebuffer.h | desktop framebuffer interface | public | Defines an axis-aligned logical rectangle used for dirty-state reporting. | desktop framebuffer |
| x | field | src/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle left coordinate in logical pixels. | desktop framebuffer |
| y | field | src/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle top coordinate in logical pixels. | desktop framebuffer |
| width | field | src/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle width in logical pixels. | desktop framebuffer |
| height | field | src/framebuffer.h | pstvnc_framebuffer_rect | public | Stores the rectangle height in logical pixels. | desktop framebuffer |
| pstvnc_framebuffer_rect_t | type | src/framebuffer.h | desktop framebuffer interface | public | Names the dirty-rectangle value type. | desktop framebuffer |
| pstvnc_framebuffer | structure | src/framebuffer.h | desktop framebuffer interface | public | Defines storage, geometry, authority, and dirty state for the remote image. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| pixels | field | src/framebuffer.h | pstvnc_framebuffer | public | Points to caller-owned CPU pixel storage for the remote desktop. | desktop framebuffer |
| pixel_capacity | field | src/framebuffer.h | pstvnc_framebuffer | public | Records allocated pixel capacity independently of current geometry. | desktop framebuffer |
| width | field | src/framebuffer.h | pstvnc_framebuffer | public | Records current logical framebuffer width. | desktop framebuffer |
| height | field | src/framebuffer.h | pstvnc_framebuffer | public | Records current logical framebuffer height. | desktop framebuffer |
| valid | field | src/framebuffer.h | pstvnc_framebuffer | public | States whether every logical pixel is currently authoritative desktop data. | authoritative framebuffer semantics |
| dirty | field | src/framebuffer.h | pstvnc_framebuffer | public | States whether the latest accepted server update changed any pixel. | framebuffer dirty state |
| dirty_rect | field | src/framebuffer.h | pstvnc_framebuffer | public | Stores a conservative bounding rectangle for the latest pixel changes. | framebuffer dirty state |
| pstvnc_framebuffer_t | type | src/framebuffer.h | desktop framebuffer interface | public | Names the authoritative framebuffer metadata and storage view. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| pstvnc_framebuffer_init | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares framebuffer initialization around caller-owned storage. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_set_geometry | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares validated geometry changes that revoke old pixel authority. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_pixel_count | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares logical pixel-count computation. | desktop framebuffer lifecycle |
| pstvnc_framebuffer_invalidate | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares authority revocation and dirty-state clearing. | authoritative framebuffer semantics |
| pstvnc_framebuffer_mark_valid | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares publication of an externally proven complete framebuffer. | authoritative framebuffer semantics |
| pstvnc_framebuffer_write_rect | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares bounded rectangle copying into CPU framebuffer storage. | framebuffer rectangle writes |
| pstvnc_framebuffer_clear_dirty | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares reset of dirty presentation state. | framebuffer dirty state |
| pstvnc_framebuffer_get_dirty | function declaration | src/framebuffer.h | desktop framebuffer interface | public | Declares inspection of the current dirty bounding rectangle. | framebuffer dirty state |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_clear_dirty | prototype | Supplies mutable framebuffer dirty-state ownership. | framebuffer dirty state |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_get_dirty | prototype | Supplies the framebuffer whose dirty state is inspected. | framebuffer dirty state |
| rect | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_get_dirty | prototype | Receives a copy of the current dirty bounding rectangle. | framebuffer dirty state |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_init | prototype | Supplies the framebuffer value whose metadata is initialized. | desktop framebuffer lifecycle |
| pixel_capacity | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_init | prototype | Gives available pixel storage capacity independent of logical geometry. | desktop framebuffer lifecycle |
| pixels | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_init | prototype | Supplies caller-owned CPU pixel storage without asserting content authority. | desktop framebuffer lifecycle |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_invalidate | prototype | Supplies the framebuffer whose authority is revoked. | failure semantics |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_mark_valid | prototype | Supplies the initialized framebuffer to promote after session proof. | authoritative framebuffer semantics |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_pixel_count | prototype | Supplies geometry used to compute the logical pixel count. | desktop framebuffer lifecycle |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_set_geometry | prototype | Supplies the framebuffer whose logical geometry is changed. | desktop framebuffer lifecycle |
| height | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_set_geometry | prototype | Gives the requested nonzero logical height. | desktop framebuffer lifecycle |
| width | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_set_geometry | prototype | Gives the requested nonzero logical width. | desktop framebuffer lifecycle |
| framebuffer | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Supplies destination framebuffer storage and geometry. | framebuffer rectangle writes |
| height | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle height. | framebuffer rectangle writes |
| source | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Points to source pixels that will be copied into framebuffer storage. | framebuffer rectangle writes |
| source_stride | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives source row stride and must cover the requested width. | framebuffer rectangle writes |
| width | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle width. | framebuffer rectangle writes |
| x | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle left coordinate. | framebuffer rectangle writes |
| y | prototype parameter | src/framebuffer.h | pstvnc_framebuffer_write_rect | prototype | Gives destination rectangle top coordinate. | framebuffer rectangle writes |
| main | function | src/main.c | process entry | process | Delegates product lifecycle to the coordinator and always converges on OSDSYS. | CLEAN_ARCHITECTURE: Startup lifecycle |
| argc | parameter | src/main.c | main | process | Receives process argument count but is intentionally unused by the clean baseline. | process entry |
| argv | parameter | src/main.c | main | process | Receives process argument vector but is intentionally unused by the clean baseline. | process entry |
| local_overlay_pixels | variable | src/app.c | application coordinator | file | Stores local overlay pixels state owned by application coordinator. | Issue #39: keyboard, OSK and local UI foreground model |
| publish_semantic_keyboard_tap | function | src/app.c | application coordinator | file | Publishes semantic keyboard tap. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | publish_semantic_keyboard_tap | local | Supplies session to publish_semantic_keyboard_tap. | Issue #39: keyboard, OSK and local UI foreground model |
| keyboard_tap | parameter | src/app.c | publish_semantic_keyboard_tap | local | Supplies keyboard tap to publish_semantic_keyboard_tap. | Issue #39: keyboard, OSK and local UI foreground model |
| sequence | variable | src/app.c | publish_semantic_keyboard_tap | local | Stores sequence while publish_semantic_keyboard_tap runs. | Issue #39: keyboard, OSK and local UI foreground model |
| event_index | variable | src/app.c | publish_semantic_keyboard_tap | local | Stores event index while publish_semantic_keyboard_tap runs. | Issue #39: keyboard, OSK and local UI foreground model |
| present_current_application_frame | function | src/app.c | application coordinator | file | Presents current application frame. | Issue #39: keyboard, OSK and local UI foreground model |
| framebuffer | parameter | src/app.c | present_current_application_frame | local | Supplies framebuffer to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| remote_frame_changed | parameter | src/app.c | present_current_application_frame | local | Supplies remote frame changed to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | present_current_application_frame | local | Supplies local ui to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | present_current_application_frame | local | Supplies osk to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| local_presentation | variable | src/app.c | present_current_application_frame | local | Stores local presentation while present_current_application_frame runs. | Issue #39: keyboard, OSK and local UI foreground model |
| platform_overlay | variable | src/app.c | present_current_application_frame | local | Stores platform overlay while present_current_application_frame runs. | Issue #39: keyboard, OSK and local UI foreground model |
| platform_overlay_ptr | variable | src/app.c | present_current_application_frame | local | Stores platform overlay ptr while present_current_application_frame runs. | Issue #39: keyboard, OSK and local UI foreground model |
| neutralize_published_pointer_for_local_foreground | function | src/app.c | application coordinator | file | Neutralizes published pointer for local foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | neutralize_published_pointer_for_local_foreground | local | Supplies session to neutralize_published_pointer_for_local_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | neutralize_published_pointer_for_local_foreground | local | Supplies published pointer to neutralize_published_pointer_for_local_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| apply_osk_activation_result | function | src/app.c | application coordinator | file | Applies osk activation result. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | apply_osk_activation_result | local | Supplies session to apply_osk_activation_result. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | apply_osk_activation_result | local | Supplies local ui to apply_osk_activation_result. | Issue #39: keyboard, OSK and local UI foreground model |
| activation | parameter | src/app.c | apply_osk_activation_result | local | Supplies activation to apply_osk_activation_result. | Issue #39: keyboard, OSK and local UI foreground model |
| open_osk_foreground | function | src/app.c | application coordinator | file | Opens osk foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | open_osk_foreground | local | Supplies input runtime to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | open_osk_foreground | local | Supplies session to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | open_osk_foreground | local | Supplies published pointer to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | open_osk_foreground | local | Supplies local ui to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | open_osk_foreground | local | Supplies osk to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | open_osk_foreground | local | Supplies mouse interpretation suspended to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| apply_local_controller_action | function | src/app.c | application coordinator | file | Applies local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | apply_local_controller_action | local | Supplies input runtime to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | apply_local_controller_action | local | Supplies session to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | apply_local_controller_action | local | Supplies published pointer to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | apply_local_controller_action | local | Supplies local ui to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | apply_local_controller_action | local | Supplies osk to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | apply_local_controller_action | local | Supplies mouse interpretation suspended to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| action | parameter | src/app.c | apply_local_controller_action | local | Supplies action to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| activation | variable | src/app.c | apply_local_controller_action | local | Stores activation while apply_local_controller_action runs. | Issue #39: keyboard, OSK and local UI foreground model |
| service_controller_state | function | src/app.c | application coordinator | file | Services controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | service_controller_state | local | Supplies input runtime to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | service_controller_state | local | Supplies session to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | service_controller_state | local | Supplies published pointer to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller | parameter | src/app.c | service_controller_state | local | Supplies local controller to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | service_controller_state | local | Supplies local ui to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | service_controller_state | local | Supplies osk to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | service_controller_state | local | Supplies mouse interpretation suspended to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| controller_state | parameter | src/app.c | service_controller_state | local | Supplies controller state to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| result | variable | src/app.c | service_controller_state | local | Stores result while service_controller_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| action_index | variable | src/app.c | service_controller_state | local | Stores action index while service_controller_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| resume_desktop_mouse_if_ready | function | src/app.c | application coordinator | file | Resumes desktop mouse if ready. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | resume_desktop_mouse_if_ready | local | Supplies input runtime to resume_desktop_mouse_if_ready. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | resume_desktop_mouse_if_ready | local | Supplies local ui to resume_desktop_mouse_if_ready. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | resume_desktop_mouse_if_ready | local | Supplies mouse interpretation suspended to resume_desktop_mouse_if_ready. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller | parameter | src/app.c | service_semantic_input_events | local | Supplies local controller to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | service_semantic_input_events | local | Supplies local ui to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | service_semantic_input_events | local | Supplies osk to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | service_semantic_input_events | local | Supplies mouse interpretation suspended to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller | variable | src/app.c | pstvnc_app_run | local | Stores local controller while pstvnc_app_run runs. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | variable | src/app.c | pstvnc_app_run | local | Stores local ui while pstvnc_app_run runs. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | variable | src/app.c | pstvnc_app_run | local | Stores osk while pstvnc_app_run runs. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | variable | src/app.c | pstvnc_app_run | local | Stores mouse interpretation suspended while pstvnc_app_run runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_H | macro | src/controller.h | controller facts interface | public | Defines the controller h value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_SELECT | macro | src/controller.h | controller facts interface | public | Defines the controller button select value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_L3 | macro | src/controller.h | controller facts interface | public | Defines the controller button l3 value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_R3 | macro | src/controller.h | controller facts interface | public | Defines the controller button r3 value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_START | macro | src/controller.h | controller facts interface | public | Defines the controller button start value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_UP | macro | src/controller.h | controller facts interface | public | Defines the controller button up value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_RIGHT | macro | src/controller.h | controller facts interface | public | Defines the controller button right value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_DOWN | macro | src/controller.h | controller facts interface | public | Defines the controller button down value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_LEFT | macro | src/controller.h | controller facts interface | public | Defines the controller button left value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_L2 | macro | src/controller.h | controller facts interface | public | Defines the controller button l2 value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_R2 | macro | src/controller.h | controller facts interface | public | Defines the controller button r2 value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_L1 | macro | src/controller.h | controller facts interface | public | Defines the controller button l1 value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_R1 | macro | src/controller.h | controller facts interface | public | Defines the controller button r1 value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_TRIANGLE | macro | src/controller.h | controller facts interface | public | Defines the controller button triangle value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_CIRCLE | macro | src/controller.h | controller facts interface | public | Defines the controller button circle value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_CROSS | macro | src/controller.h | controller facts interface | public | Defines the controller button cross value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_SQUARE | macro | src/controller.h | controller facts interface | public | Defines the controller button square value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_CONTROLLER_BUTTON_MASK | macro | src/controller.h | controller facts interface | public | Defines the controller button mask value used by controller facts interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_controller_state | structure | src/controller.h | controller facts interface | public | Defines the state carried by controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| buttons_down | field | src/controller.h | pstvnc_controller_state | public | Stores buttons down within controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| buttons_pressed | field | src/controller.h | pstvnc_controller_state | public | Stores buttons pressed within controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| buttons_released | field | src/controller.h | pstvnc_controller_state | public | Stores buttons released within controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| connection_epoch_started | field | src/controller.h | pstvnc_controller_state | public | Stores connection epoch started within controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_controller_state_t | type | src/controller.h | controller facts interface | public | Names the controller state t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_INPUT_EVENT_CONTROLLER_STATE | enum value | src/input.h | pstvnc_input_event_type | public | Represents input event controller state within input event type. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_INPUT_EVENT_KEYBOARD_TAP | enum value | src/input.h | pstvnc_input_event_type | public | Represents input event keyboard tap within input event type. | Issue #39: keyboard, OSK and local UI foreground model |
| controller_state | field | src/input.h | pstvnc_input_event_payload | public | Stores controller state within input event payload. | Issue #39: keyboard, OSK and local UI foreground model |
| keyboard_tap | field | src/input.h | pstvnc_input_event_payload | public | Stores keyboard tap within input event payload. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime_map_native_buttons | function | src/input_runtime.c | controller/input runtime | file | Implements input runtime map native buttons. | Issue #39: keyboard, OSK and local UI foreground model |
| native_buttons | parameter | src/input_runtime.c | input_runtime_map_native_buttons | local | Supplies native buttons to input_runtime_map_native_buttons. | Issue #39: keyboard, OSK and local UI foreground model |
| project_buttons | variable | src/input_runtime.c | input_runtime_map_native_buttons | local | Stores project buttons while input_runtime_map_native_buttons runs. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime_publish_controller_state | function | src/input_runtime.c | controller/input runtime | file | Implements input runtime publish controller state. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | parameter | src/input_runtime.c | input_runtime_publish_controller_state | local | Supplies runtime to input_runtime_publish_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| controller_state | parameter | src/input_runtime.c | input_runtime_publish_controller_state | local | Supplies controller state to input_runtime_publish_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| event | variable | src/input_runtime.c | input_runtime_publish_controller_state | local | Stores event while input_runtime_publish_controller_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pushed | variable | src/input_runtime.c | input_runtime_publish_controller_state | local | Stores pushed while input_runtime_publish_controller_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| controller_state | variable | src/input_runtime.c | input_runtime_process_pad_sample | local | Stores controller state while input_runtime_process_pad_sample runs. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime_enter_mouse_interpretation_suspension | function | src/input_runtime.c | controller/input runtime | file | Implements input runtime enter mouse interpretation suspension. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | parameter | src/input_runtime.c | input_runtime_enter_mouse_interpretation_suspension | local | Supplies runtime to input_runtime_enter_mouse_interpretation_suspension. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_input_runtime_suspend_mouse_interpretation | function | src/input_runtime.c | controller/input runtime | file | Implements input runtime suspend mouse interpretation. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_suspend_mouse_interpretation | local | Supplies runtime to pstvnc_input_runtime_suspend_mouse_interpretation. | Issue #39: keyboard, OSK and local UI foreground model |
| wait_step | variable | src/input_runtime.c | pstvnc_input_runtime_suspend_mouse_interpretation | local | Stores wait step while pstvnc_input_runtime_suspend_mouse_interpretation runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_input_runtime_rebase_suspended_mouse_state | function | src/input_runtime.c | controller/input runtime | file | Implements input runtime rebase suspended mouse state. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_suspended_mouse_state | local | Supplies runtime to pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_cursor_x | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_suspended_mouse_state | local | Supplies published cursor x to pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_cursor_y | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_suspended_mouse_state | local | Supplies published cursor y to pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_click_buttons | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_suspended_mouse_state | local | Supplies published click buttons to pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| wheel_mode_enabled | variable | src/input_runtime.c | pstvnc_input_runtime_rebase_suspended_mouse_state | local | Stores wheel mode enabled while pstvnc_input_runtime_rebase_suspended_mouse_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_input_runtime_resume_mouse_interpretation | function | src/input_runtime.c | controller/input runtime | file | Implements input runtime resume mouse interpretation. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_resume_mouse_interpretation | local | Supplies runtime to pstvnc_input_runtime_resume_mouse_interpretation. | Issue #39: keyboard, OSK and local UI foreground model |
| wait_step | variable | src/input_runtime.c | pstvnc_input_runtime_resume_mouse_interpretation | local | Stores wait step while pstvnc_input_runtime_resume_mouse_interpretation runs. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspend_requested | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores mouse interpretation suspend requested within input runtime. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores mouse interpretation suspended within input runtime. | Issue #39: keyboard, OSK and local UI foreground model |
| suspended_mouse_state_rebased | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores suspended mouse state rebased within input runtime. | Issue #39: keyboard, OSK and local UI foreground model |
| suspended_physical_continuity_lost | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores suspended physical continuity lost within input runtime. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_input_runtime_suspend_mouse_interpretation | function declaration | src/input_runtime.h | input runtime interface | public | Declares input runtime suspend mouse interpretation through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_suspend_mouse_interpretation | prototype | Declares the runtime argument accepted by pstvnc_input_runtime_suspend_mouse_interpretation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_input_runtime_rebase_suspended_mouse_state | function declaration | src/input_runtime.h | input runtime interface | public | Declares input runtime rebase suspended mouse state through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_suspended_mouse_state | prototype | Declares the runtime argument accepted by pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_cursor_x | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_suspended_mouse_state | prototype | Declares the published cursor x argument accepted by pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_cursor_y | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_suspended_mouse_state | prototype | Declares the published cursor y argument accepted by pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_click_buttons | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_suspended_mouse_state | prototype | Declares the published click buttons argument accepted by pstvnc_input_runtime_rebase_suspended_mouse_state. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_input_runtime_resume_mouse_interpretation | function declaration | src/input_runtime.h | input runtime interface | public | Declares input runtime resume mouse interpretation through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_resume_mouse_interpretation | prototype | Declares the runtime argument accepted by pstvnc_input_runtime_resume_mouse_interpretation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_build_tap_sequence | function | src/keyboard.c | semantic keyboard | file | Implements keyboard build tap sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/keyboard.c | pstvnc_keyboard_build_tap_sequence | local | Supplies keysym to pstvnc_keyboard_build_tap_sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | parameter | src/keyboard.c | pstvnc_keyboard_build_tap_sequence | local | Supplies modifiers to pstvnc_keyboard_build_tap_sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| sequence | parameter | src/keyboard.c | pstvnc_keyboard_build_tap_sequence | local | Supplies sequence to pstvnc_keyboard_build_tap_sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| event_index | variable | src/keyboard.c | pstvnc_keyboard_build_tap_sequence | local | Stores event index while pstvnc_keyboard_build_tap_sequence runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_H | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard h value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_MODIFIER_SHIFT | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard modifier shift value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_MODIFIER_CTRL | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard modifier ctrl value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_MODIFIER_ALT | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard modifier alt value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_MODIFIER_MASK | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard modifier mask value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_KEYSYM_BACKSPACE | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard keysym backspace value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_KEYSYM_TAB | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard keysym tab value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_KEYSYM_ENTER | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard keysym enter value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_KEYSYM_SHIFT_L | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard keysym shift l value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_KEYSYM_CONTROL_L | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard keysym control l value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_KEYSYM_ALT_L | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard keysym alt l value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_KEYBOARD_SEQUENCE_MAX_EVENTS | macro | src/keyboard.h | semantic keyboard interface | public | Defines the keyboard sequence max events value used by semantic keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_tap | structure | src/keyboard.h | semantic keyboard interface | public | Defines the state carried by keyboard tap. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | field | src/keyboard.h | pstvnc_keyboard_tap | public | Stores keysym within keyboard tap. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | field | src/keyboard.h | pstvnc_keyboard_tap | public | Stores modifiers within keyboard tap. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_tap_t | type | src/keyboard.h | semantic keyboard interface | public | Names the keyboard tap t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_key_event | structure | src/keyboard.h | semantic keyboard interface | public | Defines the state carried by keyboard key event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | field | src/keyboard.h | pstvnc_keyboard_key_event | public | Stores down within keyboard key event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | field | src/keyboard.h | pstvnc_keyboard_key_event | public | Stores keysym within keyboard key event. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_key_event_t | type | src/keyboard.h | semantic keyboard interface | public | Names the keyboard key event t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_sequence | structure | src/keyboard.h | semantic keyboard interface | public | Defines the state carried by keyboard sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| events | field | src/keyboard.h | pstvnc_keyboard_sequence | public | Stores events within keyboard sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| event_count | field | src/keyboard.h | pstvnc_keyboard_sequence | public | Stores event count within keyboard sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_sequence_t | type | src/keyboard.h | semantic keyboard interface | public | Names the keyboard sequence t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_keyboard_build_tap_sequence | function declaration | src/keyboard.h | semantic keyboard interface | public | Declares keyboard build tap sequence through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/keyboard.h | pstvnc_keyboard_build_tap_sequence | prototype | Declares the keysym argument accepted by pstvnc_keyboard_build_tap_sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | prototype parameter | src/keyboard.h | pstvnc_keyboard_build_tap_sequence | prototype | Declares the modifiers argument accepted by pstvnc_keyboard_build_tap_sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| sequence | prototype parameter | src/keyboard.h | pstvnc_keyboard_build_tap_sequence | prototype | Declares the sequence argument accepted by pstvnc_keyboard_build_tap_sequence. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_OWNED_BUTTONS | macro | src/local_controller.c | local controller router | public | Defines the local controller owned buttons value used by local controller router. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller_clear_result | function | src/local_controller.c | local controller router | file | Implements local controller clear result. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/local_controller.c | local_controller_clear_result | local | Supplies result to local_controller_clear_result. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller_state_is_valid | function | src/local_controller.c | local controller router | file | Implements local controller state is valid. | Issue #39: keyboard, OSK and local UI foreground model |
| state | parameter | src/local_controller.c | local_controller_state_is_valid | local | Supplies state to local_controller_state_is_valid. | Issue #39: keyboard, OSK and local UI foreground model |
| not_down | variable | src/local_controller.c | local_controller_state_is_valid | local | Stores not down while local_controller_state_is_valid runs. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller_append_action | function | src/local_controller.c | local controller router | file | Implements local controller append action. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/local_controller.c | local_controller_append_action | local | Supplies result to local_controller_append_action. | Issue #39: keyboard, OSK and local UI foreground model |
| action | parameter | src/local_controller.c | local_controller_append_action | local | Supplies action to local_controller_append_action. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_init | function | src/local_controller.c | local controller router | file | Implements local controller init. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | parameter | src/local_controller.c | pstvnc_local_controller_init | local | Supplies controller to pstvnc_local_controller_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_route | function | src/local_controller.c | local controller router | file | Implements local controller route. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | parameter | src/local_controller.c | pstvnc_local_controller_route | local | Supplies controller to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| foreground | parameter | src/local_controller.c | pstvnc_local_controller_route | local | Supplies foreground to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| input_quarantined | parameter | src/local_controller.c | pstvnc_local_controller_route | local | Supplies input quarantined to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| state | parameter | src/local_controller.c | pstvnc_local_controller_route | local | Supplies state to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/local_controller.c | pstvnc_local_controller_route | local | Supplies result to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| local_pressed | variable | src/local_controller.c | pstvnc_local_controller_route | local | Stores local pressed while pstvnc_local_controller_route runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_release_is_proven | function | src/local_controller.c | local controller router | file | Implements local controller release is proven. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | parameter | src/local_controller.c | pstvnc_local_controller_release_is_proven | local | Supplies controller to pstvnc_local_controller_release_is_proven. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_H | macro | src/local_controller.h | local controller interface | public | Defines the local controller h value used by local controller interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_MAX_ACTIONS | macro | src/local_controller.h | local controller interface | public | Defines the local controller max actions value used by local controller interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_action | enum | src/local_controller.h | local controller interface | public | Defines the local controller action semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action open osk within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move left within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move right within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move up within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move down within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action toggle shift within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action activate selected within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action backspace within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action enter within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_TAB | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action tab within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK | enum value | src/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action close osk within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_action_t | type | src/local_controller.h | local controller interface | public | Names the local controller action t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_result | structure | src/local_controller.h | local controller interface | public | Defines the state carried by local controller result. | Issue #39: keyboard, OSK and local UI foreground model |
| actions | field | src/local_controller.h | pstvnc_local_controller_result | public | Stores actions within local controller result. | Issue #39: keyboard, OSK and local UI foreground model |
| action_count | field | src/local_controller.h | pstvnc_local_controller_result | public | Stores action count within local controller result. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_result_t | type | src/local_controller.h | local controller interface | public | Names the local controller result t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller | structure | src/local_controller.h | local controller interface | public | Defines the state carried by local controller. | Issue #39: keyboard, OSK and local UI foreground model |
| owned_buttons_awaiting_release | field | src/local_controller.h | pstvnc_local_controller | public | Stores owned buttons awaiting release within local controller. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_t | type | src/local_controller.h | local controller interface | public | Names the local controller t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_init | function declaration | src/local_controller.h | local controller interface | public | Declares local controller init through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | prototype parameter | src/local_controller.h | pstvnc_local_controller_init | prototype | Declares the controller argument accepted by pstvnc_local_controller_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_route | function declaration | src/local_controller.h | local controller interface | public | Declares local controller route through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | prototype parameter | src/local_controller.h | pstvnc_local_controller_route | prototype | Declares the controller argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| foreground | prototype parameter | src/local_controller.h | pstvnc_local_controller_route | prototype | Declares the foreground argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| input_quarantined | prototype parameter | src/local_controller.h | pstvnc_local_controller_route | prototype | Declares the input quarantined argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| state | prototype parameter | src/local_controller.h | pstvnc_local_controller_route | prototype | Declares the state argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| result | prototype parameter | src/local_controller.h | pstvnc_local_controller_route | prototype | Declares the result argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_release_is_proven | function declaration | src/local_controller.h | local controller interface | public | Declares local controller release is proven through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | prototype parameter | src/local_controller.h | pstvnc_local_controller_release_is_proven | prototype | Declares the controller argument accepted by pstvnc_local_controller_release_is_proven. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui_advance_generation | function | src/local_ui.c | local UI foreground model | file | Implements local ui advance generation. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | local_ui_advance_generation | local | Supplies ui to local_ui_advance_generation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_init | function | src/local_ui.c | local UI foreground model | file | Implements local ui init. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_init | local | Supplies ui to pstvnc_local_ui_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_open_osk | function | src/local_ui.c | local UI foreground model | file | Implements local ui open osk. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_open_osk | local | Supplies ui to pstvnc_local_ui_open_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_close_osk | function | src/local_ui.c | local UI foreground model | file | Implements local ui close osk. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_close_osk | local | Supplies ui to pstvnc_local_ui_close_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| destination | variable | src/local_ui.c | pstvnc_local_ui_close_osk | local | Stores destination while pstvnc_local_ui_close_osk runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_input_is_quarantined | function | src/local_ui.c | local UI foreground model | file | Implements local ui input is quarantined. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_input_is_quarantined | local | Supplies ui to pstvnc_local_ui_input_is_quarantined. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_complete_input_quarantine | function | src/local_ui.c | local UI foreground model | file | Implements local ui complete input quarantine. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_complete_input_quarantine | local | Supplies ui to pstvnc_local_ui_complete_input_quarantine. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_local_change | function | src/local_ui.c | local UI foreground model | file | Implements local ui mark local change. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_mark_local_change | local | Supplies ui to pstvnc_local_ui_mark_local_change. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_generation | function | src/local_ui.c | local UI foreground model | file | Implements local ui generation. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_generation | local | Supplies ui to pstvnc_local_ui_generation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_needs_present | function | src/local_ui.c | local UI foreground model | file | Implements local ui needs present. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_needs_present | local | Supplies ui to pstvnc_local_ui_needs_present. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_presented | function | src/local_ui.c | local UI foreground model | file | Implements local ui mark presented. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui.c | pstvnc_local_ui_mark_presented | local | Supplies ui to pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | parameter | src/local_ui.c | pstvnc_local_ui_mark_presented | local | Supplies generation to pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_H | macro | src/local_ui.h | local UI interface | public | Defines the local ui h value used by local UI interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_foreground | enum | src/local_ui.h | local UI interface | public | Defines the local ui foreground semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP | enum value | src/local_ui.h | pstvnc_local_ui_foreground | public | Represents local ui foreground desktop within local ui foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_FOREGROUND_OSK | enum value | src/local_ui.h | pstvnc_local_ui_foreground | public | Represents local ui foreground osk within local ui foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_foreground_t | type | src/local_ui.h | local UI interface | public | Names the local ui foreground t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui | structure | src/local_ui.h | local UI interface | public | Defines the state carried by local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| foreground | field | src/local_ui.h | pstvnc_local_ui | public | Stores foreground within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| return_foreground | field | src/local_ui.h | pstvnc_local_ui | public | Stores return foreground within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| input_quarantined | field | src/local_ui.h | pstvnc_local_ui | public | Stores input quarantined within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | field | src/local_ui.h | pstvnc_local_ui | public | Stores generation within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| presented_generation | field | src/local_ui.h | pstvnc_local_ui | public | Stores presented generation within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| dirty | field | src/local_ui.h | pstvnc_local_ui | public | Stores dirty within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_t | type | src/local_ui.h | local UI interface | public | Names the local ui t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_init | function declaration | src/local_ui.h | local UI interface | public | Declares local ui init through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_init | prototype | Declares the ui argument accepted by pstvnc_local_ui_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_open_osk | function declaration | src/local_ui.h | local UI interface | public | Declares local ui open osk through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_open_osk | prototype | Declares the ui argument accepted by pstvnc_local_ui_open_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_close_osk | function declaration | src/local_ui.h | local UI interface | public | Declares local ui close osk through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_close_osk | prototype | Declares the ui argument accepted by pstvnc_local_ui_close_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_input_is_quarantined | function declaration | src/local_ui.h | local UI interface | public | Declares local ui input is quarantined through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_input_is_quarantined | prototype | Declares the ui argument accepted by pstvnc_local_ui_input_is_quarantined. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_complete_input_quarantine | function declaration | src/local_ui.h | local UI interface | public | Declares local ui complete input quarantine through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_complete_input_quarantine | prototype | Declares the ui argument accepted by pstvnc_local_ui_complete_input_quarantine. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_local_change | function declaration | src/local_ui.h | local UI interface | public | Declares local ui mark local change through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_mark_local_change | prototype | Declares the ui argument accepted by pstvnc_local_ui_mark_local_change. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_generation | function declaration | src/local_ui.h | local UI interface | public | Declares local ui generation through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_generation | prototype | Declares the ui argument accepted by pstvnc_local_ui_generation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_needs_present | function declaration | src/local_ui.h | local UI interface | public | Declares local ui needs present through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_needs_present | prototype | Declares the ui argument accepted by pstvnc_local_ui_needs_present. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_presented | function declaration | src/local_ui.h | local UI interface | public | Declares local ui mark presented through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui.h | pstvnc_local_ui_mark_presented | prototype | Declares the ui argument accepted by pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | prototype parameter | src/local_ui.h | pstvnc_local_ui_mark_presented | prototype | Declares the generation argument accepted by pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_BOTTOM_INSET | macro | src/local_ui_presentation.c | local UI presentation | public | Defines the local ui bottom inset value used by local UI presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| clear_presentation | function | src/local_ui_presentation.c | local UI presentation | file | Clears presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| presentation | parameter | src/local_ui_presentation.c | clear_presentation | local | Supplies presentation to clear_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_prepare_presentation | function | src/local_ui_presentation.c | local UI presentation | file | Implements local ui prepare presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies ui to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies osk to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_pixels | parameter | src/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies overlay surface pixels to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_capacity_pixels | parameter | src/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies overlay surface capacity pixels to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| presentation | parameter | src/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies presentation to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| horizontal_space | variable | src/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Stores horizontal space while pstvnc_local_ui_prepare_presentation runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_PRESENTATION_H | macro | src/local_ui_presentation.h | local UI presentation interface | public | Defines the local ui presentation h value used by local UI presentation interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_presentation | structure | src/local_ui_presentation.h | local UI presentation interface | public | Defines the state carried by local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_visible | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay visible within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_pixels | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay pixels within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_pixel_count | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay pixel count within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_width | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay width within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_height | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay height within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_x | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay x within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_y | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay y within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | field | src/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores generation within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_presentation_t | type | src/local_ui_presentation.h | local UI presentation interface | public | Names the local ui presentation t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_prepare_presentation | function declaration | src/local_ui_presentation.h | local UI presentation interface | public | Declares local ui prepare presentation through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the ui argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the osk argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_pixels | prototype parameter | src/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the overlay surface pixels argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_capacity_pixels | prototype parameter | src/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the overlay surface capacity pixels argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| presentation | prototype parameter | src/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the presentation argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_mouse_reset_transient_history | function | src/mouse.c | remote mouse policy | file | Implements mouse reset transient history. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse | parameter | src/mouse.c | pstvnc_mouse_reset_transient_history | local | Supplies mouse to pstvnc_mouse_reset_transient_history. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_mouse_reset_transient_history | function declaration | src/mouse.h | remote mouse interface | public | Declares mouse reset transient history through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_reset_transient_history | prototype | Declares the mouse argument accepted by pstvnc_mouse_reset_transient_history. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_abc_rows | variable | src/osk.c | on-screen keyboard model | file | Stores osk abc rows state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_page_row_lengths | variable | src/osk.c | on-screen keyboard model | file | Stores osk page row lengths state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_func_labels | variable | src/osk.c | on-screen keyboard model | file | Stores osk func labels state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_func_keysyms | variable | src/osk.c | on-screen keyboard model | file | Stores osk func keysyms state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_special_labels | variable | src/osk.c | on-screen keyboard model | file | Stores osk special labels state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_shifted_ascii | function | src/osk.c | on-screen keyboard model | file | Implements osk shifted ascii. | Issue #39: keyboard, OSK and local UI foreground model |
| c | parameter | src/osk.c | osk_shifted_ascii | local | Supplies c to osk_shifted_ascii. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_func_keysym | function | src/osk.c | on-screen keyboard model | file | Implements osk func keysym. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/osk.c | osk_func_keysym | local | Supplies row to osk_func_keysym. | Issue #39: keyboard, OSK and local UI foreground model |
| col | parameter | src/osk.c | osk_func_keysym | local | Supplies col to osk_func_keysym. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_real_modifier_mask | function | src/osk.c | on-screen keyboard model | file | Implements osk real modifier mask. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | osk_real_modifier_mask | local | Supplies osk to osk_real_modifier_mask. | Issue #39: keyboard, OSK and local UI foreground model |
| include_shift | parameter | src/osk.c | osk_real_modifier_mask | local | Supplies include shift to osk_real_modifier_mask. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | variable | src/osk.c | osk_real_modifier_mask | local | Stores modifiers while osk_real_modifier_mask runs. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_activation_clear | function | src/osk.c | on-screen keyboard model | file | Implements osk activation clear. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/osk.c | osk_activation_clear | local | Supplies result to osk_activation_clear. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_consume_modifiers_after_key | function | src/osk.c | on-screen keyboard model | file | Implements osk consume modifiers after key. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | osk_consume_modifiers_after_key | local | Supplies osk to osk_consume_modifiers_after_key. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/osk.c | osk_consume_modifiers_after_key | local | Supplies result to osk_consume_modifiers_after_key. | Issue #39: keyboard, OSK and local UI foreground model |
| had_modifiers | variable | src/osk.c | osk_consume_modifiers_after_key | local | Stores had modifiers while osk_consume_modifiers_after_key runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_reset_for_open | function | src/osk.c | on-screen keyboard model | file | Implements osk reset for open. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_reset_for_open | local | Supplies osk to pstvnc_osk_reset_for_open. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_clear_modifiers | function | src/osk.c | on-screen keyboard model | file | Implements osk clear modifiers. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_clear_modifiers | local | Supplies osk to pstvnc_osk_clear_modifiers. | Issue #39: keyboard, OSK and local UI foreground model |
| changed | variable | src/osk.c | pstvnc_osk_clear_modifiers | local | Stores changed while pstvnc_osk_clear_modifiers runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_toggle_shift_modifier | function | src/osk.c | on-screen keyboard model | file | Implements osk toggle shift modifier. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_toggle_shift_modifier | local | Supplies osk to pstvnc_osk_toggle_shift_modifier. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_direct_key | function | src/osk.c | on-screen keyboard model | file | Implements osk activate direct key. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_activate_direct_key | local | Supplies osk to pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/osk.c | pstvnc_osk_activate_direct_key | local | Supplies keysym to pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/osk.c | pstvnc_osk_activate_direct_key | local | Supplies result to pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_row_length | function | src/osk.c | on-screen keyboard model | file | Implements osk row length. | Issue #39: keyboard, OSK and local UI foreground model |
| page | parameter | src/osk.c | pstvnc_osk_row_length | local | Supplies page to pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/osk.c | pstvnc_osk_row_length | local | Supplies row to pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_horizontal | function | src/osk.c | on-screen keyboard model | file | Implements osk move horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_move_horizontal | local | Supplies osk to pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | parameter | src/osk.c | pstvnc_osk_move_horizontal | local | Supplies direction to pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| count | variable | src/osk.c | pstvnc_osk_move_horizontal | local | Stores count while pstvnc_osk_move_horizontal runs. | Issue #39: keyboard, OSK and local UI foreground model |
| old_col | variable | src/osk.c | pstvnc_osk_move_horizontal | local | Stores old col while pstvnc_osk_move_horizontal runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_vertical | function | src/osk.c | on-screen keyboard model | file | Implements osk move vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_move_vertical | local | Supplies osk to pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | parameter | src/osk.c | pstvnc_osk_move_vertical | local | Supplies direction to pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| old_count | variable | src/osk.c | pstvnc_osk_move_vertical | local | Stores old count while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| old_col | variable | src/osk.c | pstvnc_osk_move_vertical | local | Stores old col while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| new_row | variable | src/osk.c | pstvnc_osk_move_vertical | local | Stores new row while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| new_count | variable | src/osk.c | pstvnc_osk_move_vertical | local | Stores new count while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| new_col | variable | src/osk.c | pstvnc_osk_move_vertical | local | Stores new col while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_display_char | function | src/osk.c | on-screen keyboard model | file | Implements osk display char. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_display_char | local | Supplies osk to pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/osk.c | pstvnc_osk_display_char | local | Supplies row to pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| col | parameter | src/osk.c | pstvnc_osk_display_char | local | Supplies col to pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| c | variable | src/osk.c | pstvnc_osk_display_char | local | Stores c while pstvnc_osk_display_char runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_key_label | function | src/osk.c | on-screen keyboard model | file | Implements osk key label. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_key_label | local | Supplies osk to pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/osk.c | pstvnc_osk_key_label | local | Supplies row to pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| col | parameter | src/osk.c | pstvnc_osk_key_label | local | Supplies col to pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| count | variable | src/osk.c | pstvnc_osk_key_label | local | Stores count while pstvnc_osk_key_label runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_selected | function | src/osk.c | on-screen keyboard model | file | Implements osk activate selected. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk.c | pstvnc_osk_activate_selected | local | Supplies osk to pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/osk.c | pstvnc_osk_activate_selected | local | Supplies result to pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | variable | src/osk.c | pstvnc_osk_activate_selected | local | Stores keysym while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | variable | src/osk.c | pstvnc_osk_activate_selected | local | Stores modifiers while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| count | variable | src/osk.c | pstvnc_osk_activate_selected | local | Stores count while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| display_char | variable | src/osk.c | pstvnc_osk_activate_selected | local | Stores display char while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_H | macro | src/osk.h | on-screen keyboard interface | public | Defines the osk h value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_WIDTH | macro | src/osk.h | on-screen keyboard interface | public | Defines the osk width value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_HEIGHT | macro | src/osk.h | on-screen keyboard interface | public | Defines the osk height value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_ROWS | macro | src/osk.h | on-screen keyboard interface | public | Defines the osk rows value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ROW | macro | src/osk.h | on-screen keyboard interface | public | Defines the osk utility row value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_KEY_COUNT | macro | src/osk.h | on-screen keyboard interface | public | Defines the osk utility key count value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_page | enum | src/osk.h | on-screen keyboard interface | public | Defines the osk page semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_PAGE_ABC | enum value | src/osk.h | pstvnc_osk_page | public | Represents osk page abc within osk page. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_PAGE_FUNC | enum value | src/osk.h | pstvnc_osk_page | public | Represents osk page func within osk page. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_PAGE_COUNT | enum value | src/osk.h | pstvnc_osk_page | public | Represents osk page count within osk page. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_page_t | type | src/osk.h | on-screen keyboard interface | public | Names the osk page t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_utility_key | enum | src/osk.h | on-screen keyboard interface | public | Defines the osk utility key semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ABC | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility abc within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_FUNC | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility func within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_SHIFT | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility shift within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_CTRL | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility ctrl within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ALT | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility alt within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_SPACE | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility space within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_TAB | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility tab within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_BACKSPACE | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility backspace within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_DELETE | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility delete within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ENTER | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility enter within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ESCAPE | enum value | src/osk.h | pstvnc_osk_utility_key | public | Represents osk utility escape within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_utility_key_t | type | src/osk.h | on-screen keyboard interface | public | Names the osk utility key t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk | structure | src/osk.h | on-screen keyboard interface | public | Defines the state carried by osk. | Issue #39: keyboard, OSK and local UI foreground model |
| page | field | src/osk.h | pstvnc_osk | public | Stores page within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| row | field | src/osk.h | pstvnc_osk | public | Stores row within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| col | field | src/osk.h | pstvnc_osk | public | Stores col within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| shift | field | src/osk.h | pstvnc_osk | public | Stores shift within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| ctrl | field | src/osk.h | pstvnc_osk | public | Stores ctrl within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| alt | field | src/osk.h | pstvnc_osk | public | Stores alt within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_t | type | src/osk.h | on-screen keyboard interface | public | Names the osk t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activation | structure | src/osk.h | on-screen keyboard interface | public | Defines the state carried by osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| local_state_changed | field | src/osk.h | pstvnc_osk_activation | public | Stores local state changed within osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| produced_keyboard_tap | field | src/osk.h | pstvnc_osk_activation | public | Stores produced keyboard tap within osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| keyboard_tap | field | src/osk.h | pstvnc_osk_activation | public | Stores keyboard tap within osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activation_t | type | src/osk.h | on-screen keyboard interface | public | Names the osk activation t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_reset_for_open | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk reset for open through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_reset_for_open | prototype | Declares the osk argument accepted by pstvnc_osk_reset_for_open. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_clear_modifiers | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk clear modifiers through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_clear_modifiers | prototype | Declares the osk argument accepted by pstvnc_osk_clear_modifiers. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_toggle_shift_modifier | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk toggle shift modifier through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_toggle_shift_modifier | prototype | Declares the osk argument accepted by pstvnc_osk_toggle_shift_modifier. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_direct_key | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk activate direct key through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_activate_direct_key | prototype | Declares the osk argument accepted by pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/osk.h | pstvnc_osk_activate_direct_key | prototype | Declares the keysym argument accepted by pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| result | prototype parameter | src/osk.h | pstvnc_osk_activate_direct_key | prototype | Declares the result argument accepted by pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_row_length | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk row length through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| page | prototype parameter | src/osk.h | pstvnc_osk_row_length | prototype | Declares the page argument accepted by pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| row | prototype parameter | src/osk.h | pstvnc_osk_row_length | prototype | Declares the row argument accepted by pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_horizontal | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk move horizontal through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_move_horizontal | prototype | Declares the osk argument accepted by pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | prototype parameter | src/osk.h | pstvnc_osk_move_horizontal | prototype | Declares the direction argument accepted by pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_vertical | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk move vertical through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_move_vertical | prototype | Declares the osk argument accepted by pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | prototype parameter | src/osk.h | pstvnc_osk_move_vertical | prototype | Declares the direction argument accepted by pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_display_char | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk display char through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_display_char | prototype | Declares the osk argument accepted by pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| row | prototype parameter | src/osk.h | pstvnc_osk_display_char | prototype | Declares the row argument accepted by pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| col | prototype parameter | src/osk.h | pstvnc_osk_display_char | prototype | Declares the col argument accepted by pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_key_label | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk key label through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_key_label | prototype | Declares the osk argument accepted by pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| row | prototype parameter | src/osk.h | pstvnc_osk_key_label | prototype | Declares the row argument accepted by pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| col | prototype parameter | src/osk.h | pstvnc_osk_key_label | prototype | Declares the col argument accepted by pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_selected | function declaration | src/osk.h | on-screen keyboard interface | public | Declares osk activate selected through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk.h | pstvnc_osk_activate_selected | prototype | Declares the osk argument accepted by pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| result | prototype parameter | src/osk.h | pstvnc_osk_activate_selected | prototype | Declares the result argument accepted by pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_font_glyph | structure | src/osk_render.c | on-screen keyboard renderer | public | Defines the state carried by osk font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| character | field | src/osk_render.c | pstvnc_osk_font_glyph | public | Stores character within osk font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| rows | field | src/osk_render.c | pstvnc_osk_font_glyph | public | Stores rows within osk font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_font_glyph_t | type | src/osk_render.c | on-screen keyboard renderer | public | Names the osk font glyph t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_font_glyphs | variable | src/osk_render.c | on-screen keyboard renderer | file | Stores osk font glyphs state owned by on-screen keyboard renderer. | Issue #39: keyboard, OSK and local UI foreground model |
| pack_opaque_surface_color | function | src/osk_render.c | on-screen keyboard renderer | file | Packs opaque surface color. | Issue #39: keyboard, OSK and local UI foreground model |
| red | parameter | src/osk_render.c | pack_opaque_surface_color | local | Supplies red to pack_opaque_surface_color. | Issue #39: keyboard, OSK and local UI foreground model |
| green | parameter | src/osk_render.c | pack_opaque_surface_color | local | Supplies green to pack_opaque_surface_color. | Issue #39: keyboard, OSK and local UI foreground model |
| blue | parameter | src/osk_render.c | pack_opaque_surface_color | local | Supplies blue to pack_opaque_surface_color. | Issue #39: keyboard, OSK and local UI foreground model |
| fill_surface_rectangle | function | src/osk_render.c | on-screen keyboard renderer | file | Fills surface rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/osk_render.c | fill_surface_rectangle | local | Supplies surface pixels to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| x | parameter | src/osk_render.c | fill_surface_rectangle | local | Supplies x to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| y | parameter | src/osk_render.c | fill_surface_rectangle | local | Supplies y to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| width | parameter | src/osk_render.c | fill_surface_rectangle | local | Supplies width to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| height | parameter | src/osk_render.c | fill_surface_rectangle | local | Supplies height to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| color | parameter | src/osk_render.c | fill_surface_rectangle | local | Supplies color to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| row_index | variable | src/osk_render.c | fill_surface_rectangle | local | Stores row index while fill_surface_rectangle runs. | Issue #39: keyboard, OSK and local UI foreground model |
| column_index | variable | src/osk_render.c | fill_surface_rectangle | local | Stores column index while fill_surface_rectangle runs. | Issue #39: keyboard, OSK and local UI foreground model |
| destination | variable | src/osk_render.c | fill_surface_rectangle | local | Stores destination while fill_surface_rectangle runs. | Issue #39: keyboard, OSK and local UI foreground model |
| find_font_glyph | function | src/osk_render.c | on-screen keyboard renderer | file | Finds font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| character | parameter | src/osk_render.c | find_font_glyph | local | Supplies character to find_font_glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph_index | variable | src/osk_render.c | find_font_glyph | local | Stores glyph index while find_font_glyph runs. | Issue #39: keyboard, OSK and local UI foreground model |
| draw_surface_character | function | src/osk_render.c | on-screen keyboard renderer | file | Draws surface character. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/osk_render.c | draw_surface_character | local | Supplies surface pixels to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| x | parameter | src/osk_render.c | draw_surface_character | local | Supplies x to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| y | parameter | src/osk_render.c | draw_surface_character | local | Supplies y to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| character | parameter | src/osk_render.c | draw_surface_character | local | Supplies character to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| scale | parameter | src/osk_render.c | draw_surface_character | local | Supplies scale to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| color | parameter | src/osk_render.c | draw_surface_character | local | Supplies color to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph | variable | src/osk_render.c | draw_surface_character | local | Stores glyph while draw_surface_character runs. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph_row | variable | src/osk_render.c | draw_surface_character | local | Stores glyph row while draw_surface_character runs. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph_column | variable | src/osk_render.c | draw_surface_character | local | Stores glyph column while draw_surface_character runs. | Issue #39: keyboard, OSK and local UI foreground model |
| draw_centered_surface_text | function | src/osk_render.c | on-screen keyboard renderer | file | Draws centered surface text. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies surface pixels to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| x | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies x to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| y | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies y to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| width | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies width to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| height | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies height to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| text | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies text to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| scale | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies scale to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| color | parameter | src/osk_render.c | draw_centered_surface_text | local | Supplies color to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| character_count | variable | src/osk_render.c | draw_centered_surface_text | local | Stores character count while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| character_width | variable | src/osk_render.c | draw_centered_surface_text | local | Stores character width while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_width | variable | src/osk_render.c | draw_centered_surface_text | local | Stores text width while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_height | variable | src/osk_render.c | draw_centered_surface_text | local | Stores text height while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_x | variable | src/osk_render.c | draw_centered_surface_text | local | Stores text x while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_y | variable | src/osk_render.c | draw_centered_surface_text | local | Stores text y while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| character_index | variable | src/osk_render.c | draw_centered_surface_text | local | Stores character index while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| utility_key_is_active | function | src/osk_render.c | on-screen keyboard renderer | file | Implements utility key is active. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk_render.c | utility_key_is_active | local | Supplies osk to utility_key_is_active. | Issue #39: keyboard, OSK and local UI foreground model |
| column | parameter | src/osk_render.c | utility_key_is_active | local | Supplies column to utility_key_is_active. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_render_surface | function | src/osk_render.c | on-screen keyboard renderer | file | Implements osk render surface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/osk_render.c | pstvnc_osk_render_surface | local | Supplies osk to pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/osk_render.c | pstvnc_osk_render_surface | local | Supplies surface pixels to pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_capacity_pixels | parameter | src/osk_render.c | pstvnc_osk_render_surface | local | Supplies surface capacity pixels to pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| panel_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores panel color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| ordinary_key_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores ordinary key color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected_key_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores selected key color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| active_key_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores active key color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| border_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores border color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| ordinary_text_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores ordinary text color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected_text_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores selected text color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| row | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores row while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected_row_length | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores selected row length while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_count | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores key count while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| margin | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores margin while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| gap | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores gap while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| available_width | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores available width while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_width | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores key width while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| used_width | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores used width while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| row_x | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores row x while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| row_y | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores row y while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_height | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores key height while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| column | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores column while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_x | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores key x while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores selected while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| background_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores background color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_color | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores text color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| label | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores label while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| character_label | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores character label while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_scale | variable | src/osk_render.c | pstvnc_osk_render_surface | local | Stores text scale while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_RENDER_H | macro | src/osk_render.h | on-screen keyboard renderer interface | public | Defines the osk render h value used by on-screen keyboard renderer interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_SURFACE_PIXEL_COUNT | macro | src/osk_render.h | on-screen keyboard renderer interface | public | Defines the osk surface pixel count value used by on-screen keyboard renderer interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_render_surface | function declaration | src/osk_render.h | on-screen keyboard renderer interface | public | Declares osk render surface through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/osk_render.h | pstvnc_osk_render_surface | prototype | Declares the osk argument accepted by pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | prototype parameter | src/osk_render.h | pstvnc_osk_render_surface | prototype | Declares the surface pixels argument accepted by pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_capacity_pixels | prototype parameter | src/osk_render.h | pstvnc_osk_render_surface | prototype | Declares the surface capacity pixels argument accepted by pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| bytes | parameter | src/rfb.c | write_be32 | local | Supplies bytes to write_be32. | Issue #39: keyboard, OSK and local UI foreground model |
| value | parameter | src/rfb.c | write_be32 | local | Supplies value to write_be32. | Issue #39: keyboard, OSK and local UI foreground model |
| write_be32 | function | src/rfb.c | RFB wire encoding | file | Writes be32. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_build_key_event | function | src/rfb.c | RFB wire encoding | file | Implements rfb build key event. | Issue #39: keyboard, OSK and local UI foreground model |
| out | parameter | src/rfb.c | pstvnc_rfb_build_key_event | local | Supplies out to pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | parameter | src/rfb.c | pstvnc_rfb_build_key_event | local | Supplies down to pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/rfb.c | pstvnc_rfb_build_key_event | local | Supplies keysym to pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_RFB_KEY_EVENT_SIZE | macro | src/rfb.h | RFB wire interface | public | Defines the rfb key event size value used by RFB wire interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_build_key_event | function declaration | src/rfb.h | RFB wire interface | public | Declares rfb build key event through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_build_key_event | prototype | Declares the out argument accepted by pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | prototype parameter | src/rfb.h | pstvnc_rfb_build_key_event | prototype | Declares the down argument accepted by pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/rfb.h | pstvnc_rfb_build_key_event | prototype | Declares the keysym argument accepted by pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_session_send_key_event | function | src/rfb_session.c | RFB session | file | Implements rfb session send key event. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Supplies session to pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | parameter | src/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Supplies down to pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Supplies keysym to pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| message | variable | src/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Stores message while pstvnc_rfb_session_send_key_event runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_session_send_key_event | function declaration | src/rfb_session.h | RFB session interface | public | Declares rfb session send key event through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_key_event | prototype | Declares the session argument accepted by pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_key_event | prototype | Declares the down argument accepted by pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_key_event | prototype | Declares the keysym argument accepted by pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |

| input_event_type_is_valid | function | src/input.c | semantic input event queue | file | Validates whether an event discriminator names a semantic input payload family currently implemented by the clean input boundary. | ISSUE38 semantic event envelope |
| event_type | parameter | src/input.c | input_event_type_is_valid | local | Supplies the semantic event discriminator to validate without inspecting payload contents. | semantic event envelope validation |
| pstvnc_input_queue_init | function | src/input.c | semantic input event queue | public | Initializes one ordinary semantic input FIFO to the empty state. | ISSUE38 ordinary event queue |
| queue | parameter | src/input.c | pstvnc_input_queue_init | local | Supplies the FIFO object whose indices and accepted-event count are initialized. | semantic input queue lifecycle |
| pstvnc_input_queue_discard_all | function | src/input.c | semantic input event queue | public | Establishes a hard empty-queue boundary so previously accepted ordinary input cannot emerge afterward. | ISSUE38 ownership/context boundary |
| queue | parameter | src/input.c | pstvnc_input_queue_discard_all | local | Supplies the FIFO whose accepted ordinary semantic events are discarded. | semantic input queue boundary |
| pstvnc_input_queue_event_count | function | src/input.c | semantic input event queue | public | Reports how many accepted semantic events are currently waiting for consumption. | ISSUE38 ordinary event queue |
| queue | parameter | src/input.c | pstvnc_input_queue_event_count | local | Supplies the FIFO whose current accepted-event count is inspected. | semantic input queue observation |
| pstvnc_input_queue_push | function | src/input.c | semantic input event queue | public | Atomically appends one valid complete semantic event when bounded FIFO capacity remains. | ISSUE38 producer boundary |
| queue | parameter | src/input.c | pstvnc_input_queue_push | local | Supplies the FIFO that receives the complete semantic event. | semantic input producer boundary |
| event | parameter | src/input.c | pstvnc_input_queue_push | local | Supplies the caller-owned typed semantic event to copy into FIFO storage. | semantic input producer boundary |
| pstvnc_input_queue_pop | function | src/input.c | semantic input event queue | public | Removes and copies the oldest accepted semantic event while preserving FIFO ordering. | ISSUE38 consumer boundary |
| queue | parameter | src/input.c | pstvnc_input_queue_pop | local | Supplies the FIFO from which the oldest accepted event is consumed. | semantic input consumer boundary |
| event | parameter | src/input.c | pstvnc_input_queue_pop | local | Receives a complete copy of the oldest accepted semantic event. | semantic input consumer boundary |
| PSTVNC_INPUT_H | macro | src/input.h | semantic input interface | file | Prevents repeated inclusion of the semantic input event and FIFO declarations. | clean source interface |
| PSTVNC_INPUT_EVENT_QUEUE_CAPACITY | macro | src/input.h | semantic input event queue | public | Defines the bounded 256-entry capacity retained from the historically qualified controller producer/consumer depth. | ISSUE38 ordinary event queue |
| pstvnc_input_event_type | enum | src/input.h | semantic input interface | public | Defines the discriminator vocabulary for semantic input payload families that have actually been reconstructed. | ISSUE38 semantic event envelope |
| PSTVNC_INPUT_EVENT_NONE | enum value | src/input.h | pstvnc_input_event_type | public | Marks an event envelope as invalid or unfinished so zero initialization cannot create executable semantic work. | semantic event validation |
| PSTVNC_INPUT_EVENT_MOUSE_UPDATE | enum value | src/input.h | pstvnc_input_event_type | public | Identifies an ordinary semantic event whose payload is one remote-mouse update. | ISSUE38 first earned payload family |
| pstvnc_input_event_type_t | type | src/input.h | semantic input interface | public | Names the semantic event discriminator type used by the application-routable input envelope. | ISSUE38 semantic event envelope |
| pstvnc_input_event_payload | structure | src/input.h | semantic input interface | public | Defines typed payload storage for semantic input events without generic byte buffers, casts, or callback machinery. | ISSUE38 growth-capable event seam |
| mouse_update | field | src/input.h | pstvnc_input_event_payload | public | Stores the first earned semantic payload family: one complete remote-mouse update. | ISSUE38 mouse semantic payload |
| pstvnc_input_event_payload_t | type | src/input.h | semantic input interface | public | Names the typed semantic-event payload storage used by the event envelope. | ISSUE38 semantic event envelope |
| pstvnc_input_event | structure | src/input.h | semantic input interface | public | Defines one complete application-routable semantic input event as a discriminator plus typed payload. | ISSUE38 semantic event envelope |
| type | field | src/input.h | pstvnc_input_event | public | Identifies which implemented semantic payload family is present in the event envelope. | semantic event routing |
| payload | field | src/input.h | pstvnc_input_event | public | Carries the complete typed semantic value associated with the event discriminator. | semantic event routing |
| pstvnc_input_event_t | type | src/input.h | semantic input interface | public | Names one complete semantic input event transferred from input production toward application routing. | ISSUE38 semantic event path |
| pstvnc_input_queue | structure | src/input.h | semantic input event queue | public | Defines the pure bounded FIFO state used to transfer ordinary semantic input events in accepted order. | ISSUE38 ordinary event queue |
| event_storage | field | src/input.h | pstvnc_input_queue | public | Stores the fixed-capacity array of accepted semantic event envelopes. | semantic input FIFO storage |
| read_index | field | src/input.h | pstvnc_input_queue | public | Identifies the storage slot containing the oldest accepted event available for consumption. | semantic input FIFO ordering |
| write_index | field | src/input.h | pstvnc_input_queue | public | Identifies the next free storage slot used when a new semantic event is accepted. | semantic input FIFO ordering |
| event_count | field | src/input.h | pstvnc_input_queue | public | Records the number of accepted semantic events currently live in FIFO storage. | semantic input FIFO authority |
| pstvnc_input_queue_t | type | src/input.h | semantic input event queue | public | Names the pure bounded ordinary semantic-event FIFO state object. | ISSUE38 ordinary event queue |
| pstvnc_input_queue_init | function declaration | src/input.h | semantic input interface | public | Declares initialization of an empty ordinary semantic-event FIFO. | ISSUE38 ordinary event queue |
| queue | prototype parameter | src/input.h | pstvnc_input_queue_init | public | Declares the FIFO object initialized by the semantic input queue interface. | semantic input queue lifecycle |
| pstvnc_input_queue_discard_all | function declaration | src/input.h | semantic input interface | public | Declares explicit discard of every currently queued ordinary semantic event. | ISSUE38 hard input boundary |
| queue | prototype parameter | src/input.h | pstvnc_input_queue_discard_all | public | Declares the FIFO whose pre-boundary ordinary events are discarded. | semantic input queue boundary |
| pstvnc_input_queue_event_count | function declaration | src/input.h | semantic input interface | public | Declares inspection of the number of accepted semantic events awaiting consumption. | ISSUE38 ordinary event queue |
| queue | prototype parameter | src/input.h | pstvnc_input_queue_event_count | public | Declares the FIFO whose accepted-event count is inspected. | semantic input queue observation |
| pstvnc_input_queue_push | function declaration | src/input.h | semantic input interface | public | Declares atomic publication of one complete typed semantic event into the bounded FIFO. | ISSUE38 producer boundary |
| queue | prototype parameter | src/input.h | pstvnc_input_queue_push | public | Declares the FIFO receiving one complete semantic event. | semantic input producer boundary |
| event | prototype parameter | src/input.h | pstvnc_input_queue_push | public | Declares the caller-owned complete semantic event offered for publication. | semantic input producer boundary |
| pstvnc_input_queue_pop | function declaration | src/input.h | semantic input interface | public | Declares FIFO consumption of the oldest accepted semantic input event. | ISSUE38 consumer boundary |
| queue | prototype parameter | src/input.h | pstvnc_input_queue_pop | public | Declares the FIFO from which the oldest accepted event is consumed. | semantic input consumer boundary |
| event | prototype parameter | src/input.h | pstvnc_input_queue_pop | public | Declares destination storage receiving the oldest complete semantic event. | semantic input consumer boundary |
| input_runtime_build_mouse_input | function | src/input_runtime.c | controller/input runtime | file | Maps the current physical pad observation into one already-routed Issue #38 mouse-domain input sample. | Issue #38: controller/input runtime ownership |
| input_runtime_controller_thread | function | src/input_runtime.c | controller/input runtime | file | Runs the dedicated approximately-60-Hz controller producer, handoff state machine, physical-loss handling, and semantic publication loop. | Issue #38: controller/input runtime ownership |
| input_runtime_enter_pad_handoff | function | src/input_runtime.c | controller/input runtime | file | Establishes the worker side of a hard libpad ownership boundary by discarding unpublished queued input and invalidating derived/physical history before acknowledgement. | Issue #38: controller/input runtime ownership |
| input_runtime_handle_physical_loss | function | src/input_runtime.c | controller/input runtime | file | Handles the end of one active physical sample epoch by clearing derived mouse history and emitting at most the required ordered neutral update. | Issue #38: controller/input runtime ownership |
| input_runtime_left_stick_is_available | function | src/input_runtime.c | controller/input runtime | file | Proves that the current variable-length libpad sample reaches both left-stick coordinate fields before mouse interpretation may use them. | Issue #38: controller/input runtime ownership |
| input_runtime_mouse_directions_from_pad | function | src/input_runtime.c | controller/input runtime | file | Maps native active-high PAD_* D-pad facts into the separate mouse-domain directional vocabulary. | Issue #38: controller/input runtime ownership |
| input_runtime_mouse_update_is_meaningful | function | src/input_runtime.c | controller/input runtime | file | Reports whether one mouse result contains pointer/button change or a wheel notch that should enter the ordinary semantic event stream. | Issue #38: controller/input runtime ownership |
| input_runtime_process_pad_sample | function | src/input_runtime.c | controller/input runtime | file | Composes one accepted pad observation through mouse interpretation and ordinary semantic-event publication. | Issue #38: controller/input runtime ownership |
| input_runtime_publish_mouse_update | function | src/input_runtime.c | controller/input runtime | file | Wraps one meaningful mouse result in a typed semantic event and publishes it atomically through the semaphore-protected FIFO. | Issue #38: controller/input runtime ownership |
| input_runtime_record_worker_error | function | src/input_runtime.c | controller/input runtime | file | Records the first controller-worker failure so later cleanup events cannot erase the original reason semantic production stopped. | Issue #38: controller/input runtime ownership |
| input_runtime_stop_controller_thread | function | src/input_runtime.c | controller/input runtime | file | Requests cooperative worker shutdown, proves THS_DORMANT within the bounded lifecycle wait, and only then deletes the EE thread. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_discard_events | function | src/input_runtime.c | controller/input runtime | public | Discards all currently queued ordinary semantic input under the runtime's queue semaphore without changing other owners' state. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_event_count | function | src/input_runtime.c | controller/input runtime | public | Reads the synchronized number of ordinary semantic events waiting for application/main consumption. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_init | function | src/input_runtime.c | controller/input runtime | public | Initializes one complete controller/input runtime owner including mouse state, semantic FIFO, libpad endpoint, and queue semaphore. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_last_error | function | src/input_runtime.c | controller/input runtime | public | Reports the first worker-side runtime failure retained by the input owner. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_pad_handoff_acknowledged | function | src/input_runtime.c | controller/input runtime | public | Reports whether the controller worker has reached the current safe pre-libpad-access handoff boundary. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_pop_event | function | src/input_runtime.c | controller/input runtime | public | Consumes one oldest ordinary semantic input event under the runtime's queue semaphore for application/main routing. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_rebase_published_mouse_state | function | src/input_runtime.c | controller/input runtime | public | While handoff is acknowledged, replaces local interpreted pointer state with application/main's successfully published neutral remote pointer state. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_release_pad_handoff | function | src/input_runtime.c | controller/input runtime | public | Returns libpad ownership only after published-state rebase, withdraws the request, and proves acknowledgement withdrawal before success. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_request_pad_handoff | function | src/input_runtime.c | controller/input runtime | public | Starts one unambiguous libpad ownership-handoff epoch and rejects requests that overlap an unfinished prior epoch or worker failure. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_shutdown | function | src/input_runtime.c | controller/input runtime | public | Cooperatively stops the producer and releases its pad, semaphore, libpad, and runtime lifecycle ownership in safe order. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_start | function | src/input_runtime.c | controller/input runtime | public | Creates and starts the dedicated approximately-60-Hz EE controller producer for an initialized runtime. | Issue #38: controller/input runtime ownership |
| INPUT_RUNTIME_LIFECYCLE_WAIT_STEPS | macro | src/input_runtime.c | controller/input runtime | file | Defines the maximum lifecycle observation count before thread-stop or handoff-release proof fails closed. | Issue #38: controller/input runtime ownership |
| INPUT_RUNTIME_LIFECYCLE_WAIT_STEP_US | macro | src/input_runtime.c | controller/input runtime | file | Defines one short observation interval used by bounded application-side worker lifecycle waits. | Issue #38: controller/input runtime ownership |
| INPUT_RUNTIME_POLL_DELAY_US | macro | src/input_runtime.c | controller/input runtime | file | Defines the approximately-60-Hz worker delay retained for qualified mouse response timing. | Issue #38: controller/input runtime ownership |
| INPUT_RUNTIME_THREAD_PRIORITY | macro | src/input_runtime.c | controller/input runtime | file | Defines the EE controller-worker priority retained from the qualified historical scheduling point. | Issue #38: controller/input runtime ownership |
| mouse_input | parameter | src/input_runtime.c | input_runtime_build_mouse_input | local | Receives the already-routed mouse-domain sample built from the current physical controller observation. | Issue #38: controller/input runtime ownership |
| pad | parameter | src/input_runtime.c | input_runtime_build_mouse_input | local | Supplies the current physical pad observation used by input_runtime_build_mouse_input. | Issue #38: controller/input runtime ownership |
| argument | parameter | src/input_runtime.c | input_runtime_controller_thread | local | Carries the pstvnc_input_runtime_t pointer supplied as the EE controller-thread entry argument. | Issue #38: controller/input runtime ownership |
| pad_poll_result | variable | src/input_runtime.c | input_runtime_controller_thread | local | Stores whether the current bounded pad poll produced a sample, no sample, or a failure. | Issue #38: controller/input runtime ownership |
| runtime | variable | src/input_runtime.c | input_runtime_controller_thread | local | Points to the complete runtime owner supplied to the dedicated EE controller worker. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | input_runtime_enter_pad_handoff | local | Supplies the controller/input runtime instance operated on by input_runtime_enter_pad_handoff. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | input_runtime_handle_physical_loss | local | Supplies the controller/input runtime instance operated on by input_runtime_handle_physical_loss. | Issue #38: controller/input runtime ownership |
| mouse_update | variable | src/input_runtime.c | input_runtime_handle_physical_loss | local | Holds the semantic neutralization result produced after physical sample continuity is revoked. | Issue #38: controller/input runtime ownership |
| neutral_input | variable | src/input_runtime.c | input_runtime_handle_physical_loss | local | Holds the zeroed mouse-domain sample used to neutralize locally interpreted buttons after physical continuity loss. | Issue #38: controller/input runtime ownership |
| pad | parameter | src/input_runtime.c | input_runtime_left_stick_is_available | local | Supplies the current physical pad observation used by input_runtime_left_stick_is_available. | Issue #38: controller/input runtime ownership |
| required_bytes | variable | src/input_runtime.c | input_runtime_left_stick_is_available | local | Stores the minimum current sample length required to prove both left-stick coordinates are present. | Issue #38: controller/input runtime ownership |
| buttons_down | parameter | src/input_runtime.c | input_runtime_mouse_directions_from_pad | local | Supplies the current active-high native PAD_* physical button mask. | Issue #38: controller/input runtime ownership |
| directions | variable | src/input_runtime.c | input_runtime_mouse_directions_from_pad | local | Accumulates mouse-domain direction bits selected from the current native D-pad mask. | Issue #38: controller/input runtime ownership |
| mouse_update | parameter | src/input_runtime.c | input_runtime_mouse_update_is_meaningful | local | Supplies the semantic mouse result examined or published by input_runtime_mouse_update_is_meaningful. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | input_runtime_process_pad_sample | local | Supplies the controller/input runtime instance operated on by input_runtime_process_pad_sample. | Issue #38: controller/input runtime ownership |
| mouse_input | variable | src/input_runtime.c | input_runtime_process_pad_sample | local | Holds one already-routed mouse-domain input sample derived from the current accepted pad observation. | Issue #38: controller/input runtime ownership |
| mouse_update | variable | src/input_runtime.c | input_runtime_process_pad_sample | local | Holds the semantic mouse result produced from the current routed sample. | Issue #38: controller/input runtime ownership |
| mouse_update | parameter | src/input_runtime.c | input_runtime_publish_mouse_update | local | Supplies the semantic mouse result examined or published by input_runtime_publish_mouse_update. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | input_runtime_publish_mouse_update | local | Supplies the controller/input runtime instance operated on by input_runtime_publish_mouse_update. | Issue #38: controller/input runtime ownership |
| event | variable | src/input_runtime.c | input_runtime_publish_mouse_update | local | Holds the typed semantic event envelope constructed for one mouse result. | Issue #38: controller/input runtime ownership |
| pushed | variable | src/input_runtime.c | input_runtime_publish_mouse_update | local | Records whether the complete semantic mouse event was accepted by the bounded FIFO. | Issue #38: controller/input runtime ownership |
| error | parameter | src/input_runtime.c | input_runtime_record_worker_error | local | Supplies the worker-side failure retained if no earlier worker failure has been recorded. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | input_runtime_record_worker_error | local | Supplies the controller/input runtime instance operated on by input_runtime_record_worker_error. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | input_runtime_stop_controller_thread | local | Supplies the controller/input runtime instance operated on by input_runtime_stop_controller_thread. | Issue #38: controller/input runtime ownership |
| status | variable | src/input_runtime.c | input_runtime_stop_controller_thread | local | Stores the current EE thread status inspected while proving cooperative dormancy. | Issue #38: controller/input runtime ownership |
| wait_step | variable | src/input_runtime.c | input_runtime_stop_controller_thread | local | Counts bounded lifecycle observations while waiting for the worker to become THS_DORMANT. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_discard_events | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_discard_events. | Issue #38: controller/input runtime ownership |
| event_count | parameter | src/input_runtime.c | pstvnc_input_runtime_event_count | local | Receives the synchronized number of ordinary semantic events currently waiting in the FIFO. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_event_count | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_event_count. | Issue #38: controller/input runtime ownership |
| height | parameter | src/input_runtime.c | pstvnc_input_runtime_init | local | Supplies the logical remote desktop height used to initialize mouse interpretation. | Issue #38: controller/input runtime ownership |
| port | parameter | src/input_runtime.c | pstvnc_input_runtime_init | local | Selects the native libpad controller port owned by this runtime. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_init | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_init. | Issue #38: controller/input runtime ownership |
| slot | parameter | src/input_runtime.c | pstvnc_input_runtime_init | local | Selects the native libpad slot owned by this runtime. | Issue #38: controller/input runtime ownership |
| width | parameter | src/input_runtime.c | pstvnc_input_runtime_init | local | Supplies the logical remote desktop width used to initialize mouse interpretation. | Issue #38: controller/input runtime ownership |
| semaphore | variable | src/input_runtime.c | pstvnc_input_runtime_init | local | Holds EE semaphore creation parameters for the ordinary semantic FIFO lock. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_last_error | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_last_error. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_pad_handoff_acknowledged | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_pad_handoff_acknowledged. | Issue #38: controller/input runtime ownership |
| event | parameter | src/input_runtime.c | pstvnc_input_runtime_pop_event | local | Supplies or receives one complete ordinary semantic input event across the application/runtime queue boundary. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_pop_event | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_pop_event. | Issue #38: controller/input runtime ownership |
| popped | variable | src/input_runtime.c | pstvnc_input_runtime_pop_event | local | Records whether the pure FIFO returned one complete ordinary semantic event. | Issue #38: controller/input runtime ownership |
| published_click_buttons | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_published_mouse_state | local | Supplies application/main's successfully published remote ordinary mouse-button mask. | Issue #38: controller/input runtime ownership |
| published_cursor_x | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_published_mouse_state | local | Supplies application/main's last successfully published remote cursor X coordinate. | Issue #38: controller/input runtime ownership |
| published_cursor_y | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_published_mouse_state | local | Supplies application/main's last successfully published remote cursor Y coordinate. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_rebase_published_mouse_state | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_rebase_published_mouse_state. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_release_pad_handoff | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_release_pad_handoff. | Issue #38: controller/input runtime ownership |
| wait_step | variable | src/input_runtime.c | pstvnc_input_runtime_release_pad_handoff | local | Counts bounded lifecycle observations while waiting for the worker-owned handoff acknowledgement to withdraw. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_request_pad_handoff | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_request_pad_handoff. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_shutdown | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_shutdown. | Issue #38: controller/input runtime ownership |
| result | variable | src/input_runtime.c | pstvnc_input_runtime_shutdown | local | Accumulates shutdown failure if an owned resource cannot be released cleanly. | Issue #38: controller/input runtime ownership |
| runtime | parameter | src/input_runtime.c | pstvnc_input_runtime_start | local | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_start. | Issue #38: controller/input runtime ownership |
| thread | variable | src/input_runtime.c | pstvnc_input_runtime_start | local | Holds EE thread creation parameters for the dedicated controller producer. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_error | enum | src/input_runtime.h | input runtime interface | public | Defines worker-side failures that make continued semantic input production unsafe. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_discard_events | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_discard_events(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_event_count | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_event_count(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_init | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_init(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_last_error | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_last_error(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_pad_handoff_acknowledged | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_pad_handoff_acknowledged(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_pop_event | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_pop_event(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_rebase_published_mouse_state | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_rebase_published_mouse_state(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_release_pad_handoff | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_release_pad_handoff(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_request_pad_handoff | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_request_pad_handoff(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_shutdown | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_shutdown(). | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_start | function declaration | src/input_runtime.h | input runtime interface | public | Declares the public controller/input runtime operation corresponding to pstvnc_input_runtime_start(). | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_H | macro | src/input_runtime.h | input runtime interface | file | Prevents repeated inclusion of the controller/input runtime interface. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_THREAD_STACK_SIZE | macro | src/input_runtime.h | input runtime interface | public | Defines the 8192-byte controller-worker stack size retained from the qualified historical input path. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime | structure | src/input_runtime.h | input runtime interface | public | Defines the complete owner state for one synchronized PS2 controller/input producer. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_error_t | type | src/input_runtime.h | input runtime interface | public | Names the controller/input worker failure enumeration. | Issue #38: controller/input runtime ownership |
| pstvnc_input_runtime_t | type | src/input_runtime.h | input runtime interface | public | Names one complete controller/input runtime owner instance. | Issue #38: controller/input runtime ownership |
| controller_thread_id | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores the EE kernel ID of the dedicated controller producer thread. | Issue #38: controller/input runtime ownership |
| controller_thread_stack | field | src/input_runtime.h | pstvnc_input_runtime | public | Provides the runtime-owned aligned 8192-byte stack for the dedicated EE controller producer. | Issue #38: controller/input runtime ownership |
| controller_thread_started | field | src/input_runtime.h | pstvnc_input_runtime | public | Records whether the dedicated controller producer was successfully started and still has thread lifecycle ownership. | Issue #38: controller/input runtime ownership |
| event_queue | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores ordinary typed semantic events awaiting application/main consumption. | Issue #38: controller/input runtime ownership |
| event_queue_sema_id | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores the EE semaphore protecting cross-thread access to the ordinary event FIFO. | Issue #38: controller/input runtime ownership |
| handoff_published_state_rebased | field | src/input_runtime.h | pstvnc_input_runtime | public | Records that application/main reconciled the paused mouse interpreter to successfully published neutral remote pointer state. | Issue #38: controller/input runtime ownership |
| initialized | field | src/input_runtime.h | pstvnc_input_runtime | public | Records whether runtime-owned pad, mouse, queue, and synchronization resources are initialized. | Issue #38: controller/input runtime ownership |
| mouse | field | src/input_runtime.h | pstvnc_input_runtime | public | Owns the local pure mouse interpreter advanced by accepted controller observations. | Issue #38: controller/input runtime ownership |
| pad | field | src/input_runtime.h | pstvnc_input_runtime | public | Owns the single direct-libpad physical controller endpoint used by this runtime. | Issue #38: controller/input runtime ownership |
| pad_handoff_acknowledged | field | src/input_runtime.h | pstvnc_input_runtime | public | Carries worker-owned proof that it stopped immediately before further libpad access for the current handoff epoch. | Issue #38: controller/input runtime ownership |
| pad_handoff_requested | field | src/input_runtime.h | pstvnc_input_runtime | public | Carries the application/main request for temporary exclusive libpad ownership. | Issue #38: controller/input runtime ownership |
| physical_sample_active | field | src/input_runtime.h | pstvnc_input_runtime | public | Records whether the worker currently owns an active continuous physical sample epoch whose loss may require semantic neutralization. | Issue #38: controller/input runtime ownership |
| stop_requested | field | src/input_runtime.h | pstvnc_input_runtime | public | Carries the application/main cooperative stop request observed by the controller worker. | Issue #38: controller/input runtime ownership |
| worker_error | field | src/input_runtime.h | pstvnc_input_runtime | public | Stores the first worker-side failure that made continued semantic input production unsafe. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_discard_events | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_discard_events. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_MOUSE_UPDATE | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Records failure while advancing the pure mouse interpreter. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_NONE | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Represents an input worker with no recorded failure. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_PAD_POLL | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Records failure of the owned physical pad polling boundary. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_FULL | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Records bounded ordinary semantic-event FIFO exhaustion. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_SIGNAL | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Records failure to release the ordinary semantic-event queue semaphore. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_QUEUE_WAIT | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Records failure to acquire the ordinary semantic-event queue semaphore. | Issue #38: controller/input runtime ownership |
| PSTVNC_INPUT_RUNTIME_ERROR_THREAD_DELAY | enum value | src/input_runtime.h | pstvnc_input_runtime_error | public | Records failure of the controller worker's scheduled DelayThread operation. | Issue #38: controller/input runtime ownership |
| event_count | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_event_count | prototype | Receives the synchronized number of ordinary semantic events currently waiting in the FIFO. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_event_count | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_event_count. | Issue #38: controller/input runtime ownership |
| height | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_init | prototype | Supplies the logical remote desktop height used to initialize mouse interpretation. | Issue #38: controller/input runtime ownership |
| port | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_init | prototype | Selects the native libpad controller port owned by this runtime. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_init | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_init. | Issue #38: controller/input runtime ownership |
| slot | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_init | prototype | Selects the native libpad slot owned by this runtime. | Issue #38: controller/input runtime ownership |
| width | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_init | prototype | Supplies the logical remote desktop width used to initialize mouse interpretation. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_last_error | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_last_error. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_pad_handoff_acknowledged | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_pad_handoff_acknowledged. | Issue #38: controller/input runtime ownership |
| event | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_pop_event | prototype | Supplies or receives one complete ordinary semantic input event across the application/runtime queue boundary. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_pop_event | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_pop_event. | Issue #38: controller/input runtime ownership |
| published_click_buttons | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_published_mouse_state | prototype | Supplies application/main's successfully published remote ordinary mouse-button mask. | Issue #38: controller/input runtime ownership |
| published_cursor_x | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_published_mouse_state | prototype | Supplies application/main's last successfully published remote cursor X coordinate. | Issue #38: controller/input runtime ownership |
| published_cursor_y | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_published_mouse_state | prototype | Supplies application/main's last successfully published remote cursor Y coordinate. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_rebase_published_mouse_state | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_rebase_published_mouse_state. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_release_pad_handoff | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_release_pad_handoff. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_request_pad_handoff | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_request_pad_handoff. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_shutdown | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_shutdown. | Issue #38: controller/input runtime ownership |
| runtime | prototype parameter | src/input_runtime.h | pstvnc_input_runtime_start | prototype | Supplies the controller/input runtime instance operated on by pstvnc_input_runtime_start. | Issue #38: controller/input runtime ownership |
| MOUSE_ANALOG_DEADZONE | macro | src/mouse.c | mouse response policy | file | Defines the centered raw-stick deadzone below which analog pointer or wheel motion is suppressed. | Test11K qualified analog response |
| MOUSE_ANALOG_HALF_RAW | macro | src/mouse.c | mouse response policy | file | Marks the low-range analog response point through which the qualified curve remains deliberately shallow. | Test11K qualified analog response |
| MOUSE_ANALOG_KNEE_RAW | macro | src/mouse.c | mouse response policy | file | Marks the analog response knee where the shallow low/mid curve rejoins the final acceleration region. | Test11K qualified analog response |
| MOUSE_ANALOG_MAX_RAW | macro | src/mouse.c | mouse response policy | file | Names the maximum signed raw stick magnitude used to cap analog response calculations. | Test11K qualified analog response |
| MOUSE_ANALOG_CURVE_ONE | macro | src/mouse.c | mouse response policy | file | Defines unity in the Q12 response-curve scale used by pointer and wheel calculations. | Test11K qualified analog response |
| MOUSE_ANALOG_CURVE_KNEE | macro | src/mouse.c | mouse response policy | file | Defines the qualified Q12 response value reached at the analog acceleration knee. | Test11K qualified analog response |
| MOUSE_DPAD_ACCEL_START_TICKS | macro | src/mouse.c | mouse response policy | file | Defines the approximately one-second 60 Hz hold point where D-pad precision motion begins continuous acceleration. | Test11K qualified D-pad response |
| MOUSE_DPAD_ACCEL_RAMP_TICKS | macro | src/mouse.c | mouse response policy | file | Defines the approximately 1.5-second 60 Hz ramp from D-pad precision speed to maximum pointer speed. | Test11K qualified D-pad response |
| MOUSE_DPAD_BASE_VELOCITY_Q8 | macro | src/mouse.c | mouse response policy | file | Defines the fractional Q8 base velocity used when accelerated D-pad motion begins. | Test11K qualified D-pad response |
| MOUSE_DPAD_MAX_VELOCITY_Q8 | macro | src/mouse.c | mouse response policy | file | Defines the six-pixel-per-poll Q8 ceiling shared by accelerated D-pad pointer motion. | Test11K qualified D-pad response |
| MOUSE_DPAD_WHEEL_REPEAT_POLLS | macro | src/mouse.c | mouse response policy | file | Defines the fixed repeat countdown used for held Triangle-plus-D-pad wheel input. | Test11K qualified D-pad response |
| reset_dpad_motion | function | src/mouse.c | mouse response | file | Clears D-pad hold identity, timing, and fractional pointer motion so no prior D-pad gesture can resume across an ownership boundary. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | reset_dpad_motion | local | Supplies the persistent pure mouse state operated on by this function. | B07 stale-state invalidation |
| reset_analog_motion | function | src/mouse.c | mouse response | file | Clears fractional analog pointer motion so a partial pre-boundary pixel cannot emerge after a later stick gesture. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | reset_analog_motion | local | Supplies the persistent pure mouse state operated on by this function. | B07 stale-state invalidation |
| analog_speed_curve_q12 | function | src/mouse.c | mouse response | file | Maps raw stick magnitude through the qualified shallow-low-range and accelerated-high-range response curve. | Issue #38: pure mouse boundary |
| raw_magnitude | parameter | src/mouse.c | analog_speed_curve_q12 | local | Supplies one absolute centered-stick magnitude for response-curve evaluation. | B07 qualified analog pointer response |
| usable | variable | src/mouse.c | analog_speed_curve_q12 | local | Stores deadzone-adjusted analog magnitude used by the response curve. | B07 qualified analog pointer response |
| knee_usable | variable | src/mouse.c | analog_speed_curve_q12 | local | Stores the deadzone-adjusted magnitude of the qualified analog response knee. | B07 qualified analog pointer response |
| max_usable | variable | src/mouse.c | analog_speed_curve_q12 | local | Stores the deadzone-adjusted maximum analog magnitude. | B07 qualified analog pointer response |
| curve | variable | src/mouse.c | analog_speed_curve_q12 | local | Stores the current Q12 response-curve value. | B07 qualified analog pointer response |
| scale_q12 | variable | src/mouse.c | analog_speed_curve_q12 | local | Stores the Q12 low-range scale used to keep shallow stick travel deliberately slow. | B07 qualified analog pointer response |
| stick_velocity_q8 | function | src/mouse.c | mouse response | file | Converts one raw centered stick axis into signed fractional Q8 pointer velocity. | Issue #38: pure mouse boundary |
| value | parameter | src/mouse.c | stick_velocity_q8 | local | Supplies one native unsigned raw stick-axis value centered at 128. | B07 qualified analog pointer response |
| displacement | variable | src/mouse.c | stick_velocity_q8 | local | Stores signed raw stick displacement from the native center value 128. | B07 qualified analog pointer response |
| sign | variable | src/mouse.c | stick_velocity_q8 | local | Stores the sign of the current centered stick displacement. | B07 qualified analog pointer response |
| raw | variable | src/mouse.c | stick_velocity_q8 | local | Stores the absolute or reconstructed raw magnitude used by the current response calculation. | B07 qualified analog pointer response |
| curve | variable | src/mouse.c | stick_velocity_q8 | local | Stores the current Q12 response-curve value. | B07 qualified analog pointer response |
| velocity | variable | src/mouse.c | stick_velocity_q8 | local | Stores the current signed or unsigned fractional Q8 movement velocity. | B07 qualified analog pointer response |
| drain_q8 | function | src/mouse.c | mouse response | file | Extracts whole pixels from one signed Q8 accumulator while preserving the remaining fractional motion. | Issue #38: pure mouse boundary |
| accumulator | parameter | src/mouse.c | drain_q8 | local | Supplies the signed Q8 motion accumulator from which whole pixels are drained. | B07 qualified analog pointer response |
| pixels | variable | src/mouse.c | drain_q8 | local | Accumulates whole pixels drained from one signed Q8 motion accumulator. | B07 qualified analog pointer response |
| dpad_hold_velocity_q8 | function | src/mouse.c | mouse response | file | Computes progressive D-pad hold velocity after the precision phase using the qualified analog-shaped acceleration curve. | Issue #38: pure mouse boundary |
| hold_ticks | parameter | src/mouse.c | dpad_hold_velocity_q8 | local | Supplies the elapsed approximately 60 Hz sample count for the current D-pad hold. | B07 qualified D-pad pointer response |
| elapsed | variable | src/mouse.c | dpad_hold_velocity_q8 | local | Stores the number of D-pad hold samples elapsed beyond the acceleration threshold. | B07 qualified D-pad pointer response |
| raw | variable | src/mouse.c | dpad_hold_velocity_q8 | local | Stores the absolute or reconstructed raw magnitude used by the current response calculation. | B07 qualified D-pad pointer response |
| curve | variable | src/mouse.c | dpad_hold_velocity_q8 | local | Stores the current Q12 response-curve value. | B07 qualified D-pad pointer response |
| apply_dpad_pointer | function | src/mouse.c | mouse response | file | Applies fresh-step, delayed-repeat, and accelerated D-pad pointer behavior to one mouse sample. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | apply_dpad_pointer | local | Supplies the persistent pure mouse state operated on by this function. | B07 qualified D-pad pointer response |
| directions | parameter | src/mouse.c | apply_dpad_pointer | local | Supplies the current mouse-domain directional bit mask. | B07 qualified D-pad pointer response |
| dx | parameter | src/mouse.c | apply_dpad_pointer | local | Supplies the horizontal pixel-displacement accumulator updated by this response helper. | B07 qualified D-pad pointer response |
| dy | parameter | src/mouse.c | apply_dpad_pointer | local | Supplies the vertical pixel-displacement accumulator updated by this response helper. | B07 qualified D-pad pointer response |
| do_step | variable | src/mouse.c | apply_dpad_pointer | local | Records whether the current D-pad sample emits the immediate or delayed two-pixel precision step. | B07 qualified D-pad pointer response |
| velocity | variable | src/mouse.c | apply_dpad_pointer | local | Stores the current signed or unsigned fractional Q8 movement velocity. | B07 qualified D-pad pointer response |
| apply_analog_pointer | function | src/mouse.c | mouse response | file | Accumulates qualified left-stick pointer velocity and emits whole-pixel displacement for one sample. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | apply_analog_pointer | local | Supplies the persistent pure mouse state operated on by this function. | B07 qualified analog pointer response |
| stick_x | parameter | src/mouse.c | apply_analog_pointer | local | Supplies the native unsigned horizontal left-stick sample. | B07 qualified analog pointer response |
| stick_y | parameter | src/mouse.c | apply_analog_pointer | local | Supplies the native unsigned vertical left-stick sample. | B07 qualified analog pointer response |
| dx | parameter | src/mouse.c | apply_analog_pointer | local | Supplies the horizontal pixel-displacement accumulator updated by this response helper. | B07 qualified analog pointer response |
| dy | parameter | src/mouse.c | apply_analog_pointer | local | Supplies the vertical pixel-displacement accumulator updated by this response helper. | B07 qualified analog pointer response |
| vx | variable | src/mouse.c | apply_analog_pointer | local | Stores horizontal analog pointer velocity in signed Q8 units. | B07 qualified analog pointer response |
| vy | variable | src/mouse.c | apply_analog_pointer | local | Stores vertical analog pointer velocity in signed Q8 units. | B07 qualified analog pointer response |
| wheel_direction_for_dpad | function | src/mouse.c | mouse response | file | Resolves routed D-pad directions into one wheel direction with qualified vertical precedence for diagonals. | Issue #38: pure mouse boundary |
| directions | parameter | src/mouse.c | wheel_direction_for_dpad | local | Supplies the current mouse-domain directional bit mask. | B07 scroll semantics / Issue #38 wheel behavior |
| wheel_direction_for_stick | function | src/mouse.c | mouse response | file | Resolves left-stick displacement into one dominant-axis analog wheel direction and magnitude. | Issue #38: pure mouse boundary |
| stick_x | parameter | src/mouse.c | wheel_direction_for_stick | local | Supplies the native unsigned horizontal left-stick sample. | B07 scroll semantics / Issue #38 wheel behavior |
| stick_y | parameter | src/mouse.c | wheel_direction_for_stick | local | Supplies the native unsigned vertical left-stick sample. | B07 scroll semantics / Issue #38 wheel behavior |
| magnitude | parameter | src/mouse.c | wheel_direction_for_stick | local | Receives or supplies the dominant analog deflection magnitude used for wheel timing. | B07 scroll semantics / Issue #38 wheel behavior |
| x | variable | src/mouse.c | wheel_direction_for_stick | local | Stores centered horizontal stick displacement for dominant-axis wheel selection. | B07 scroll semantics / Issue #38 wheel behavior |
| y | variable | src/mouse.c | wheel_direction_for_stick | local | Stores centered vertical stick displacement for dominant-axis wheel selection. | B07 scroll semantics / Issue #38 wheel behavior |
| absolute_x | variable | src/mouse.c | wheel_direction_for_stick | local | Stores absolute horizontal stick displacement for dominant-axis comparison. | B07 scroll semantics / Issue #38 wheel behavior |
| absolute_y | variable | src/mouse.c | wheel_direction_for_stick | local | Stores absolute vertical stick displacement for dominant-axis comparison. | B07 scroll semantics / Issue #38 wheel behavior |
| wheel_repeat_delay | function | src/mouse.c | mouse response | file | Converts analog wheel deflection magnitude into the qualified variable wheel-repeat countdown. | Issue #38: pure mouse boundary |
| magnitude | parameter | src/mouse.c | wheel_repeat_delay | local | Receives or supplies the dominant analog deflection magnitude used for wheel timing. | B07 scroll semantics / Issue #38 wheel behavior |
| curve | variable | src/mouse.c | wheel_repeat_delay | local | Stores the current Q12 response-curve value. | B07 scroll semantics / Issue #38 wheel behavior |
| clicks_per_second_x10 | variable | src/mouse.c | wheel_repeat_delay | local | Stores analog wheel repeat rate in tenths of a notch per second. | B07 scroll semantics / Issue #38 wheel behavior |
| interval_polls | variable | src/mouse.c | wheel_repeat_delay | local | Stores the rounded approximately 60 Hz sample interval between analog wheel notches. | B07 scroll semantics / Issue #38 wheel behavior |
| clamp_position | function | src/mouse.c | mouse response | file | Clamps the persistent cursor position to the current logical remote desktop. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | clamp_position | local | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| pstvnc_mouse_init | function | src/mouse.c | mouse state | public | Declares initialization of pure mouse state for the current logical remote desktop. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | pstvnc_mouse_init | local | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| width | parameter | src/mouse.c | pstvnc_mouse_init | local | Supplies the logical remote desktop width in pixels. | Issue #38: pure mouse boundary |
| height | parameter | src/mouse.c | pstvnc_mouse_init | local | Supplies the logical remote desktop height in pixels. | Issue #38: pure mouse boundary |
| pstvnc_mouse_reset_derived | function | src/mouse.c | mouse state | public | Clears controller-derived motion, repeat, and wheel-mode history while preserving the interpreter's current durable cursor/button state. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | pstvnc_mouse_reset_derived | local | Supplies the persistent pure mouse state operated on by this function. | B07 stale-state invalidation |
| pstvnc_mouse_update | function | src/mouse.c | mouse state | public | Declares advancement of pure semantic mouse state from one already-routed input sample. | Issue #38: pure mouse boundary |
| mouse | parameter | src/mouse.c | pstvnc_mouse_update | local | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| input | parameter | src/mouse.c | pstvnc_mouse_update | local | Supplies one already-routed mouse-domain input sample. | Issue #38: pure mouse boundary |
| update | parameter | src/mouse.c | pstvnc_mouse_update | local | Receives the semantic mouse result produced for this sample. | Issue #38: pure mouse boundary |
| old_x | variable | src/mouse.c | pstvnc_mouse_update | local | Snapshots cursor X before the current sample so pointer-state change can be detected. | Issue #38: pure mouse boundary |
| old_y | variable | src/mouse.c | pstvnc_mouse_update | local | Snapshots cursor Y before the current sample so pointer-state change can be detected. | Issue #38: pure mouse boundary |
| old_click_buttons | variable | src/mouse.c | pstvnc_mouse_update | local | Snapshots ordinary click-button state before the current sample so publication changes can be detected. | Issue #38: pure mouse boundary |
| dx | variable | src/mouse.c | pstvnc_mouse_update | local | Stores the local `dx` intermediate used by `pstvnc_mouse_update` while evaluating the current mouse sample. | Issue #38: pure mouse boundary |
| dy | variable | src/mouse.c | pstvnc_mouse_update | local | Stores the local `dy` intermediate used by `pstvnc_mouse_update` while evaluating the current mouse sample. | Issue #38: pure mouse boundary |
| wheel_direction | variable | src/mouse.c | pstvnc_mouse_update | local | Stores the semantic wheel direction selected for the current sample. | Issue #38: pure mouse boundary |
| wheel_repeat_delay_polls | variable | src/mouse.c | pstvnc_mouse_update | local | Stores the repeat countdown selected for the current wheel source and magnitude. | Issue #38: pure mouse boundary |
| wheel_magnitude | variable | src/mouse.c | pstvnc_mouse_update | local | Stores dominant analog wheel deflection magnitude for variable repeat timing. | Issue #38: pure mouse boundary |
| wheel_mode_toggled | variable | src/mouse.c | pstvnc_mouse_update | local | Records that the current L3 press changed analog wheel mode and therefore owns this stick sample. | Issue #38: pure mouse boundary |
| pstvnc_mouse_rebase_published_state | function | src/mouse.c | mouse state | public | Replaces locally interpreted durable cursor/button state with application/main's exact successfully published remote pointer state and clears controller-derived response history. | Issue #38: published-state rebase boundary |
| mouse | parameter | src/mouse.c | pstvnc_mouse_rebase_published_state | local | Supplies the pure mouse-interpreter state whose published-state boundary is being reconciled. | Issue #38: published-state rebase boundary |
| published_click_buttons | parameter | src/mouse.c | pstvnc_mouse_rebase_published_state | local | Supplies application/main's successfully published remote ordinary mouse-button mask. | Issue #38: published-state rebase boundary |
| published_cursor_x | parameter | src/mouse.c | pstvnc_mouse_rebase_published_state | local | Supplies application/main's last successfully published remote cursor X coordinate. | Issue #38: published-state rebase boundary |
| published_cursor_y | parameter | src/mouse.c | pstvnc_mouse_rebase_published_state | local | Supplies application/main's last successfully published remote cursor Y coordinate. | Issue #38: published-state rebase boundary |
| PSTVNC_MOUSE_H | macro | src/mouse.h | mouse interface | file | Prevents repeated inclusion of the pure remote-mouse interface. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DIRECTION_UP | macro | src/mouse.h | mouse interface | public | Defines the mouse-domain upward directional bit supplied by higher-level input routing. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DIRECTION_DOWN | macro | src/mouse.h | mouse interface | public | Defines the mouse-domain downward directional bit supplied by higher-level input routing. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DIRECTION_LEFT | macro | src/mouse.h | mouse interface | public | Defines the mouse-domain left directional bit supplied by higher-level input routing. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DIRECTION_RIGHT | macro | src/mouse.h | mouse interface | public | Defines the mouse-domain right directional bit supplied by higher-level input routing. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DIRECTION_MASK | macro | src/mouse.h | mouse interface | public | Combines all mouse-domain directional bits accepted by D-pad pointer and wheel interpretation. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_BUTTON_LCLICK | macro | src/mouse.h | mouse interface | public | Defines the semantic ordinary left mouse-button bit independent of RFB wire serialization. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_BUTTON_RCLICK | macro | src/mouse.h | mouse interface | public | Defines the semantic ordinary right mouse-button bit independent of RFB wire serialization. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_BUTTON_MASK | macro | src/mouse.h | mouse interface | public | Combines the semantic ordinary mouse-button bits accepted by the pure mouse state machine. | Issue #38: pure mouse boundary |
| pstvnc_mouse_dpad_mode | enum | src/mouse.h | mouse interface | public | Defines whether the routed D-pad is unavailable to the mouse, owns pointer motion, or owns wheel motion. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DPAD_NONE | enum value | src/mouse.h | pstvnc_mouse_dpad_mode | public | States that the current D-pad sample belongs to another responsibility and must not affect mouse behavior. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DPAD_POINTER | enum value | src/mouse.h | pstvnc_mouse_dpad_mode | public | States that the current D-pad sample controls remote cursor movement. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_DPAD_WHEEL | enum value | src/mouse.h | pstvnc_mouse_dpad_mode | public | States that the current D-pad sample controls remote wheel direction. | Issue #38: pure mouse boundary |
| pstvnc_mouse_dpad_mode_t | type | src/mouse.h | mouse interface | public | Names the routed D-pad ownership mode consumed by one mouse input sample. | Issue #38: pure mouse boundary |
| pstvnc_mouse_wheel_direction | enum | src/mouse.h | mouse interface | public | Defines the semantic wheel directions emitted by the pure mouse state machine. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_WHEEL_NONE | enum value | src/mouse.h | pstvnc_mouse_wheel_direction | public | States that no semantic wheel notch is emitted for the current mouse sample. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_WHEEL_UP | enum value | src/mouse.h | pstvnc_mouse_wheel_direction | public | Represents one upward semantic remote wheel notch. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_WHEEL_DOWN | enum value | src/mouse.h | pstvnc_mouse_wheel_direction | public | Represents one downward semantic remote wheel notch. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_WHEEL_LEFT | enum value | src/mouse.h | pstvnc_mouse_wheel_direction | public | Represents one leftward semantic remote wheel notch. | Issue #38: pure mouse boundary |
| PSTVNC_MOUSE_WHEEL_RIGHT | enum value | src/mouse.h | pstvnc_mouse_wheel_direction | public | Represents one rightward semantic remote wheel notch. | Issue #38: pure mouse boundary |
| pstvnc_mouse_wheel_direction_t | type | src/mouse.h | mouse interface | public | Names one semantic remote wheel direction or the absence of a wheel event. | Issue #38: pure mouse boundary |
| pstvnc_mouse_input | structure | src/mouse.h | mouse interface | public | Defines one already-routed mouse-domain input sample independent of libpad and higher-level action arbitration. | Issue #38: pure mouse boundary |
| dpad_mode | field | src/mouse.h | pstvnc_mouse_input | public | States which mouse responsibility, if any, owns the routed D-pad for this sample. | Issue #38: pure mouse boundary |
| dpad_directions | field | src/mouse.h | pstvnc_mouse_input | public | Carries the active mouse-domain D-pad direction bits for this sample. | Issue #38: pure mouse boundary |
| stick_available | field | src/mouse.h | pstvnc_mouse_input | public | States whether valid raw left-stick data is currently available to mouse interpretation. | Issue #38: pure mouse boundary |
| stick_x | field | src/mouse.h | pstvnc_mouse_input | public | Carries the native unsigned horizontal left-stick sample when analog input is available. | Issue #38: pure mouse boundary |
| stick_y | field | src/mouse.h | pstvnc_mouse_input | public | Carries the native unsigned vertical left-stick sample when analog input is available. | Issue #38: pure mouse boundary |
| wheel_click_pressed | field | src/mouse.h | pstvnc_mouse_input | public | Carries one routed L3 press edge used to toggle analog wheel mode. | Issue #38: L3 click-to-toggle analog wheel mode |
| click_buttons | field | src/mouse.h | pstvnc_mouse_input | public | Carries the currently held semantic ordinary mouse-button mask. | Issue #38: pure mouse boundary |
| pstvnc_mouse_input_t | type | src/mouse.h | mouse interface | public | Names the already-routed mouse-domain sample consumed by the pure mouse state machine. | Issue #38: pure mouse boundary |
| pstvnc_mouse_update | structure | src/mouse.h | mouse interface | public | Declares advancement of pure semantic mouse state from one already-routed input sample. | Issue #38: pure mouse boundary |
| pointer_changed | field | src/mouse.h | pstvnc_mouse_update | public | States that cursor position or ordinary click-button state changed and should be published remotely. | Issue #38: pure mouse boundary |
| cursor_x | field | src/mouse.h | pstvnc_mouse_update | public | Reports the current clamped logical remote cursor X coordinate. | Issue #38: pure mouse boundary |
| cursor_y | field | src/mouse.h | pstvnc_mouse_update | public | Reports the current clamped logical remote cursor Y coordinate. | Issue #38: pure mouse boundary |
| click_buttons | field | src/mouse.h | pstvnc_mouse_update | public | Reports the current semantic ordinary remote mouse-button state. | Issue #38: pure mouse boundary |
| wheel_direction | field | src/mouse.h | pstvnc_mouse_update | public | Reports exactly one semantic wheel notch direction or NONE for this sample. | Issue #38: pure mouse boundary |
| pstvnc_mouse_update_t | type | src/mouse.h | mouse interface | public | Names the semantic remote-mouse update returned to a higher-level event publisher. | Issue #38: pure mouse boundary |
| pstvnc_mouse | structure | src/mouse.h | mouse interface | public | Defines persistent pure mouse-interpreter state: locally interpreted cursor/button state plus controller-derived response history. | Issue #38: pure mouse boundary |
| width | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative logical desktop width used for cursor clamping. | Issue #38: pure mouse boundary |
| height | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative logical desktop height used for cursor clamping. | Issue #38: pure mouse boundary |
| cursor_x | field | src/mouse.h | pstvnc_mouse | public | Stores the current locally interpreted cursor X position, which may temporarily lead the last successfully published remote pointer state. | Issue #38: pure mouse boundary |
| cursor_y | field | src/mouse.h | pstvnc_mouse | public | Stores the current locally interpreted cursor Y position, which may temporarily lead the last successfully published remote pointer state. | Issue #38: pure mouse boundary |
| click_buttons | field | src/mouse.h | pstvnc_mouse | public | Stores the current locally interpreted ordinary mouse-button state; application/main separately owns what was successfully published. | Issue #38: pure mouse boundary |
| wheel_mode_enabled | field | src/mouse.h | pstvnc_mouse | public | Records whether the Issue 38 L3 toggle currently routes the left stick to analog wheel behavior. | Issue #38: L3 click-to-toggle analog wheel mode |
| dpad_hold_direction | field | src/mouse.h | pstvnc_mouse | public | Stores the exact D-pad direction combination currently owning one pointer-hold gesture. | Issue #38: pure mouse boundary |
| dpad_hold_ticks | field | src/mouse.h | pstvnc_mouse | public | Counts approximately 60 Hz samples in the current D-pad hold gesture. | Issue #38: pure mouse boundary |
| dpad_x_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 horizontal D-pad pointer motion across samples. | Issue #38: pure mouse boundary |
| dpad_y_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 vertical D-pad pointer motion across samples. | Issue #38: pure mouse boundary |
| analog_x_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 horizontal analog pointer motion across samples. | Issue #38: pure mouse boundary |
| analog_y_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 vertical analog pointer motion across samples. | Issue #38: pure mouse boundary |
| last_wheel_direction | field | src/mouse.h | pstvnc_mouse | public | Stores the prior active wheel direction used to distinguish a fresh direction from held repeat. | Issue #38: pure mouse boundary |
| wheel_repeat_countdown | field | src/mouse.h | pstvnc_mouse | public | Stores the remaining sample countdown before a held wheel direction may emit another notch. | Issue #38: pure mouse boundary |
| pstvnc_mouse_t | type | src/mouse.h | mouse interface | public | Names the persistent pure mouse-interpreter state object. | Issue #38: pure mouse boundary |
| pstvnc_mouse_init | function declaration | src/mouse.h | mouse interface | public | Declares initialization of pure mouse state for the current logical remote desktop. | Issue #38: pure mouse boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_init | prototype | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| width | prototype parameter | src/mouse.h | pstvnc_mouse_init | prototype | Supplies the logical remote desktop width in pixels. | Issue #38: pure mouse boundary |
| height | prototype parameter | src/mouse.h | pstvnc_mouse_init | prototype | Supplies the logical remote desktop height in pixels. | Issue #38: pure mouse boundary |
| pstvnc_mouse_reset_derived | function declaration | src/mouse.h | mouse interface | public | Declares the hard-boundary reset that preserves locally interpreted cursor/button state while forgetting controller-derived response history. | Issue #38: pure mouse boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_reset_derived | prototype | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| pstvnc_mouse_update | function declaration | src/mouse.h | mouse interface | public | Declares advancement of pure semantic mouse state from one already-routed input sample. | Issue #38: pure mouse boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_update | prototype | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| input | prototype parameter | src/mouse.h | pstvnc_mouse_update | prototype | Supplies one already-routed mouse-domain input sample. | Issue #38: pure mouse boundary |
| update | prototype parameter | src/mouse.h | pstvnc_mouse_update | prototype | Receives the semantic mouse result produced for this sample. | Issue #38: pure mouse boundary |
| pstvnc_mouse_rebase_published_state | function declaration | src/mouse.h | mouse interface | public | Declares explicit reconciliation of locally interpreted cursor/button state with application/main's successfully published remote pointer state. | Issue #38: published-state rebase boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_rebase_published_state | prototype | Supplies the pure mouse-interpreter state whose published-state boundary is being reconciled. | Issue #38: published-state rebase boundary |
| published_click_buttons | prototype parameter | src/mouse.h | pstvnc_mouse_rebase_published_state | prototype | Supplies application/main's successfully published remote ordinary mouse-button mask. | Issue #38: published-state rebase boundary |
| published_cursor_x | prototype parameter | src/mouse.h | pstvnc_mouse_rebase_published_state | prototype | Supplies application/main's last successfully published remote cursor X coordinate. | Issue #38: published-state rebase boundary |
| published_cursor_y | prototype parameter | src/mouse.h | pstvnc_mouse_rebase_published_state | prototype | Supplies application/main's last successfully published remote cursor Y coordinate. | Issue #38: published-state rebase boundary |
| libpad_ready | variable | src/pad.c | pad lifecycle | file static | Records whether PS-to-VNC currently owns initialized process-wide libpad state. | ADR 0002: direct libpad ownership |
| pad_state_is_readable | function | src/pad.c | pad acquisition | file | Reports whether one native libpad state permits ordinary controller observation. | libpad state machine |
| state | parameter | src/pad.c | pad_state_is_readable | function | Supplies the native PAD_STATE_* value to classify. | libpad state machine |
| configure_controller_mode | function | src/pad.c | physical pad acquisition | file | Advances digital/DualShock controller-mode negotiation by at most one bounded libpad step per poll and verifies a completed asynchronous DualShock request before publishing the connection as configured. | ADR0002 libpad mode handling; ISSUE38 controller ownership |
| pad | parameter | src/pad.c | configure_controller_mode | function | Supplies the connected endpoint whose native libpad operating mode is configured. | controller mode negotiation |
| modes | variable | src/pad.c | configure_controller_mode | local | Stores the controller mode-table entry count reported by libpad. | controller capability discovery |
| i | variable | src/pad.c | configure_controller_mode | local | Iterates native libpad mode-table entries while searching for DualShock support. | controller capability discovery |
| pstvnc_pad_init | function | src/pad.c | pad lifecycle | public | Initializes process-wide libpad ownership exactly once for PS-to-VNC. | ADR 0002: direct libpad ownership |
| pstvnc_pad_shutdown | function | src/pad.c | pad lifecycle | public | Ends owned process-wide libpad state after endpoint users have been closed. | pad lifecycle |
| pstvnc_pad_open | function | src/pad.c | pad lifecycle | public | Opens one explicit libpad port/slot endpoint using its instance-owned aligned DMA buffer. | physical pad endpoint ownership |
| pad | parameter | src/pad.c | pstvnc_pad_open | function | Supplies storage that becomes the owned PS-to-VNC pad endpoint instance. | physical pad endpoint ownership |
| port | parameter | src/pad.c | pstvnc_pad_open | function | Selects the native libpad controller port to open. | libpad port/slot addressing |
| slot | parameter | src/pad.c | pstvnc_pad_open | function | Selects the native libpad slot within the chosen controller port. | libpad port/slot addressing |
| pstvnc_pad_poll | function | src/pad.c | pad acquisition | public | Produces one latest native controller sample and immediate active-high press/release masks when readable. | Issue #38 minimal physical pad foundation |
| pad | parameter | src/pad.c | pstvnc_pad_poll | function | Supplies the opened endpoint whose physical controller state is polled. | physical pad endpoint ownership |
| state | variable | src/pad.c | pstvnc_pad_poll | local | Stores the current native libpad endpoint state used to accept readable samples or invalidate continuity across unavailable states. | libpad state machine |
| configuration_result | variable | src/pad.c | pstvnc_pad_poll | local | Stores whether current-connection controller-mode preparation succeeded, disconnected, or failed. | controller mode negotiation |
| sample | variable | src/pad.c | pstvnc_pad_poll | local | Holds one freshly cleared native padButtonStatus so short pad packets cannot retain stale tail fields. | libpad padRead semantics |
| sample_length | variable | src/pad.c | pstvnc_pad_poll | local | Stores the exact byte count copied by the current successful padRead call. | libpad padRead semantics |
| new_buttons_down | variable | src/pad.c | pstvnc_pad_poll | local | Stores the newest active-high 16-bit physical PAD_* button mask after libpad active-low inversion. | immediate physical pad state |
| required_button_bytes | variable | src/pad.c | pstvnc_pad_poll | local | Computes the minimum returned sample length required before the btns field is trusted. | libpad variable-length sample safety |
| pstvnc_pad_close | function | src/pad.c | pad lifecycle | public | Closes one libpad endpoint and invalidates all physical history associated with that ownership. | pad lifecycle |
| pad | parameter | src/pad.c | pstvnc_pad_close | function | Supplies the opened pad instance whose endpoint ownership is released. | pad lifecycle |
| pstvnc_pad_invalidate_connection_epoch | function | src/pad.c | physical pad connection epoch | public | Ends the current physical controller connection epoch by revoking mode-configuration authority and remembered sample/edge continuity without closing the libpad endpoint. | Issue #38: libpad connection-epoch ownership |
| pad | parameter | src/pad.c | pstvnc_pad_invalidate_connection_epoch | local | Supplies the pad endpoint whose current physical connection epoch is invalidated. | Issue #38: libpad connection-epoch ownership |
| PSTVNC_PAD_H | include-guard macro | src/pad.h | pad interface | header | Prevents repeated inclusion of the direct-libpad PS-to-VNC pad interface. | clean source interface |
| pstvnc_pad | structure | src/pad.h | pad interface | public | Defines one PS-to-VNC-owned libpad endpoint plus only its immediate physical observation state. | ADR 0002: pad boundary |
| port | field | src/pad.h | pstvnc_pad | public | Stores the native libpad port identifying this physical endpoint. | libpad port/slot addressing |
| slot | field | src/pad.h | pstvnc_pad | public | Stores the native libpad slot identifying this physical endpoint. | libpad port/slot addressing |
| state | field | src/pad.h | pstvnc_pad | public | Stores the latest native PAD_STATE_* value observed for this endpoint. | libpad state machine |
| opened | field | src/pad.h | pstvnc_pad | public | Records whether this instance currently owns an opened libpad port/slot endpoint. | pad lifecycle |
| connection_configured | field | src/pad.h | pstvnc_pad | public | Records whether mode negotiation has completed for the current physical connection epoch. | controller mode negotiation |
| dualshock_mode_request_pending | field | src/pad.h | pstvnc_pad | public | Records that padSetMainMode() successfully started an asynchronous DualShock lock request whose completion must be verified by a later readable poll. | ISSUE38 bounded libpad ownership lifecycle |
| history_valid | field | src/pad.h | pstvnc_pad | public | Records whether buttons_down has a continuous prior sample suitable for deriving physical edges. | immediate physical pad history |
| sample_length | field | src/pad.h | pstvnc_pad | public | Stores the exact leading-byte count in buttons supplied by the latest accepted padRead sample. | libpad variable-length sample semantics |
| buttons | field | src/pad.h | pstvnc_pad | public | Stores the latest accepted native libpad padButtonStatus without a replacement controller representation. | ADR 0002: native libpad representation |
| buttons_down | field | src/pad.h | pstvnc_pad | public | Stores the current active-high physical PAD_* button mask derived from native active-low btns. | immediate physical pad state |
| buttons_pressed | field | src/pad.h | pstvnc_pad | public | Stores buttons newly observed down relative to the immediately preceding valid physical sample. | immediate physical pad history |
| buttons_released | field | src/pad.h | pstvnc_pad | public | Stores buttons newly observed up relative to the immediately preceding valid physical sample. | immediate physical pad history |
| dma_buffer | field | src/pad.h | pstvnc_pad | public | Provides the 256-byte 64-byte-aligned caller-owned DMA region required by current libpad for this endpoint. | libpad padPortOpen contract |
| pstvnc_pad_t | type | src/pad.h | pad interface | public | Names one owned physical libpad endpoint instance without replacing libpad controller vocabulary. | ADR 0002: pad boundary |
| pstvnc_pad_init | function declaration | src/pad.h | pad interface | public | Declares process-wide PS-to-VNC libpad initialization. | pad lifecycle |
| pstvnc_pad_shutdown | function declaration | src/pad.h | pad interface | public | Declares release of process-wide PS-to-VNC libpad ownership. | pad lifecycle |
| pstvnc_pad_open | function declaration | src/pad.h | pad interface | public | Declares opening one explicit native libpad port/slot endpoint. | physical pad endpoint ownership |
| pad | prototype parameter | src/pad.h | pstvnc_pad_open | prototype | Names the pad instance storage populated by the public open operation. | clean source interface |
| port | prototype parameter | src/pad.h | pstvnc_pad_open | prototype | Names the native libpad port argument in the public open contract. | libpad port/slot addressing |
| slot | prototype parameter | src/pad.h | pstvnc_pad_open | prototype | Names the native libpad slot argument in the public open contract. | libpad port/slot addressing |
| pstvnc_pad_poll | function declaration | src/pad.h | pad interface | public | Declares one physical libpad observation attempt with immediate edge derivation. | Issue #38 minimal physical pad foundation |
| pad | prototype parameter | src/pad.h | pstvnc_pad_poll | prototype | Names the opened pad instance supplied to the public polling operation. | clean source interface |
| pstvnc_pad_close | function declaration | src/pad.h | pad interface | public | Declares release of one owned libpad endpoint and its immediate history. | pad lifecycle |
| pad | prototype parameter | src/pad.h | pstvnc_pad_close | prototype | Names the pad instance supplied to the public close operation. | clean source interface |
| pstvnc_pad_invalidate_connection_epoch | function declaration | src/pad.h | pad interface | public | Declares the pad owner's hard stale-state boundary that ends the current physical connection epoch without closing the endpoint. | Issue #38: libpad connection-epoch ownership |
| pad | prototype parameter | src/pad.h | pstvnc_pad_invalidate_connection_epoch | prototype | Supplies the pad endpoint whose current physical connection epoch is invalidated. | Issue #38: libpad connection-epoch ownership |
| read_be16 | function | src/rfb.c | RFB wire format | file | Decodes one unsigned 16-bit big-endian RFB field without side effects. | RFB wire contract |
| bytes | parameter | src/rfb.c | read_be16 | local | Points to the two network-order bytes to decode. | RFB wire contract |
| read_be32 | function | src/rfb.c | RFB wire format | file | Decodes one unsigned 32-bit big-endian RFB field without side effects. | RFB wire contract |
| bytes | parameter | src/rfb.c | read_be32 | local | Points to the four network-order bytes to decode. | RFB wire contract |
| write_be16 | function | src/rfb.c | RFB wire format | file | Encodes one unsigned 16-bit value into network byte order. | RFB wire contract |
| bytes | parameter | src/rfb.c | write_be16 | local | Points to the two output bytes receiving network-order data. | RFB wire contract |
| value | parameter | src/rfb.c | write_be16 | local | Supplies the host-order value to encode. | RFB wire contract |
| pstvnc_rfb_parse_protocol_version | function | src/rfb.c | RFB wire format | public | Validates and decodes the fixed 12-byte RFB protocol banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| banner | parameter | src/rfb.c | pstvnc_rfb_parse_protocol_version | local | Supplies the exact server protocol banner bytes to validate. | RFB wire contract |
| major | parameter | src/rfb.c | pstvnc_rfb_parse_protocol_version | local | Receives the parsed protocol major version. | RFB wire contract |
| minor | parameter | src/rfb.c | pstvnc_rfb_parse_protocol_version | local | Receives the parsed protocol minor version. | RFB wire contract |
| i | variable | src/rfb.c | pstvnc_rfb_parse_protocol_version | local | Iterates over fixed decimal version digits in the wire banner. | RFB wire contract |
| parsed_major | variable | src/rfb.c | pstvnc_rfb_parse_protocol_version | local | Accumulates the validated three-digit server major version. | RFB wire contract |
| parsed_minor | variable | src/rfb.c | pstvnc_rfb_parse_protocol_version | local | Accumulates the validated three-digit server minor version. | RFB wire contract |
| pstvnc_rfb_build_client_version | function | src/rfb.c | RFB wire format | public | Writes the fixed RFB 3.8 client banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| out | parameter | src/rfb.c | pstvnc_rfb_build_client_version | local | Receives the exact fixed-length client protocol banner. | RFB wire contract |
| version | variable | src/rfb.c | pstvnc_rfb_build_client_version | local | Stores the immutable RFB 3.8 client banner bytes. | RFB wire contract |
| pstvnc_rfb_choose_security_none | function | src/rfb.c | RFB wire format | public | Selects SecurityType None only when the server explicitly offers it. | Issue #7 security baseline |
| types | parameter | src/rfb.c | pstvnc_rfb_choose_security_none | local | Supplies the server-advertised security type list. | RFB wire contract |
| count | parameter | src/rfb.c | pstvnc_rfb_choose_security_none | local | Gives the exact number of advertised security type bytes. | RFB wire contract |
| choice | parameter | src/rfb.c | pstvnc_rfb_choose_security_none | local | Receives the selected None security type on success. | Issue #7 security baseline |
| i | variable | src/rfb.c | pstvnc_rfb_choose_security_none | local | Iterates across advertised security types. | RFB wire contract |
| pstvnc_rfb_security_result_ok | function | src/rfb.c | RFB wire format | public | Accepts only a zero RFB SecurityResult status. | Issue #7 security baseline |
| result | parameter | src/rfb.c | pstvnc_rfb_security_result_ok | local | Supplies the four network-order SecurityResult bytes. | RFB wire contract |
| pstvnc_rfb_client_init_shared | function | src/rfb.c | RFB wire format | public | Returns the fixed shared-session ClientInit flag. | Issue #7 RFB baseline |
| pstvnc_rfb_parse_server_init | function | src/rfb.c | RFB wire format | public | Parses fixed ServerInit fields and rejects zero geometry. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| bytes | parameter | src/rfb.c | pstvnc_rfb_parse_server_init | local | Supplies the fixed-size ServerInit header bytes. | RFB wire contract |
| out | parameter | src/rfb.c | pstvnc_rfb_parse_server_init | local | Receives parsed server geometry and pixel-format metadata. | RFB wire contract |
| pstvnc_rfb_build_set_pixel_format_gs555 | function | src/rfb.c | RFB wire format | public | Builds the fixed little-endian B5:G5:R5 SetPixelFormat request. | fixed GS-friendly pixel format |
| out | parameter | src/rfb.c | pstvnc_rfb_build_set_pixel_format_gs555 | local | Receives the fixed SetPixelFormat wire message. | fixed GS-friendly pixel format |
| message | variable | src/rfb.c | pstvnc_rfb_build_set_pixel_format_gs555 | local | Stores the immutable GS-friendly SetPixelFormat message bytes. | fixed GS-friendly pixel format |
| pstvnc_rfb_build_set_encodings_raw | function | src/rfb.c | RFB wire format | public | Builds the one-entry SetEncodings request advertising Raw only. | Raw-only Issue #7 baseline |
| out | parameter | src/rfb.c | pstvnc_rfb_build_set_encodings_raw | local | Receives the fixed Raw-only SetEncodings wire message. | Raw-only Issue #7 baseline |
| message | variable | src/rfb.c | pstvnc_rfb_build_set_encodings_raw | local | Stores the immutable Raw-only SetEncodings message bytes. | Raw-only Issue #7 baseline |
| pstvnc_rfb_build_framebuffer_update_request | function | src/rfb.c | RFB wire format | public | Serializes one full-coordinate FramebufferUpdateRequest. | RFB wire contract |
| out | parameter | src/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Receives the fixed-size FramebufferUpdateRequest bytes. | RFB wire contract |
| incremental | parameter | src/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Selects full versus incremental server update semantics. | RFB update requests |
| x | parameter | src/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle left coordinate. | RFB update requests |
| y | parameter | src/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle top coordinate. | RFB update requests |
| width | parameter | src/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle width. | RFB update requests |
| height | parameter | src/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle height. | RFB update requests |
| pstvnc_rfb_build_pointer_event | function | src/rfb.c | RFB wire format | public | Builds one exact six-byte RFB PointerEvent from an already-mapped native RFB button mask and logical coordinates. | Issue #38 pointer publication |
| out | parameter | src/rfb.c | pstvnc_rfb_build_pointer_event | local | Receives the exact six-byte PointerEvent wire message. | Issue #38 pointer publication |
| button_mask | parameter | src/rfb.c | pstvnc_rfb_build_pointer_event | local | Supplies native RFB button-mask vocabulary without semantic input interpretation. | Issue #38 pointer publication |
| x | parameter | src/rfb.c | pstvnc_rfb_build_pointer_event | local | Gives the logical remote pointer X coordinate encoded big-endian on the wire. | Issue #38 pointer publication |
| y | parameter | src/rfb.c | pstvnc_rfb_build_pointer_event | local | Gives the logical remote pointer Y coordinate encoded big-endian on the wire. | Issue #38 pointer publication |
| PSTVNC_RFB_H | include guard | src/rfb.h | RFB wire interface | file | Prevents repeated inclusion of pure RFB wire declarations. | clean source interface |
| PSTVNC_RFB_PROTOCOL_VERSION_SIZE | macro | src/rfb.h | RFB wire interface | public | Names the exact byte length of an RFB version banner. | RFB wire contract |
| PSTVNC_RFB_SERVER_INIT_SIZE | macro | src/rfb.h | RFB wire interface | public | Names the fixed byte length of the ServerInit header before desktop name. | RFB wire contract |
| PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE | macro | src/rfb.h | RFB wire interface | public | Names the exact SetPixelFormat client-message byte length. | RFB wire contract |
| PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE | macro | src/rfb.h | RFB wire interface | public | Names the exact one-entry Raw SetEncodings byte length. | Raw-only Issue #7 baseline |
| PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE | macro | src/rfb.h | RFB wire interface | public | Names the exact FramebufferUpdateRequest byte length. | RFB wire contract |
| PSTVNC_RFB_SECURITY_NONE | macro | src/rfb.h | RFB wire interface | public | Names RFB SecurityType None required by the minimal baseline. | Issue #7 security baseline |
| PSTVNC_RFB_ENCODING_RAW | macro | src/rfb.h | RFB wire interface | public | Names the Raw framebuffer encoding and no other encoding. | Raw-only Issue #7 baseline |
| pstvnc_rfb_server_init | structure | src/rfb.h | RFB wire interface | public | Defines parsed fixed ServerInit geometry and pixel-format metadata. | RFB wire contract |
| width | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised framebuffer width. | RFB ServerInit |
| height | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised framebuffer height. | RFB ServerInit |
| bits_per_pixel | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised bits per pixel. | RFB ServerInit |
| depth | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised color depth. | RFB ServerInit |
| big_endian | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised pixel byte-order flag. | RFB ServerInit |
| true_color | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised true-color flag. | RFB ServerInit |
| red_max | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised maximum red component value. | RFB ServerInit |
| green_max | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised maximum green component value. | RFB ServerInit |
| blue_max | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised maximum blue component value. | RFB ServerInit |
| red_shift | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised red component bit shift. | RFB ServerInit |
| green_shift | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised green component bit shift. | RFB ServerInit |
| blue_shift | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised blue component bit shift. | RFB ServerInit |
| name_length | field | src/rfb.h | pstvnc_rfb_server_init | public | Stores the declared byte length of the following desktop name. | RFB ServerInit |
| pstvnc_rfb_server_init_t | type | src/rfb.h | RFB wire interface | public | Names parsed ServerInit metadata used by synchronized session state. | RFB wire contract |
| pstvnc_rfb_build_client_version | function declaration | src/rfb.h | RFB wire interface | public | Declares construction of the fixed client RFB protocol-version banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_build_framebuffer_update_request | function declaration | src/rfb.h | RFB wire interface | public | Declares construction of a framebuffer-update request for one explicit rectangle and incremental/full mode. | RFB wire contract |
| pstvnc_rfb_build_set_encodings_raw | function declaration | src/rfb.h | RFB wire interface | public | Declares construction of the SetEncodings request selecting Raw encoding only. | Raw-only Issue #7 baseline |
| pstvnc_rfb_build_set_pixel_format_gs555 | function declaration | src/rfb.h | RFB wire interface | public | Declares construction of the SetPixelFormat request for the clean GS555 framebuffer contract. | fixed GS-friendly pixel format |
| pstvnc_rfb_choose_security_none | function declaration | src/rfb.h | RFB wire interface | public | Declares selection of RFB None security from the server-advertised security-type list. | Issue #7 security baseline |
| pstvnc_rfb_client_init_shared | function declaration | src/rfb.h | RFB wire interface | public | Declares the ClientInit shared-desktop flag used by the clean RFB connection. | Issue #7 RFB baseline |
| pstvnc_rfb_parse_protocol_version | function declaration | src/rfb.h | RFB wire interface | public | Declares validation and parsing of one RFB protocol-version banner into its major and minor components. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_parse_server_init | function declaration | src/rfb.h | RFB wire interface | public | Declares decoding of the fixed ServerInit header into typed server framebuffer metadata. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_security_result_ok | function declaration | src/rfb.h | RFB wire interface | public | Declares interpretation of the four-byte RFB SecurityResult field as success or failure. | Issue #7 security baseline |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_build_client_version | prototype | Receives the exact fixed-length client protocol banner. | RFB wire contract |
| height | prototype parameter | src/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle height. | RFB update requests |
| incremental | prototype parameter | src/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Selects full versus incremental server update semantics. | RFB update requests |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Receives the fixed-size FramebufferUpdateRequest bytes. | RFB wire contract |
| width | prototype parameter | src/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle width. | RFB update requests |
| x | prototype parameter | src/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle left coordinate. | RFB update requests |
| y | prototype parameter | src/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle top coordinate. | RFB update requests |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_build_set_encodings_raw | prototype | Receives the fixed Raw-only SetEncodings wire message. | Raw-only Issue #7 baseline |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_build_set_pixel_format_gs555 | prototype | Receives the fixed SetPixelFormat wire message. | fixed GS-friendly pixel format |
| choice | prototype parameter | src/rfb.h | pstvnc_rfb_choose_security_none | prototype | Receives the selected None security type on success. | Issue #7 security baseline |
| count | prototype parameter | src/rfb.h | pstvnc_rfb_choose_security_none | prototype | Gives the exact number of advertised security type bytes. | RFB wire contract |
| types | prototype parameter | src/rfb.h | pstvnc_rfb_choose_security_none | prototype | Supplies the server-advertised security type list. | RFB wire contract |
| banner | prototype parameter | src/rfb.h | pstvnc_rfb_parse_protocol_version | prototype | Supplies the exact server protocol banner bytes to validate. | RFB wire contract |
| major | prototype parameter | src/rfb.h | pstvnc_rfb_parse_protocol_version | prototype | Receives the parsed protocol major version. | RFB wire contract |
| minor | prototype parameter | src/rfb.h | pstvnc_rfb_parse_protocol_version | prototype | Receives the parsed protocol minor version. | RFB wire contract |
| bytes | prototype parameter | src/rfb.h | pstvnc_rfb_parse_server_init | prototype | Supplies the fixed-size ServerInit header bytes. | RFB wire contract |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_parse_server_init | prototype | Receives parsed server geometry and pixel-format metadata. | RFB wire contract |
| result | prototype parameter | src/rfb.h | pstvnc_rfb_security_result_ok | prototype | Supplies the four network-order SecurityResult bytes. | RFB wire contract |
| PSTVNC_RFB_POINTER_EVENT_SIZE | macro | src/rfb.h | RFB wire interface | public | Names the exact six-byte RFB PointerEvent message size. | Issue #38 pointer publication |
| PSTVNC_RFB_POINTER_BUTTON_LEFT | macro | src/rfb.h | RFB wire interface | public | Names native RFB button 1, conventionally the left pointer button. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_BUTTON_MIDDLE | macro | src/rfb.h | RFB wire interface | public | Names native RFB button 2, conventionally the middle pointer button. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_BUTTON_RIGHT | macro | src/rfb.h | RFB wire interface | public | Names native RFB button 3, conventionally the right pointer button. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_UP | macro | src/rfb.h | RFB wire interface | public | Names the adopted native RFB button-4 mask used for one wheel-up press. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_DOWN | macro | src/rfb.h | RFB wire interface | public | Names the adopted native RFB button-5 mask used for one wheel-down press. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_LEFT | macro | src/rfb.h | RFB wire interface | public | Names the adopted native RFB button-6 mask used for one wheel-left press. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_RIGHT | macro | src/rfb.h | RFB wire interface | public | Names the adopted native RFB button-7 mask used for one wheel-right press. | RFB PointerEvent vocabulary |
| pstvnc_rfb_build_pointer_event | function declaration | src/rfb.h | RFB wire interface | public | Declares pure construction of one already-mapped native RFB PointerEvent. | Issue #38 pointer publication |
| out | prototype parameter | src/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares destination storage for the exact six-byte PointerEvent. | Issue #38 pointer publication |
| button_mask | prototype parameter | src/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares the already-mapped native RFB button mask accepted by the wire helper. | Issue #38 pointer publication |
| x | prototype parameter | src/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares the logical remote pointer X coordinate. | Issue #38 pointer publication |
| y | prototype parameter | src/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares the logical remote pointer Y coordinate. | Issue #38 pointer publication |
| PSTVNC_RFB_IO_H | include guard | src/rfb_io.h | RFB transport interface | file | Prevents repeated inclusion of exact I/O seam declarations. | clean source interface |
| pstvnc_rfb_io_read_exact | function declaration | src/rfb_io.h | RFB transport seam | platform | Requires the transport to deliver an exact protocol byte count or fail. | RFB framing across TCP |
| socket_fd | parameter | src/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Identifies the connected socket from which exact bytes are required. | RFB framing across TCP |
| buffer | parameter | src/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Receives exactly the requested number of protocol bytes. | RFB framing across TCP |
| count | parameter | src/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Gives the exact byte count whose transfer defines success. | RFB framing across TCP |
| pstvnc_rfb_io_write_exact | function declaration | src/rfb_io.h | RFB transport seam | platform | Requires the transport to send an exact protocol byte count or fail. | RFB framing across TCP |
| socket_fd | parameter | src/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Identifies the connected socket to which exact bytes are written. | RFB framing across TCP |
| buffer | parameter | src/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Supplies exactly the protocol bytes that must be written. | RFB framing across TCP |
| count | parameter | src/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Gives the exact byte count whose transfer defines success. | RFB framing across TCP |
| pstvnc_rfb_io_poll_receive | function declaration | src/rfb_io.h | RFB transport seam | platform | Declares a nonblocking receive-prefetch check that reports whether at least one server byte is ready without advancing protocol parsing. | Issue #38 responsive RFB scheduling |
| socket_fd | prototype parameter | src/rfb_io.h | pstvnc_rfb_io_poll_receive | platform | Declares the synchronized RFB socket whose receive readiness is checked without blocking. | RFB exact I/O seam |
| PSTVNC_RFB_INITIAL_COVERAGE_BYTES | macro | src/rfb_session.c | RFB session | file | Sizes the bitmap used to prove unique initial-frame pixel coverage. | initial authoritative Raw frame proof |
| initial_frame_coverage | variable | src/rfb_session.c | RFB session | file | Stores temporary per-pixel initial-frame coverage bits for the single owned session. | initial authoritative Raw frame proof |
| read_be32 | function | src/rfb_session.c | RFB session parser | file | Decodes one four-byte big-endian field used by streamed server messages. | RFB server-message framing |
| bytes | parameter | src/rfb_session.c | read_be32 | local | Supplies four network-order bytes to decode. | RFB server-message framing |
| read_be16 | function | src/rfb_session.c | RFB session parser | file | Decodes one two-byte big-endian field used by streamed server messages. | RFB server-message framing |
| bytes | parameter | src/rfb_session.c | read_be16 | local | Supplies two network-order bytes to decode. | RFB server-message framing |
| read_exact | function | src/rfb_session.c | RFB session parser | file | Converts the platform exact-read convention into a boolean session helper. | RFB framing across TCP |
| socket_fd | parameter | src/rfb_session.c | read_exact | local | Identifies the synchronized session socket to read. | RFB framing across TCP |
| buffer | parameter | src/rfb_session.c | read_exact | local | Receives the exact requested bytes. | RFB framing across TCP |
| count | parameter | src/rfb_session.c | read_exact | local | Gives the exact protocol byte count required. | RFB framing across TCP |
| write_exact | function | src/rfb_session.c | RFB session parser | file | Converts the platform exact-write convention into a boolean session helper. | RFB framing across TCP |
| socket_fd | parameter | src/rfb_session.c | write_exact | local | Identifies the synchronized session socket to write. | RFB framing across TCP |
| buffer | parameter | src/rfb_session.c | write_exact | local | Supplies the exact protocol bytes to write. | RFB framing across TCP |
| count | parameter | src/rfb_session.c | write_exact | local | Gives the exact protocol byte count required. | RFB framing across TCP |
| read_bounded_text | function | src/rfb_session.c | RFB session parser | file | Retains bounded diagnostic text while consuming the server field to its exact boundary. | RFB server-message framing |
| socket_fd | parameter | src/rfb_session.c | read_bounded_text | local | Identifies the socket carrying the declared text field. | RFB server-message framing |
| length | parameter | src/rfb_session.c | read_bounded_text | local | Gives the complete server-declared field length that must be consumed. | RFB server-message framing |
| out | parameter | src/rfb_session.c | read_bounded_text | local | Receives the bounded retained prefix plus terminator. | RFB server-message framing |
| discard | variable | src/rfb_session.c | read_bounded_text | local | Provides bounded scratch for consuming text beyond retained storage. | RFB server-message framing |
| take | variable | src/rfb_session.c | read_bounded_text | local | Stores the number of declared text bytes retained locally. | RFB server-message framing |
| remaining | variable | src/rfb_session.c | read_bounded_text | local | Tracks declared text bytes still requiring stream consumption. | RFB server-message framing |
| chunk | variable | src/rfb_session.c | read_bounded_text | local | Bounds each discard read to scratch capacity. | RFB server-message framing |
| discard_exact | function | src/rfb_session.c | RFB session parser | file | Consumes exactly a declared server payload length without retaining its bytes. | RFB server-message framing |
| socket_fd | parameter | src/rfb_session.c | discard_exact | local | Identifies the synchronized session socket to consume. | RFB server-message framing |
| count | parameter | src/rfb_session.c | discard_exact | local | Tracks the exact number of bytes still requiring discard. | RFB server-message framing |
| discard | variable | src/rfb_session.c | discard_exact | local | Provides fixed scratch for bounded exact payload consumption. | RFB server-message framing |
| chunk | variable | src/rfb_session.c | discard_exact | local | Bounds each discard operation to scratch capacity. | RFB server-message framing |
| fail | function | src/rfb_session.c | RFB session failure policy | file | Publishes terminal session failure and its precise classification. | fail-closed session semantics |
| session | parameter | src/rfb_session.c | fail | local | Supplies the session whose terminal state is published. | fail-closed session semantics |
| error | parameter | src/rfb_session.c | fail | local | Supplies the precise terminal error classification. | fail-closed session semantics |
| fail_frame | function | src/rfb_session.c | RFB session failure policy | file | Revokes framebuffer authority before publishing a terminal decode/session failure. | authoritative framebuffer failure semantics |
| session | parameter | src/rfb_session.c | fail_frame | local | Supplies the session whose failure state is published. | fail-closed session semantics |
| framebuffer | parameter | src/rfb_session.c | fail_frame | local | Supplies framebuffer authority that must be revoked after possible partial mutation. | authoritative framebuffer failure semantics |
| error | parameter | src/rfb_session.c | fail_frame | local | Supplies the precise terminal error classification. | fail-closed session semantics |
| framebuffer_matches_session | function | src/rfb_session.c | RFB session parser | file | Verifies socket ownership and exact framebuffer/server geometry agreement. | RFB session authority boundary |
| session | parameter | src/rfb_session.c | framebuffer_matches_session | local | Supplies synchronized session state to compare against framebuffer state. | RFB session authority boundary |
| framebuffer | parameter | src/rfb_session.c | framebuffer_matches_session | local | Supplies framebuffer geometry and initialization state for agreement checks. | RFB session authority boundary |
| region_fits | function | src/rfb_session.c | RFB session parser | file | Validates server rectangle geometry using widened arithmetic. | RFB rectangle bounds |
| x | parameter | src/rfb_session.c | region_fits | local | Gives server rectangle left coordinate. | RFB rectangle bounds |
| y | parameter | src/rfb_session.c | region_fits | local | Gives server rectangle top coordinate. | RFB rectangle bounds |
| width | parameter | src/rfb_session.c | region_fits | local | Gives server rectangle width. | RFB rectangle bounds |
| height | parameter | src/rfb_session.c | region_fits | local | Gives server rectangle height. | RFB rectangle bounds |
| framebuffer_width | parameter | src/rfb_session.c | region_fits | local | Gives authoritative logical framebuffer width. | RFB rectangle bounds |
| framebuffer_height | parameter | src/rfb_session.c | region_fits | local | Gives authoritative logical framebuffer height. | RFB rectangle bounds |
| mark_initial_frame_coverage | function | src/rfb_session.c | initial frame proof | file | Rejects duplicate pixels and counts unique authority for one initial Raw rectangle. | initial authoritative Raw frame proof |
| framebuffer | parameter | src/rfb_session.c | mark_initial_frame_coverage | local | Supplies logical row stride used to map rectangles to pixel identities. | initial authoritative Raw frame proof |
| x | parameter | src/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle left coordinate. | initial authoritative Raw frame proof |
| y | parameter | src/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle top coordinate. | initial authoritative Raw frame proof |
| width | parameter | src/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle width. | initial authoritative Raw frame proof |
| height | parameter | src/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle height. | initial authoritative Raw frame proof |
| covered_pixels | parameter | src/rfb_session.c | mark_initial_frame_coverage | local | Receives the cumulative count of uniquely covered logical pixels. | initial authoritative Raw frame proof |
| row | variable | src/rfb_session.c | mark_initial_frame_coverage | local | Iterates over logical rows covered by the rectangle. | initial authoritative Raw frame proof |
| column | variable | src/rfb_session.c | mark_initial_frame_coverage | local | Iterates over logical columns covered by the rectangle. | initial authoritative Raw frame proof |
| pixel_index | variable | src/rfb_session.c | mark_initial_frame_coverage | local | Maps one logical pixel coordinate to its unique linear identity. | initial authoritative Raw frame proof |
| byte_index | variable | src/rfb_session.c | mark_initial_frame_coverage | local | Locates the coverage byte containing the current pixel bit. | initial authoritative Raw frame proof |
| bit | variable | src/rfb_session.c | mark_initial_frame_coverage | local | Selects the coverage bit representing the current logical pixel. | initial authoritative Raw frame proof |
| read_raw_row | function | src/rfb_session.c | Raw decoder | file | Reads and converts exactly one little-endian Raw pixel row into bounded scratch. | Raw-only Issue #7 baseline |
| session | parameter | src/rfb_session.c | read_raw_row | local | Supplies synchronized socket state and row scratch storage. | Raw decoder |
| width | parameter | src/rfb_session.c | read_raw_row | local | Gives the number of 16-bit Raw pixels expected in the row. | Raw decoder |
| bytes | variable | src/rfb_session.c | read_raw_row | local | Aliases row scratch as bytes for exact network receive. | Raw decoder |
| column | variable | src/rfb_session.c | read_raw_row | local | Iterates across received Raw row pixels during endian conversion. | Raw decoder |
| byte_count | variable | src/rfb_session.c | read_raw_row | local | Stores the exact two-bytes-per-pixel row payload length. | Raw decoder |
| receive_framebuffer_update | function | src/rfb_session.c | RFB session parser | file | Consumes asynchronous server messages until one complete Raw framebuffer update is resolved. | shared Raw server-message parser |
| session | parameter | src/rfb_session.c | receive_framebuffer_update | local | Supplies synchronized session state and transport ownership. | shared Raw server-message parser |
| framebuffer | parameter | src/rfb_session.c | receive_framebuffer_update | local | Supplies mutable authoritative framebuffer state. | shared Raw server-message parser |
| require_full | parameter | src/rfb_session.c | receive_framebuffer_update | local | Selects strict initial full-frame proof versus ordinary incremental semantics. | initial authoritative Raw frame proof |
| message_type | variable | src/rfb_session.c | receive_framebuffer_update | local | Holds the next server-to-client message discriminator. | RFB server-message framing |
| cut_header | variable | src/rfb_session.c | receive_framebuffer_update | local | Holds the fixed remainder of a ServerCutText header. | RFB server-message framing |
| text_length | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores declared ServerCutText payload length for exact discard. | RFB server-message framing |
| color_header | variable | src/rfb_session.c | receive_framebuffer_update | local | Holds the fixed remainder of a SetColorMapEntries header. | RFB server-message framing |
| color_count | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the declared color-map entry count. | RFB server-message framing |
| payload_length | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the exact color-map payload byte length to consume. | RFB server-message framing |
| update_header | variable | src/rfb_session.c | receive_framebuffer_update | local | Holds the fixed remainder of a FramebufferUpdate header. | RFB server-message framing |
| rectangle_count | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the number of rectangles declared by one framebuffer update. | Raw framebuffer update |
| rectangle_index | variable | src/rfb_session.c | receive_framebuffer_update | local | Iterates through declared framebuffer rectangles. | Raw framebuffer update |
| covered_pixels | variable | src/rfb_session.c | receive_framebuffer_update | local | Counts unique initial-frame pixels proven by the coverage bitmap. | initial authoritative Raw frame proof |
| total_pixel_bytes | variable | src/rfb_session.c | receive_framebuffer_update | local | Counts decoded Raw payload bytes for independent full-frame size proof. | initial authoritative Raw frame proof |
| framebuffer_pixels | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the exact logical framebuffer pixel count under test. | initial authoritative Raw frame proof |
| required_pixel_bytes | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the exact Raw byte total required for the complete logical desktop. | initial authoritative Raw frame proof |
| coverage_bytes | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the active initial-coverage bitmap length for current geometry. | initial authoritative Raw frame proof |
| rectangle_header | variable | src/rfb_session.c | receive_framebuffer_update | local | Holds one fixed-size RFB rectangle header. | Raw framebuffer update |
| x | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle left coordinate. | Raw framebuffer update |
| y | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle top coordinate. | Raw framebuffer update |
| width | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle width. | Raw framebuffer update |
| height | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle height. | Raw framebuffer update |
| encoding | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle encoding and must equal Raw. | Raw-only Issue #7 baseline |
| row | variable | src/rfb_session.c | receive_framebuffer_update | local | Iterates through Raw rectangle rows written into framebuffer storage. | Raw framebuffer update |
| pstvnc_rfb_session_init | function | src/rfb_session.c | RFB session | public | Establishes the new disconnected baseline for one synchronized session. | CLEAN_ARCHITECTURE: RFB client/session |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_init | local | Supplies the session object to reset to NEW state. | RFB session lifecycle |
| pstvnc_rfb_session_start | function | src/rfb_session.c | RFB session | public | Performs exact RFB 3.8/None handshake, validates geometry, configures Raw, and requests the first full desktop. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_start | local | Supplies session storage that becomes owner of synchronized protocol state. | RFB session lifecycle |
| socket_fd | parameter | src/rfb_session.c | pstvnc_rfb_session_start | local | Supplies the already-connected VNC socket descriptor. | RFB session lifecycle |
| expected_width | parameter | src/rfb_session.c | pstvnc_rfb_session_start | local | Gives the only acceptable server framebuffer width for this milestone. | fixed 704x462 baseline |
| expected_height | parameter | src/rfb_session.c | pstvnc_rfb_session_start | local | Gives the only acceptable server framebuffer height for this milestone. | fixed 704x462 baseline |
| banner | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the exact server and client protocol version banner. | RFB handshake |
| security_count | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Stores the server-advertised number of security types. | RFB handshake |
| security_types | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the complete one-byte security-type list allowed by RFB 3.8. | RFB handshake |
| security_choice | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Stores SecurityType None selected from the advertised list. | RFB handshake |
| security_result | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the exact four-byte server SecurityResult. | RFB handshake |
| shared_flag | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Stores the fixed shared ClientInit flag sent to the server. | RFB handshake |
| server_init_bytes | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the fixed ServerInit header before desktop-name consumption. | RFB handshake |
| message | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Provides bounded client-message scratch sized for the largest startup message. | RFB handshake |
| reason_length_bytes | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the four-byte rejection-reason length when no security types are offered. | RFB handshake failure |
| reason_length | variable | src/rfb_session.c | pstvnc_rfb_session_start | local | Stores decoded rejection-reason byte length for bounded exact consumption. | RFB handshake failure |
| pstvnc_rfb_session_request_update | function | src/rfb_session.c | RFB session | public | Sends one full-desktop update request only from a synchronized READY session. | Issue #7 live loop |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_request_update | local | Supplies the READY session and validated server geometry. | Issue #7 live loop |
| incremental | parameter | src/rfb_session.c | pstvnc_rfb_session_request_update | local | Selects ordinary incremental service versus a nonincremental request. | Issue #7 live loop |
| message | variable | src/rfb_session.c | pstvnc_rfb_session_request_update | local | Buffers the exact serialized FramebufferUpdateRequest. | Issue #7 live loop |
| pstvnc_rfb_session_receive_initial_frame | function | src/rfb_session.c | RFB session | public | Receives and proves the requested full initial Raw desktop before publishing READY. | initial authoritative Raw frame proof |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_receive_initial_frame | local | Supplies the session awaiting its first complete framebuffer. | initial authoritative Raw frame proof |
| framebuffer | parameter | src/rfb_session.c | pstvnc_rfb_session_receive_initial_frame | local | Supplies framebuffer storage whose authority remains revoked until proof succeeds. | initial authoritative Raw frame proof |
| pstvnc_rfb_session_receive_update | function | src/rfb_session.c | RFB session | public | Receives one ordinary update only when session and framebuffer authority are already valid. | Issue #7 live loop |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_receive_update | local | Supplies the synchronized READY session. | Issue #7 live loop |
| framebuffer | parameter | src/rfb_session.c | pstvnc_rfb_session_receive_update | local | Supplies the currently authoritative framebuffer to update or invalidate on failure. | authoritative framebuffer semantics |
| pstvnc_rfb_session_send_pointer_event | function | src/rfb_session.c | RFB session | public | Serializes and sends one already-mapped PointerEvent only through a valid READY session and in-bounds negotiated geometry. | Issue #38 main-thread RFB pointer publication |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Supplies the synchronized READY RFB session that exclusively owns socket transmission. | Issue #38 main-thread RFB pointer publication |
| button_mask | parameter | src/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Supplies an already-mapped native RFB button mask; semantic input mapping remains outside the session. | Issue #38 main-thread RFB pointer publication |
| x | parameter | src/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Gives the pointer X coordinate validated against negotiated server width before transmission. | Issue #38 main-thread RFB pointer publication |
| y | parameter | src/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Gives the pointer Y coordinate validated against negotiated server height before transmission. | Issue #38 main-thread RFB pointer publication |
| message | variable | src/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Buffers one exact serialized PointerEvent before the session's exact write. | Issue #38 main-thread RFB pointer publication |
| pstvnc_rfb_receive_update_result | enum | src/rfb_session.c | RFB session receive parser | file | Defines internal complete, idle, and failed outcomes for one framebuffer-update parser service attempt. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_RECEIVE_UPDATE_FAILED | enum value | src/rfb_session.c | pstvnc_rfb_receive_update_result | file | Marks an internal receive/parser failure that has already applied fail-closed session semantics. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_RECEIVE_UPDATE_COMPLETE | enum value | src/rfb_session.c | pstvnc_rfb_receive_update_result | file | Marks completion of one whole server FramebufferUpdate message. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_RECEIVE_UPDATE_IDLE | enum value | src/rfb_session.c | pstvnc_rfb_receive_update_result | file | Marks a benign yield reached only before the next server-message byte is consumed. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_receive_update_result_t | type | src/rfb_session.c | RFB session receive parser | file | Names the internal tri-state result used by blocking and responsive receive wrappers. | Issue #38 responsive RFB scheduling |
| allow_idle | parameter | src/rfb_session.c | receive_framebuffer_update | local | Selects whether the parser may return a benign idle result at a complete server-message boundary. | Issue #38 responsive RFB scheduling |
| receive_ready | variable | src/rfb_session.c | receive_framebuffer_update | local | Stores the transport readiness result checked before consuming the next server-message byte. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_session_try_receive_update | function | src/rfb_session.c | RFB session | public | Services live server input without blocking while idle, but never yields after beginning a server message. | Issue #38 responsive RFB scheduling |
| session | parameter | src/rfb_session.c | pstvnc_rfb_session_try_receive_update | local | Supplies the synchronized READY session whose server input is serviced. | Issue #38 responsive RFB scheduling |
| framebuffer | parameter | src/rfb_session.c | pstvnc_rfb_session_try_receive_update | local | Supplies valid authoritative framebuffer storage for any completed update. | Issue #38 responsive RFB scheduling |
| result | variable | src/rfb_session.c | pstvnc_rfb_session_try_receive_update | local | Holds the internal parser outcome translated into the public receive-service result. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_H | include guard | src/rfb_session.h | RFB session interface | file | Prevents repeated inclusion of RFB session state and lifecycle declarations. | clean source interface |
| PSTVNC_RFB_SESSION_TEXT_MAX | macro | src/rfb_session.h | RFB session interface | public | Bounds retained desktop and server-rejection diagnostic text. | bounded RFB session storage |
| PSTVNC_RFB_SESSION_MAX_ROW_PIXELS | macro | src/rfb_session.h | RFB session interface | public | Bounds the reusable Raw decode row scratch capacity. | bounded RFB session storage |
| PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS | macro | src/rfb_session.h | RFB session interface | public | Bounds the fixed Issue #7 initial-frame coverage proof to 704x462 pixels. | fixed 704x462 baseline |
| pstvnc_rfb_session_state | enum | src/rfb_session.h | RFB session interface | public | Defines the only lifecycle states of the synchronized clean session. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_NEW | enum value | src/rfb_session.h | pstvnc_rfb_session_state | public | Identifies an initialized session with no completed startup handshake. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME | enum value | src/rfb_session.h | pstvnc_rfb_session_state | public | Identifies a session whose handshake succeeded but initial authority is unproven. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_READY | enum value | src/rfb_session.h | pstvnc_rfb_session_state | public | Identifies a synchronized session with an authoritative initial framebuffer. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_FAILED | enum value | src/rfb_session.h | pstvnc_rfb_session_state | public | Identifies terminal session failure requiring coordinator convergence. | fail-closed session semantics |
| pstvnc_rfb_session_state_t | type | src/rfb_session.h | RFB session interface | public | Names the synchronized session lifecycle-state enum. | RFB session lifecycle |
| pstvnc_rfb_session_error | enum | src/rfb_session.h | RFB session interface | public | Defines precise fail-closed classifications for startup and framebuffer service. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_NONE | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Indicates no currently published session error. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_IO | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies failure to transfer an exact required protocol byte count. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_PROTOCOL_VERSION | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies malformed or unsupported server protocol version negotiation. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SERVER_REJECTED | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an RFB 3.8 server rejection before security selection. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SECURITY_NONE_UNAVAILABLE | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies absence of the required None security type. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SECURITY_RESULT | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies a nonzero SecurityResult after selecting None. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SERVER_INIT | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies invalid fixed ServerInit fields. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_GEOMETRY | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies server geometry that differs from the fixed expected desktop. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_SERVER_MESSAGE | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies a server message type not supported by the synchronized baseline parser. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_EMPTY_UPDATE | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an empty update where the initial full desktop is mandatory. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_ENCODING | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies any framebuffer rectangle encoding other than Raw. | Raw-only Issue #7 baseline |
| PSTVNC_RFB_SESSION_ERROR_RECTANGLE_BOUNDS | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an out-of-bounds or unwritable framebuffer rectangle. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_SIZE | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an initial Raw byte total that differs from the exact desktop size. | initial authoritative Raw frame proof |
| PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies duplicate, missing, excessive, or otherwise unproven initial pixel coverage. | initial authoritative Raw frame proof |
| PSTVNC_RFB_SESSION_ERROR_ROW_WIDTH | enum value | src/rfb_session.h | pstvnc_rfb_session_error | public | Classifies framebuffer width that exceeds bounded Raw row scratch. | bounded RFB session storage |
| pstvnc_rfb_session_error_t | type | src/rfb_session.h | RFB session interface | public | Names the precise synchronized-session error enum. | failure classification |
| pstvnc_rfb_session | structure | src/rfb_session.h | RFB session interface | public | Defines owned handshake state, errors, parsed server data, and bounded scratch. | CLEAN_ARCHITECTURE: RFB client/session |
| socket_fd | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores the connected socket descriptor owned by the synchronized session. | RFB session ownership |
| state | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores current synchronized lifecycle state. | RFB session lifecycle |
| error | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores the current precise failure classification. | failure classification |
| server_major | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores parsed server protocol major version. | RFB handshake |
| server_minor | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores parsed server protocol minor version. | RFB handshake |
| server_init | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores parsed server geometry and pixel-format metadata. | RFB handshake |
| desktop_name | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores a bounded retained server desktop name for diagnostics. | bounded RFB session storage |
| server_rejection | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores a bounded retained RFB server rejection reason. | bounded RFB session storage |
| row_scratch | field | src/rfb_session.h | pstvnc_rfb_session | public | Stores one bounded Raw row used during exact stream decoding. | Raw decoder |
| pstvnc_rfb_session_t | type | src/rfb_session.h | RFB session interface | public | Names the complete synchronized RFB session value. | CLEAN_ARCHITECTURE: RFB client/session |
| pstvnc_rfb_session_init | function declaration | src/rfb_session.h | RFB session interface | public | Declares reset to the NEW disconnected session baseline. | RFB session lifecycle |
| pstvnc_rfb_session_start | function declaration | src/rfb_session.h | RFB session interface | public | Declares fixed RFB startup and initial full-frame request. | RFB session lifecycle |
| pstvnc_rfb_session_request_update | function declaration | src/rfb_session.h | RFB session interface | public | Declares synchronized full-desktop update requests. | Issue #7 live loop |
| pstvnc_rfb_session_receive_initial_frame | function declaration | src/rfb_session.h | RFB session interface | public | Declares strict initial-frame receive and authority publication. | initial authoritative Raw frame proof |
| pstvnc_rfb_session_receive_update | function declaration | src/rfb_session.h | RFB session interface | public | Declares ordinary update receive against an already authoritative framebuffer. | Issue #7 live loop |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_init | prototype | Supplies the session object to reset to NEW state. | RFB session lifecycle |
| framebuffer | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_receive_initial_frame | prototype | Supplies framebuffer storage whose authority remains revoked until proof succeeds. | initial authoritative Raw frame proof |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_receive_initial_frame | prototype | Supplies the session awaiting its first complete framebuffer. | initial authoritative Raw frame proof |
| framebuffer | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_receive_update | prototype | Supplies the currently authoritative framebuffer to update or invalidate on failure. | authoritative framebuffer semantics |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_receive_update | prototype | Supplies the synchronized READY session. | Issue #7 live loop |
| incremental | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_request_update | prototype | Selects ordinary incremental service versus a nonincremental request. | Issue #7 live loop |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_request_update | prototype | Supplies the READY session and validated server geometry. | Issue #7 live loop |
| expected_height | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_start | prototype | Gives the only acceptable server framebuffer height for this milestone. | fixed 704x462 baseline |
| expected_width | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_start | prototype | Gives the only acceptable server framebuffer width for this milestone. | fixed 704x462 baseline |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_start | prototype | Supplies session storage that becomes owner of synchronized protocol state. | RFB session lifecycle |
| socket_fd | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_start | prototype | Supplies the already-connected VNC socket descriptor. | RFB session lifecycle |
| pstvnc_rfb_session_send_pointer_event | function declaration | src/rfb_session.h | RFB session interface | public | Declares READY-session transmission of one already-mapped native RFB PointerEvent. | Issue #38 main-thread RFB pointer publication |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the synchronized READY session that owns PointerEvent transmission. | Issue #38 main-thread RFB pointer publication |
| button_mask | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the already-mapped native RFB button mask supplied by application routing. | Issue #38 main-thread RFB pointer publication |
| x | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the logical remote pointer X coordinate to validate and publish. | Issue #38 main-thread RFB pointer publication |
| y | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the logical remote pointer Y coordinate to validate and publish. | Issue #38 main-thread RFB pointer publication |
| pstvnc_rfb_session_receive_result | enum | src/rfb_session.h | RFB session interface | public | Defines failed, idle, and completed-update outcomes for responsive live receive service. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_RECEIVE_FAILED | enum value | src/rfb_session.h | pstvnc_rfb_session_receive_result | public | Reports that responsive receive service failed and ordinary session failure handling applies. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_RECEIVE_IDLE | enum value | src/rfb_session.h | pstvnc_rfb_session_receive_result | public | Reports that no next server-message byte is currently available and the READY session remains valid. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_RECEIVE_UPDATE | enum value | src/rfb_session.h | pstvnc_rfb_session_receive_result | public | Reports successful consumption of one complete server FramebufferUpdate. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_session_receive_result_t | type | src/rfb_session.h | RFB session interface | public | Names the public tri-state outcome of one responsive receive-service attempt. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_session_try_receive_update | function declaration | src/rfb_session.h | RFB session interface | public | Declares nonblocking-at-idle live receive service with server-message-boundary yield semantics. | Issue #38 responsive RFB scheduling |
| session | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_try_receive_update | public | Declares the READY synchronized RFB session to service. | Issue #38 responsive RFB scheduling |
| framebuffer | prototype parameter | src/rfb_session.h | pstvnc_rfb_session_try_receive_update | public | Declares authoritative framebuffer storage for a completed server update. | Issue #38 responsive RFB scheduling |
| PSTVNC_APP_CONTROLLER_PORT | macro | src/app.c | application coordinator | file | Names the direct-libpad controller port owned by the Stage 1 input runtime. | Issue #38 controller input integration |
| PSTVNC_APP_CONTROLLER_SLOT | macro | src/app.c | application coordinator | file | Names the direct-libpad controller slot owned by the Stage 1 input runtime. | Issue #38 controller input integration |
| PSTVNC_APP_IDLE_POLL_DELAY_US | macro | src/app.c | application coordinator | file | Defines the short cooperative delay used when responsive RFB receive polling reports no server message ready. | Issue #38 responsive RFB scheduling |
| app_published_pointer_state | structure | src/app.c | application input publication | file | Groups the pointer coordinates and ordinary semantic click state last proven published to the remote RFB server. | Issue #38 published-state authority |
| app_published_pointer_state_t | type | src/app.c | application input publication | file | Names the application-owned record of successfully published remote pointer authority. | Issue #38 published-state authority |
| cursor_x | field | src/app.c | app_published_pointer_state | file | Stores the horizontal pointer coordinate last successfully published to the RFB server. | Issue #38 published-state authority |
| cursor_y | field | src/app.c | app_published_pointer_state | file | Stores the vertical pointer coordinate last successfully published to the RFB server. | Issue #38 published-state authority |
| click_buttons | field | src/app.c | app_published_pointer_state | file | Stores the ordinary semantic click buttons last successfully published to the RFB server. | Issue #38 published-state authority |
| map_semantic_clicks_to_rfb_buttons | function | src/app.c | application input publication | file | Maps project semantic left and right click state into the native RFB PointerEvent button mask. | Issue #38 RFB input serialization |
| semantic_clicks | parameter | src/app.c | map_semantic_clicks_to_rfb_buttons | local | Supplies the project semantic ordinary-click mask to translate into native RFB button bits. | Issue #38 RFB input serialization |
| rfb_buttons | parameter | src/app.c | map_semantic_clicks_to_rfb_buttons | local | Receives the native RFB ordinary-button mask produced from semantic click state. | Issue #38 RFB input serialization |
| mapped | variable | src/app.c | map_semantic_clicks_to_rfb_buttons | local | Accumulates the native RFB button bits corresponding to the supplied semantic click state. | Issue #38 RFB input serialization |
| map_wheel_direction_to_rfb_button | function | src/app.c | application input publication | file | Maps one semantic wheel direction to the corresponding native RFB transient wheel-button bit. | Issue #38 RFB input serialization |
| direction | parameter | src/app.c | map_wheel_direction_to_rfb_button | local | Supplies the semantic wheel direction whose native RFB button bit is requested. | Issue #38 RFB input serialization |
| rfb_button | parameter | src/app.c | map_wheel_direction_to_rfb_button | local | Receives the native RFB transient wheel-button bit for the supplied semantic direction. | Issue #38 RFB input serialization |
| publish_semantic_mouse_update | function | src/app.c | application input publication | file | Serializes one semantic mouse update as native RFB PointerEvents and advances published pointer authority only after exact send success. | Issue #38 published-state authority |
| session | parameter | src/app.c | publish_semantic_mouse_update | local | Supplies the main-thread-owned synchronized RFB session used to publish the semantic mouse update. | Issue #38 RFB input serialization |
| mouse_update | parameter | src/app.c | publish_semantic_mouse_update | local | Supplies one queued semantic mouse update for native RFB publication. | Issue #38 semantic input pipeline |
| published_pointer | parameter | src/app.c | publish_semantic_mouse_update | local | Supplies mutable application ownership of the last successfully published remote pointer state. | Issue #38 published-state authority |
| ordinary_rfb_buttons | variable | src/app.c | publish_semantic_mouse_update | local | Stores the native RFB ordinary-button mask derived from the semantic click state being published. | Issue #38 RFB input serialization |
| wheel_rfb_button | variable | src/app.c | publish_semantic_mouse_update | local | Stores the transient native RFB wheel-button bit used for one press-and-release wheel pulse. | Issue #38 RFB input serialization |
| service_semantic_input_events | function | src/app.c | application input publication | file | Drains queued controller-derived semantic events on the main thread and publishes each through the synchronized RFB session. | Issue #38 semantic input pipeline |
| input_runtime | parameter | src/app.c | service_semantic_input_events | local | Supplies the controller/input runtime whose bounded semantic queue is consumed by the main thread. | Issue #38 semantic input pipeline |
| session | parameter | src/app.c | service_semantic_input_events | local | Supplies the sole main-thread-owned RFB session through which queued semantic input effects are published. | Issue #38 RFB ownership |
| published_pointer | parameter | src/app.c | service_semantic_input_events | local | Supplies mutable remote pointer publication authority shared across drained semantic input events. | Issue #38 published-state authority |
| event | variable | src/app.c | service_semantic_input_events | local | Holds one semantic input event removed from the bounded runtime queue for main-thread processing. | Issue #38 semantic input pipeline |
| pop_result | variable | src/app.c | service_semantic_input_events | local | Stores the semantic-queue pop result used to distinguish an event, an empty queue, and runtime failure. | Issue #38 semantic input pipeline |
| input_ready | variable | src/app.c | pstvnc_app_run | local | Holds the fixed diagnostic stage record emitted after the controller/input runtime has successfully started. | Issue #38 diagnostics |
| input_runtime | variable | src/app.c | pstvnc_app_run | local | Owns the persistent controller producer, mouse interpreter, semantic queue, and handoff state for the live application lifetime. | Issue #38 controller input owner |
| published_pointer | variable | src/app.c | pstvnc_app_run | local | Tracks the remote pointer coordinates and ordinary clicks last proven published successfully to the VNC server. | Issue #38 published-state authority |
| input_runtime_ready | variable | src/app.c | pstvnc_app_run | local | Tracks whether the controller/input runtime was started and therefore requires cooperative shutdown during cleanup. | Issue #38 lifecycle |
| receive_result | variable | src/app.c | pstvnc_app_run | local | Stores the responsive RFB receive-service result used to distinguish idle polling, a completed update, and session failure. | Issue #38 responsive RFB scheduling |
