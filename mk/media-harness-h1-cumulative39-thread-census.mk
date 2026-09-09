# H1 cumulative through-Issue-39 EE-thread census build.
#
# Purpose:
#   Produce one resident diagnostic ELF containing the clean reconstruction
#   subsystem implementations completed through Issue #39 plus the current H1
#   AUDIO/MPEG harness.
#
# Runtime contract for the first hardware census:
#   - H1 AUDIO/MPEG path is active according to CONFIG;
#   - RFB/input/OSK/local-UI modules are linked but not started;
#   - therefore the exact same binary can later activate those subsystems
#     deliberately without changing their source population.
#
# The old Issue-39 app.c coordinator is intentionally NOT linked. It owns fixed
# desktop-sized static presentation buffers and startup policy that would be
# live ELF state even while RFB is disabled. H1 remains the resident coordinator
# and will instantiate each clean module only when its session toggle is ON.
#
# Likewise the Issue-7/39 deterministic sendto wrapper is not linked into this
# H1 transport experiment: wrapping all sendto calls would be a transport-side
# behavior change even when product diagnostics are unused. The ordinary
# diagnostics module itself remains available and inert until explicitly used.
#
# RFB mux preparation rule:
#   The clean through-Issue-39 RFB parser/session source remains mechanically
#   unchanged. In this H1-only cumulative target its three rfb_io.h calls are
#   preprocessor-renamed to experiment-owned mux adapter symbols. The logical
#   channel, CONFIG-sized resource bundle, host-tested credit policy, live
#   channel-1 transport mechanics, adapter binding, and lifecycle cleanup are
#   linked here. PSTVNC_H1_RFB_MUX_PREP enables only cumulative hooks inside the
#   existing H1 transport: resource preparation occurs before the sole receiver
#   starts; channel-1 DATA dispatch and initial/returned CREDIT are wired behind
#   the still-closed CONFIG RFB-ON gate. No Pi VNC bridge or RFB presentation is
#   activated by this build rule.

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
	$(BUILD_DIR)/diagnostics39.o \
	$(BUILD_DIR)/rfb39.o \
	$(BUILD_DIR)/framebuffer39.o \
	$(BUILD_DIR)/rfb_session39.o \
	$(BUILD_DIR)/h1_rfb_mux_io.o \
	$(BUILD_DIR)/h1_rfb_channel.o \
	$(BUILD_DIR)/h1_rfb_credit_policy.o \
	$(BUILD_DIR)/h1_rfb_runtime_resources.o \
	$(BUILD_DIR)/h1_rfb_transport_live.o \
	$(BUILD_DIR)/h1_transport_runtime_rfb_lifecycle.o \
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

# Preserve the qualified libraries required by the linked clean modules. H1
# already supplies shared PS2 system/network objects and embedded SIO2/PAD/DEV9/
# NETMAN/SMAP modules, so those are deliberately not duplicated here.
EXTRA_EE_LIBS := \
	-L$(GSKIT)/lib \
	-lgskit \
	-ldmakit \
	-lpad

#
# Cumulative-only transport seam:
#   Rename the original public lifecycle definitions to inner names, retain the
#   wrapper for cleanup, and enable the RFB-preparation hook points in the same
#   source. Other H1 targets are unaffected by these target-specific flags.
#
$(BUILD_DIR)/h1_transport_runtime.o: EE_CFLAGS += \
	-DPSTVNC_H1_RFB_MUX_PREP=1 \
	-Dpstvnc_h1_transport_start=pstvnc_h1_transport_start_inner \
	-Dpstvnc_h1_transport_shutdown=pstvnc_h1_transport_shutdown_inner

include mk/media-harness-h1-thread-census-diag.mk

$(BUILD_DIR)/diagnostics39.o: \
	src/diagnostics/diagnostics.c \
	src/diagnostics/diagnostics.h | $(BUILD_DIR)
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
	src/framebuffer/framebuffer.h \
	experiments/media-harness-h1/h1_rfb_mux_io.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) \
		-Dpstvnc_rfb_io_read_exact=pstvnc_h1_rfb_mux_io_read_exact \
		-Dpstvnc_rfb_io_poll_receive=pstvnc_h1_rfb_mux_io_poll_receive \
		-Dpstvnc_rfb_io_write_exact=pstvnc_h1_rfb_mux_io_write_exact \
		-c $< -o $@

$(BUILD_DIR)/h1_rfb_mux_io.o: \
	experiments/media-harness-h1/h1_rfb_mux_io.c \
	experiments/media-harness-h1/h1_rfb_mux_io.h \
	experiments/media-harness-h1/h1_rfb_transport_live.h \
	experiments/media-harness-h1/h1_transport_runtime.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_channel.o: \
	experiments/media-harness-h1/h1_rfb_channel.c \
	experiments/media-harness-h1/h1_rfb_channel.h \
	experiments/audio-transport/common/transport_queue.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_credit_policy.o: \
	experiments/media-harness-h1/h1_rfb_credit_policy.c \
	experiments/media-harness-h1/h1_rfb_credit_policy.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_runtime_resources.o: \
	experiments/media-harness-h1/h1_rfb_runtime_resources.c \
	experiments/media-harness-h1/h1_rfb_runtime_resources.h \
	experiments/media-harness-h1/h1_rfb_channel.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_transport_live.o: \
	experiments/media-harness-h1/h1_rfb_transport_live.c \
	experiments/media-harness-h1/h1_rfb_transport_live.h \
	experiments/media-harness-h1/h1_rfb_credit_policy.h \
	experiments/media-harness-h1/h1_rfb_mux_io.h \
	experiments/media-harness-h1/h1_transport_runtime.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_transport_runtime_rfb_lifecycle.o: \
	experiments/media-harness-h1/h1_transport_runtime_rfb_lifecycle.c \
	experiments/media-harness-h1/h1_transport_runtime.h \
	experiments/media-harness-h1/h1_rfb_transport_live.h | $(BUILD_DIR)
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
