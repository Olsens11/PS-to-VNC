# CP2K visible RFB-only H1 hardware candidate.
#
# This target deliberately reuses the CP2J cumulative through-Issue-39 source
# population, one-socket RFB transport, CONFIG gate, parser seam, and clean
# quiesce machinery. CP2K has one presentation owner: the through-Issue-39 PS2
# graphics path called by h1_main_rfb_visible.c.
#
# CONFIG mode 2 is mechanically scoped onto the already-qualified CP2J mode-1
# transport/session implementation through tiny wrapper translation units. This
# keeps mode 1 headless while avoiding any algorithmic queue/credit/mux/quiesce
# change before visible hardware qualification.
#
# AUDIO, MPEG, controller input, pointer, keyboard, OSK, and local UI remain
# runtime-inactive. This is not the future hybrid compositor.

BUILD_DIR := build/experiments/media-harness-h1-cp2k-visible-rfb/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2K-VisibleRFB.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_entry.c

include mk/media-harness-h1-cumulative39-thread-census.mk

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
