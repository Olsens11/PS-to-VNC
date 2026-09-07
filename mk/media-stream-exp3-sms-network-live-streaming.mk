# Media Stream EXP3 O3 — finite live X11 MPEG streaming to SMS.
#
# O3 preserves the hardware-qualified SMS/IPU/GS path and O2's dynamic
# byte-ring/credit architecture. The only new runtime boundary is that MPEG
# bytes are produced live by ffmpeg and terminated by explicit producer
# metadata instead of a pre-known fixture size.
#
# System -lmpeg is deliberately NOT linked.

BUILD_DIR ?= \
	build/experiments/media-stream-exp3/sms-network-live-streaming

GEN_DIR = \
	$(BUILD_DIR)/generated

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-SMS-NetworkLiveStreaming.ELF

SMS_VENDOR = \
	experiments/media-stream-exp3/vendor/sms-libmpeg

SMS_INC = \
	$(SMS_VENDOR)/include

SMS_SRC = \
	$(SMS_VENDOR)/src

PS2IP_LIB = \
	baseline/frozen-b4a/libps2ip_mtu1458_wscale128.a

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
	-Iexperiments/media-stream-exp3 \
	-Iexperiments/audio-transport/common \
	-Isrc/platform \
	-Isrc/rfb

EE_OBJS = \
	$(BUILD_DIR)/reference_network_live_streaming.o \
	$(BUILD_DIR)/network_live_stream_runtime.o \
	$(BUILD_DIR)/transport_queue_exp3.o \
	$(BUILD_DIR)/transport_protocol_exp3.o \
	$(BUILD_DIR)/ps2_system_exp3.o \
	$(BUILD_DIR)/ps2_network_exp3.o \
	$(BUILD_DIR)/sms_libmpeg.o \
	$(BUILD_DIR)/sms_libmpeg_core.o \
	$(BUILD_DIR)/SIO2MAN_irx.o \
	$(BUILD_DIR)/PADMAN_irx.o \
	$(BUILD_DIR)/DEV9_irx.o \
	$(BUILD_DIR)/NETMAN_irx.o \
	$(BUILD_DIR)/SMAP_irx.o

EE_LIBS = \
	-ldraw \
	-lgraph \
	-ldma \
	-lnetman \
	$(PS2IP_LIB) \
	-lpatches \
	-lc \
	-lpacket \
	-Wl,--wrap=sendto

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR) $(GEN_DIR):
	mkdir -p $@

$(BUILD_DIR)/reference_network_live_streaming.o: \
	experiments/media-stream-exp3/reference_network_live_streaming.c \
	experiments/media-stream-exp3/network_live_stream_runtime.h | $(BUILD_DIR)
	$(EE_CC) \
		$(EE_CFLAGS) \
		$(EE_INCS) \
		-c $< \
		-o $@

$(BUILD_DIR)/network_live_stream_runtime.o: \
	experiments/media-stream-exp3/network_live_stream_runtime.c \
	experiments/media-stream-exp3/network_live_stream_runtime.h \
	experiments/audio-transport/common/transport_queue.h \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) \
		$(EE_CFLAGS) \
		$(EE_INCS) \
		-c $< \
		-o $@

$(BUILD_DIR)/transport_queue_exp3.o: \
	experiments/audio-transport/common/transport_queue.c \
	experiments/audio-transport/common/transport_queue.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_protocol_exp3.o: \
	experiments/audio-transport/common/transport_protocol.c \
	experiments/audio-transport/common/transport_protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_system_exp3.o: \
	src/platform/ps2_system.c \
	src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_network_exp3.o: \
	src/platform/ps2_network.c \
	src/platform/ps2_network.h \
	src/rfb/rfb_io.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/sms_libmpeg.o: \
	$(SMS_SRC)/libmpeg.c \
	$(SMS_INC)/libmpeg.h \
	$(SMS_INC)/libmpeg_internal.h | $(BUILD_DIR)
	$(EE_CC) \
		$(SMS_DECODER_CFLAGS) \
		-I$(SMS_INC) \
		$(EE_INCS) \
		-c $< \
		-o $@

$(BUILD_DIR)/sms_libmpeg_core.o: \
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

$(BUILD_DIR)/SIO2MAN_irx.o: \
	$(GEN_DIR)/SIO2MAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/PADMAN_irx.o: \
	$(GEN_DIR)/PADMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/DEV9_irx.o: \
	$(GEN_DIR)/DEV9_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/NETMAN_irx.o: \
	$(GEN_DIR)/NETMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/SMAP_irx.o: \
	$(GEN_DIR)/SMAP_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
