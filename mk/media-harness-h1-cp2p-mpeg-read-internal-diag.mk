# CP2P priority-67 persistent MPEG witness plus internal queue-read stages.
#
# This derivative includes the exact parent CP2P application-link composition
# and overrides only the h1_transport_runtime.o source recipe so the ordinary
# transport implementation is compiled through the observation-only wrapper.
# All inherited target-specific EE_CFLAGS remain in force, including the RFB mux
# preparation, lifecycle symbol renames, and capability override.

include mk/media-harness-h1-cp2p-application-link.mk

H1_MPEG_READ_DIAG_SOURCE := \
	experiments/media-harness-h1/h1_transport_runtime_mpeg_read_diag.c
H1_TRANSPORT_SOURCE := \
	experiments/media-harness-h1/h1_transport_runtime.c

$(BUILD_DIR)/h1_transport_runtime.o: \
	$(H1_MPEG_READ_DIAG_SOURCE) \
	$(H1_TRANSPORT_SOURCE) \
	experiments/media-harness-h1/h1_transport_runtime.h \
	experiments/media-harness-h1/h1_config.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $(H1_MPEG_READ_DIAG_SOURCE) -o $@

.PHONY: cp2p-mpeg-read-internal-diag-check
cp2p-mpeg-read-internal-diag-check: cp2p-application-link-check
	@test -f $(BUILD_DIR)/h1_transport_runtime.o
	@echo H1_CP2P_MPEG_READ_INTERNAL_WITNESS=PASS
	@echo H1_CP2P_MPEG_READ_PARENT=P67_PERSISTENT_WITNESS
