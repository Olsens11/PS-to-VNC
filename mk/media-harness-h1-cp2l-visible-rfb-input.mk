# CP2L visible RFB + PS2 mouse-input H1 hardware candidate.
#
# This target retains CP2K's mode-2 visible RFB transport/presenter boundary and
# adds only the existing through-Issue-39 controller/input runtime plus one narrow
# experiment-owned application-side mouse service. The controller worker does not
# own RFB writes; the H1 RFB coordinator drains semantic input only at complete
# server-message boundaries on the main thread.
#
# AUDIO, MPEG, keyboard, OSK, and local UI remain inactive. One physical PSTV
# TCP connection remains the sole Pi-to-PS2 transport. This is not Issue #40 and
# not the future hybrid compositor.

BUILD_DIR := build/experiments/media-harness-h1-cp2l-visible-rfb-input/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2L-VisibleRFBInput.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_input_entry.c

include mk/media-harness-h1-cumulative39-thread-census.mk

# The cumulative makefile's EE_OBJS references EXTRA_EE_OBJS recursively, so
# this append adds only the CP2L experiment-owned input adapter to the already-
# linked through-Issue-39 input/runtime/pad objects.
EXTRA_EE_OBJS += $(BUILD_DIR)/h1_rfb_input_service.o

$(BUILD_DIR)/h1_main.o: \
	experiments/media-harness-h1/h1_main_rfb_visible_input_entry.c \
	experiments/media-harness-h1/h1_main_rfb_visible_input.c \
	experiments/media-harness-h1/h1_rfb_input_service.h \
	experiments/media-harness-h1/h1_rfb_session_runtime.h \
	experiments/media-harness-h1/h1_transport_runtime.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_rfb_input_service.o: \
	experiments/media-harness-h1/h1_rfb_input_service.c \
	experiments/media-harness-h1/h1_rfb_input_service.h \
	src/input/input_runtime.h \
	src/input/input.h \
	src/input/mouse.h \
	src/rfb/rfb_session.h \
	src/rfb/rfb.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

# Preserve the already-qualified CP2J queue/credit/mux/quiesce implementation
# under CONFIG mode 2 exactly as CP2K does.
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
