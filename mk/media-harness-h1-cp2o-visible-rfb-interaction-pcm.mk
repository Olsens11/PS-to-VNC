# CP2O visible RFB + real through-Issue-39 interaction + existing PCM H1 candidate.
#
# Builds directly from the CP2N hardware-qualified composition. The cumulative
# H1 target already links the canonical PCM/AUDSRV runtime; CP2O changes only
# activation/composition policy so visible RFB mode 2 may run concurrently with
# PCM while MPEG remains OFF.
#
# This is still one physical PSTV TCP connection. No new audio implementation,
# RFB parser, controller semantic, keyboard semantic, or transport framing is
# introduced by this target.

BUILD_DIR := build/experiments/media-harness-h1-cp2o-visible-rfb-interaction-pcm/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2O-VisibleRFBInteractionPCM.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_interaction_pcm.c

include mk/media-harness-h1-cumulative39-thread-census.mk

EXTRA_EE_OBJS += $(BUILD_DIR)/h1_interaction_coordinator.o

$(EE_BIN): $(BUILD_DIR)/h1_interaction_coordinator.o

$(BUILD_DIR)/h1_main.o: \
	experiments/media-harness-h1/h1_main_rfb_visible_interaction_pcm.c \
	experiments/media-harness-h1/h1_audio_runtime.h \
	experiments/media-harness-h1/h1_media_clock.h \
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

# Replace only the cumulative experiment's public CONFIG gate. The underlying
# h1_config.c object remains the same renamed inner validator.
$(BUILD_DIR)/h1_config_rfb_activation_gate.o: \
	experiments/media-harness-h1/h1_config_rfb_pcm_activation_gate.c \
	experiments/media-harness-h1/h1_config.h \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c \
		experiments/media-harness-h1/h1_config_rfb_pcm_activation_gate.c \
		-o $@

# Preserve CP2N's exact visible RFB transport and presenter mechanics.
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
