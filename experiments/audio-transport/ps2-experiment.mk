# Dedicated-link transport experimental PS2 build.
#
# Extends the Issue #39 chassis only for this distinct ELF.

BUILD_DIR ?= build/experiments/audio-transport/ps2
EE_BIN ?= $(BUILD_DIR)/PS-to-VNC-AudioTransport-EXP1.ELF

include mk/issue39-keyboard-osk-local-ui.mk

TRANSPORT_EXP_OBJS = \
	$(BUILD_DIR)/transport_protocol_exp.o \
	$(BUILD_DIR)/transport_queue_exp.o \
	$(BUILD_DIR)/transport_runtime_exp.o \
	$(BUILD_DIR)/audio_runtime_exp.o \
	$(BUILD_DIR)/experiment_adapter_exp.o \
	$(BUILD_DIR)/AUDSRV_irx.o

EE_OBJS += $(TRANSPORT_EXP_OBJS)

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

$(EE_BIN): $(TRANSPORT_EXP_OBJS)

$(BUILD_DIR)/transport_protocol_exp.o: \
	experiments/audio-transport/common/transport_protocol.c \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_queue_exp.o: \
	experiments/audio-transport/common/transport_queue.c \
	experiments/audio-transport/common/transport_queue.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_runtime_exp.o: \
	experiments/audio-transport/ps2/transport_runtime.c \
	experiments/audio-transport/ps2/transport_runtime.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/audio_runtime_exp.o: \
	experiments/audio-transport/ps2/audio_runtime.c \
	experiments/audio-transport/ps2/audio_runtime.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/experiment_adapter_exp.o: \
	experiments/audio-transport/ps2/experiment_adapter.c \
	experiments/audio-transport/ps2/audio_runtime.h \
	experiments/audio-transport/ps2/transport_runtime.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(GEN_DIR)/AUDSRV_irx.c: \
	$(PS2SDK)/iop/irx/audsrv.irx | $(GEN_DIR)
	bin2c $< $@ AUDSRV_irx

$(BUILD_DIR)/AUDSRV_irx.o: \
	$(GEN_DIR)/AUDSRV_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@
