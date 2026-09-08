# H1 media-harness host checks.
#
# The PS2 build recipe is added only after the mux runtime/video descendant is
# present. These host checks establish CONFIG-v2 wire/profile semantics first.

HOST_BUILD_DIR ?= build-host
HOST_CC ?= cc
HOST_CFLAGS ?= -std=c99 -Wall -Wextra -Werror -pedantic

.PHONY: host-test host-clean

host-test: $(HOST_BUILD_DIR)/test_h1_config
	./$(HOST_BUILD_DIR)/test_h1_config
	@echo H1_MEDIA_HARNESS_HOST_TESTS=PASS

$(HOST_BUILD_DIR):
	mkdir -p $@

$(HOST_BUILD_DIR)/test_h1_config: \
	h1_config.c \
	h1_config.h \
	tests/test_h1_config.c \
	../audio-transport/common/transport_protocol.c \
	../audio-transport/common/transport_protocol.h | $(HOST_BUILD_DIR)
	$(HOST_CC) $(HOST_CFLAGS) \
		-I. \
		-I../audio-transport/common \
		h1_config.c \
		../audio-transport/common/transport_protocol.c \
		tests/test_h1_config.c \
		-o $@

host-clean:
	rm -rf $(HOST_BUILD_DIR)
