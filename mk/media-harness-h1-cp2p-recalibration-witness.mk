# CP2P recalibration/retirement persistent-witness derivative.
#
# This makefile includes the exact priority-67 persistent MPEG witness baseline
# and overrides only generated observation-only copies of the interaction
# coordinator, session coordinator, MPEG worker, and video runtime.
#
# No runtime policy, queue size, timing, geometry, decoder policy, transport
# ownership, RFB flow, retirement semantics, or thread priority is changed.

include mk/media-harness-h1-cp2p-application-link.mk

CP2P_RECAL_WITNESS_GENERATOR := \
	experiments/media-harness-h1/generate_h1_cp2p_recalibration_witness.py
CP2P_INTERACTION_RECAL_WITNESS_GENERATOR := \
	experiments/media-harness-h1/generate_h1_interaction_recalibration_witness.py
CP2P_RECAL_WITNESS_DIR := $(BUILD_DIR)/generated-recalibration-witness

CP2P_INTERACTION_SOURCE := \
	experiments/media-harness-h1/h1_interaction_coordinator.c
CP2P_RECAL_SESSION_SOURCE := \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.c
CP2P_RECAL_WORKER_SOURCE := \
	experiments/media-harness-h1/h1_cp2p_mpeg_worker.c

CP2P_INTERACTION_GENERATED := \
	$(CP2P_RECAL_WITNESS_DIR)/h1_interaction_coordinator_recalibration_witness.c
CP2P_RECAL_SESSION_GENERATED := \
	$(CP2P_RECAL_WITNESS_DIR)/h1_cp2p_session_coordinator_recalibration_witness.c
CP2P_RECAL_WORKER_GENERATED := \
	$(CP2P_RECAL_WITNESS_DIR)/h1_cp2p_mpeg_worker_recalibration_witness.c
CP2P_RECAL_VIDEO_GENERATED := \
	$(CP2P_RECAL_WITNESS_DIR)/h1_video_runtime_cp2p_recalibration_witness.c

$(CP2P_RECAL_WITNESS_DIR):
	mkdir -p $@

$(CP2P_INTERACTION_GENERATED): $(CP2P_INTERACTION_SOURCE) \
	$(CP2P_INTERACTION_RECAL_WITNESS_GENERATOR) | $(CP2P_RECAL_WITNESS_DIR)
	python3 $(CP2P_INTERACTION_RECAL_WITNESS_GENERATOR) \
		--input $(CP2P_INTERACTION_SOURCE) --output $@

$(CP2P_RECAL_SESSION_GENERATED): $(CP2P_RECAL_SESSION_SOURCE) \
	$(CP2P_RECAL_WITNESS_GENERATOR) | $(CP2P_RECAL_WITNESS_DIR)
	python3 $(CP2P_RECAL_WITNESS_GENERATOR) \
		--mode session --input $(CP2P_RECAL_SESSION_SOURCE) --output $@

$(CP2P_RECAL_WORKER_GENERATED): $(CP2P_RECAL_WORKER_SOURCE) \
	$(CP2P_RECAL_WITNESS_GENERATOR) | $(CP2P_RECAL_WITNESS_DIR)
	python3 $(CP2P_RECAL_WITNESS_GENERATOR) \
		--mode worker --input $(CP2P_RECAL_WORKER_SOURCE) --output $@

$(CP2P_RECAL_VIDEO_GENERATED): $(CP2P_VIDEO_GENERATED) \
	$(CP2P_RECAL_WITNESS_GENERATOR) | $(CP2P_RECAL_WITNESS_DIR)
	python3 $(CP2P_RECAL_WITNESS_GENERATOR) \
		--mode video --input $(CP2P_VIDEO_GENERATED) --output $@

# Override only compile recipes; inherited prerequisites and target-specific
# coordinator symbol-renaming flags remain in force.
$(BUILD_DIR)/h1_interaction_coordinator.o: $(CP2P_INTERACTION_GENERATED)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) \
		-c $(CP2P_INTERACTION_GENERATED) -o $@

$(BUILD_DIR)/h1_cp2p_session_coordinator.o: $(CP2P_RECAL_SESSION_GENERATED)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) \
		-c $(CP2P_RECAL_SESSION_GENERATED) -o $@

$(BUILD_DIR)/h1_cp2p_mpeg_worker.o: $(CP2P_RECAL_WORKER_GENERATED)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) \
		-c $(CP2P_RECAL_WORKER_GENERATED) -o $@

$(BUILD_DIR)/h1_video_runtime_cp2p.o: $(CP2P_RECAL_VIDEO_GENERATED)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) \
		-c $(CP2P_RECAL_VIDEO_GENERATED) -o $@

.PHONY: cp2p-recalibration-witness-check
cp2p-recalibration-witness-check: cp2p-application-link-check \
	$(CP2P_INTERACTION_GENERATED) \
	$(CP2P_RECAL_SESSION_GENERATED) \
	$(CP2P_RECAL_WORKER_GENERATED) \
	$(CP2P_RECAL_VIDEO_GENERATED)
	@grep -Fq '0xE0000001u' $(CP2P_INTERACTION_GENERATED)
	@grep -Fq '0xE0000005u' $(CP2P_INTERACTION_GENERATED)
	@grep -Fq '0xE000000Bu' $(CP2P_INTERACTION_GENERATED)
	@grep -Fq '0xE1000006u' $(CP2P_RECAL_SESSION_GENERATED)
	@grep -Fq '0xE2000004u' $(CP2P_RECAL_SESSION_GENERATED)
	@grep -Fq '0xE3000003u' $(CP2P_RECAL_WORKER_GENERATED)
	@grep -Fq '0xE3100000u' $(CP2P_RECAL_WORKER_GENERATED)
	@grep -Fq 'thread.initial_priority = 67;' $(CP2P_RECAL_WORKER_GENERATED)
	@! grep -Fq 'session->transport->producer_stop_reason = stage;' \
		$(CP2P_RECAL_VIDEO_GENERATED)
	@echo H1_CP2P_RECALIBRATION_PERSISTENT_WITNESS=PASS
	@echo H1_CP2P_MPEG_WORKER_PRIORITY=67
	@echo H1_CP2P_RUNTIME_POLICY=UNCHANGED
