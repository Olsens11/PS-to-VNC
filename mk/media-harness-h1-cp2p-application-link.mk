# CP2P explicit application-link checkpoint derived directly from CP2O.
#
# Purpose:
#   Produce a distinct CP2P PS2 ELF while preserving CP2O's proven visible-RFB
#   + optional-PCM resident shell and using the real CP2P session coordinator
#   for calibration acceptance, generation ownership, START transmission, and
#   the combined RFB policy.
#
# Item #9 links the real exact-generation MPEG worker and generated CP2P
# decoder/runtime. The public CONFIG gate remains MPEG OFF until item #10, so
# this checkpoint proves worker readiness without claiming all-guns activation.

BUILD_DIR := build/experiments/media-harness-h1-cp2p-application-link/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2P-ApplicationLink.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_cp2p_visible_interaction_pcm.c

include mk/media-harness-h1-cp2o-visible-rfb-interaction-pcm.mk

CP2P_CALIBRATION_OBJECTS := \
	$(BUILD_DIR)/h1_mpeg_presentation_owner.o \
	$(BUILD_DIR)/h1_mpeg_start_handoff.o \
	$(BUILD_DIR)/h1_mpeg_cp2p_rfb_flow.o \
	$(BUILD_DIR)/h1_mpeg_recalibration.o \
	$(BUILD_DIR)/h1_mpeg_start_wire.o \
	$(BUILD_DIR)/h1_mpeg_start_transport.o

CP2P_SESSION_OBJECTS := \
	$(BUILD_DIR)/h1_cp2p_session_coordinator.o \
	$(BUILD_DIR)/h1_cp2p_mpeg_worker.o \
	$(BUILD_DIR)/h1_video_runtime_cp2p.o

EXTRA_EE_OBJS += $(CP2P_CALIBRATION_OBJECTS) $(CP2P_SESSION_OBJECTS)
# Replace the inherited standalone GS-owning runtime with the CP2P compositor runtime.
EE_OBJS := $(filter-out $(BUILD_DIR)/h1_video_runtime.o,$(EE_OBJS))
$(EE_BIN): $(CP2P_CALIBRATION_OBJECTS) $(CP2P_SESSION_OBJECTS)

# Build the explicit CP2P derivative while leaving CP2O's qualified main source
# byte-for-byte untouched.
$(BUILD_DIR)/h1_main.o: \
	experiments/media-harness-h1/h1_main_cp2p_visible_interaction_pcm.c \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.h \
	experiments/media-harness-h1/h1_cp2p_mpeg_worker.h
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(CP2P_CALIBRATION_OBJECTS): $(BUILD_DIR)/%.o: \
	$(CALIBRATION_DIR)/%.c $(CALIBRATION_HEADERS) | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/h1_cp2p_session_coordinator.o: \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.c \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.h \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	$(CALIBRATION_HEADERS) | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@


CP2P_GEN_DIR := $(BUILD_DIR)/generated-cp2p
CP2P_VIDEO_GENERATOR := experiments/media-harness-h1/generate_h1_video_runtime_cp2p.py
CP2P_VIDEO_SOURCE := experiments/media-harness-h1/h1_video_runtime.c
CP2P_VIDEO_GENERATED := $(CP2P_GEN_DIR)/h1_video_runtime_cp2p_generated.c

$(CP2P_GEN_DIR):
	mkdir -p $@

$(CP2P_VIDEO_GENERATED): $(CP2P_VIDEO_SOURCE) $(CP2P_VIDEO_GENERATOR) | $(CP2P_GEN_DIR)
	python3 $(CP2P_VIDEO_GENERATOR) --input $(CP2P_VIDEO_SOURCE) --output $@

$(BUILD_DIR)/h1_video_runtime_cp2p.o: $(CP2P_VIDEO_GENERATED) \
	experiments/media-harness-h1/h1_video_runtime_cp2p.h \
	experiments/media-harness-h1/h1_cumulative39_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $(CP2P_VIDEO_GENERATED) -o $@

$(BUILD_DIR)/h1_cp2p_mpeg_worker.o: \
	experiments/media-harness-h1/h1_cp2p_mpeg_worker.c \
	experiments/media-harness-h1/h1_cp2p_mpeg_worker.h \
	experiments/media-harness-h1/h1_video_runtime_cp2p.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/ps2_graphics39.o: \
	experiments/media-harness-h1/h1_cumulative39_graphics.c \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h \
	$(CALIBRATION_DIR)/h1_mpeg_presentation_owner.h \
	$(CALIBRATION_DIR)/mpeg_presentation_calibration.h \
	src/platform/ps2_graphics.c \
	src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c \
		experiments/media-harness-h1/h1_cumulative39_graphics.c -o $@

.PHONY: cp2p-application-link-check
cp2p-application-link-check: $(EE_BIN)
	@test -f $(BUILD_DIR)/h1_main.o
	@test -f $(BUILD_DIR)/h1_cp2p_session_coordinator.o
	@test -f $(BUILD_DIR)/h1_cp2p_mpeg_worker.o
	@test -f $(BUILD_DIR)/h1_video_runtime_cp2p.o
	@test -f $(BUILD_DIR)/h1_mpeg_presentation_owner.o
	@test -f $(BUILD_DIR)/h1_mpeg_start_handoff.o
	@test -f $(BUILD_DIR)/h1_mpeg_cp2p_rfb_flow.o
	@test -f $(BUILD_DIR)/h1_mpeg_recalibration.o
	@test -f $(BUILD_DIR)/h1_mpeg_start_wire.o
	@test -f $(BUILD_DIR)/h1_mpeg_start_transport.o
	@test -f $(BUILD_DIR)/ps2_graphics39.o
	@echo H1_CP2P_APPLICATION_LINK=PASS
	@echo H1_CP2P_BASELINE=CP2O
	@echo H1_CP2P_SESSION_COORDINATOR=LIVE
	@echo H1_CP2P_ACCEPT_TO_START=LIVE
	@echo H1_CP2P_GRAPHICS_OWNER=SHARED_COMPOSITOR
	@echo H1_CP2P_MPEG_WORKER=GENERATION_BOUND_GATE_DORMANT
	@echo H1_CP2P_MPEG_CANCELLABLE_READ=LIVE
