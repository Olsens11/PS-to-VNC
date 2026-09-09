# CP2K visible RFB-only H1 hardware candidate.
#
# This target deliberately reuses the CP2J cumulative through-Issue-39 source
# population, one-socket RFB transport, CONFIG gate, parser seam, and clean
# quiesce machinery. The only coordinator substitution is h1_main_rfb_visible.c,
# which initializes the already-existing through-Issue-39 PS2 graphics presenter
# and invokes the optional complete-frame callback seam.
#
# AUDIO, MPEG, controller input, pointer, keyboard, OSK, and local UI remain
# runtime-inactive. This is not the future hybrid compositor.

BUILD_DIR := build/experiments/media-harness-h1-cp2k-visible-rfb/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2K-VisibleRFB.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible.c

include mk/media-harness-h1-cumulative39-thread-census.mk
