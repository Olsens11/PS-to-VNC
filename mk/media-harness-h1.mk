# H1 resident configurable AUDIO + MPEG2 media harness.
#
# One PS2-facing PSTV connection carries logical AUDIO and MPEG2 channels.
# The mature SMS decoder objects retain the exact EXP3 build flags; H1 adds
# session-scoped transport/audio/video ownership and explicit teardown.

BUILD_DIR ?= build/experiments/media-harness-h1/ps2

GEN_DIR = $(BUILD_DIR)/generated

EE_BIN ?= $(BUILD_DIR)/PS2VNC-H1-MediaHarness.ELF

SMS_VENDOR = experiments/media-stream-exp3/vendor/sms-libmpeg
SMS_INC = $(SMS_VENDOR)/include
SMS_SRC = $(SMS_VENDOR)/src

PS2IP_LIB = baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a

SMS_DECODER_CFLAGS = \
	-D_EE \
	-O2 \
	-G8192 \
	-mgpopt \
	-mno-abicalls \
	-Wall \
	-mno-check-zero-division

EE_INCS += \
	-I$(SMS_INC) \
	-Iexperiments/media-harness-h1 \
	-Iexperiments/audio-transport/common \
	-Isrc/platform \
	-Isrc/rfb

EE_OBJS = \
	$(BUILD_DIR)/h1_main.o \
	$(BUILD_DIR)/h1_config.o \
	$(BUILD_DIR)/h1_transport_runtime.o \
	$(BUILD_DIR)/h1_media_clock.o \
	$(BUILD_DIR)/h1_audio_runtime.o \
	$(BUILD_DIR)/h1_video_runtime.o \
	$(BUILD_DIR)/transport_queue_h1.o \
	$(BUILD_DIR)/transport_protocol_h1.o \
	$(BUILD_DIR)/ps2_system_h1.o \
	$(BUILD_DIR)/ps2_network_h1.o \
	$(BUILD_DIR)/sms_libmpeg_h1.o \
	$(BUILD_DIR)/sms_libmpeg_core_h1.o \
	$(BUILD_DIR)/SIO2MAN_irx_h1.o \
	$(BUILD_DIR)/PADMAN_irx_h1.o \
	$(BUILD_DIR)/DEV9_irx_h1.o \
	$(BUILD_DIR)/NETMAN_irx_h1.o \
	$(BUILD_DIR)/SMAP_irx_h1.o \
	$(BUILD_DIR)/AUDSRV_irx_h1.o

EE_LIBS = \
	-laudsrv \
	-ldraw \
	-lgraph \
	-ldma \
	-lnetman \
	$(PS2IP_LIB) \
	-lpatches \
	-lc \
	-lpacket

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR) $(GEN_DIR):
	mkdir -p $@

$(BUILD_DIR)/h1_main.o: \
	experiments/media-harness-h1/h1_main.c \
	experiments/media-harness-h1/h1_audio_runtime.h \
	experiments/media-harness-h1/h1_video_runtime.h \
	experiments/media-harness-h1/h1_transport_runtime.h \
	experiments/media-harness-h1/h1_media_clock.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_config.o: \
	experiments/media-harness-h1/h1_config.c \
	experiments/media-harness-h1/h1_config.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_transport_runtime.o: \
	experiments/media-harness-h1/h1_transport_runtime.c \
	experiments/media-harness-h1/h1_transport_runtime.h \
	experiments/media-harness-h1/h1_config.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_media_clock.o: \
	experiments/media-harness-h1/h1_media_clock.c \
	experiments/media-harness-h1/h1_media_clock.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_audio_runtime.o: \
	experiments/media-harness-h1/h1_audio_runtime.c \
	experiments/media-harness-h1/h1_audio_runtime.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/h1_video_runtime.o: \
	experiments/media-harness-h1/h1_video_runtime.c \
	experiments/media-harness-h1/h1_video_runtime.h \
	$(SMS_INC)/libmpeg.h \
	$(SMS_INC)/libmpeg_internal.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_queue_h1.o: \
	experiments/audio-transport/common/transport_queue.c \
	experiments/audio-transport/common/transport_queue.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_protocol_h1.o: \
	experiments/audio-transport/common/transport_protocol.c \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_system_h1.o: \
	src/platform/ps2_system.c \
	src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_network_h1.o: \
	src/platform/ps2_network.c \
	src/platform/ps2_network.h \
	src/rfb/rfb_io.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/sms_libmpeg_h1.o: \
	$(SMS_SRC)/libmpeg.c \
	$(SMS_INC)/libmpeg.h \
	$(SMS_INC)/libmpeg_internal.h | $(BUILD_DIR)
	$(EE_CC) \
		$(SMS_DECODER_CFLAGS) \
		-I$(SMS_INC) \
		$(EE_INCS) \
		-c $< \
		-o $@

$(BUILD_DIR)/sms_libmpeg_core_h1.o: \
	$(SMS_SRC)/libmpeg_core.S | $(BUILD_DIR)
	$(EE_CC) \
		$(SMS_DECODER_CFLAGS) \
		-I$(SMS_INC) \
		$(EE_INCS) \
		-c $< \
		-o $@

$(GEN_DIR)/SIO2MAN_irx.c: \
	$(PS2SDK)/iop/irx/freesio2.irx | $(GEN_DIR)
	bin2c $< $@ SIO2MAN_irx

$(GEN_DIR)/PADMAN_irx.c: \
	$(PS2SDK)/iop/irx/freepad.irx | $(GEN_DIR)
	bin2c $< $@ PADMAN_irx

$(GEN_DIR)/DEV9_irx.c: \
	$(PS2SDK)/iop/irx/ps2dev9.irx | $(GEN_DIR)
	bin2c $< $@ DEV9_irx

$(GEN_DIR)/NETMAN_irx.c: \
	$(PS2SDK)/iop/irx/netman.irx | $(GEN_DIR)
	bin2c $< $@ NETMAN_irx

$(GEN_DIR)/SMAP_irx.c: \
	$(PS2SDK)/iop/irx/smap.irx | $(GEN_DIR)
	bin2c $< $@ SMAP_irx

$(GEN_DIR)/AUDSRV_irx.c: \
	$(PS2SDK)/iop/irx/audsrv.irx | $(GEN_DIR)
	bin2c $< $@ AUDSRV_irx

$(BUILD_DIR)/SIO2MAN_irx_h1.o: $(GEN_DIR)/SIO2MAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/PADMAN_irx_h1.o: $(GEN_DIR)/PADMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/DEV9_irx_h1.o: $(GEN_DIR)/DEV9_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/NETMAN_irx_h1.o: $(GEN_DIR)/NETMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/SMAP_irx_h1.o: $(GEN_DIR)/SMAP_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/AUDSRV_irx_h1.o: $(GEN_DIR)/AUDSRV_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
