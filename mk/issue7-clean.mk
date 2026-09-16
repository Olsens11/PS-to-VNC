# PS-to-VNC clean reconstruction — Issue #7 linked build.
#
# This is deliberately separate from scripts/build.sh, which remains the
# historical B4A/reference build authority until the clean executable has been
# qualified on hardware.

BUILD_DIR ?= build/reconstruction/issue7
GEN_DIR = $(BUILD_DIR)/generated
DEP_DIR = $(BUILD_DIR)/deps

PS2IP_LIB = $(DEP_DIR)/libps2ip_mtu1458_wscale128.a
EE_BIN ?= $(BUILD_DIR)/PS-to-VNC-Issue7.ELF

EXTRA_EE_OBJS ?=

EE_OBJS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/app.o \
	$(BUILD_DIR)/diagnostics.o \
	$(BUILD_DIR)/diagnostics_identity.o \
	$(BUILD_DIR)/rfb.o \
	$(BUILD_DIR)/framebuffer.o \
	$(BUILD_DIR)/rfb_session.o \
	$(BUILD_DIR)/display.o \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/mouse.o \
	$(BUILD_DIR)/input_runtime.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/pad.o \
	$(BUILD_DIR)/local_controller.o \
	$(BUILD_DIR)/local_ui.o \
	$(BUILD_DIR)/osk.o \
	$(BUILD_DIR)/osk_render.o \
	$(BUILD_DIR)/local_ui_presentation.o \
	$(BUILD_DIR)/transport_protocol.o \
	$(BUILD_DIR)/transport_physical_stream.o \
	$(BUILD_DIR)/transport_rfb_channel.o \
	$(BUILD_DIR)/transport_runtime.o \
	$(BUILD_DIR)/transport_quiesce.o \
	$(BUILD_DIR)/transport_bridge.o \
	$(BUILD_DIR)/ps2_system.o \
	$(BUILD_DIR)/ps2_network.o \
	$(BUILD_DIR)/ps2_graphics.o \
	$(BUILD_DIR)/SIO2MAN_irx.o \
	$(BUILD_DIR)/PADMAN_irx.o \
	$(BUILD_DIR)/DEV9_irx.o \
	$(BUILD_DIR)/NETMAN_irx.o \
	$(BUILD_DIR)/SMAP_irx.o

EE_OBJS += $(EXTRA_EE_OBJS)

EE_INCS = -Isrc -Isrc/input -Isrc/ui -Isrc/rfb -Isrc/framebuffer -Isrc/display -Isrc/diagnostics -Isrc/platform -Isrc/transport -I$(GSKIT)/include
EE_LIBS = -L$(GSKIT)/lib -lgskit -ldmakit -lnetman -lpad $(PS2IP_LIB) -lpatches -Wl,--wrap=sendto

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR) $(GEN_DIR) $(DEP_DIR):
	mkdir -p $@

$(PS2IP_LIB):
	@echo "Missing qualified PS2IP dependency: $(PS2IP_LIB)" >&2
	@echo "Run scripts/build-issue7-clean.sh from the repository root." >&2
	@false

$(EE_BIN): $(PS2IP_LIB) $(EE_OBJS) | $(BUILD_DIR)

$(BUILD_DIR)/main.o: src/main.c src/app.h src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app.o: src/app.c src/app.h src/diagnostics/diagnostics.h src/display/display.h src/framebuffer/framebuffer.h src/input/input.h src/input/input_runtime.h src/ui/local_ui_presentation.h src/ui/local_ui.h src/ui/osk.h src/ui/osk_render.h src/input/mouse.h src/input/pad.h src/rfb/rfb_session.h src/transport/bridge.h src/transport/transport.h src/platform/ps2_graphics.h src/platform/ps2_network.h src/platform/ps2_system.h src/input/controller.h src/ui/local_controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics.o: src/diagnostics/diagnostics.c src/diagnostics/diagnostics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics_identity.o: src/diagnostics/identity.c src/diagnostics/identity.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb.o: src/rfb/rfb.c src/rfb/rfb.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: src/framebuffer/framebuffer.c src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_session.o: src/rfb/rfb_session.c src/rfb/rfb_session.h src/rfb/rfb.h src/framebuffer/framebuffer.h src/transport/bridge.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/display.o: src/display/display.c src/display/display.h src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/input.o: src/input/input.c src/input/input.h src/input/mouse.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mouse.o: src/input/mouse.c src/input/mouse.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/input_runtime.o: src/input/input_runtime.c src/input/input_runtime.h src/input/input.h src/input/controller.h src/input/mouse.h src/input/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: src/input/keyboard.c src/input/keyboard.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/pad.o: src/input/pad.c src/input/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_controller.o: src/ui/local_controller.c src/ui/local_controller.h src/input/controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_ui.o: src/ui/local_ui.c src/ui/local_ui.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/osk.o: src/ui/osk.c src/ui/osk.h src/input/keyboard.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/osk_render.o: src/ui/osk_render.c src/ui/osk_render.h src/ui/osk.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_ui_presentation.o: src/ui/local_ui_presentation.c src/ui/local_ui_presentation.h src/ui/local_ui.h src/ui/osk.h src/ui/osk_render.h src/display/display.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_protocol.o: src/transport/protocol.c src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_physical_stream.o: src/transport/physical_stream.c src/transport/physical_stream.h src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_rfb_channel.o: src/transport/rfb_channel.c src/transport/rfb_channel.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_runtime.o: src/transport/runtime.c src/transport/runtime.h src/transport/physical_stream.h src/transport/rfb_channel.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_quiesce.o: src/transport/quiesce.c src/transport/runtime.h src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_bridge.o: src/transport/bridge.c src/transport/bridge.h src/transport/runtime.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_system.o: src/platform/ps2_system.c src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_network.o: src/platform/ps2_network.c src/platform/ps2_network.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_graphics.o: src/platform/ps2_graphics.c src/platform/ps2_graphics.h src/display/display.h | $(BUILD_DIR)
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