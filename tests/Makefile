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
	$(BUILD_DIR)/display_test
	./$(BUILD_DIR)/config_text_test
	./$(BUILD_DIR)/video_mode_test
	./$(BUILD_DIR)/rfb_wire_test
	./$(BUILD_DIR)/framebuffer_test
	./$(BUILD_DIR)/rfb_session_test
	./$(BUILD_DIR)/rfb_initial_frame_test
	./$(BUILD_DIR)/rfb_initial_coverage_test
	./$(BUILD_DIR)/display_test

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
