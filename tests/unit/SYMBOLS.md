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
| EV_DELAY | enum value | tests/unit/app_test.c | application coordinator test harness | file | Records invocation of the host stub for coordinator-owned reconnect backoff delay. | explicit connection-loss recovery |
| request_errors | variable | tests/unit/app_test.c | scripted RFB request fixture | file | Supplies the RFB error classification associated with each scripted failed update request. | explicit connection-loss recovery |
| pstvnc_ps2_system_delay_ms | function | tests/unit/app_test.c | PS2 system test stub | test seam | Records reconnect-delay requests without sleeping during deterministic host application tests. | explicit connection-loss recovery |
| milliseconds | parameter | tests/unit/app_test.c | pstvnc_ps2_system_delay_ms | local | Receives the coordinator-selected delay ignored by the deterministic host stub. | host test seam |
| index | variable | tests/unit/app_test.c | pstvnc_rfb_session_request_update | local | Captures the scripted request position before advancing the deterministic request fixture. | application coordinator test fixture |
| test_io_loss_reconnects_only_rfb_session | function | tests/unit/app_test.c | application coordinator test | file | Proves explicit RFB I/O loss replaces only transport/session state while preserving initialized network and graphics lifetime. | explicit connection-loss recovery |
