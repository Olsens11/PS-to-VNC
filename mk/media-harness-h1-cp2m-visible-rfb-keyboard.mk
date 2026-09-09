# CP2M visible RFB + PS2 mouse/keyboard H1 hardware candidate.
#
# This target retains CP2L's hardware-qualified visible RFB + controller mouse
# boundary and adds only the historical Test11F transient L1+D-pad arrow-key
# chord. The controller worker remains a semantic producer. Main/application
# serializes pointer and key messages through the already-synchronized RFB
# session at complete server-message boundaries.
#
# AUDIO, MPEG, OSK, local UI, and general runtime-hotkey behavior remain
# inactive. One physical PSTV TCP connection remains the sole Pi-to-PS2
# transport. This is not Issue #40 and not the future hybrid compositor.

BUILD_DIR := build/experiments/media-harness-h1-cp2m-visible-rfb-keyboard/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2M-VisibleRFBKeyboard.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_keyboard_input_entry.c

include mk/media-harness-h1-cumulative39-thread-census.mk

EXTRA_EE_OBJS += \
	$(BUILD_DIR)/h1_rfb_keyboard_chord.o \
	$(BUILD_DIR)/h1_rfb_keyboard_input_service.o

# The inherited final-link rule was parsed before the CP2M-only append above.
# Make both new objects explicit prerequisites so make cannot reach the link
# before their recipes have completed.
$(EE_BIN): \
	$(BUILD_DIR)/h1_rfb_keyboard_chord.o \
	$(BUILD_DIR)/h1_rfb_keyboard_input_service.o

$(BUILD_DIR)/h1_main.o: \
	experiments/media-harness-h1/h1_main_rfb_visible_keyboard_input_entry.c \
	experiments/media-harness-h1/h1_main_rfb_visible_keyboard_input.c \
	experiments/media-harness-h1/h1_rfb_keyboard_input_service.h \
	experiments/media-harness-h1/h1_rfb_session_runtime.h \
	experiments/media-harness-h1/h1_transport_runtime.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_keyboard_chord.o: \
	experiments/media-harness-h1/h1_rfb_keyboard_chord.c \
	experiments/media-harness-h1/h1_rfb_keyboard_chord.h \
	src/input/controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_keyboard_input_service.o: \
	experiments/media-harness-h1/h1_rfb_keyboard_input_service.c \
	experiments/media-harness-h1/h1_rfb_keyboard_input_service.h \
	experiments/media-harness-h1/h1_rfb_keyboard_chord.h \
	src/input/input_runtime.h \
	src/input/input.h \
	src/input/controller.h \
	src/input/keyboard.h \
	src/input/mouse.h \
	src/rfb/rfb_session.h \
	src/rfb/rfb.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

# Preserve the already-qualified CP2J queue/credit/mux/quiesce implementation
# under CONFIG mode 2 exactly as CP2K and CP2L do.
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
