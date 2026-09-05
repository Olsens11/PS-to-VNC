# Clean symbols — `src`

DIRECTORY=src
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the application coordinator, authoritative framebuffer,
minimal diagnostics transport, display conversion, direct libpad pad
acquisition, ordinary semantic input-event FIFO, pure remote-mouse response
policy, and RFB wire/session core.
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
| pstvnc_mouse_reset_derived | function | src/mouse.c | mouse state | public | Declares the hard-boundary reset that preserves remote cursor and click state while forgetting controller-derived response history. | Issue #38: pure mouse boundary |
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
| pstvnc_mouse | structure | src/mouse.h | mouse interface | public | Defines persistent pure mouse state including cursor authority and controller-derived response history. | Issue #38: pure mouse boundary |
| width | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative logical desktop width used for cursor clamping. | Issue #38: pure mouse boundary |
| height | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative logical desktop height used for cursor clamping. | Issue #38: pure mouse boundary |
| cursor_x | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative logical remote cursor X position. | Issue #38: pure mouse boundary |
| cursor_y | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative logical remote cursor Y position. | Issue #38: pure mouse boundary |
| click_buttons | field | src/mouse.h | pstvnc_mouse | public | Stores the authoritative currently published ordinary remote mouse-button state. | Issue #38: pure mouse boundary |
| wheel_mode_enabled | field | src/mouse.h | pstvnc_mouse | public | Records whether the Issue 38 L3 toggle currently routes the left stick to analog wheel behavior. | Issue #38: L3 click-to-toggle analog wheel mode |
| dpad_hold_direction | field | src/mouse.h | pstvnc_mouse | public | Stores the exact D-pad direction combination currently owning one pointer-hold gesture. | Issue #38: pure mouse boundary |
| dpad_hold_ticks | field | src/mouse.h | pstvnc_mouse | public | Counts approximately 60 Hz samples in the current D-pad hold gesture. | Issue #38: pure mouse boundary |
| dpad_x_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 horizontal D-pad pointer motion across samples. | Issue #38: pure mouse boundary |
| dpad_y_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 vertical D-pad pointer motion across samples. | Issue #38: pure mouse boundary |
| analog_x_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 horizontal analog pointer motion across samples. | Issue #38: pure mouse boundary |
| analog_y_q8 | field | src/mouse.h | pstvnc_mouse | public | Accumulates fractional Q8 vertical analog pointer motion across samples. | Issue #38: pure mouse boundary |
| last_wheel_direction | field | src/mouse.h | pstvnc_mouse | public | Stores the prior active wheel direction used to distinguish a fresh direction from held repeat. | Issue #38: pure mouse boundary |
| wheel_repeat_countdown | field | src/mouse.h | pstvnc_mouse | public | Stores the remaining sample countdown before a held wheel direction may emit another notch. | Issue #38: pure mouse boundary |
| pstvnc_mouse_t | type | src/mouse.h | mouse interface | public | Names the persistent pure remote-mouse state object. | Issue #38: pure mouse boundary |
| pstvnc_mouse_init | function declaration | src/mouse.h | mouse interface | public | Declares initialization of pure mouse state for the current logical remote desktop. | Issue #38: pure mouse boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_init | prototype | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| width | prototype parameter | src/mouse.h | pstvnc_mouse_init | prototype | Supplies the logical remote desktop width in pixels. | Issue #38: pure mouse boundary |
| height | prototype parameter | src/mouse.h | pstvnc_mouse_init | prototype | Supplies the logical remote desktop height in pixels. | Issue #38: pure mouse boundary |
| pstvnc_mouse_reset_derived | function declaration | src/mouse.h | mouse interface | public | Declares the hard-boundary reset that preserves remote cursor and click state while forgetting controller-derived response history. | Issue #38: pure mouse boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_reset_derived | prototype | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| pstvnc_mouse_update | function declaration | src/mouse.h | mouse interface | public | Declares advancement of pure semantic mouse state from one already-routed input sample. | Issue #38: pure mouse boundary |
| mouse | prototype parameter | src/mouse.h | pstvnc_mouse_update | prototype | Supplies the persistent pure mouse state operated on by this function. | Issue #38: pure mouse boundary |
| input | prototype parameter | src/mouse.h | pstvnc_mouse_update | prototype | Supplies one already-routed mouse-domain input sample. | Issue #38: pure mouse boundary |
| update | prototype parameter | src/mouse.h | pstvnc_mouse_update | prototype | Receives the semantic mouse result produced for this sample. | Issue #38: pure mouse boundary |
| libpad_ready | variable | src/pad.c | pad lifecycle | file static | Records whether PS-to-VNC currently owns initialized process-wide libpad state. | ADR 0002: direct libpad ownership |
| pad_state_is_readable | function | src/pad.c | pad acquisition | file | Reports whether one native libpad state permits ordinary controller observation. | libpad state machine |
| state | parameter | src/pad.c | pad_state_is_readable | function | Supplies the native PAD_STATE_* value to classify. | libpad state machine |
| wait_for_pad_settle | function | src/pad.c | pad acquisition | file | Waits through transient libpad command/search states until the endpoint is readable, disconnected, or failed. | OPL/PS2SDK pad practice |
| pad | parameter | src/pad.c | wait_for_pad_settle | function | Supplies the owned endpoint whose native libpad state is observed. | pad lifecycle |
| state | variable | src/pad.c | wait_for_pad_settle | local | Stores each native libpad state observed while waiting for a settled endpoint. | libpad state machine |
| invalidate_observation | function | src/pad.c | physical pad history | file | Clears the latest sample and immediate history so an ownership boundary cannot manufacture input edges. | stale-state invalidation |
| pad | parameter | src/pad.c | invalidate_observation | function | Supplies the pad instance whose physical observation continuity is revoked. | stale-state invalidation |
| configure_controller_mode | function | src/pad.c | pad acquisition | file | Detects DualShock capability and deterministically requests locked analog mode when the controller supports it. | ADR 0002; OPL pad practice |
| pad | parameter | src/pad.c | configure_controller_mode | function | Supplies the connected endpoint whose native libpad operating mode is configured. | controller mode negotiation |
| state | variable | src/pad.c | configure_controller_mode | local | Stores settled native libpad state before and after any mode request. | controller mode negotiation |
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
| PSTVNC_PAD_H | include-guard macro | src/pad.h | pad interface | header | Prevents repeated inclusion of the direct-libpad PS-to-VNC pad interface. | clean source interface |
| pstvnc_pad | structure | src/pad.h | pad interface | public | Defines one PS-to-VNC-owned libpad endpoint plus only its immediate physical observation state. | ADR 0002: pad boundary |
| port | field | src/pad.h | pstvnc_pad | public | Stores the native libpad port identifying this physical endpoint. | libpad port/slot addressing |
| slot | field | src/pad.h | pstvnc_pad | public | Stores the native libpad slot identifying this physical endpoint. | libpad port/slot addressing |
| state | field | src/pad.h | pstvnc_pad | public | Stores the latest native PAD_STATE_* value observed for this endpoint. | libpad state machine |
| opened | field | src/pad.h | pstvnc_pad | public | Records whether this instance currently owns an opened libpad port/slot endpoint. | pad lifecycle |
| connection_configured | field | src/pad.h | pstvnc_pad | public | Records whether mode negotiation has completed for the current physical connection epoch. | controller mode negotiation |
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
| PSTVNC_RFB_IO_H | include guard | src/rfb_io.h | RFB transport interface | file | Prevents repeated inclusion of exact I/O seam declarations. | clean source interface |
| pstvnc_rfb_io_read_exact | function declaration | src/rfb_io.h | RFB transport seam | platform | Requires the transport to deliver an exact protocol byte count or fail. | RFB framing across TCP |
| socket_fd | parameter | src/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Identifies the connected socket from which exact bytes are required. | RFB framing across TCP |
| buffer | parameter | src/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Receives exactly the requested number of protocol bytes. | RFB framing across TCP |
| count | parameter | src/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Gives the exact byte count whose transfer defines success. | RFB framing across TCP |
| pstvnc_rfb_io_write_exact | function declaration | src/rfb_io.h | RFB transport seam | platform | Requires the transport to send an exact protocol byte count or fail. | RFB framing across TCP |
| socket_fd | parameter | src/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Identifies the connected socket to which exact bytes are written. | RFB framing across TCP |
| buffer | parameter | src/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Supplies exactly the protocol bytes that must be written. | RFB framing across TCP |
| count | parameter | src/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Gives the exact byte count whose transfer defines success. | RFB framing across TCP |
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
