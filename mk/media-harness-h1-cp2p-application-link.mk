# CP2P explicit application-link checkpoint derived directly from CP2O.
#
# Purpose:
#   Produce a distinct CP2P PS2 ELF while preserving CP2O's proven visible-RFB
#   + optional-PCM resident shell and using the real CP2P session coordinator
#   for calibration acceptance, generation ownership, START transmission, and
#   the combined RFB policy.
#
# The MPEG worker itself remains dormant at this checkpoint. Item #9 will
# replace the no-producer retirement seam with the real worker stop/drain path.
# This target therefore proves application composition without falsely claiming
# live all-guns MPEG execution.

BUILD_DIR := build/experiments/media-harness-h1-cp2p-application-link/ps2
EE_BIN := $(BUILD_DIR)/PS2VNC-H1-CP2P-ApplicationLink.ELF
H1_MAIN_SOURCE := experiments/media-harness-h1/h1_main_rfb_visible_interaction_pcm.c

include mk/media-harness-h1-cp2o-visible-rfb-interaction-pcm.mk

CP2P_CALIBRATION_OBJECTS := \
	$(BUILD_DIR)/h1_mpeg_presentation_owner.o \
	$(BUILD_DIR)/h1_mpeg_start_handoff.o \
	$(BUILD_DIR)/h1_mpeg_cp2p_rfb_flow.o \
	$(BUILD_DIR)/h1_mpeg_recalibration.o \
	$(BUILD_DIR)/h1_mpeg_start_wire.o \
	$(BUILD_DIR)/h1_mpeg_start_transport.o

CP2P_SESSION_OBJECTS := \
	$(BUILD_DIR)/h1_cp2p_session_coordinator.o

# EE_OBJS is deliberately recursive in the inherited H1 build. Appending here
# adds the CP2P session/lifecycle objects to the final link without copying or
# replacing CP2O's application/object population.
EXTRA_EE_OBJS += $(CP2P_CALIBRATION_OBJECTS) $(CP2P_SESSION_OBJECTS)

# The inherited link target was parsed before these descendant-only objects were
# declared, so make their build ordering explicit as well as adding them to the
# eventual recursive EE_OBJS link list above.
$(EE_BIN): $(CP2P_CALIBRATION_OBJECTS) $(CP2P_SESSION_OBJECTS)

# Compile the shared CP2O main as the CP2P application shell. This one flag is
# the deliberate application-composition seam: CP2O compiles the same source
# without it and therefore retains its established coordinator behavior.
$(BUILD_DIR)/h1_main.o: EE_CFLAGS += -DPSTVNC_H1_CP2P_APPLICATION=1
$(BUILD_DIR)/h1_main.o: experiments/media-harness-h1/h1_cp2p_session_coordinator.h

# Compile the CP2P-only calibration/presentation modules from their authoritative
# experiment-local sources. CP2O's existing calibration objects remain untouched.
$(CP2P_CALIBRATION_OBJECTS): $(BUILD_DIR)/%.o: \
	$(CALIBRATION_DIR)/%.c $(CALIBRATION_HEADERS) | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

$(BUILD_DIR)/h1_cp2p_session_coordinator.o: \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.c \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.h \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	$(CALIBRATION_HEADERS) | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c $< -o $@

# CP2P's compositor is a wrapper around the exact through-Issue-39 graphics
# owner, not a second graphics owner. It includes src/platform/ps2_graphics.c
# under private *_base_c39 symbols and exports the same public init/present/
# shutdown API plus the MPEG compositor calls. Override the inherited
# ps2_graphics39.o recipe so the final ELF has one public graphics owner only.
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
	@echo H1_CP2P_MPEG_WORKER=DORMANT
