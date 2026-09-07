# Media Stream EXP3 — standalone embedded libmpeg hardware smoke test.
#
# This target is intentionally independent of the PS2VNC product link.
# A generated raw MPEG-2 elementary stream is converted to an EE relocatable
# binary object and linked directly into the smoke-test ELF.

BUILD_DIR ?= build/experiments/media-stream-exp3/embedded-libmpeg-smoke

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-EmbeddedMPEG-Smoke.ELF

STREAM = $(BUILD_DIR)/test.bin

EE_OBJS = \
	$(BUILD_DIR)/libmpeg_embedded_smoke.o \
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
	$(EE_STRIP) --strip-all $(EE_BIN)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/libmpeg_embedded_smoke.o: \
	experiments/media-stream-exp3/libmpeg_embedded_smoke.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/test_bin.o: $(STREAM) | $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(EE_LD) -r -b binary -o test_bin.o test.bin

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
