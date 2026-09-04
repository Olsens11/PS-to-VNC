# PS-to-VNC clean reconstruction — Issue #7 linked build.
#
# This is deliberately separate from scripts/build.sh, which remains the
# historical B4A/reference build authority until the clean executable has been
# qualified on hardware.

BUILD_DIR ?= build/reconstruction/issue7
GEN_DIR = $(BUILD_DIR)/generated
DEP_DIR = $(BUILD_DIR)/deps

PS2IP_LIB = $(DEP_DIR)/libps2ip_mtu1458_wscale128.a
EE_BIN = $(BUILD_DIR)/PS-to-VNC-Issue7.ELF

EE_OBJS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/app.o \
	$(BUILD_DIR)/diagnostics.o \
	$(BUILD_DIR)/diagnostics_identity.o \
	$(BUILD_DIR)/rfb.o \
	$(BUILD_DIR)/framebuffer.o \
	$(BUILD_DIR)/rfb_session.o \
	$(BUILD_DIR)/display.o \
	$(BUILD_DIR)/pad.o \
	$(BUILD_DIR)/ps2_system.o \
	$(BUILD_DIR)/ps2_network.o \
	$(BUILD_DIR)/ps2_graphics.o \
	$(BUILD_DIR)/SIO2MAN_irx.o \
	$(BUILD_DIR)/PADMAN_irx.o \
	$(BUILD_DIR)/DEV9_irx.o \
	$(BUILD_DIR)/NETMAN_irx.o \
	$(BUILD_DIR)/SMAP_irx.o

EE_INCS = -Isrc -Isrc/platform -I$(GSKIT)/include
EE_LIBS = -L$(GSKIT)/lib -lgskit -ldmakit -lnetman -lpad $(PS2IP_LIB) -lpatches -Wl,--wrap=sendto

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR) $(GEN_DIR) $(DEP_DIR):
	mkdir -p $@

$(PS2IP_LIB):
	@echo "Missing qualified PS2IP dependency: $(PS2IP_LIB)" >&2
	@echo "Run scripts/build-issue7-clean.sh from the repository root." >&2
	@false

$(EE_BIN): $(PS2IP_LIB) | $(BUILD_DIR)

$(BUILD_DIR)/main.o: src/main.c src/app.h src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app.o: src/app.c src/app.h src/diagnostics.h src/display.h src/framebuffer.h src/rfb_session.h src/platform/ps2_graphics.h src/platform/ps2_network.h src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics.o: src/diagnostics.c src/diagnostics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics_identity.o: src/diagnostics/identity.c src/diagnostics/identity.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb.o: src/rfb.c src/rfb.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: src/framebuffer.c src/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_session.o: src/rfb_session.c src/rfb_session.h src/rfb.h src/rfb_io.h src/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/display.o: src/display.c src/display.h src/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/pad.o: src/pad.c src/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_system.o: src/platform/ps2_system.c src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_network.o: src/platform/ps2_network.c src/platform/ps2_network.h src/rfb_io.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_graphics.o: src/platform/ps2_graphics.c src/platform/ps2_graphics.h src/display.h | $(BUILD_DIR)
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
