# Q9/Q12 Wire reconnect real-hardware proof build.
#
# This target is deliberately experiment-scoped. It reuses the qualified PS2
# platform/network seams and clean Wire header codec, but does not link/start
# the current rider-oriented application runtime.

BUILD_DIR ?= build/experiments/wire-q1-q12-proof/ps2
GEN_DIR = $(BUILD_DIR)/generated
DEP_DIR = $(BUILD_DIR)/deps

PS2IP_LIB = $(DEP_DIR)/libps2ip_mtu1458_wscale128.a
EE_BIN ?= $(BUILD_DIR)/PS2VNC-Wire-Q1Q12-Proof2-Reconnect.ELF

EE_INCS = \
	-Isrc \
	-Isrc/platform \
	-Isrc/transport \
	-Isrc/diagnostics \
	-Iexperiments/wire-q1-q12-proof

EE_OBJS = \
	$(BUILD_DIR)/proof2_reconnect_ps2.o \
	$(BUILD_DIR)/wire_establish_protocol.o \
	$(BUILD_DIR)/transport_protocol.o \
	$(BUILD_DIR)/ps2_system.o \
	$(BUILD_DIR)/ps2_network.o \
	$(BUILD_DIR)/diagnostics_identity.o \
	$(BUILD_DIR)/SIO2MAN_irx.o \
	$(BUILD_DIR)/PADMAN_irx.o \
	$(BUILD_DIR)/DEV9_irx.o \
	$(BUILD_DIR)/NETMAN_irx.o \
	$(BUILD_DIR)/SMAP_irx.o

EE_LIBS = \
	-lnetman \
	$(PS2IP_LIB) \
	-ldebug \
	-lpatches \
	-Wl,--wrap=sendto

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR) $(GEN_DIR) $(DEP_DIR):
	mkdir -p $@

$(PS2IP_LIB):
	@echo "Missing qualified PS2IP dependency: $(PS2IP_LIB)" >&2
	@echo "Run scripts/build-wire-q1-q12-proof.sh from repository root." >&2
	@false

$(BUILD_DIR)/proof2_reconnect_ps2.o: \
	experiments/wire-q1-q12-proof/proof2_reconnect_ps2.c \
	experiments/wire-q1-q12-proof/wire_establish_protocol.h \
	src/platform/ps2_system.h \
	src/platform/ps2_network.h \
	src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/wire_establish_protocol.o: \
	experiments/wire-q1-q12-proof/wire_establish_protocol.c \
	experiments/wire-q1-q12-proof/wire_establish_protocol.h \
	src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_protocol.o: \
	src/transport/protocol.c \
	src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_system.o: \
	src/platform/ps2_system.c \
	src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_network.o: \
	src/platform/ps2_network.c \
	src/platform/ps2_network.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics_identity.o: \
	src/diagnostics/identity.c \
	src/diagnostics/identity.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(GEN_DIR)/SIO2MAN_irx.c: $(PS2SDK)/iop/irx/freesio2.irx | $(GEN_DIR)
	bin2c $< $@ SIO2MAN_irx

$(GEN_DIR)/PADMAN_irx.c: $(PS2SDK)/iop/irx/freepad.irx | $(GEN_DIR)
	bin2c $< $@ PADMAN_irx

$(GEN_DIR)/DEV9_irx.c: $(PS2SDK)/iop/irx/ps2dev9.irx | $(GEN_DIR)
	bin2c $< $@ DEV9_irx

$(GEN_DIR)/NETMAN_irx.c: $(PS2SDK)/iop/irx/netman.irx | $(GEN_DIR)
	bin2c $< $@ NETMAN_irx

$(GEN_DIR)/SMAP_irx.c: $(PS2SDK)/iop/irx/smap.irx | $(GEN_DIR)
	bin2c $< $@ SMAP_irx

$(BUILD_DIR)/SIO2MAN_irx.o: $(GEN_DIR)/SIO2MAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/PADMAN_irx.o: $(GEN_DIR)/PADMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/DEV9_irx.o: $(GEN_DIR)/DEV9_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/NETMAN_irx.o: $(GEN_DIR)/NETMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/SMAP_irx.o: $(GEN_DIR)/SMAP_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
