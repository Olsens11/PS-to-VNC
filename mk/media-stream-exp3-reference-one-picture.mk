# Media Stream EXP3 — reference-derived one-picture MPEG hardware smoke.
#
# This target proves the decoder path separately from decoded-picture GS
# presentation. It intentionally avoids dma_wait_fast.

BUILD_DIR ?= \
	build/experiments/media-stream-exp3/reference-one-picture

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-Reference-OnePicture.ELF

STREAM = \
	$(BUILD_DIR)/test.bin

EE_OBJS = \
	$(BUILD_DIR)/reference_one_picture_smoke.o \
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

$(BUILD_DIR)/reference_one_picture_smoke.o: \
	experiments/media-stream-exp3/reference_one_picture_smoke.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/test_bin.o: $(STREAM) | $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(EE_LD) -r -b binary -o test_bin.o test.bin

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
