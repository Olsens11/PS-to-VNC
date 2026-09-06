# Configurable dedicated-link transport EXP2 PS2 build.
#
# This is intentionally separate from ps2-experiment.mk so the proven EXP1
# source/build recipe remains available unchanged for A/B comparison.

BUILD_DIR ?= build/experiments/audio-config-exp2/ps2

EE_BIN ?= \
	$(BUILD_DIR)/PS-to-VNC-AudioTransport-CONFIG-EXP2.ELF

include mk/issue39-keyboard-osk-local-ui.mk

TRANSPORT_EXP2_OBJS = \
	$(BUILD_DIR)/transport_protocol_exp2.o \
	$(BUILD_DIR)/transport_config_exp2.o \
	$(BUILD_DIR)/transport_queue_exp2.o \
	$(BUILD_DIR)/transport_runtime_exp2.o \
	$(BUILD_DIR)/audio_runtime_exp2.o \
	$(BUILD_DIR)/experiment_adapter_exp2.o \
	$(BUILD_DIR)/AUDSRV_irx_exp2.o

EE_OBJS += $(TRANSPORT_EXP2_OBJS)

EE_INCS += \
	-Iexperiments/audio-transport/common \
	-Iexperiments/audio-transport/ps2

EE_LIBS += \
	-laudsrv \
	-Wl,--wrap=pstvnc_ps2_network_connect_vnc \
	-Wl,--wrap=pstvnc_ps2_network_close \
	-Wl,--wrap=pstvnc_rfb_io_read_exact \
	-Wl,--wrap=pstvnc_rfb_io_poll_receive \
	-Wl,--wrap=pstvnc_rfb_io_write_exact

$(EE_BIN): $(TRANSPORT_EXP2_OBJS)

$(BUILD_DIR)/transport_protocol_exp2.o: \
	experiments/audio-transport/common/transport_protocol.c \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_config_exp2.o: \
	experiments/audio-transport/common/transport_config.c \
	experiments/audio-transport/common/transport_config.h \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_queue_exp2.o: \
	experiments/audio-transport/common/transport_queue.c \
	experiments/audio-transport/common/transport_queue.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_runtime_exp2.o: \
	experiments/audio-transport/ps2/transport_runtime_exp2.c \
	experiments/audio-transport/ps2/transport_runtime_exp2.h \
	experiments/audio-transport/common/transport_config.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/audio_runtime_exp2.o: \
	experiments/audio-transport/ps2/audio_runtime_exp2.c \
	experiments/audio-transport/ps2/audio_runtime_exp2.h \
	experiments/audio-transport/ps2/transport_runtime_exp2.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/experiment_adapter_exp2.o: \
	experiments/audio-transport/ps2/experiment_adapter_exp2.c \
	experiments/audio-transport/ps2/audio_runtime_exp2.h \
	experiments/audio-transport/ps2/transport_runtime_exp2.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(GEN_DIR)/AUDSRV_irx.c: \
	$(PS2SDK)/iop/irx/audsrv.irx | $(GEN_DIR)
	bin2c $< $@ AUDSRV_irx

$(BUILD_DIR)/AUDSRV_irx_exp2.o: \
	$(GEN_DIR)/AUDSRV_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
