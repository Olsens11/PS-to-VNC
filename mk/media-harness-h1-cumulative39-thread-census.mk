# H1 cumulative through-Issue-39 EE-thread census build.
#
# Purpose:
#   Produce one resident diagnostic ELF containing the clean reconstruction
#   modules completed through Issue #39 plus the current H1 AUDIO/MPEG harness.
#
# Runtime contract for the first hardware census:
#   - H1 AUDIO/MPEG path is active according to CONFIG;
#   - RFB/input/OSK/local-UI modules are linked but not started;
#   - therefore the exact same binary can later activate those subsystems
#     deliberately without changing the code population being diagnosed.
#
# This file intentionally reuses the H1 thread-census build rather than copying
# media/runtime rules. The clean product modules keep their own source files and
# boundaries; this makefile only composes them into the cumulative executable.

BUILD_DIR ?= build/experiments/media-harness-h1-cumulative39-thread-census/ps2
EE_BIN ?= $(BUILD_DIR)/PS2VNC-H1-Cumulative39-ThreadCensus.ELF

EXTRA_EE_INCS := \
	-Isrc \
	-Isrc/input \
	-Isrc/ui \
	-Isrc/rfb \
	-Isrc/framebuffer \
	-Isrc/display \
	-Isrc/diagnostics \
	-Isrc/platform \
	-I$(GSKIT)/include

EXTRA_EE_OBJS := \
	$(BUILD_DIR)/app39.o \
	$(BUILD_DIR)/diagnostics39.o \
	$(BUILD_DIR)/diagnostics_identity39.o \
	$(BUILD_DIR)/rfb39.o \
	$(BUILD_DIR)/framebuffer39.o \
	$(BUILD_DIR)/rfb_session39.o \
	$(BUILD_DIR)/display39.o \
	$(BUILD_DIR)/input39.o \
	$(BUILD_DIR)/mouse39.o \
	$(BUILD_DIR)/input_runtime39.o \
	$(BUILD_DIR)/keyboard39.o \
	$(BUILD_DIR)/local_ui39.o \
	$(BUILD_DIR)/osk39.o \
	$(BUILD_DIR)/osk_render39.o \
	$(BUILD_DIR)/local_ui_presentation39.o \
	$(BUILD_DIR)/local_controller39.o \
	$(BUILD_DIR)/pad39.o \
	$(BUILD_DIR)/ps2_graphics39.o

# Preserve the qualified Issue #39 link requirements. The H1 build already
# supplies the shared PS2 system/network objects and embedded SIO2/PAD/DEV9/
# NETMAN/SMAP modules, so those are deliberately not duplicated here.
EXTRA_EE_LIBS := \
	-L$(GSKIT)/lib \
	-lgskit \
	-ldmakit \
	-lpad \
	-Wl,--wrap=sendto

include mk/media-harness-h1-thread-census-diag.mk

$(BUILD_DIR)/app39.o: \
	src/app.c \
	src/app.h \
	src/diagnostics/diagnostics.h \
	src/display/display.h \
	src/framebuffer/framebuffer.h \
	src/input/input.h \
	src/input/input_runtime.h \
	src/ui/local_ui_presentation.h \
	src/ui/local_ui.h \
	src/ui/osk.h \
	src/ui/osk_render.h \
	src/input/mouse.h \
	src/input/pad.h \
	src/rfb/rfb_session.h \
	src/platform/ps2_graphics.h \
	src/platform/ps2_network.h \
	src/platform/ps2_system.h \
	src/input/controller.h \
	src/ui/local_controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics39.o: \
	src/diagnostics/diagnostics.c \
	src/diagnostics/diagnostics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics_identity39.o: \
	src/diagnostics/identity.c \
	src/diagnostics/identity.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb39.o: \
	src/rfb/rfb.c \
	src/rfb/rfb.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/framebuffer39.o: \
	src/framebuffer/framebuffer.c \
	src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_session39.o: \
	src/rfb/rfb_session.c \
	src/rfb/rfb_session.h \
	src/rfb/rfb.h \
	src/rfb/rfb_io.h \
	src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/display39.o: \
	src/display/display.c \
	src/display/display.h \
	src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/input39.o: \
	src/input/input.c \
	src/input/input.h \
	src/input/mouse.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mouse39.o: \
	src/input/mouse.c \
	src/input/mouse.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/input_runtime39.o: \
	src/input/input_runtime.c \
	src/input/input_runtime.h \
	src/input/input.h \
	src/input/controller.h \
	src/input/mouse.h \
	src/input/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/keyboard39.o: \
	src/input/keyboard.c \
	src/input/keyboard.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_ui39.o: \
	src/ui/local_ui.c \
	src/ui/local_ui.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/osk39.o: \
	src/ui/osk.c \
	src/ui/osk.h \
	src/input/keyboard.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/osk_render39.o: \
	src/ui/osk_render.c \
	src/ui/osk_render.h \
	src/ui/osk.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_ui_presentation39.o: \
	src/ui/local_ui_presentation.c \
	src/ui/local_ui_presentation.h \
	src/ui/local_ui.h \
	src/ui/osk.h \
	src/ui/osk_render.h \
	src/display/display.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_controller39.o: \
	src/ui/local_controller.c \
	src/ui/local_controller.h \
	src/input/controller.h \
	src/ui/local_ui.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/pad39.o: \
	src/input/pad.c \
	src/input/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_graphics39.o: \
	src/platform/ps2_graphics.c \
	src/platform/ps2_graphics.h \
	src/display/display.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
