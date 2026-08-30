CC ?= cc
CFLAGS ?=

COMMON_CFLAGS = -O2 -std=c99 -Wall -Wextra -Werror -pedantic
BUILD_DIR = .build

.PHONY: unit clean

unit: \
	$(BUILD_DIR)/config_text_test \
	$(BUILD_DIR)/video_mode_test
	./$(BUILD_DIR)/config_text_test
	./$(BUILD_DIR)/video_mode_test

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

clean:
	rm -rf $(BUILD_DIR)
