# Media Stream EXP3 — SMS full-stream natural-EOF characterization.
#
# IMPORTANT:
# The test harness itself is the exact same source used by the preceding
# modern-PS2SDK-libmpeg one-picture test.
#
# The only architectural substitution here is decoder implementation:
#
#   modern PS2SDK:
#       system libmpeg.a with the post-2025 C IPU-management core
#
#   this target:
#       pinned SMS src/libmpeg.c
#       pinned SMS src/libmpeg_core.S
#
# System -lmpeg is deliberately NOT linked.

BUILD_DIR ?= \
	build/experiments/media-stream-exp3/sms-full-stream-eof-characterization

EE_BIN ?= \
	$(BUILD_DIR)/PS2VNC-EXP3-SMS-FullStream-EOF-Characterization.ELF

STREAM = \
	$(BUILD_DIR)/test.bin

SMS_VENDOR = \
	experiments/media-stream-exp3/vendor/sms-libmpeg

SMS_INC = \
	$(SMS_VENDOR)/include

SMS_SRC = \
	$(SMS_VENDOR)/src

# Preserve SMS's historical small-data model for the decoder implementation.
#
# SMS itself used -G8192 -mgpopt. This matters because the assembly core
# contains symbolic memory operations in branch delay slots; compiling that
# source under the control harness's -G0 model can force pseudo-instruction
# expansion and alter instruction placement.
SMS_DECODER_CFLAGS = \
	-D_EE \
	-O2 \
	-G8192 \
	-mgpopt \
	-mno-abicalls \
	-Wall \
	-mno-check-zero-division

EE_OBJS = \
	$(BUILD_DIR)/reference_full_stream_eof_characterization.o \
	$(BUILD_DIR)/sms_libmpeg.o \
	$(BUILD_DIR)/sms_libmpeg_core.o \
	$(BUILD_DIR)/test_bin.o

EE_LIBS = \
	-ldraw \
	-lgraph \
	-ldma \
	-lc \
	-lpacket

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/reference_full_stream_eof_characterization.o: \
	experiments/media-stream-exp3/reference_full_stream_eof_characterization.c | $(BUILD_DIR)
	$(EE_CC) \
		$(EE_CFLAGS) \
		-I$(SMS_INC) \
		$(EE_INCS) \
		-c $< \
		-o $@

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

$(BUILD_DIR)/test_bin.o: $(STREAM) | $(BUILD_DIR)
	cd $(BUILD_DIR) && \
		$(EE_LD) -r -b binary -o test_bin.o test.bin

$(EE_BIN): $(EE_OBJS)

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
