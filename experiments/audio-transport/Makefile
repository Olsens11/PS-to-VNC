# Dedicated-link transport experiment build helpers.
#
# This makefile is intentionally local to experiments/audio-transport.
# It does not modify the qualified clean-product build.

HOST_CC ?= cc

HOST_CFLAGS ?= \
	-std=c99 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic

COMMON_DIR := common
TEST_DIR := tests
HOST_BUILD_DIR := build-host

PROTOCOL_TEST := \
	$(HOST_BUILD_DIR)/test_transport_protocol

QUEUE_TEST := \
	$(HOST_BUILD_DIR)/test_transport_queue

.PHONY: host-test host-clean

host-test: $(PROTOCOL_TEST) $(QUEUE_TEST)
	./$(PROTOCOL_TEST)
	./$(QUEUE_TEST)
	@echo "AUDIO_TRANSPORT_HOST_TESTS=PASS"

$(HOST_BUILD_DIR):
	mkdir -p $@

$(PROTOCOL_TEST): \
	$(COMMON_DIR)/transport_protocol.c \
	$(COMMON_DIR)/transport_protocol.h \
	$(TEST_DIR)/test_transport_protocol.c | $(HOST_BUILD_DIR)
	$(HOST_CC) $(HOST_CFLAGS) \
		-I$(COMMON_DIR) \
		$(COMMON_DIR)/transport_protocol.c \
		$(TEST_DIR)/test_transport_protocol.c \
		-o $@

$(QUEUE_TEST): \
	$(COMMON_DIR)/transport_queue.c \
	$(COMMON_DIR)/transport_queue.h \
	$(TEST_DIR)/test_transport_queue.c | $(HOST_BUILD_DIR)
	$(HOST_CC) $(HOST_CFLAGS) \
		-I$(COMMON_DIR) \
		$(COMMON_DIR)/transport_queue.c \
		$(TEST_DIR)/test_transport_queue.c \
		-o $@

host-clean:
	rm -rf $(HOST_BUILD_DIR)
