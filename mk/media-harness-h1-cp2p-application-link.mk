# CP2P explicit application-link checkpoint derived directly from CP2O.
#
# Purpose:
#   Produce the first distinct CP2P PS2 ELF while preserving CP2O's proven
#   visible-RFB + optional-PCM resident coordinator, CONFIG gate, one PSTV
#   transport owner, RFB mechanics, input/UI behavior, and dormant MPEG policy.
#
# This checkpoint intentionally does NOT start MPEG and does NOT replace the
# canonical H1 video runtime yet. Those are later CP2P worker/runtime and CONFIG
# activation steps. Here we only link the already-host-proven CP2P ownership,
# START-wire/transport, combined RFB-flow, and shared-compositor machinery into
# the real CP2O-derived application target so later wiring has one concrete ELF.

BUILD_DIR := build/experiments/media-harness-h1-cp2p-application-link/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2P-ApplicationLink.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_interaction_pcm.c

include mk/media-harness-h1-cp2o-visible-rfb-interaction-pcm.mk

CP2P_CALIBRATION_OBJECTS := \
	$(BUILD_DIR)/h1_mpeg_presentation_owner.o \
	$(BUILD_DIR)/h1_mpeg_start_handoff.o \
	$(BUILD_DIR)/h1_mpeg_cp2p_rfb_flow.o \
	$(BUILD_DIR)/h1_mpeg_start_wire.o \
	$(BUILD_DIR)/h1_mpeg_start_transport.o

CP2P_APPLICATION_OBJECTS := \
	$(CP2P_CALIBRATION_OBJECTS) \
	$(BUILD_DIR)/h1_cumulative39_graphics.o

# EE_OBJS is deliberately recursive in the inherited H1 build. Appending here
# therefore adds these dormant CP2P objects to the final link without copying or
# replacing CP2O's application/object population.
EXTRA_EE_OBJS += $(CP2P_APPLICATION_OBJECTS)

# The inherited link target was parsed before these descendant-only objects were
# declared, so make their build ordering explicit as well as adding them to the
# eventual recursive EE_OBJS link list above.
$(EE_BIN): $(CP2P_APPLICATION_OBJECTS)

# Compile the CP2P-only calibration/presentation modules from their authoritative
# experiment-local sources. CP2O's existing calibration objects remain untouched.
$(CP2P_CALIBRATION_OBJECTS): $(BUILD_DIR)/%.o: \
	$(CALIBRATION_DIR)/%.c $(CALIBRATION_HEADERS) | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

# Link the recovered shared compositor as a dormant owner. The CP2O coordinator
# continues presenting RFB exactly as before until the later CP2P runtime hookup.
$(BUILD_DIR)/h1_cumulative39_graphics.o: \
	experiments/media-harness-h1/h1_cumulative39_graphics.c \
	experiments/media-harness-h1/h1_cumulative39_graphics.h \
	$(CALIBRATION_DIR)/h1_mpeg_start_handoff.h \
	$(CALIBRATION_DIR)/h1_mpeg_presentation_owner.h \
	$(CALIBRATION_DIR)/mpeg_presentation_calibration.h \
	src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

.PHONY: cp2p-application-link-check

cp2p-application-link-check: $(EE_BIN)
	@test -f $(BUILD_DIR)/h1_main.o
	@test -f $(BUILD_DIR)/h1_mpeg_presentation_owner.o
	@test -f $(BUILD_DIR)/h1_mpeg_start_handoff.o
	@test -f $(BUILD_DIR)/h1_mpeg_cp2p_rfb_flow.o
	@test -f $(BUILD_DIR)/h1_mpeg_start_wire.o
	@test -f $(BUILD_DIR)/h1_mpeg_start_transport.o
	@test -f $(BUILD_DIR)/h1_cumulative39_graphics.o
	@echo H1_CP2P_APPLICATION_LINK=PASS
	@echo H1_CP2P_BASELINE=CP2O
	@echo H1_CP2P_MPEG_STARTUP=DORMANT
