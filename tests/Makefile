CC ?= cc

CPPFLAGS ?=
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Werror -pedantic

BUILD_DIR := .build
CONFIG_TEXT_TEST := $(BUILD_DIR)/config_text_test

.PHONY: all unit clean

all: unit

unit: $(CONFIG_TEXT_TEST)
	./$(CONFIG_TEXT_TEST)

$(CONFIG_TEXT_TEST): \
	unit/config_text_test.c \
	../src/config/text.c \
	../src/config/text.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) \
		-I../src \
		unit/config_text_test.c \
		../src/config/text.c \
		-o $(CONFIG_TEXT_TEST)

clean:
	rm -rf $(BUILD_DIR)
