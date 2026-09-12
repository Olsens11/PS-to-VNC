# CP2P calibrated shared-compositor compile-only diagnostic.
#
# This target does not produce or link a runnable ELF. It exists only to make
# the pinned PS2 EE/gsKit toolchain compile the dormant CP2P graphics owner
# before that owner is connected to MPEG/session policy.

BUILD_DIR := build/experiments/media-harness-h1-cp2p-compositor-check/ps2

include mk/media-harness-h1-cumulative39-thread-census.mk

.PHONY: cp2p-compositor-check

cp2p-compositor-check: $(BUILD_DIR)/h1_cp2p_shared_compositor_check.o
	@echo H1_CP2P_SHARED_COMPOSITOR_COMPILE=PASS

$(BUILD_DIR)/h1_cp2p_shared_compositor_check.o: \
	experiments/media-harness-h1/h1_cumulative39_graphics.c \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_start_handoff.h \
	experiments/media-harness-h1/mpeg_presentation_calibration/h1_mpeg_presentation_owner.h \
	experiments/media-harness-h1/mpeg_presentation_calibration/mpeg_presentation_calibration.h \
	src/platform/ps2_graphics.c \
	src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
