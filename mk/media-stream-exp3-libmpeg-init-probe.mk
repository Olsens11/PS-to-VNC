# Media Stream EXP3 — MPEG_Initialize-only hardware bisection probe.
#
# Keeps the exact embedded MPEG payload and proven graph/IPU chassis.
# This stage calls MPEG_Initialize but never calls MPEG_Picture.

BUILD_DIR ?= \
	build/experiments/media-stream-exp3/libmpeg-init-probe

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-LibmpegInitialize-Probe.ELF

STREAM = $(BUILD_DIR)/test.bin

EE_OBJS = \
	$(BUILD_DIR)/libmpeg_initialize_probe.o \
	$(BUILD_DIR)/test_bin.o

EE_LIBS = \
	-ldraw \
	-lgraph \
	-ldma \
	-lmpeg \
	-lc \
	-lpacket

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/libmpeg_initialize_probe.o: \
	experiments/media-stream-exp3/libmpeg_initialize_probe.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/test_bin.o: $(STREAM) | $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(EE_LD) -r -b binary -o test_bin.o test.bin

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
