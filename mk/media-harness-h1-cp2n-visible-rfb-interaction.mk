# CP2N visible RFB + real through-Issue-39 local interaction H1 candidate.
#
# Retains CP2L's hardware-qualified one-socket mux/RFB/visible/mouse foundation
# and composes the already-linked clean local-controller, local-UI, OSK,
# keyboard, local-presentation, display, graphics, and input-runtime modules.
# CP2N adds no experiment-specific keyboard chord or controller semantics.
#
# AUDIO and MPEG remain OFF. This checkpoint is not Issue #40 and is not the
# later architectural-alignment/integration pass.

BUILD_DIR := build/experiments/media-harness-h1-cp2n-visible-rfb-interaction/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2N-VisibleRFBInteraction.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_interaction_entry.c

include mk/media-harness-h1-cumulative39-thread-census.mk

# The clean through-Issue-39 domain objects are already linked by the cumulative
# target. CP2N adds only experiment-local cross-domain composition scaffolding.
EXTRA_EE_OBJS += $(BUILD_DIR)/h1_interaction_coordinator.o

$(EE_BIN): $(BUILD_DIR)/h1_interaction_coordinator.o

$(BUILD_DIR)/h1_main.o: \
	experiments/media-harness-h1/h1_main_rfb_visible_interaction_entry.c \
	experiments/media-harness-h1/h1_main_rfb_visible_interaction.c \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	experiments/media-harness-h1/h1_rfb_session_runtime.h \
	experiments/media-harness-h1/h1_transport_runtime.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_interaction_coordinator.o: \
	experiments/media-harness-h1/h1_interaction_coordinator.c \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	src/input/input_runtime.h \
	src/input/input.h \
	src/input/controller.h \
	src/input/keyboard.h \
	src/input/mouse.h \
	src/ui/local_controller.h \
	src/ui/local_ui.h \
	src/ui/local_ui_presentation.h \
	src/ui/osk.h \
	src/ui/osk_render.h \
	src/rfb/rfb.h \
	src/rfb/rfb_session.h \
	src/framebuffer/framebuffer.h \
	src/display/display.h \
	src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

# Preserve the already-qualified CP2J queue/credit/mux/quiesce implementation
# under visible CONFIG mode 2 exactly as CP2K and CP2L do.
$(BUILD_DIR)/h1_rfb_transport_live.o: \
	experiments/media-harness-h1/h1_rfb_transport_live_visible.c \
	experiments/media-harness-h1/h1_rfb_transport_live.c \
	experiments/media-harness-h1/h1_rfb_transport_live.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_session_runtime.o: \
	experiments/media-harness-h1/h1_rfb_session_runtime_visible.c \
	experiments/media-harness-h1/h1_rfb_session_runtime.c \
	experiments/media-harness-h1/h1_rfb_session_runtime.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
