# Q3 Wire Channel Relay real-hardware proof build.
#
# This experiment keeps one physical PSTV stream alive from Q4 establishment
# through the RFB Relay discriminator. It reuses the real physical stream,
# bounded RFB channel, and RFB public bridge while keeping proof-only Transport
# glue outside clean product source.

BUILD_DIR ?= build/experiments/wire-q1-q12-proof/ps2
GEN_DIR = $(BUILD_DIR)/generated
DEP_DIR = $(BUILD_DIR)/deps

PS2IP_LIB = $(DEP_DIR)/libps2ip_mtu1458_wscale128.a
EE_BIN ?= $(BUILD_DIR)/PS2VNC-Wire-Q1Q12-Proof3-RFB-Relay.ELF

EE_INCS = \
	-Isrc \
	-Isrc/platform \
	-Isrc/rfb \
	-Isrc/transport \
	-Isrc/diagnostics \
	-Iexperiments/wire-q1-q12-proof

EE_OBJS = \
	$(BUILD_DIR)/proof3_rfb_relay_ps2.o \
	$(BUILD_DIR)/wire_establish_protocol.o \
	$(BUILD_DIR)/transport_protocol.o \
	$(BUILD_DIR)/transport_physical_stream.o \
	$(BUILD_DIR)/transport_rfb_channel.o \
	$(BUILD_DIR)/rfb_bridge.o \
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
	@false

$(BUILD_DIR)/proof3_rfb_relay_ps2.o: \
	experiments/wire-q1-q12-proof/proof3_rfb_relay_ps2.c \
	experiments/wire-q1-q12-proof/wire_establish_protocol.h \
	src/platform/ps2_system.h \
	src/platform/ps2_network.h \
	src/rfb/bridge.h \
	src/transport/bridge.h \
	src/transport/physical_stream.h \
	src/transport/rfb_channel.h \
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

$(BUILD_DIR)/transport_physical_stream.o: \
	src/transport/physical_stream.c \
	src/transport/physical_stream.h \
	src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_rfb_channel.o: \
	src/transport/rfb_channel.c \
	src/transport/rfb_channel.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_bridge.o: \
	src/rfb/bridge.c \
	src/rfb/bridge.h \
	src/transport/bridge.h \
	src/transport/transport.h | $(BUILD_DIR)
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
