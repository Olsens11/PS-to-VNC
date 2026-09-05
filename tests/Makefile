CC ?= cc
CFLAGS ?=

COMMON_CFLAGS = -O2 -std=c99 -Wall -Wextra -Werror -pedantic
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
	../src/rfb.c \
	../src/rfb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_wire_test.c \
		../src/rfb.c \
		-o $@

$(BUILD_DIR)/framebuffer_test: \
	unit/framebuffer_test.c \
	../src/framebuffer.c \
	../src/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/framebuffer_test.c \
		../src/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_session_test: \
	unit/rfb_session_test.c \
	../src/rfb.c \
	../src/rfb.h \
	../src/rfb_io.h \
	../src/rfb_session.c \
	../src/rfb_session.h \
	../src/framebuffer.c \
	../src/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_session_test.c \
		../src/rfb_session.c \
		../src/rfb.c \
		../src/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_initial_frame_test: \
	unit/rfb_initial_frame_test.c \
	../src/rfb.c \
	../src/rfb.h \
	../src/rfb_io.h \
	../src/rfb_session.c \
	../src/rfb_session.h \
	../src/framebuffer.c \
	../src/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_initial_frame_test.c \
		../src/rfb_session.c \
		../src/rfb.c \
		../src/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_initial_coverage_test: \
	unit/rfb_initial_coverage_test.c \
	../src/rfb.c \
	../src/rfb.h \
	../src/rfb_io.h \
	../src/rfb_session.c \
	../src/rfb_session.h \
	../src/framebuffer.c \
	../src/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_initial_coverage_test.c \
		../src/rfb_session.c \
		../src/rfb.c \
		../src/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_async_framing_test: \
	unit/rfb_async_framing_test.c \
	../src/rfb.c \
	../src/rfb.h \
	../src/rfb_io.h \
	../src/rfb_session.c \
	../src/rfb_session.h \
	../src/framebuffer.c \
	../src/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_async_framing_test.c \
		../src/rfb_session.c \
		../src/rfb.c \
		../src/framebuffer.c \
		-o $@

$(BUILD_DIR)/display_test: \
	unit/display_test.c \
	../src/display.c \
	../src/display.h \
	../src/framebuffer.c \
	../src/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/display_test.c \
		../src/display.c \
		../src/framebuffer.c \
		-o $@

$(BUILD_DIR)/mouse_test: \
	unit/mouse_test.c \
	../src/mouse.c \
	../src/mouse.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mouse_test.c \
		../src/mouse.c \
		-o $@

$(BUILD_DIR)/keyboard_test: \
	unit/keyboard_test.c \
	../src/keyboard.c \
	../src/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/keyboard_test.c \
		../src/keyboard.c \
		-o $@

$(BUILD_DIR)/local_ui_test: \
	unit/local_ui_test.c \
	../src/local_ui.c \
	../src/local_ui.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_ui_test.c \
		../src/local_ui.c \
		-o $@

$(BUILD_DIR)/osk_test: \
	unit/osk_test.c \
	../src/osk.c \
	../src/osk.h \
	../src/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_test.c \
		../src/osk.c \
		-o $@

$(BUILD_DIR)/osk_render_test: \
	unit/osk_render_test.c \
	../src/osk_render.c \
	../src/osk_render.h \
	../src/osk.c \
	../src/osk.h \
	../src/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_render_test.c \
		../src/osk_render.c \
		../src/osk.c \
		-o $@

$(BUILD_DIR)/local_ui_presentation_test: \
	unit/local_ui_presentation_test.c \
	../src/local_ui_presentation.c \
	../src/local_ui_presentation.h \
	../src/local_ui.c \
	../src/local_ui.h \
	../src/osk.c \
	../src/osk.h \
	../src/osk_render.c \
	../src/osk_render.h \
	../src/display.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_ui_presentation_test.c \
		../src/local_ui_presentation.c \
		../src/local_ui.c \
		../src/osk.c \
		../src/osk_render.c \
		-o $@

$(BUILD_DIR)/input_test: \
	unit/input_test.c \
	../src/input.c \
	../src/input.h \
	../src/keyboard.h \
	../src/mouse.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/input_test.c \
		../src/input.c \
		-o $@

$(BUILD_DIR)/controller_event_test: \
	unit/controller_event_test.c \
	../src/input.c \
	../src/input.h \
	../src/controller.h \
	../src/mouse.h \
	../src/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/controller_event_test.c \
		../src/input.c \
		-o $@

$(BUILD_DIR)/local_controller_test: \
	unit/local_controller_test.c \
	../src/local_controller.c \
	../src/local_controller.h \
	../src/controller.h \
	../src/local_ui.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_controller_test.c \
		../src/local_controller.c \
		-o $@

$(BUILD_DIR)/osk_direct_test: \
	unit/osk_direct_test.c \
	../src/osk.c \
	../src/osk.h \
	../src/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_direct_test.c \
		../src/osk.c \
		-o $@

$(BUILD_DIR)/app_test: \
	unit/app_test.c \
	support/libpad.h \
	../src/app.c \
	../src/app.h \
	../src/diagnostics.h \
	../src/display.h \
	../src/framebuffer.h \
	../src/input.h \
	../src/input_runtime.h \
	../src/keyboard.c \
	../src/keyboard.h \
	../src/controller.h \
	../src/local_controller.c \
	../src/local_controller.h \
	../src/local_ui.c \
	../src/local_ui.h \
	../src/local_ui_presentation.c \
	../src/local_ui_presentation.h \
	../src/mouse.h \
	../src/osk.c \
	../src/osk.h \
	../src/osk_render.c \
	../src/osk_render.h \
	../src/pad.h \
	../src/platform/ps2_graphics.h \
	../src/platform/ps2_network.h \
	../src/platform/ps2_system.h \
	../src/rfb.h \
	../src/rfb_session.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Isupport \
		-I../src \
		unit/app_test.c \
		../src/app.c \
		../src/keyboard.c \
		../src/local_controller.c \
		../src/local_ui.c \
		../src/local_ui_presentation.c \
		../src/osk.c \
		../src/osk_render.c \
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
