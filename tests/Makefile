CC ?= cc
CFLAGS ?=

COMMON_CFLAGS = -O2 -std=c99 -Wall -Wextra -Werror -pedantic -I../src -I../src/input -I../src/ui -I../src/rfb -I../src/framebuffer -I../src/display -I../src/diagnostics -I../src/platform -I../src/transport
BUILD_DIR = .build

.PHONY: unit clean

unit: \
	$(BUILD_DIR)/config_text_test \
	$(BUILD_DIR)/video_mode_test \
	$(BUILD_DIR)/rfb_wire_test \
	$(BUILD_DIR)/framebuffer_test \
	$(BUILD_DIR)/rfb_session_test \
	$(BUILD_DIR)/rfb_initial_frame_test \
	$(BUILD_DIR)/rfb_initial_coverage_test \
	$(BUILD_DIR)/rfb_async_framing_test \
	$(BUILD_DIR)/display_test \
	$(BUILD_DIR)/mouse_test \
	$(BUILD_DIR)/keyboard_test \
	$(BUILD_DIR)/local_ui_test \
	$(BUILD_DIR)/osk_test \
	$(BUILD_DIR)/osk_render_test \
	$(BUILD_DIR)/local_ui_presentation_test \
	$(BUILD_DIR)/input_test \
	$(BUILD_DIR)/controller_event_test \
	$(BUILD_DIR)/local_controller_test \
	$(BUILD_DIR)/osk_direct_test \
	$(BUILD_DIR)/app_test
	./$(BUILD_DIR)/config_text_test
	./$(BUILD_DIR)/video_mode_test
	./$(BUILD_DIR)/rfb_wire_test
	./$(BUILD_DIR)/framebuffer_test
	./$(BUILD_DIR)/rfb_session_test
	./$(BUILD_DIR)/rfb_initial_frame_test
	./$(BUILD_DIR)/rfb_initial_coverage_test
	./$(BUILD_DIR)/rfb_async_framing_test
	./$(BUILD_DIR)/display_test
	./$(BUILD_DIR)/mouse_test
	./$(BUILD_DIR)/keyboard_test
	./$(BUILD_DIR)/local_ui_test
	./$(BUILD_DIR)/osk_test
	./$(BUILD_DIR)/osk_render_test
	./$(BUILD_DIR)/local_ui_presentation_test
	./$(BUILD_DIR)/input_test
	./$(BUILD_DIR)/controller_event_test
	./$(BUILD_DIR)/local_controller_test
	./$(BUILD_DIR)/osk_direct_test
	./$(BUILD_DIR)/app_test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/config_text_test: \
	unit/config_text_test.c \
	../src/config/text.c \
	../src/config/text.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/config_text_test.c \
		../src/config/text.c \
		-o $@

$(BUILD_DIR)/video_mode_test: \
	unit/video_mode_test.c \
	../src/video/mode.c \
	../src/video/mode.h \
	support/gsKit.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Isupport \
		-I../src \
		unit/video_mode_test.c \
		../src/video/mode.c \
		-o $@

$(BUILD_DIR)/rfb_wire_test: \
	unit/rfb_wire_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_wire_test.c \
		../src/rfb/rfb.c \
		-o $@

$(BUILD_DIR)/framebuffer_test: \
	unit/framebuffer_test.c \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/framebuffer_test.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_session_test: \
	unit/rfb_session_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/bridge.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_session_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_initial_frame_test: \
	unit/rfb_initial_frame_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/bridge.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_initial_frame_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_initial_coverage_test: \
	unit/rfb_initial_coverage_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/bridge.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_initial_coverage_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/rfb_async_framing_test: \
	unit/rfb_async_framing_test.c \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/bridge.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_async_framing_test.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/display_test: \
	unit/display_test.c \
	../src/display/display.c \
	../src/display/display.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/display_test.c \
		../src/display/display.c \
		../src/framebuffer/framebuffer.c \
		-o $@

$(BUILD_DIR)/mouse_test: \
	unit/mouse_test.c \
	../src/input/mouse.c \
	../src/input/mouse.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mouse_test.c \
		../src/input/mouse.c \
		-o $@

$(BUILD_DIR)/keyboard_test: \
	unit/keyboard_test.c \
	../src/input/keyboard.c \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/keyboard_test.c \
		../src/input/keyboard.c \
		-o $@

$(BUILD_DIR)/local_ui_test: \
	unit/local_ui_test.c \
	../src/ui/local_ui.c \
	../src/ui/local_ui.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_ui_test.c \
		../src/ui/local_ui.c \
		-o $@

$(BUILD_DIR)/osk_test: \
	unit/osk_test.c \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_test.c \
		../src/ui/osk.c \
		-o $@

$(BUILD_DIR)/osk_render_test: \
	unit/osk_render_test.c \
	../src/ui/osk_render.c \
	../src/ui/osk_render.h \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_render_test.c \
		../src/ui/osk_render.c \
		../src/ui/osk.c \
		-o $@

$(BUILD_DIR)/local_ui_presentation_test: \
	unit/local_ui_presentation_test.c \
	../src/ui/local_ui_presentation.c \
	../src/ui/local_ui_presentation.h \
	../src/ui/local_ui.c \
	../src/ui/local_ui.h \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/ui/osk_render.c \
	../src/ui/osk_render.h \
	../src/display/display.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_ui_presentation_test.c \
		../src/ui/local_ui_presentation.c \
		../src/ui/local_ui.c \
		../src/ui/osk.c \
		../src/ui/osk_render.c \
		-o $@

$(BUILD_DIR)/input_test: \
	unit/input_test.c \
	../src/input/input.c \
	../src/input/input.h \
	../src/input/mouse.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/input_test.c \
		../src/input/input.c \
		-o $@

$(BUILD_DIR)/controller_event_test: \
	unit/controller_event_test.c \
	../src/input/input.c \
	../src/input/input.h \
	../src/input/controller.h \
	../src/input/mouse.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/controller_event_test.c \
		../src/input/input.c \
		-o $@

$(BUILD_DIR)/local_controller_test: \
	unit/local_controller_test.c \
	../src/ui/local_controller.c \
	../src/ui/local_controller.h \
	../src/input/controller.h \
	../src/ui/local_ui.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/local_controller_test.c \
		../src/ui/local_controller.c \
		-o $@

$(BUILD_DIR)/osk_direct_test: \
	unit/osk_direct_test.c \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/input/keyboard.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/osk_direct_test.c \
		../src/ui/osk.c \
		-o $@

$(BUILD_DIR)/app_test: \
	unit/app_test.c \
	support/libpad.h \
	../src/app.c \
	../src/app.h \
	../src/diagnostics/diagnostics.h \
	../src/display/display.h \
	../src/framebuffer/framebuffer.h \
	../src/input/input.h \
	../src/input/input_runtime.h \
	../src/input/keyboard.c \
	../src/input/keyboard.h \
	../src/input/controller.h \
	../src/ui/local_controller.c \
	../src/ui/local_controller.h \
	../src/ui/local_ui.c \
	../src/ui/local_ui.h \
	../src/ui/local_ui_presentation.c \
	../src/ui/local_ui_presentation.h \
	../src/input/mouse.h \
	../src/ui/osk.c \
	../src/ui/osk.h \
	../src/ui/osk_render.c \
	../src/ui/osk_render.h \
	../src/input/pad.h \
	../src/platform/ps2_graphics.h \
	../src/platform/ps2_network.h \
	../src/platform/ps2_system.h \
	../src/rfb/rfb.h \
	../src/rfb/rfb_session.h \
	../src/rfb/flow_policy.h \
	../src/transport/transport.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Isupport \
		-I../src \
		unit/app_test.c \
		../src/app.c \
		../src/input/keyboard.c \
		../src/ui/local_controller.c \
		../src/ui/local_ui.c \
		../src/ui/local_ui_presentation.c \
		../src/ui/osk.c \
		../src/ui/osk_render.c \
		-o $@

clean:
	rm -rf $(BUILD_DIR)


GEOMETRY_TEST := .build/video_geometry_test

.PHONY: geometry-unit
unit: geometry-unit

geometry-unit: $(GEOMETRY_TEST)
	./$(GEOMETRY_TEST)

$(GEOMETRY_TEST): unit/video_geometry_test.c ../src/video/geometry.c ../src/video/geometry.h ../src/video/mode.c ../src/video/mode.h support/gsKit.h
	mkdir -p .build
	$(CC) -O2 -std=c99 -Wall -Wextra -Werror -pedantic \
		-Isupport \
		-I../src \
		unit/video_geometry_test.c \
		../src/video/geometry.c \
		../src/video/mode.c \
		-o $(GEOMETRY_TEST)

TRANSPORT_PROTOCOL_TEST := .build/transport_protocol_test
TRANSPORT_RFB_CHANNEL_TEST := .build/transport_rfb_channel_test
TRANSPORT_BRIDGE_TEST := .build/transport_bridge_test
RFB_BRIDGE_TEST := .build/rfb_bridge_test

.PHONY: transport-protocol-unit transport-rfb-channel-unit transport-bridge-unit rfb-bridge-unit
unit: transport-protocol-unit transport-rfb-channel-unit transport-bridge-unit rfb-bridge-unit

transport-protocol-unit: $(TRANSPORT_PROTOCOL_TEST)
	./$(TRANSPORT_PROTOCOL_TEST)

$(TRANSPORT_PROTOCOL_TEST): unit/transport_protocol_test.c ../src/transport/protocol.c ../src/transport/protocol.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/transport_protocol_test.c \
		../src/transport/protocol.c \
		-o $(TRANSPORT_PROTOCOL_TEST)

transport-rfb-channel-unit: $(TRANSPORT_RFB_CHANNEL_TEST)
	./$(TRANSPORT_RFB_CHANNEL_TEST)

$(TRANSPORT_RFB_CHANNEL_TEST): unit/transport_rfb_channel_test.c ../src/transport/rfb_channel.c ../src/transport/rfb_channel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/transport_rfb_channel_test.c \
		../src/transport/rfb_channel.c \
		-o $(TRANSPORT_RFB_CHANNEL_TEST)

transport-bridge-unit: $(TRANSPORT_BRIDGE_TEST)
	./$(TRANSPORT_BRIDGE_TEST)

$(TRANSPORT_BRIDGE_TEST): unit/transport_bridge_test.c ../src/transport/bridge.c ../src/transport/bridge.h ../src/transport/runtime.h ../src/transport/transport.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/transport_bridge_test.c \
		../src/transport/bridge.c \
		-o $(TRANSPORT_BRIDGE_TEST)

rfb-bridge-unit: $(RFB_BRIDGE_TEST)
	./$(RFB_BRIDGE_TEST)

$(RFB_BRIDGE_TEST): unit/rfb_bridge_test.c ../src/rfb/bridge.c ../src/rfb/bridge.h ../src/transport/bridge.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_bridge_test.c \
		../src/rfb/bridge.c \
		-o $(RFB_BRIDGE_TEST)

TRANSPORT_PHYSICAL_STREAM_TEST := .build/transport_physical_stream_test
TRANSPORT_RUNTIME_TEST := .build/transport_runtime_test
TRANSPORT_AUDIO_TEST := .build/transport_audio_test
TRANSPORT_MPEG_TEST := .build/transport_mpeg_test

.PHONY: transport-physical-stream-unit transport-runtime-unit transport-audio-unit transport-mpeg-unit
unit: transport-physical-stream-unit transport-runtime-unit transport-audio-unit transport-mpeg-unit

transport-physical-stream-unit: $(TRANSPORT_PHYSICAL_STREAM_TEST)
	./$(TRANSPORT_PHYSICAL_STREAM_TEST)

$(TRANSPORT_PHYSICAL_STREAM_TEST): \
	unit/transport_physical_stream_test.c \
	unit/transport_host_stubs/kernel.h \
	unit/transport_host_stubs/ps2ip.h \
	../src/transport/physical_stream.c \
	../src/transport/physical_stream.h \
	../src/transport/protocol.c \
	../src/transport/protocol.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Iunit/transport_host_stubs \
		-I../src \
		unit/transport_physical_stream_test.c \
		../src/transport/physical_stream.c \
		../src/transport/protocol.c \
		-o $(TRANSPORT_PHYSICAL_STREAM_TEST)

transport-runtime-unit: $(TRANSPORT_RUNTIME_TEST)
	./$(TRANSPORT_RUNTIME_TEST)

$(TRANSPORT_RUNTIME_TEST): \
	unit/transport_runtime_test.c \
	unit/transport_host_stubs/kernel.h \
	../src/transport/runtime.c \
	../src/transport/runtime.h \
	../src/transport/quiesce.c \
	../src/transport/audio_channel.c \
	../src/transport/audio_channel.h \
	../src/transport/mpeg_channel.c \
	../src/transport/mpeg_channel.h \
	../src/transport/rfb_channel.c \
	../src/transport/rfb_channel.h \
	../src/transport/protocol.c \
	../src/transport/protocol.h \
	../src/transport/physical_stream.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Iunit/transport_host_stubs \
		-I../src \
		-pthread \
		unit/transport_runtime_test.c \
		../src/transport/runtime.c \
		../src/transport/quiesce.c \
		../src/transport/audio_channel.c \
		../src/transport/mpeg_channel.c \
		../src/transport/rfb_channel.c \
		../src/transport/protocol.c \
		-o $(TRANSPORT_RUNTIME_TEST)

transport-audio-unit: $(TRANSPORT_AUDIO_TEST)
	./$(TRANSPORT_AUDIO_TEST)

$(TRANSPORT_AUDIO_TEST): \
	unit/transport_audio_test.c \
	unit/transport_host_stubs/kernel.h \
	../src/transport/runtime.c \
	../src/transport/runtime.h \
	../src/transport/quiesce.c \
	../src/transport/audio_channel.c \
	../src/transport/audio_channel.h \
	../src/transport/mpeg_channel.c \
	../src/transport/mpeg_channel.h \
	../src/transport/rfb_channel.c \
	../src/transport/rfb_channel.h \
	../src/transport/protocol.c \
	../src/transport/protocol.h \
	../src/transport/physical_stream.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Iunit/transport_host_stubs \
		-I../src \
		-pthread \
		unit/transport_audio_test.c \
		../src/transport/runtime.c \
		../src/transport/quiesce.c \
		../src/transport/audio_channel.c \
		../src/transport/mpeg_channel.c \
		../src/transport/rfb_channel.c \
		../src/transport/protocol.c \
		-o $(TRANSPORT_AUDIO_TEST)

transport-mpeg-unit: $(TRANSPORT_MPEG_TEST)
	./$(TRANSPORT_MPEG_TEST)

$(TRANSPORT_MPEG_TEST): \
	unit/transport_mpeg_test.c \
	unit/transport_host_stubs/kernel.h \
	../src/transport/runtime.c \
	../src/transport/runtime.h \
	../src/transport/quiesce.c \
	../src/transport/audio_channel.c \
	../src/transport/audio_channel.h \
	../src/transport/mpeg_channel.c \
	../src/transport/mpeg_channel.h \
	../src/transport/rfb_channel.c \
	../src/transport/rfb_channel.h \
	../src/transport/protocol.c \
	../src/transport/protocol.h \
	../src/transport/physical_stream.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Iunit/transport_host_stubs \
		-I../src \
		-pthread \
		unit/transport_mpeg_test.c \
		../src/transport/runtime.c \
		../src/transport/quiesce.c \
		../src/transport/audio_channel.c \
		../src/transport/mpeg_channel.c \
		../src/transport/rfb_channel.c \
		../src/transport/protocol.c \
		-o $(TRANSPORT_MPEG_TEST)

CONFIG_PROFILE_TEST := .build/config_profile_test
CONFIG_MPEG_RUNTIME_PROFILE_TEST := .build/config_mpeg_runtime_profile_test

.PHONY: config-profile-unit config-mpeg-runtime-profile-unit
unit: config-profile-unit config-mpeg-runtime-profile-unit

config-profile-unit: $(CONFIG_PROFILE_TEST)
	./$(CONFIG_PROFILE_TEST)

$(CONFIG_PROFILE_TEST): \
	unit/config_profile_test.c \
	../src/config/profile.c \
	../src/config/profile.h \
	../src/transport/transport.h \
	../src/transport/protocol.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/config_profile_test.c \
		../src/config/profile.c \
		-o $(CONFIG_PROFILE_TEST)

config-mpeg-runtime-profile-unit: $(CONFIG_MPEG_RUNTIME_PROFILE_TEST)
	./$(CONFIG_MPEG_RUNTIME_PROFILE_TEST)

$(CONFIG_MPEG_RUNTIME_PROFILE_TEST): \
	unit/config_mpeg_runtime_profile_test.c \
	../src/config/mpeg_runtime_profile.c \
	../src/config/mpeg_runtime_profile.h \
	../src/config/profile.h \
	../src/transport/transport.h \
	../src/mpeg/decoder.h \
	../src/mpeg/worker.h \
	../src/mpeg/ps2_worker_runtime.h \
	../src/display/mpeg_scheduler.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/config_mpeg_runtime_profile_test.c \
		../src/config/mpeg_runtime_profile.c \
		-o $(CONFIG_MPEG_RUNTIME_PROFILE_TEST)

MEDIA_CLOCK_TEST := .build/media_clock_test

.PHONY: media-clock-unit
unit: media-clock-unit

media-clock-unit: $(MEDIA_CLOCK_TEST)
	./$(MEDIA_CLOCK_TEST)

$(MEDIA_CLOCK_TEST): \
	unit/media_clock_test.c \
	../src/media/clock.c \
	../src/media/clock.h \
	../src/config/profile.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/media_clock_test.c \
		../src/media/clock.c \
		-o $(MEDIA_CLOCK_TEST)

AUDIO_PLAYBACK_TEST := .build/audio_playback_test
AUDIO_AUDSRV_SERVICE_TEST := .build/audio_audsrv_service_test
AUDIO_SESSION_TEST := .build/audio_session_test

.PHONY: audio-playback-unit audio-audsrv-service-unit audio-session-unit
unit: audio-playback-unit audio-audsrv-service-unit audio-session-unit

audio-playback-unit: $(AUDIO_PLAYBACK_TEST)
	./$(AUDIO_PLAYBACK_TEST)

$(AUDIO_PLAYBACK_TEST): \
	unit/audio_playback_test.c \
	../src/audio/playback.c \
	../src/audio/playback.h \
	../src/config/profile.h \
	../src/transport/bridge.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/audio_playback_test.c \
		../src/audio/playback.c \
		-o $(AUDIO_PLAYBACK_TEST)

audio-audsrv-service-unit: $(AUDIO_AUDSRV_SERVICE_TEST)
	./$(AUDIO_AUDSRV_SERVICE_TEST)

$(AUDIO_AUDSRV_SERVICE_TEST): \
	unit/audio_audsrv_service_test.c \
	unit/audio_host_stubs/audsrv.h \
	../src/audio/audsrv_service.c \
	../src/audio/audsrv_service.h \
	../src/audio/playback.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-Iunit/audio_host_stubs \
		-I../src \
		unit/audio_audsrv_service_test.c \
		../src/audio/audsrv_service.c \
		-o $(AUDIO_AUDSRV_SERVICE_TEST)

audio-session-unit: $(AUDIO_SESSION_TEST)
	./$(AUDIO_SESSION_TEST)

$(AUDIO_SESSION_TEST): \
	unit/audio_session_test.c \
	../src/audio/session.c \
	../src/audio/session.h \
	../src/audio/playback.h \
	../src/config/profile.h \
	../src/media/clock.h \
	../src/transport/bridge.h \
	../src/transport/transport.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/audio_session_test.c \
		../src/audio/session.c \
		-o $(AUDIO_SESSION_TEST)

MPEG_DECODER_TEST := .build/mpeg_decoder_test

.PHONY: mpeg-decoder-unit
unit: mpeg-decoder-unit

mpeg-decoder-unit: $(MPEG_DECODER_TEST)
	./$(MPEG_DECODER_TEST)

$(MPEG_DECODER_TEST): \
	unit/mpeg_decoder_test.c \
	../src/mpeg/decoder.c \
	../src/mpeg/decoder.h \
	../src/transport/bridge.h \
	../src/transport/transport.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mpeg_decoder_test.c \
		../src/mpeg/decoder.c \
		-o $(MPEG_DECODER_TEST)

MPEG_CALIBRATION_TEST := .build/mpeg_calibration_test

.PHONY: mpeg-calibration-unit
unit: mpeg-calibration-unit

mpeg-calibration-unit: $(MPEG_CALIBRATION_TEST)
	./$(MPEG_CALIBRATION_TEST)

$(MPEG_CALIBRATION_TEST): \
	unit/mpeg_calibration_test.c \
	../src/ui/mpeg_calibration.c \
	../src/ui/mpeg_calibration.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mpeg_calibration_test.c \
		../src/ui/mpeg_calibration.c \
		-o $(MPEG_CALIBRATION_TEST)

RFB_FLOW_POLICY_TEST := .build/rfb_flow_policy_test

.PHONY: rfb-flow-policy-unit
unit: rfb-flow-policy-unit

rfb-flow-policy-unit: $(RFB_FLOW_POLICY_TEST)
	./$(RFB_FLOW_POLICY_TEST)

$(RFB_FLOW_POLICY_TEST): \
	unit/rfb_flow_policy_test.c \
	../src/rfb/flow_policy.c \
	../src/rfb/flow_policy.h \
	../src/rfb/rfb.c \
	../src/rfb/rfb.h \
	../src/rfb/rfb_session.c \
	../src/rfb/rfb_session.h \
	../src/rfb/bridge.h \
	../src/framebuffer/framebuffer.c \
	../src/framebuffer/framebuffer.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/rfb_flow_policy_test.c \
		../src/rfb/flow_policy.c \
		../src/rfb/rfb_session.c \
		../src/rfb/rfb.c \
		../src/framebuffer/framebuffer.c \
		-o $(RFB_FLOW_POLICY_TEST)

MPEG_PRESENTATION_TEST := .build/mpeg_presentation_test

.PHONY: mpeg-presentation-unit
unit: mpeg-presentation-unit

mpeg-presentation-unit: $(MPEG_PRESENTATION_TEST)
	./$(MPEG_PRESENTATION_TEST)

$(MPEG_PRESENTATION_TEST): \
	unit/mpeg_presentation_test.c \
	../src/display/mpeg_presentation.c \
	../src/display/mpeg_presentation.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mpeg_presentation_test.c \
		../src/display/mpeg_presentation.c \
		-o $(MPEG_PRESENTATION_TEST)

MPEG_SCHEDULER_TEST := .build/mpeg_scheduler_test

.PHONY: mpeg-scheduler-unit
unit: mpeg-scheduler-unit

mpeg-scheduler-unit: $(MPEG_SCHEDULER_TEST)
	./$(MPEG_SCHEDULER_TEST)

$(MPEG_SCHEDULER_TEST): \
	unit/mpeg_scheduler_test.c \
	../src/display/mpeg_scheduler.c \
	../src/display/mpeg_scheduler.h \
	../src/media/clock.c \
	../src/media/clock.h \
	../src/config/profile.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mpeg_scheduler_test.c \
		../src/display/mpeg_scheduler.c \
		../src/media/clock.c \
		-o $(MPEG_SCHEDULER_TEST)

MPEG_COMPOSITOR_TEST := .build/mpeg_compositor_test

.PHONY: mpeg-compositor-unit
unit: mpeg-compositor-unit

mpeg-compositor-unit: $(MPEG_COMPOSITOR_TEST)
	./$(MPEG_COMPOSITOR_TEST)

$(MPEG_COMPOSITOR_TEST): \
	unit/mpeg_compositor_test.c \
	../src/display/mpeg_frame.c \
	../src/display/mpeg_frame.h \
	../src/display/mpeg_presentation.c \
	../src/display/mpeg_presentation.h \
	../src/display/mpeg_compositor.c \
	../src/display/mpeg_compositor.h \
	../src/media/clock.c \
	../src/media/clock.h \
	../src/platform/ps2_graphics.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mpeg_compositor_test.c \
		../src/display/mpeg_frame.c \
		../src/display/mpeg_presentation.c \
		../src/display/mpeg_compositor.c \
		../src/media/clock.c \
		-o $(MPEG_COMPOSITOR_TEST)

MPEG_WORKER_TEST := .build/mpeg_worker_test

.PHONY: mpeg-worker-unit
unit: mpeg-worker-unit

mpeg-worker-unit: $(MPEG_WORKER_TEST)
	./$(MPEG_WORKER_TEST)

$(MPEG_WORKER_TEST): \
	unit/mpeg_worker_test.c \
	../src/mpeg/worker.c \
	../src/mpeg/worker.h \
	../src/mpeg/decoder.c \
	../src/mpeg/decoder.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/mpeg_worker_test.c \
		../src/mpeg/worker.c \
		../src/mpeg/decoder.c \
		-o $(MPEG_WORKER_TEST)

APP_MPEG_FRAME_TEST := .build/app_mpeg_frame_test

.PHONY: app-mpeg-frame-unit
unit: app-mpeg-frame-unit

app-mpeg-frame-unit: $(APP_MPEG_FRAME_TEST)
	./$(APP_MPEG_FRAME_TEST)

$(APP_MPEG_FRAME_TEST): \
	unit/app_mpeg_frame_test.c \
	../src/app_mpeg_frame.c \
	../src/app_mpeg_frame.h \
	../src/mpeg/worker.h \
	../src/display/mpeg_frame.c \
	../src/display/mpeg_frame.h \
	../src/display/mpeg_presentation.c \
	../src/display/mpeg_presentation.h \
	../src/display/mpeg_scheduler.c \
	../src/display/mpeg_scheduler.h \
	../src/display/mpeg_compositor.h \
	../src/media/clock.c \
	../src/media/clock.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/app_mpeg_frame_test.c \
		../src/app_mpeg_frame.c \
		../src/display/mpeg_frame.c \
		../src/display/mpeg_presentation.c \
		../src/display/mpeg_scheduler.c \
		../src/media/clock.c \
		-o $(APP_MPEG_FRAME_TEST)

PI_WIRE_SERVER_TEST := unit/pi_wire_server_test.py
PI_RFB_ATTACHMENT_TEST := unit/pi_rfb_attachment_test.py
PI_NATIVE_RFB_PROVIDER_TEST := unit/pi_native_rfb_provider_test.py
PI_INTERNAL_RFB_PROVIDER_TEST := unit/pi_internal_rfb_provider_test.py
PI_MPEG_GENERATION_TEST := unit/pi_mpeg_generation_test.py

.PHONY: pi-wire-server-unit pi-rfb-attachment-unit pi-native-rfb-provider-unit pi-internal-rfb-provider-unit pi-mpeg-generation-unit
unit: pi-wire-server-unit pi-rfb-attachment-unit pi-native-rfb-provider-unit pi-internal-rfb-provider-unit pi-mpeg-generation-unit

pi-wire-server-unit:
	python3 $(PI_WIRE_SERVER_TEST)

pi-rfb-attachment-unit:
	python3 $(PI_RFB_ATTACHMENT_TEST)

pi-native-rfb-provider-unit:
	python3 $(PI_NATIVE_RFB_PROVIDER_TEST)

pi-internal-rfb-provider-unit:
	python3 $(PI_INTERNAL_RFB_PROVIDER_TEST)

pi-mpeg-generation-unit:
	python3 $(PI_MPEG_GENERATION_TEST)


CONFIG_RFB_RUNTIME_PROFILE_TEST := .build/config_rfb_runtime_profile_test
PI_RFB_RUNTIME_PROFILE_TEST := unit/pi_rfb_runtime_profile_test.py

.PHONY: config-rfb-runtime-profile-unit pi-rfb-runtime-profile-unit rfb-runtime-profile-generation-unit
unit: config-rfb-runtime-profile-unit pi-rfb-runtime-profile-unit rfb-runtime-profile-generation-unit

config-rfb-runtime-profile-unit: $(CONFIG_RFB_RUNTIME_PROFILE_TEST)
	./$(CONFIG_RFB_RUNTIME_PROFILE_TEST)

$(CONFIG_RFB_RUNTIME_PROFILE_TEST): \
	unit/config_rfb_runtime_profile_test.c \
	../src/config/rfb_runtime_profile.c \
	../src/config/rfb_runtime_profile.h \
	../src/config/rfb_runtime_profile_generated.h \
	../src/config/profile.h \
	../src/transport/transport.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(COMMON_CFLAGS) \
		-I../src \
		unit/config_rfb_runtime_profile_test.c \
		../src/config/rfb_runtime_profile.c \
		-o $(CONFIG_RFB_RUNTIME_PROFILE_TEST)

pi-rfb-runtime-profile-unit:
	python3 $(PI_RFB_RUNTIME_PROFILE_TEST)

rfb-runtime-profile-generation-unit:
	python3 ../scripts/generate-rfb-runtime-profile.py --check
