CC ?= cc
CFLAGS ?=

COMMON_CFLAGS = -O2 -std=c99 -Wall -Wextra -Werror -pedantic -I../src -I../src/input -I../src/ui -I../src/rfb -I../src/framebuffer -I../src/display -I../src/diagnostics -I../src/platform
BUILD_DIR = .build

.PHONY: unit clean

unit: \
	$(BUILD_DIR)/config_text_test \
	$(BUILD_DIR)/video_mode_test \
	$(BUILD_DIR)/rfb_wire_test \
	$(BUILD_DIR)/framebuffer_test \
	$(BUILD_DIR)/rfb_session_test \
	$(BUILD_DIR)/rfb_initial_frame_test \
	$(BUILD_DIR)/rfb_initial_coverage_test \
	$(BUILD_DIR)/rfb_async_framing_test \
	$(BUILD_DIR)/display_test \
	$(BUILD_DIR)/mouse_test \
	$(BUILD_DIR)/keyboard_test \
	$(BUILD_DIR)/local_ui_test \
	$(BUILD_DIR)/osk_test \
	$(BUILD_DIR)/osk_render_test \
	$(BUILD_DIR)/local_ui_presentation_test \
	$(BUILD_DIR)/input_test \
	$(BUILD_DIR)/controller_event_test \
	$(BUILD_DIR)/local_controller_test \
	$(BUILD_DIR)/osk_direct_test \
	$(BUILD_DIR)/app_test
	./$(BUILD_DIR)/config_text_test
	./$(BUILD_DIR)/video_mode_test
	./$(BUILD_DIR)/rfb_wire_test
	./$(BUILD_DIR)/framebuffer_test
	./$(BUILD_DIR)/rfb_session_test
	./$(BUILD_DIR)/rfb_initial_frame_test
	./$(BUILD_DIR)/rfb_initial_coverage_test
	./$(BUILD_DIR)/rfb_async_framing_test
	./$(BUILD_DIR)/display_test
	./$(BUILD_DIR)/mouse_test
	./$(BUILD_DIR)/keyboard_test
	./$(BUILD_DIR)/local_ui_test
	./$(BUILD_DIR)/osk_test
	./$(BUILD_DIR)/osk_render_test
	./$(BUILD_DIR)/local_ui_presentation_test
	./$(BUILD_DIR)/input_test
	./$(BUILD_DIR)/controller_event_test
	./$(BUILD_DIR)/local_controller_test
	./$(BUILD_DIR)/osk_direct_test
	./$(BUILD_DIR)/app_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/config_text_test: \
	unit/config_text_test.c \
	../src/config/text.c \
	../src/config/text.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/config_text_test.c \
		../src/config/text.c \
		-o $@

$(BUILD_DIR)/video_mode_test: \
	unit/video_mode_test.c \
	../src/video/mode.c \
	../src/video/mode.h \
	support/gsKit.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Isupport \
		-I../src \
		unit/video_mode_test.c \
		../src/video/mode.c \
		-o $@

$(BUILD_DIR)/rfb_wire_test: \
	unit/rfb_wire_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_wire_test.c \
		../src/rfb/rfb.c \
		-o $@

$(BUILD_DIR)/framebuffer_test: \
	unit/framebuffer_test.c \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/framebuffer_test.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_session_test: \
	unit/rfb_session_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/rfb_io.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_session_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_initial_frame_test: \
	unit/rfb_initial_frame_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/rfb_io.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_initial_frame_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_initial_coverage_test: \
	unit/rfb_initial_coverage_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/rfb_io.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_initial_coverage_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_async_framing_test: \
	unit/rfb_async_framing_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/rfb_io.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_async_framing_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/display_test: \
	unit/display_test.c \
	../src/display/display.c \
	../src/display/display.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/display_test.c \
		../src/display/display.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/mouse_test: \
	unit/mouse_test.c \
	../src/input/mouse.c \
	../src/input/mouse.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mouse_test.c \
		../src/input/mouse.c \
		-o $@

$(BUILD_DIR)/keyboard_test: \
	unit/keyboard_test.c \
	../src/input/keyboard.c \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/keyboard_test.c \
		../src/input/keyboard.c \
		-o $@

$(BUILD_DIR)/local_ui_test: \
	unit/local_ui_test.c \
	../src/ui/local_ui.c \
	../src/ui/local_ui.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_ui_test.c \
		../src/ui/local_ui.c \
		-o $@

$(BUILD_DIR)/osk_test: \
	unit/osk_test.c \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_test.c \
		../src/ui/osk.c \
		-o $@

$(BUILD_DIR)/osk_render_test: \
	unit/osk_render_test.c \
	../src/ui/osk_render.c \
	../src/ui/osk_render.h \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_render_test.c \
		../src/ui/osk_render.c \
		../src/ui/osk.c \
		-o $@

$(BUILD_DIR)/local_ui_presentation_test: \
	unit/local_ui_presentation_test.c \
	../src/ui/local_ui_presentation.c \
	../src/ui/local_ui_presentation.h \
	../src/ui/local_ui.c \
	../src/ui/local_ui.h \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/ui/osk_render.c \
	../src/ui/osk_render.h \
	../src/display/display.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_ui_presentation_test.c \
		../src/ui/local_ui_presentation.c \
		../src/ui/local_ui.c \
		../src/ui/osk.c \
		../src/ui/osk_render.c \
		-o $@

$(BUILD_DIR)/input_test: \
	unit/input_test.c \
	../src/input/input.c \
	../src/input/input.h \
	../src/input/keyboard.h \
	../src/input/mouse.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/input_test.c \
		../src/input/input.c \
		-o $@

$(BUILD_DIR)/controller_event_test: \
	unit/controller_event_test.c \
	../src/input/input.c \
	../src/input/input.h \
	../src/input/controller.h \
	../src/input/mouse.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/controller_event_test.c \
		../src/input/input.c \
		-o $@

$(BUILD_DIR)/local_controller_test: \
	unit/local_controller_test.c \
	../src/ui/local_controller.c \
	../src/ui/local_controller.h \
	../src/input/controller.h \
	../src/ui/local_ui.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_controller_test.c \
		../src/ui/local_controller.c \
		-o $@

$(BUILD_DIR)/osk_direct_test: \
	unit/osk_direct_test.c \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_direct_test.c \
		../src/ui/osk.c \
		-o $@

$(BUILD_DIR)/app_test: \
	unit/app_test.c \
	support/libpad.h \
	../src/app.c \
	../src/app.h \
	../src/diagnostics/diagnostics.h \
	../src/display/display.h \
	../src/framebuffer/framebuffer.h \
	../src/input/input.h \
	../src/input/input_runtime.h \
	../src/input/keyboard.c \
	../src/input/keyboard.h \
	../src/input/controller.h \
	../src/ui/local_controller.c \
	../src/ui/local_controller.h \
	../src/ui/local_ui.c \
	../src/ui/local_ui.h \
	../src/ui/local_ui_presentation.c \
	../src/ui/local_ui_presentation.h \
	../src/input/mouse.h \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/ui/osk_render.c \
	../src/ui/osk_render.h \
	../src/input/pad.h \
	../src/platform/ps2_graphics.h \
	../src/platform/ps2_network.h \
	../src/platform/ps2_system.h \
	../src/rfb/rfb.h \
	../src/rfb/rfb_session.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Isupport \
		-I../src \
		unit/app_test.c \
		../src/app.c \
		../src/input/keyboard.c \
		../src/ui/local_controller.c \
		../src/ui/local_ui.c \
		../src/ui/local_ui_presentation.c \
		../src/ui/osk.c \
		../src/ui/osk_render.c \
		-o $@

clean:
	rm -rf $(BUILD_DIR)


GEOMETRY_TEST := .build/video_geometry_test

.PHONY: geometry-unit
unit: geometry-unit

geometry-unit: $(GEOMETRY_TEST)
	./$(GEOMETRY_TEST)

$(GEOMETRY_TEST): unit/video_geometry_test.c ../src/video/geometry.c ../src/video/geometry.h ../src/video/mode.c ../src/video/mode.h support/gsKit.h
	mkdir -p .build
	$(CC) -O2 -std=c99 -Wall -Wextra -Werror -pedantic \
		-Isupport \
		-I../src \
		unit/video_geometry_test.c \
		../src/video/geometry.c \
		../src/video/mode.c \
		-o $(GEOMETRY_TEST)

TRANSPORT_RFB_CHANNEL_TEST := .build/transport_rfb_channel_test

.PHONY: transport-rfb-channel-unit
unit: transport-rfb-channel-unit

transport-rfb-channel-unit: $(TRANSPORT_RFB_CHANNEL_TEST)
	./$(TRANSPORT_RFB_CHANNEL_TEST)

$(TRANSPORT_RFB_CHANNEL_TEST): unit/transport_rfb_channel_test.c ../src/transport/rfb_channel.c ../src/transport/rfb_channel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/transport_rfb_channel_test.c \
		../src/transport/rfb_channel.c \
		-o $(TRANSPORT_RFB_CHANNEL_TEST)
