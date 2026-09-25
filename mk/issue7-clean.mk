# PS-to-VNC clean reconstruction — Issue #7 linked build.
#
# This is deliberately separate from scripts/build.sh, which remains the
# historical B4A/reference build authority until the clean executable has been
# qualified on hardware.

BUILD_DIR ?= build/reconstruction/issue7
GEN_DIR = $(BUILD_DIR)/generated
DEP_DIR = $(BUILD_DIR)/deps

PS2IP_LIB = $(DEP_DIR)/libps2ip_mtu1458_wscale128.a
EE_BIN ?= $(BUILD_DIR)/PS-to-VNC-Issue7.ELF

SMS_VENDOR = vendor/sms-libmpeg
SMS_INC = $(SMS_VENDOR)/include
SMS_SRC = $(SMS_VENDOR)/src
SMS_DECODER_CFLAGS = \
	-D_EE \
	-O2 \
	-G8192 \
	-mgpopt \
	-mno-abicalls \
	-Wall \
	-mno-check-zero-division

EXTRA_EE_OBJS ?=

EE_OBJS = \
	$(BUILD_DIR)/main.o \
	$(BUILD_DIR)/app.o \
	$(BUILD_DIR)/app_mpeg_activation.o \
	$(BUILD_DIR)/app_mpeg_calibration.o \
	$(BUILD_DIR)/app_mpeg_frame.o \
	$(BUILD_DIR)/app_mpeg_run.o \
	$(BUILD_DIR)/audio_playback.o \
	$(BUILD_DIR)/audio_audsrv_service.o \
	$(BUILD_DIR)/audio_session.o \
	$(BUILD_DIR)/config_profile.o \
	$(BUILD_DIR)/config_rfb_runtime_profile.o \
	$(BUILD_DIR)/config_mpeg_runtime_profile.o \
	$(BUILD_DIR)/config_media_clock_profile.o \
	$(BUILD_DIR)/config_text.o \
	$(BUILD_DIR)/media_clock.o \
	$(BUILD_DIR)/mpeg_decoder.o \
	$(BUILD_DIR)/mpeg_worker.o \
	$(BUILD_DIR)/mpeg_ps2_worker_runtime.o \
	$(BUILD_DIR)/mpeg_ps2_decoder_backend.o \
	$(BUILD_DIR)/sms_libmpeg.o \
	$(BUILD_DIR)/sms_libmpeg_core.o \
	$(BUILD_DIR)/diagnostics.o \
	$(BUILD_DIR)/diagnostics_identity.o \
	$(BUILD_DIR)/rfb.o \
	$(BUILD_DIR)/framebuffer.o \
	$(BUILD_DIR)/rfb_bridge.o \
	$(BUILD_DIR)/rfb_session.o \
	$(BUILD_DIR)/rfb_flow_policy.o \
	$(BUILD_DIR)/display.o \
	$(BUILD_DIR)/mpeg_frame.o \
	$(BUILD_DIR)/mpeg_presentation.o \
	$(BUILD_DIR)/mpeg_scheduler.o \
	$(BUILD_DIR)/mpeg_compositor.o \
	$(BUILD_DIR)/input.o \
	$(BUILD_DIR)/product_action.o \
	$(BUILD_DIR)/mouse.o \
	$(BUILD_DIR)/input_runtime.o \
	$(BUILD_DIR)/keyboard.o \
	$(BUILD_DIR)/pad.o \
	$(BUILD_DIR)/local_controller.o \
	$(BUILD_DIR)/local_ui.o \
	$(BUILD_DIR)/osk.o \
	$(BUILD_DIR)/osk_render.o \
	$(BUILD_DIR)/local_ui_presentation.o \
	$(BUILD_DIR)/mpeg_calibration.o \
	$(BUILD_DIR)/mpeg_calibration_manual.o \
	$(BUILD_DIR)/transport_protocol.o \
	$(BUILD_DIR)/transport_physical_stream.o \
	$(BUILD_DIR)/transport_rfb_channel.o \
	$(BUILD_DIR)/transport_audio_channel.o \
	$(BUILD_DIR)/transport_mpeg_channel.o \
	$(BUILD_DIR)/transport_runtime.o \
	$(BUILD_DIR)/transport_quiesce.o \
	$(BUILD_DIR)/transport_bridge.o \
	$(BUILD_DIR)/ps2_system.o \
	$(BUILD_DIR)/ps2_network.o \
	$(BUILD_DIR)/ps2_graphics.o \
	$(BUILD_DIR)/ps2_media_clock.o \
	$(BUILD_DIR)/SIO2MAN_irx.o \
	$(BUILD_DIR)/PADMAN_irx.o \
	$(BUILD_DIR)/DEV9_irx.o \
	$(BUILD_DIR)/NETMAN_irx.o \
	$(BUILD_DIR)/SMAP_irx.o

EE_OBJS += $(EXTRA_EE_OBJS)

EE_INCS = -I$(SMS_INC) -Isrc -Isrc/audio -Isrc/config -Isrc/media -Isrc/mpeg -Isrc/input -Isrc/ui -Isrc/rfb -Isrc/framebuffer -Isrc/display -Isrc/diagnostics -Isrc/platform -Isrc/transport -I$(GSKIT)/include
EE_LIBS = -L$(GSKIT)/lib -lgskit -ldmakit -ldma -lnetman -lpad -laudsrv $(PS2IP_LIB) -lpatches -Wl,--wrap=sendto

.PHONY: all clean

all: $(EE_BIN)

$(BUILD_DIR) $(GEN_DIR) $(DEP_DIR):
	mkdir -p $@

$(PS2IP_LIB):
	@echo "Missing qualified PS2IP dependency: $(PS2IP_LIB)" >&2
	@echo "Run scripts/build-issue7-clean.sh from the repository root." >&2
	@false

$(EE_BIN): $(PS2IP_LIB) $(EE_OBJS) | $(BUILD_DIR)

$(BUILD_DIR)/main.o: src/main.c src/app.h src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app.o: src/app.c src/app.h src/config/rfb_runtime_profile.h src/config/mpeg_runtime_profile.h src/config/media_clock_profile.h src/diagnostics/diagnostics.h src/display/display.h src/framebuffer/framebuffer.h src/input/input.h src/input/input_runtime.h src/ui/local_ui_presentation.h src/ui/local_ui.h src/ui/osk.h src/ui/osk_render.h src/input/mouse.h src/input/pad.h src/rfb/rfb_session.h src/rfb/flow_policy.h src/transport/bridge.h src/transport/transport.h src/media/clock.h src/platform/ps2_graphics.h src/platform/ps2_media_clock.h src/platform/ps2_network.h src/platform/ps2_system.h src/input/controller.h src/ui/local_controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app_mpeg_activation.o: src/app_mpeg_activation.c src/app_mpeg_activation.h src/app_mpeg_calibration.h src/app_mpeg_run.h src/rfb/flow_policy.h src/display/mpeg_presentation.h src/media/clock.h src/transport/bridge.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app_mpeg_calibration.o: src/app_mpeg_calibration.c src/app_mpeg_calibration.h src/ui/mpeg_calibration_manual.h src/ui/mpeg_calibration.h src/rfb/flow_policy.h src/rfb/rfb_session.h src/input/input_runtime.h src/ui/local_ui.h src/display/mpeg_presentation.h src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app_mpeg_frame.o: src/app_mpeg_frame.c src/app_mpeg_frame.h src/mpeg/worker.h src/display/mpeg_frame.h src/display/mpeg_presentation.h src/display/mpeg_scheduler.h src/display/mpeg_compositor.h src/media/clock.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/app_mpeg_run.o: src/app_mpeg_run.c src/app_mpeg_run.h src/app_mpeg_frame.h src/config/mpeg_runtime_profile.h src/display/mpeg_presentation.h src/media/clock.h src/mpeg/ps2_decoder_backend.h src/mpeg/ps2_worker_runtime.h src/mpeg/worker.h src/rfb/flow_policy.h src/transport/bridge.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/audio_playback.o: src/audio/playback.c src/audio/playback.h src/config/profile.h src/transport/bridge.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/audio_audsrv_service.o: src/audio/audsrv_service.c src/audio/audsrv_service.h src/audio/playback.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/audio_session.o: src/audio/session.c src/audio/session.h src/audio/playback.h src/media/clock.h src/transport/bridge.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/config_profile.o: src/config/profile.c src/config/profile.h src/transport/transport.h src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/config_rfb_runtime_profile.o: src/config/rfb_runtime_profile.c src/config/rfb_runtime_profile.h src/config/rfb_runtime_profile_generated.h src/config/profile.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/config_mpeg_runtime_profile.o: src/config/mpeg_runtime_profile.c src/config/mpeg_runtime_profile.h src/transport/transport.h src/mpeg/decoder.h src/mpeg/worker.h src/mpeg/ps2_worker_runtime.h src/display/mpeg_scheduler.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/config_media_clock_profile.o: src/config/media_clock_profile.c src/config/media_clock_profile.h src/config/profile.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/config_text.o: src/config/text.c src/config/text.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/media_clock.o: src/media/clock.c src/media/clock.h src/config/profile.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_decoder.o: src/mpeg/decoder.c src/mpeg/decoder.h src/transport/bridge.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_worker.o: src/mpeg/worker.c src/mpeg/worker.h src/mpeg/decoder.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_ps2_worker_runtime.o: src/mpeg/ps2_worker_runtime.c src/mpeg/ps2_worker_runtime.h src/mpeg/worker.h src/mpeg/decoder.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_ps2_decoder_backend.o: src/mpeg/ps2_decoder_backend.c src/mpeg/ps2_decoder_backend.h src/mpeg/decoder.h $(SMS_INC)/libmpeg.h $(SMS_INC)/libmpeg_internal.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/sms_libmpeg.o: $(SMS_SRC)/libmpeg.c $(SMS_INC)/libmpeg.h $(SMS_INC)/libmpeg_internal.h | $(BUILD_DIR)
	$(EE_CC) $(SMS_DECODER_CFLAGS) -I$(SMS_INC) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/sms_libmpeg_core.o: $(SMS_SRC)/libmpeg_core.S $(SMS_INC)/libmpeg_internal.h | $(BUILD_DIR)
	$(EE_CC) $(SMS_DECODER_CFLAGS) -I$(SMS_INC) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics.o: src/diagnostics/diagnostics.c src/diagnostics/diagnostics.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/diagnostics_identity.o: src/diagnostics/identity.c src/diagnostics/identity.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb.o: src/rfb/rfb.c src/rfb/rfb.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/framebuffer.o: src/framebuffer/framebuffer.c src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_bridge.o: src/rfb/bridge.c src/rfb/bridge.h src/transport/bridge.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_session.o: src/rfb/rfb_session.c src/rfb/rfb_session.h src/rfb/rfb.h src/rfb/bridge.h src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/rfb_flow_policy.o: src/rfb/flow_policy.c src/rfb/flow_policy.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/display.o: src/display/display.c src/display/display.h src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_frame.o: src/display/mpeg_frame.c src/display/mpeg_frame.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_presentation.o: src/display/mpeg_presentation.c src/display/mpeg_presentation.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_scheduler.o: src/display/mpeg_scheduler.c src/display/mpeg_scheduler.h src/media/clock.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_compositor.o: src/display/mpeg_compositor.c src/display/mpeg_compositor.h src/display/mpeg_frame.h src/display/mpeg_presentation.h src/media/clock.h src/platform/ps2_graphics.h src/display/display.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/input.o: src/input/input.c src/input/input.h src/input/mouse.h src/input/product_action.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/product_action.o: src/input/product_action.c src/input/product_action.h src/input/controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mouse.o: src/input/mouse.c src/input/mouse.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/input_runtime.o: src/input/input_runtime.c src/input/input_runtime.h src/input/input.h src/input/product_action.h src/input/controller.h src/input/mouse.h src/input/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/keyboard.o: src/input/keyboard.c src/input/keyboard.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/pad.o: src/input/pad.c src/input/pad.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_controller.o: src/ui/local_controller.c src/ui/local_controller.h src/input/controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_ui.o: src/ui/local_ui.c src/ui/local_ui.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/osk.o: src/ui/osk.c src/ui/osk.h src/input/keyboard.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/osk_render.o: src/ui/osk_render.c src/ui/osk_render.h src/ui/osk.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/local_ui_presentation.o: src/ui/local_ui_presentation.c src/ui/local_ui_presentation.h src/ui/local_ui.h src/ui/osk.h src/ui/osk_render.h src/display/display.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_calibration.o: src/ui/mpeg_calibration.c src/ui/mpeg_calibration.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/mpeg_calibration_manual.o: src/ui/mpeg_calibration_manual.c src/ui/mpeg_calibration_manual.h src/ui/mpeg_calibration.h src/input/controller.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_protocol.o: src/transport/protocol.c src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_physical_stream.o: src/transport/physical_stream.c src/transport/physical_stream.h src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_rfb_channel.o: src/transport/rfb_channel.c src/transport/rfb_channel.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_audio_channel.o: src/transport/audio_channel.c src/transport/audio_channel.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_mpeg_channel.o: src/transport/mpeg_channel.c src/transport/mpeg_channel.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_runtime.o: src/transport/runtime.c src/transport/runtime.h src/transport/physical_stream.h src/transport/rfb_channel.h src/transport/audio_channel.h src/transport/mpeg_channel.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_quiesce.o: src/transport/quiesce.c src/transport/runtime.h src/transport/protocol.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/transport_bridge.o: src/transport/bridge.c src/transport/bridge.h src/transport/runtime.h src/transport/transport.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_system.o: src/platform/ps2_system.c src/platform/ps2_system.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_network.o: src/platform/ps2_network.c src/platform/ps2_network.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_graphics.o: src/platform/ps2_graphics.c src/platform/ps2_graphics.h src/display/display.h src/display/mpeg_frame.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/ps2_media_clock.o: src/platform/ps2_media_clock.c src/platform/ps2_media_clock.h src/media/clock.h | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(GEN_DIR)/SIO2MAN_irx.c: $(PS2SDK)/iop/irx/freesio2.irx | $(GEN_DIR)
	bin2c $< $@ SIO2MAN_irx

$(GEN_DIR)/PADMAN_irx.c: $(PS2SDK)/iop/irx/freepad.irx | $(GEN_DIR)
	bin2c $< $@ PADMAN_irx

$(GEN_DIR)/DEV9_irx.c: $(PS2SDK)/iop/irx/ps2dev9.irx | $(GEN_DIR)
	bin2c $< $@ DEV9_irx

$(GEN_DIR)/NETMAN_irx.c: $(PS2SDK)/iop/irx/netman.irx | $(GEN_DIR)
	bin2c $< $@ NETMAN_irx

$(GEN_DIR)/SMAP_irx.c: $(PS2SDK)/iop/irx/smap.irx | $(GEN_DIR)
	bin2c $< $@ SMAP_irx

$(BUILD_DIR)/SIO2MAN_irx.o: $(GEN_DIR)/SIO2MAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/PADMAN_irx.o: $(GEN_DIR)/PADMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/DEV9_irx.o: $(GEN_DIR)/DEV9_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/NETMAN_irx.o: $(GEN_DIR)/NETMAN_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(BUILD_DIR)/SMAP_irx.o: $(GEN_DIR)/SMAP_irx.c | $(BUILD_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal