# Media Stream EXP3 graph-only hardware bisection probe.

BUILD_DIR ?= build/experiments/media-stream-exp3/graph-probe

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-GraphOnly-Probe.ELF

EE_OBJS = \
	$(BUILD_DIR)/graph_only_probe.o

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

$(BUILD_DIR)/graph_only_probe.o: \
	experiments/media-stream-exp3/graph_only_probe.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
