# Clean symbols — `tests`

DIRECTORY=tests
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the host-side Make graph that compiles and runs clean-source
unit tests. Source-level test symbols remain owned by `tests/unit/SYMBOLS.md`;
this dictionary records only the project-defined Make variables and targets in
`tests/Makefile`.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| COMMON_CFLAGS | variable | tests/Makefile | host unit build | build | Defines the strict common C99 warning, optimization, and error-policy flags used by the ordinary host unit-test binaries. | host unit compilation |
| BUILD_DIR | variable | tests/Makefile | host unit build | build | Names the disposable `.build` directory that receives ordinary host unit-test executables. | host unit artifacts |
| GEOMETRY_TEST | variable | tests/Makefile | geometry unit build | build | Names the host executable produced for the video geometry and mode integration test. | video geometry unit test |
| TRANSPORT_RFB_CHANNEL_TEST | variable | tests/Makefile | transport unit build | build | Names the host executable for backend-independent logical RFB channel storage tests. | A001 transport logical RFB storage |
| unit | target | tests/Makefile | host unit build | build | Builds and runs the ordinary host unit-test executables and also requires the separate geometry and transport logical-RFB unit targets. | aggregate host unit suite |
| $(BUILD_DIR) | target | tests/Makefile | host unit build | build | Creates the disposable directory that receives host unit-test binaries. | host unit artifacts |
| $(BUILD_DIR)/config_text_test | target | tests/Makefile | host unit build | build | Compiles the configuration-text unit test with the clean configuration text implementation. | configuration text |
| $(BUILD_DIR)/video_mode_test | target | tests/Makefile | host unit build | build | Compiles the video-mode unit test with the clean mode implementation and host gsKit compatibility header. | video modes |
| $(BUILD_DIR)/rfb_wire_test | target | tests/Makefile | host unit build | build | Compiles the host test for clean RFB wire-format parsing and serialization behavior. | RFB wire protocol |
| $(BUILD_DIR)/framebuffer_test | target | tests/Makefile | host unit build | build | Compiles the host test for clean framebuffer initialization, geometry, validity, and update semantics. | framebuffer semantics |
| $(BUILD_DIR)/rfb_session_test | target | tests/Makefile | host unit build | build | Compiles the general clean RFB-session host test with its RFB and framebuffer dependencies. | RFB session |
| $(BUILD_DIR)/rfb_initial_frame_test | target | tests/Makefile | host unit build | build | Compiles the host test for establishing and consuming the initial authoritative RFB framebuffer update. | initial RFB frame |
| $(BUILD_DIR)/rfb_initial_coverage_test | target | tests/Makefile | host unit build | build | Compiles the host regression test covering initial-session RFB message handling and framebuffer authority cases. | initial RFB coverage |
| $(BUILD_DIR)/rfb_async_framing_test | target | tests/Makefile | host unit build | build | Compiles the live-session asynchronous RFB framing and fail-closed regression test. | RFB asynchronous framing |
| $(BUILD_DIR)/display_test | target | tests/Makefile | host unit build | build | Compiles the clean display-domain host test with the framebuffer implementation it presents. | display conversion |
| $(BUILD_DIR)/app_test | target | tests/Makefile | host unit build | build | Compiles the application-orchestration host test against the clean app implementation and stubbed subsystem interfaces. | application orchestration |
| clean | target | tests/Makefile | host unit build | build | Removes the disposable host unit-test build directory and every test executable beneath it. | test build hygiene |
| geometry-unit | target | tests/Makefile | geometry unit build | build | Builds and executes the dedicated video geometry test executable. | video geometry unit test |
| $(GEOMETRY_TEST) | target | tests/Makefile | geometry unit build | build | Compiles the video geometry test with clean geometry and mode implementations using the host gsKit compatibility header. | video geometry |
| transport-rfb-channel-unit | target | tests/Makefile | transport unit build | build | Builds and executes the backend-independent logical RFB channel regression test. | A001 transport logical RFB storage |
| $(TRANSPORT_RFB_CHANNEL_TEST) | target | tests/Makefile | transport unit build | build | Compiles the logical RFB channel test directly with `src/transport/rfb_channel.c`. | A001 transport logical RFB storage |
