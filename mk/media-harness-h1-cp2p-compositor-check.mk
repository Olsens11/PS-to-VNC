# CP2P calibrated shared-compositor/runtime compile-only diagnostic.
#
# This target does not produce or link a runnable ELF. It exists only to make
# the pinned PS2 EE/gsKit toolchain compile the dormant CP2P graphics owner,
# generated MPEG runtime, and the CP2P-specific interaction/recalibration wiring
# before final all-guns session policy is enabled.

BUILD_DIR := build/experiments/media-harness-h1-cp2p-compositor-check/ps2
CP2P_GEN_DIR := $(BUILD_DIR)/generated-cp2p
CP2P_VIDEO_GENERATED := $(CP2P_GEN_DIR)/h1_video_runtime_cp2p_generated.c
CALIBRATION_DIR := experiments/media-harness-h1/mpeg_presentation_calibration

include mk/media-harness-h1-cumulative39-thread-census.mk

.PHONY: cp2p-compositor-check

cp2p-compositor-check: \
	$(BUILD_DIR)/h1_cp2p_shared_compositor_check.o \
	$(BUILD_DIR)/h1_cp2p_video_runtime_check.o \
	$(BUILD_DIR)/h1_cp2p_recalibration_entry_bridge_check.o \
	$(BUILD_DIR)/h1_cp2p_interaction_coordinator_check.o \
	$(BUILD_DIR)/h1_cp2p_interaction_wrapper_check.o
	@echo H1_CP2P_SHARED_COMPOSITOR_COMPILE=PASS
	@echo H1_CP2P_VIDEO_RUNTIME_COMPILE=PASS
	@echo H1_CP2P_RECALIBRATION_ENTRY_BRIDGE_COMPILE=PASS
	@echo H1_CP2P_INTERACTION_COORDINATOR_COMPILE=PASS
	@echo H1_CP2P_INTERACTION_WRAPPER_COMPILE=PASS

$(CP2P_GEN_DIR):
	mkdir -p $@

$(BUILD_DIR)/h1_cp2p_shared_compositor_check.o: \
	experiments/media-harness-h1/h1_cumulative39_graphics.c \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h \
	$(CALIBRATION_DIR)/h1_mpeg_presentation_owner.h \
	$(CALIBRATION_DIR)/mpeg_presentation_calibration.h \
	src/platform/ps2_graphics.c \
	src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(CP2P_VIDEO_GENERATED): \
	experiments/media-harness-h1/generate_h1_video_runtime_cp2p.py \
	experiments/media-harness-h1/h1_video_runtime.c | $(CP2P_GEN_DIR)
	python3 experiments/media-harness-h1/generate_h1_video_runtime_cp2p.py \
		--input experiments/media-harness-h1/h1_video_runtime.c \
		--output $@

$(BUILD_DIR)/h1_cp2p_video_runtime_check.o: \
	$(CP2P_VIDEO_GENERATED) \
	experiments/media-harness-h1/h1_video_runtime.h \
	experiments/media-harness-h1/h1_video_runtime_cp2p.h \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h \
	$(CALIBRATION_DIR)/h1_mpeg_presentation_owner.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $(CP2P_VIDEO_GENERATED) -o $@

$(BUILD_DIR)/h1_cp2p_recalibration_entry_bridge_check.o: \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration_entry_bridge.c \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration_entry_bridge.h \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration.h \
	$(CALIBRATION_DIR)/h1_mpeg_calibration_entry_hold.h \
	$(CALIBRATION_DIR)/h1_mpeg_cp2p_rfb_flow.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/h1_cp2p_interaction_coordinator_check.o: \
	experiments/media-harness-h1/h1_interaction_coordinator_cp2p.c \
	experiments/media-harness-h1/h1_interaction_coordinator.c \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration_entry_bridge.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/h1_cp2p_interaction_wrapper_check.o: \
	experiments/media-harness-h1/h1_interaction_cp2p.c \
	experiments/media-harness-h1/h1_interaction_cp2p.h \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	$(CALIBRATION_DIR)/h1_mpeg_recalibration_entry_bridge.h \
	$(CALIBRATION_DIR)/h1_mpeg_cp2p_rfb_flow.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@
