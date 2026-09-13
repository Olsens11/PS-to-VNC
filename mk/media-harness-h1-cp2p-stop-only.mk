# CP2P MPEG stop-only lifecycle diagnostic.
#
# Purpose:
#   Build the exact priority-67 / persistent-MPEG-witness CP2P application-link
#   baseline while replacing only the session coordinator translation unit with
#   the disposable stop-only wrapper. The first MPEG-owned generation is
#   automatically retired after ten seconds and returned to RFB_ONLY. No second
#   calibration is entered automatically.

include mk/media-harness-h1-cp2p-application-link.mk

STOP_ONLY_SOURCE := \
	experiments/media-harness-h1/h1_cp2p_session_coordinator_stop_only.c

# Override only this object's recipe. The wrapper includes the existing
# coordinator implementation under a private service symbol, then exposes the
# stop-only service wrapper under the ordinary public symbol.
$(BUILD_DIR)/h1_cp2p_session_coordinator.o: \
	$(STOP_ONLY_SOURCE) \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.c \
	experiments/media-harness-h1/h1_cp2p_session_coordinator.h \
	experiments/media-harness-h1/h1_interaction_coordinator.h \
	$(CALIBRATION_HEADERS) | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -I$(CALIBRATION_DIR) -c \
		$(STOP_ONLY_SOURCE) -o $@

.PHONY: cp2p-stop-only-check
cp2p-stop-only-check: cp2p-application-link-check
	@grep -Fq 'H1_CP2P_STOP_ONLY_DELAY_US UINT64_C(10000000)' $(STOP_ONLY_SOURCE)
	@grep -Fq 'H1_CP2P_STOP_ONLY=RETIRE_BEGIN' $(STOP_ONLY_SOURCE)
	@grep -Fq 'H1_CP2P_STOP_ONLY=RFB_ONLY' $(STOP_ONLY_SOURCE)
	@grep -Fq 'pstvnc_h1_mpeg_start_handoff_stop' $(STOP_ONLY_SOURCE)
	@grep -Fq 'thread.initial_priority = 67;' \
		experiments/media-harness-h1/h1_cp2p_mpeg_worker.c
	@echo H1_CP2P_STOP_ONLY=PASS
	@echo H1_CP2P_STOP_ONLY_DELAY_SECONDS=10
	@echo H1_CP2P_STOP_ONLY_SECOND_CALIBRATION=AUTO_DISABLED
	@echo H1_CP2P_STOP_ONLY_PCM_POLICY=UNCHANGED
	@echo H1_CP2P_STOP_ONLY_RFB_POLICY=UNCHANGED
	@echo H1_CP2P_STOP_ONLY_MPEG_PRIORITY=67
