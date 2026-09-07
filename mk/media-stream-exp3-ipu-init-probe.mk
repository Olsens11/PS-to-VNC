# Media Stream EXP3 — IPU DMA initialization-only bisection probe.
#
# This retains the exact large embedded MPEG payload from the passing
# payload-only probe but does not link libmpeg or transfer payload bytes into
# the IPU.

BUILD_DIR ?= \
	build/experiments/media-stream-exp3/ipu-init-probe

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-IPUInit-Probe.ELF

STREAM = $(BUILD_DIR)/test.bin

EE_OBJS = \
	$(BUILD_DIR)/graph_payload_ipu_init_probe.o \
	$(BUILD_DIR)/test_bin.o

EE_LIBS = \
	-ldraw \
	-lgraph \
	-ldma \
	-lc \
	-lpacket

.PHONY: all clean

all: $(EE_BIN)
	$(EE_STRIP) --strip-all $(EE_BIN)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/graph_payload_ipu_init_probe.o: \
	experiments/media-stream-exp3/graph_payload_ipu_init_probe.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/test_bin.o: $(STREAM) | $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(EE_LD) -r -b binary -o test_bin.o test.bin

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
