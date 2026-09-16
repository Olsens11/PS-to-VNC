# Clean symbols — `tests/unit`

DIRECTORY=tests/unit
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This directory owns host-only unit and failure-path tests for clean-generation
source. Existing pre-policy tests enter the dictionary as the retrofit reaches
them; newly written tests are indexed immediately.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| failures | variable | tests/unit/rfb_async_framing_test.c | test harness | file | Counts failed assertions in the live RFB asynchronous-framing unit test. | RFB client/session framing |
| script_reset | function | tests/unit/rfb_async_framing_test.c | scripted transport | file | Clears the deterministic scripted input stream before each framing case. | RFB client/session framing |
| append_input | function | tests/unit/rfb_async_framing_test.c | scripted transport | file | Appends exact server bytes to the bounded deterministic input fixture. | RFB client/session framing |
| pstvnc_rfb_io_read_exact | function | tests/unit/rfb_async_framing_test.c | RFB transport stub | test seam | Implements exact scripted reads and reports truncation as an I/O failure. | RFB client/session framing |
| pstvnc_rfb_io_write_exact | function | tests/unit/rfb_async_framing_test.c | RFB transport stub | test seam | Accepts outbound client writes because these cases exercise receive framing only. | RFB client/session framing |
| prepare_live | function | tests/unit/rfb_async_framing_test.c | test fixture | file | Establishes a READY session and valid 4x3 authoritative framebuffer baseline. | authoritative framebuffer semantics |
| check_io_failure | function | tests/unit/rfb_async_framing_test.c | failure assertion helper | file | Verifies truncated asynchronous messages fail the session and invalidate framebuffer authority. | RFB client/session failure behavior |
| test_server_cut_text_truncation | function | tests/unit/rfb_async_framing_test.c | framing test | file | Proves truncated ServerCutText headers and payloads fail closed on a live stream. | RFB client/session framing |
| test_color_map_truncation | function | tests/unit/rfb_async_framing_test.c | framing test | file | Proves truncated SetColorMapEntries headers and payloads fail closed on a live stream. | RFB client/session framing |
| test_unsupported_message_invalidates | function | tests/unit/rfb_async_framing_test.c | framing test | file | Proves an unsupported live server-message type destroys framebuffer authority. | RFB client/session failure behavior |
| test_bell_then_empty_update_preserves_authority | function | tests/unit/rfb_async_framing_test.c | framing test | file | Proves Bell is consumed to the next update boundary without invalidating a valid framebuffer. | RFB client/session framing |
| main | function | tests/unit/rfb_async_framing_test.c | test runner | process | Runs all asynchronous-framing cases and returns nonzero if any assertion fails. | host unit test |
| failures | variable | tests/unit/transport_rfb_channel_test.c | test harness | file | Counts failed logical-RFB channel assertions. | A001 transport logical RFB storage |
| CHECK | macro | tests/unit/transport_rfb_channel_test.c | test harness | file | Records an assertion failure with source location while allowing the remaining channel cases to run. | A001 transport logical RFB storage |
| test_initialize_and_capacity_rejection | function | tests/unit/transport_rfb_channel_test.c | channel test | file | Verifies empty initialization, committed producer activity, and fail-closed over-capacity commit without state mutation. | A001 transport logical RFB storage |
| test_incremental_wraparound_consumption | function | tests/unit/transport_rfb_channel_test.c | channel test | file | Verifies incremental parser consumption across circular-buffer wraparound and producer generation advancement. | A001 transport logical RFB storage |
| test_exact_read_is_atomic_on_short_input | function | tests/unit/transport_rfb_channel_test.c | channel test | file | Verifies an insufficient exact read leaves committed bytes and output authority unchanged. | A001 transport logical RFB storage |
| test_terminal_residual_discard_is_distinct | function | tests/unit/transport_rfb_channel_test.c | channel test | file | Verifies terminal residual discard requires the exact snapshot, empties the queue, and does not manufacture producer activity. | A001 transport RFB quiescence |
| main | function | tests/unit/transport_rfb_channel_test.c | test runner | process | Runs the logical RFB channel regression cases and returns nonzero on any failed assertion. | A001 transport logical RFB storage |
