# Clean symbols — `src/transport`

DIRECTORY=src/transport
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the one physical PSTV stream, sole receive owner, framed sequence/serialized send, logical RFB storage and credit, explicit finite-session quiescence, and Transport-owned fatal-session convergence.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| buffer | parameter | src/transport/bridge.c | pstvnc_transport_rfb_read_exact | local | Supplies buffer to pstvnc_transport_rfb_read_exact. | A001 Transport |
| buffer | parameter | src/transport/bridge.c | pstvnc_transport_rfb_write_exact | local | Supplies buffer to pstvnc_transport_rfb_write_exact. | A001 Transport |
| config | parameter | src/transport/bridge.c | pstvnc_transport_session_open | local | Supplies config to pstvnc_transport_session_open. | A001 Transport |
| count | parameter | src/transport/bridge.c | pstvnc_transport_rfb_read_exact | local | Supplies count to pstvnc_transport_rfb_read_exact. | A001 Transport |
| count | parameter | src/transport/bridge.c | pstvnc_transport_rfb_write_exact | local | Supplies count to pstvnc_transport_rfb_write_exact. | A001 Transport |
| discarded_count | parameter | src/transport/bridge.c | pstvnc_transport_rfb_discard_quiesce_residual | local | Supplies discarded_count to pstvnc_transport_rfb_discard_quiesce_residual. | A001 Transport |
| expected_count | parameter | src/transport/bridge.c | pstvnc_transport_rfb_discard_quiesce_residual | local | Supplies expected_count to pstvnc_transport_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_bridge_finish_release | function | src/transport/bridge.c | Transport | file | Defines pstvnc_transport_bridge_finish_release. | A001 Transport |
| pstvnc_transport_bridge_runtime | variable | src/transport/bridge.c | Transport | file | Stores pstvnc_transport_bridge_runtime for Transport. | A001 Transport |
| pstvnc_transport_bridge_session_active | variable | src/transport/bridge.c | Transport | file | Stores pstvnc_transport_bridge_session_active for Transport. | A001 Transport |
| pstvnc_transport_bridge_terminal_result | function | src/transport/bridge.c | Transport | file | Defines pstvnc_transport_bridge_terminal_result. | A001 Transport |
| pstvnc_transport_rfb_discard_quiesce_residual | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_rfb_poll_receive | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_poll_receive. | A001 Transport |
| pstvnc_transport_rfb_quiesce_requested | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_quiesce_requested. | A001 Transport |
| pstvnc_transport_rfb_read_exact | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_read_exact. | A001 Transport |
| pstvnc_transport_rfb_send_quiesce_boundary | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_send_quiesce_boundary. | A001 Transport |
| pstvnc_transport_rfb_send_quiesce_complete | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_send_quiesce_complete. | A001 Transport |
| pstvnc_transport_rfb_snapshot_quiesce_residual | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_snapshot_quiesce_residual. | A001 Transport |
| pstvnc_transport_rfb_wait_quiesce_commit | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_wait_quiesce_commit. | A001 Transport |
| pstvnc_transport_rfb_write_exact | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_rfb_write_exact. | A001 Transport |
| pstvnc_transport_session_abort | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_session_abort. | A001 Transport |
| pstvnc_transport_session_close | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_session_close. | A001 Transport |
| pstvnc_transport_session_open | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_session_open. | A001 Transport |
| pstvnc_transport_session_wait_receiver_done | function | src/transport/bridge.c | Transport | public | Defines pstvnc_transport_session_wait_receiver_done. | A001 Transport |
| released | variable | src/transport/bridge.c | pstvnc_transport_bridge_finish_release | local | Stores released for pstvnc_transport_bridge_finish_release. | A001 Transport |
| requested | variable | src/transport/bridge.c | pstvnc_transport_rfb_quiesce_requested | local | Stores requested for pstvnc_transport_rfb_quiesce_requested. | A001 Transport |
| residual_count | parameter | src/transport/bridge.c | pstvnc_transport_rfb_snapshot_quiesce_residual | local | Supplies residual_count to pstvnc_transport_rfb_snapshot_quiesce_residual. | A001 Transport |
| result | variable | src/transport/bridge.c | pstvnc_transport_rfb_poll_receive | local | Stores result for pstvnc_transport_rfb_poll_receive. | A001 Transport |
| socket_fd | parameter | src/transport/bridge.c | pstvnc_transport_session_open | local | Supplies socket_fd to pstvnc_transport_session_open. | A001 Transport |
| PSTVNC_TRANSPORT_BRIDGE_H | include guard | src/transport/bridge.h | Transport | file | Defines PSTVNC_TRANSPORT_BRIDGE_H type. | A001 Transport |
| buffer | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_read_exact | prototype | Supplies buffer to pstvnc_transport_rfb_read_exact. | A001 Transport |
| buffer | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_write_exact | prototype | Supplies buffer to pstvnc_transport_rfb_write_exact. | A001 Transport |
| config | prototype parameter | src/transport/bridge.h | pstvnc_transport_session_open | prototype | Supplies config to pstvnc_transport_session_open. | A001 Transport |
| count | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_read_exact | prototype | Supplies count to pstvnc_transport_rfb_read_exact. | A001 Transport |
| count | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_write_exact | prototype | Supplies count to pstvnc_transport_rfb_write_exact. | A001 Transport |
| discarded_count | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_discard_quiesce_residual | prototype | Supplies discarded_count to pstvnc_transport_rfb_discard_quiesce_residual. | A001 Transport |
| expected_count | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_discard_quiesce_residual | prototype | Supplies expected_count to pstvnc_transport_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_rfb_discard_quiesce_residual | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_rfb_poll_receive | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_poll_receive. | A001 Transport |
| pstvnc_transport_rfb_quiesce_requested | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_quiesce_requested. | A001 Transport |
| pstvnc_transport_rfb_read_exact | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_read_exact. | A001 Transport |
| pstvnc_transport_rfb_send_quiesce_boundary | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_send_quiesce_boundary. | A001 Transport |
| pstvnc_transport_rfb_send_quiesce_complete | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_send_quiesce_complete. | A001 Transport |
| pstvnc_transport_rfb_snapshot_quiesce_residual | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_snapshot_quiesce_residual. | A001 Transport |
| pstvnc_transport_rfb_wait_quiesce_commit | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_wait_quiesce_commit. | A001 Transport |
| pstvnc_transport_rfb_write_exact | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_rfb_write_exact. | A001 Transport |
| pstvnc_transport_session_abort | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_session_abort. | A001 Transport |
| pstvnc_transport_session_close | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_session_close. | A001 Transport |
| pstvnc_transport_session_open | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_session_open. | A001 Transport |
| pstvnc_transport_session_wait_receiver_done | function declaration | src/transport/bridge.h | Transport | public | Declares pstvnc_transport_session_wait_receiver_done. | A001 Transport |
| residual_count | prototype parameter | src/transport/bridge.h | pstvnc_transport_rfb_snapshot_quiesce_residual | prototype | Supplies residual_count to pstvnc_transport_rfb_snapshot_quiesce_residual. | A001 Transport |
| socket_fd | prototype parameter | src/transport/bridge.h | pstvnc_transport_session_open | prototype | Supplies socket_fd to pstvnc_transport_session_open. | A001 Transport |
| byte_count | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_exact | local | Supplies byte_count to pstvnc_transport_physical_stream_receive_exact. | A001 Transport |
| byte_count | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_exact | local | Supplies byte_count to pstvnc_transport_physical_stream_send_exact. | A001 Transport |
| bytes | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_exact | local | Supplies bytes to pstvnc_transport_physical_stream_receive_exact. | A001 Transport |
| bytes | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_exact | local | Supplies bytes to pstvnc_transport_physical_stream_send_exact. | A001 Transport |
| channel | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Supplies channel to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| flags | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Supplies flags to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| header | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_frame | local | Supplies header to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| header | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Stores header for pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| kind | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Supplies kind to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| payload | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_frame | local | Supplies payload to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| payload | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Supplies payload to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| payload_capacity | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_frame | local | Supplies payload_capacity to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| payload_length | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Supplies payload_length to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| pstvnc_transport_physical_stream_adopt | function | src/transport/physical_stream.c | Transport | public | Defines pstvnc_transport_physical_stream_adopt. | A001 Transport |
| pstvnc_transport_physical_stream_receive_exact | function | src/transport/physical_stream.c | Transport | file | Defines pstvnc_transport_physical_stream_receive_exact. | A001 Transport |
| pstvnc_transport_physical_stream_receive_frame | function | src/transport/physical_stream.c | Transport | public | Defines pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| pstvnc_transport_physical_stream_release | function | src/transport/physical_stream.c | Transport | public | Defines pstvnc_transport_physical_stream_release. | A001 Transport |
| pstvnc_transport_physical_stream_send_exact | function | src/transport/physical_stream.c | Transport | file | Defines pstvnc_transport_physical_stream_send_exact. | A001 Transport |
| pstvnc_transport_physical_stream_send_frame | function | src/transport/physical_stream.c | Transport | public | Defines pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| pstvnc_transport_physical_stream_shutdown_io | function | src/transport/physical_stream.c | Transport | public | Defines pstvnc_transport_physical_stream_shutdown_io. | A001 Transport |
| received | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_exact | local | Stores received for pstvnc_transport_physical_stream_receive_exact. | A001 Transport |
| received_total | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_exact | local | Stores received_total for pstvnc_transport_physical_stream_receive_exact. | A001 Transport |
| send_mutex | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_adopt | local | Stores send_mutex for pstvnc_transport_physical_stream_adopt. | A001 Transport |
| send_semaphore_id | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_adopt | local | Stores send_semaphore_id for pstvnc_transport_physical_stream_adopt. | A001 Transport |
| send_succeeded | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Stores send_succeeded for pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| sent | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_exact | local | Stores sent for pstvnc_transport_physical_stream_send_exact. | A001 Transport |
| sent_total | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_exact | local | Stores sent_total for pstvnc_transport_physical_stream_send_exact. | A001 Transport |
| socket_fd | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_adopt | local | Supplies socket_fd to pstvnc_transport_physical_stream_adopt. | A001 Transport |
| socket_fd | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_exact | local | Supplies socket_fd to pstvnc_transport_physical_stream_receive_exact. | A001 Transport |
| socket_fd | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_exact | local | Supplies socket_fd to pstvnc_transport_physical_stream_send_exact. | A001 Transport |
| stream | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_adopt | local | Supplies stream to pstvnc_transport_physical_stream_adopt. | A001 Transport |
| stream | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_frame | local | Supplies stream to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| stream | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_release | local | Supplies stream to pstvnc_transport_physical_stream_release. | A001 Transport |
| stream | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Supplies stream to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| stream | parameter | src/transport/physical_stream.c | pstvnc_transport_physical_stream_shutdown_io | local | Supplies stream to pstvnc_transport_physical_stream_shutdown_io. | A001 Transport |
| wire_header | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_receive_frame | local | Stores wire_header for pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| wire_header | variable | src/transport/physical_stream.c | pstvnc_transport_physical_stream_send_frame | local | Stores wire_header for pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| PSTVNC_TRANSPORT_PHYSICAL_STREAM_H | include guard | src/transport/physical_stream.h | Transport | file | Defines PSTVNC_TRANSPORT_PHYSICAL_STREAM_H type. | A001 Transport |
| channel | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_send_frame | prototype | Supplies channel to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| expected_receive_sequence | field | src/transport/physical_stream.h | pstvnc_transport_physical_stream | public | Stores expected_receive_sequence in pstvnc_transport_physical_stream. | A001 Transport |
| flags | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_send_frame | prototype | Supplies flags to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| header | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_receive_frame | prototype | Supplies header to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| kind | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_send_frame | prototype | Supplies kind to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| next_send_sequence | field | src/transport/physical_stream.h | pstvnc_transport_physical_stream | public | Stores next_send_sequence in pstvnc_transport_physical_stream. | A001 Transport |
| payload | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_receive_frame | prototype | Supplies payload to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| payload | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_send_frame | prototype | Supplies payload to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| payload_capacity | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_receive_frame | prototype | Supplies payload_capacity to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| payload_length | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_send_frame | prototype | Supplies payload_length to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| pstvnc_transport_physical_stream | structure | src/transport/physical_stream.h | Transport | public | Defines pstvnc_transport_physical_stream type. | A001 Transport |
| pstvnc_transport_physical_stream_adopt | function declaration | src/transport/physical_stream.h | Transport | public | Declares pstvnc_transport_physical_stream_adopt. | A001 Transport |
| pstvnc_transport_physical_stream_receive_frame | function declaration | src/transport/physical_stream.h | Transport | public | Declares pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| pstvnc_transport_physical_stream_release | function declaration | src/transport/physical_stream.h | Transport | public | Declares pstvnc_transport_physical_stream_release. | A001 Transport |
| pstvnc_transport_physical_stream_send_frame | function declaration | src/transport/physical_stream.h | Transport | public | Declares pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| pstvnc_transport_physical_stream_shutdown_io | function declaration | src/transport/physical_stream.h | Transport | public | Declares pstvnc_transport_physical_stream_shutdown_io. | A001 Transport |
| pstvnc_transport_physical_stream_t | type | src/transport/physical_stream.h | Transport | public | Defines pstvnc_transport_physical_stream_t type. | A001 Transport |
| send_semaphore_id | field | src/transport/physical_stream.h | pstvnc_transport_physical_stream | public | Stores send_semaphore_id in pstvnc_transport_physical_stream. | A001 Transport |
| socket_fd | field | src/transport/physical_stream.h | pstvnc_transport_physical_stream | public | Stores socket_fd in pstvnc_transport_physical_stream. | A001 Transport |
| socket_fd | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_adopt | prototype | Supplies socket_fd to pstvnc_transport_physical_stream_adopt. | A001 Transport |
| stream | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_adopt | prototype | Supplies stream to pstvnc_transport_physical_stream_adopt. | A001 Transport |
| stream | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_receive_frame | prototype | Supplies stream to pstvnc_transport_physical_stream_receive_frame. | A001 Transport |
| stream | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_release | prototype | Supplies stream to pstvnc_transport_physical_stream_release. | A001 Transport |
| stream | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_send_frame | prototype | Supplies stream to pstvnc_transport_physical_stream_send_frame. | A001 Transport |
| stream | prototype parameter | src/transport/physical_stream.h | pstvnc_transport_physical_stream_shutdown_io | prototype | Supplies stream to pstvnc_transport_physical_stream_shutdown_io. | A001 Transport |
| header | parameter | src/transport/protocol.c | pstvnc_transport_header_decode | local | Supplies header to pstvnc_transport_header_decode. | A001 Transport |
| header | parameter | src/transport/protocol.c | pstvnc_transport_header_encode | local | Supplies header to pstvnc_transport_header_encode. | A001 Transport |
| input | parameter | src/transport/protocol.c | pstvnc_transport_header_decode | local | Supplies input to pstvnc_transport_header_decode. | A001 Transport |
| input | parameter | src/transport/protocol.c | pstvnc_transport_read_be32 | local | Supplies input to pstvnc_transport_read_be32. | A001 Transport |
| output | parameter | src/transport/protocol.c | pstvnc_transport_header_encode | local | Supplies output to pstvnc_transport_header_encode. | A001 Transport |
| output | parameter | src/transport/protocol.c | pstvnc_transport_write_be32 | local | Supplies output to pstvnc_transport_write_be32. | A001 Transport |
| pstvnc_transport_header_decode | function | src/transport/protocol.c | Transport | public | Defines pstvnc_transport_header_decode. | A001 Transport |
| pstvnc_transport_header_encode | function | src/transport/protocol.c | Transport | public | Defines pstvnc_transport_header_encode. | A001 Transport |
| pstvnc_transport_read_be32 | function | src/transport/protocol.c | Transport | public | Defines pstvnc_transport_read_be32. | A001 Transport |
| pstvnc_transport_write_be32 | function | src/transport/protocol.c | Transport | public | Defines pstvnc_transport_write_be32. | A001 Transport |
| value | parameter | src/transport/protocol.c | pstvnc_transport_write_be32 | local | Supplies value to pstvnc_transport_write_be32. | A001 Transport |
| PSTVNC_TRANSPORT_CHANNEL_AUDIO | enum value | src/transport/protocol.h | pstvnc_transport_channel | public | Defines PSTVNC_TRANSPORT_CHANNEL_AUDIO value. | A001 Transport |
| PSTVNC_TRANSPORT_CHANNEL_CONTROL | enum value | src/transport/protocol.h | pstvnc_transport_channel | public | Defines PSTVNC_TRANSPORT_CHANNEL_CONTROL value. | A001 Transport |
| PSTVNC_TRANSPORT_CHANNEL_MPEG2 | enum value | src/transport/protocol.h | pstvnc_transport_channel | public | Defines PSTVNC_TRANSPORT_CHANNEL_MPEG2 value. | A001 Transport |
| PSTVNC_TRANSPORT_CHANNEL_RFB | enum value | src/transport/protocol.h | pstvnc_transport_channel | public | Defines PSTVNC_TRANSPORT_CHANNEL_RFB value. | A001 Transport |
| PSTVNC_TRANSPORT_CHANNEL_TELEMETRY | enum value | src/transport/protocol.h | pstvnc_transport_channel | public | Defines PSTVNC_TRANSPORT_CHANNEL_TELEMETRY value. | A001 Transport |
| PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE | macro | src/transport/protocol.h | Transport | public | Defines PSTVNC_TRANSPORT_CREDIT_PAYLOAD_SIZE value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_CONFIG | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_CONFIG value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_CREDIT | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_CREDIT value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_DATA | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_DATA value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_ERROR | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_ERROR value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_HEARTBEAT | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_HEARTBEAT value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_HELLO | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_HELLO value. | A001 Transport |
| PSTVNC_TRANSPORT_FRAME_TELEMETRY | enum value | src/transport/protocol.h | pstvnc_transport_frame_kind | public | Defines PSTVNC_TRANSPORT_FRAME_TELEMETRY value. | A001 Transport |
| PSTVNC_TRANSPORT_HEADER_SIZE | macro | src/transport/protocol.h | Transport | public | Defines PSTVNC_TRANSPORT_HEADER_SIZE value. | A001 Transport |
| PSTVNC_TRANSPORT_MAGIC | macro | src/transport/protocol.h | Transport | public | Defines PSTVNC_TRANSPORT_MAGIC value. | A001 Transport |
| PSTVNC_TRANSPORT_MAX_PAYLOAD | macro | src/transport/protocol.h | Transport | public | Defines PSTVNC_TRANSPORT_MAX_PAYLOAD value. | A001 Transport |
| PSTVNC_TRANSPORT_PROTOCOL_H | include guard | src/transport/protocol.h | Transport | file | Defines PSTVNC_TRANSPORT_PROTOCOL_H type. | A001 Transport |
| PSTVNC_TRANSPORT_VERSION | macro | src/transport/protocol.h | Transport | public | Defines PSTVNC_TRANSPORT_VERSION value. | A001 Transport |
| channel | field | src/transport/protocol.h | pstvnc_transport_header | public | Stores channel in pstvnc_transport_header. | A001 Transport |
| flags | field | src/transport/protocol.h | pstvnc_transport_header | public | Stores flags in pstvnc_transport_header. | A001 Transport |
| header | prototype parameter | src/transport/protocol.h | pstvnc_transport_header_decode | prototype | Supplies header to pstvnc_transport_header_decode. | A001 Transport |
| header | prototype parameter | src/transport/protocol.h | pstvnc_transport_header_encode | prototype | Supplies header to pstvnc_transport_header_encode. | A001 Transport |
| input | prototype parameter | src/transport/protocol.h | pstvnc_transport_header_decode | prototype | Supplies input to pstvnc_transport_header_decode. | A001 Transport |
| input | prototype parameter | src/transport/protocol.h | pstvnc_transport_read_be32 | prototype | Supplies input to pstvnc_transport_read_be32. | A001 Transport |
| kind | field | src/transport/protocol.h | pstvnc_transport_header | public | Stores kind in pstvnc_transport_header. | A001 Transport |
| output | prototype parameter | src/transport/protocol.h | pstvnc_transport_header_encode | prototype | Supplies output to pstvnc_transport_header_encode. | A001 Transport |
| output | prototype parameter | src/transport/protocol.h | pstvnc_transport_write_be32 | prototype | Supplies output to pstvnc_transport_write_be32. | A001 Transport |
| payload_length | field | src/transport/protocol.h | pstvnc_transport_header | public | Stores payload_length in pstvnc_transport_header. | A001 Transport |
| pstvnc_transport_channel | enum | src/transport/protocol.h | Transport | public | Defines pstvnc_transport_channel type. | A001 Transport |
| pstvnc_transport_channel_t | type | src/transport/protocol.h | Transport | public | Defines pstvnc_transport_channel_t type. | A001 Transport |
| pstvnc_transport_frame_kind | enum | src/transport/protocol.h | Transport | public | Defines pstvnc_transport_frame_kind type. | A001 Transport |
| pstvnc_transport_frame_kind_t | type | src/transport/protocol.h | Transport | public | Defines pstvnc_transport_frame_kind_t type. | A001 Transport |
| pstvnc_transport_header | structure | src/transport/protocol.h | Transport | public | Defines pstvnc_transport_header type. | A001 Transport |
| pstvnc_transport_header_decode | function declaration | src/transport/protocol.h | Transport | public | Declares pstvnc_transport_header_decode. | A001 Transport |
| pstvnc_transport_header_encode | function declaration | src/transport/protocol.h | Transport | public | Declares pstvnc_transport_header_encode. | A001 Transport |
| pstvnc_transport_header_t | type | src/transport/protocol.h | Transport | public | Defines pstvnc_transport_header_t type. | A001 Transport |
| pstvnc_transport_read_be32 | function declaration | src/transport/protocol.h | Transport | public | Declares pstvnc_transport_read_be32. | A001 Transport |
| pstvnc_transport_write_be32 | function declaration | src/transport/protocol.h | Transport | public | Declares pstvnc_transport_write_be32. | A001 Transport |
| sequence | field | src/transport/protocol.h | pstvnc_transport_header | public | Stores sequence in pstvnc_transport_header. | A001 Transport |
| value | prototype parameter | src/transport/protocol.h | pstvnc_transport_write_be32 | prototype | Supplies value to pstvnc_transport_write_be32. | A001 Transport |
| version | field | src/transport/protocol.h | pstvnc_transport_header | public | Stores version in pstvnc_transport_header. | A001 Transport |
| activity_sequence | variable | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_wait_quiesce_commit | local | Stores activity_sequence for pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| committed | variable | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_wait_quiesce_commit | local | Stores committed for pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| discarded | variable | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_discard_quiesce_residual | local | Stores discarded for pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| discarded_count | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_discard_quiesce_residual | local | Supplies discarded_count to pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| expected_count | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_discard_quiesce_residual | local | Supplies expected_count to pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_runtime_rfb_discard_quiesce_residual | function | src/transport/quiesce.c | Transport | public | Defines pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_runtime_rfb_quiesce_requested | function | src/transport/quiesce.c | Transport | public | Defines pstvnc_transport_runtime_rfb_quiesce_requested. | A001 Transport |
| pstvnc_transport_runtime_rfb_send_quiesce_boundary | function | src/transport/quiesce.c | Transport | public | Defines pstvnc_transport_runtime_rfb_send_quiesce_boundary. | A001 Transport |
| pstvnc_transport_runtime_rfb_send_quiesce_complete | function | src/transport/quiesce.c | Transport | public | Defines pstvnc_transport_runtime_rfb_send_quiesce_complete. | A001 Transport |
| pstvnc_transport_runtime_rfb_snapshot_residual | function | src/transport/quiesce.c | Transport | public | Defines pstvnc_transport_runtime_rfb_snapshot_residual. | A001 Transport |
| pstvnc_transport_runtime_rfb_wait_quiesce_commit | function | src/transport/quiesce.c | Transport | public | Defines pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| pstvnc_transport_runtime_send_quiesce_marker | function | src/transport/quiesce.c | Transport | file | Defines pstvnc_transport_runtime_send_quiesce_marker. | A001 Transport |
| requested | variable | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_quiesce_requested | local | Stores requested for pstvnc_transport_runtime_rfb_quiesce_requested. | A001 Transport |
| residual_count | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_snapshot_residual | local | Supplies residual_count to pstvnc_transport_runtime_rfb_snapshot_residual. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_discard_quiesce_residual | local | Supplies runtime to pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_quiesce_requested | local | Supplies runtime to pstvnc_transport_runtime_rfb_quiesce_requested. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_send_quiesce_boundary | local | Supplies runtime to pstvnc_transport_runtime_rfb_send_quiesce_boundary. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_send_quiesce_complete | local | Supplies runtime to pstvnc_transport_runtime_rfb_send_quiesce_complete. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_snapshot_residual | local | Supplies runtime to pstvnc_transport_runtime_rfb_snapshot_residual. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_wait_quiesce_commit | local | Supplies runtime to pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| runtime | parameter | src/transport/quiesce.c | pstvnc_transport_runtime_send_quiesce_marker | local | Supplies runtime to pstvnc_transport_runtime_send_quiesce_marker. | A001 Transport |
| terminal | variable | src/transport/quiesce.c | pstvnc_transport_runtime_rfb_wait_quiesce_commit | local | Stores terminal for pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| buffer | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_available | local | Supplies buffer to pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| buffer | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_exact | local | Supplies buffer to pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| capacity | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_initialize | local | Supplies capacity to pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_activity_generation | local | Supplies channel to pstvnc_transport_rfb_channel_activity_generation. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_available | local | Supplies channel to pstvnc_transport_rfb_channel_available. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_commit | local | Supplies channel to pstvnc_transport_rfb_channel_commit. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_discard_residual | local | Supplies channel to pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_initialize | local | Supplies channel to pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_available | local | Supplies channel to pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| channel | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_exact | local | Supplies channel to pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| count | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_exact | local | Supplies count to pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| count | variable | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_available | local | Stores count for pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| discarded_count | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_discard_residual | local | Supplies discarded_count to pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| expected_count | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_discard_residual | local | Supplies expected_count to pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| first_part | variable | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_commit | local | Stores first_part for pstvnc_transport_rfb_channel_commit. | A001 Transport |
| first_part | variable | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_available | local | Stores first_part for pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| maximum_count | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_read_available | local | Supplies maximum_count to pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| payload | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_commit | local | Supplies payload to pstvnc_transport_rfb_channel_commit. | A001 Transport |
| payload_length | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_commit | local | Supplies payload_length to pstvnc_transport_rfb_channel_commit. | A001 Transport |
| pstvnc_transport_rfb_channel_activity_generation | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_activity_generation. | A001 Transport |
| pstvnc_transport_rfb_channel_available | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_available. | A001 Transport |
| pstvnc_transport_rfb_channel_commit | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_commit. | A001 Transport |
| pstvnc_transport_rfb_channel_discard_residual | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| pstvnc_transport_rfb_channel_initialize | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| pstvnc_transport_rfb_channel_read_available | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| pstvnc_transport_rfb_channel_read_exact | function | src/transport/rfb_channel.c | Transport | public | Defines pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| storage | parameter | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_initialize | local | Supplies storage to pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| write_offset | variable | src/transport/rfb_channel.c | pstvnc_transport_rfb_channel_commit | local | Stores write_offset for pstvnc_transport_rfb_channel_commit. | A001 Transport |
| PSTVNC_TRANSPORT_RFB_CHANNEL_H | include guard | src/transport/rfb_channel.h | Transport | file | Defines PSTVNC_TRANSPORT_RFB_CHANNEL_H type. | A001 Transport |
| activity_generation | field | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel | public | Stores activity_generation in pstvnc_transport_rfb_channel. | A001 Transport |
| buffer | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_read_available | prototype | Supplies buffer to pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| buffer | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_read_exact | prototype | Supplies buffer to pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| byte_count | field | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel | public | Stores byte_count in pstvnc_transport_rfb_channel. | A001 Transport |
| capacity | field | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel | public | Stores capacity in pstvnc_transport_rfb_channel. | A001 Transport |
| capacity | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_initialize | prototype | Supplies capacity to pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_activity_generation | prototype | Supplies channel to pstvnc_transport_rfb_channel_activity_generation. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_available | prototype | Supplies channel to pstvnc_transport_rfb_channel_available. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_commit | prototype | Supplies channel to pstvnc_transport_rfb_channel_commit. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_discard_residual | prototype | Supplies channel to pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_initialize | prototype | Supplies channel to pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_read_available | prototype | Supplies channel to pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| channel | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_read_exact | prototype | Supplies channel to pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| count | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_read_exact | prototype | Supplies count to pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| discarded_count | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_discard_residual | prototype | Supplies discarded_count to pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| expected_count | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_discard_residual | prototype | Supplies expected_count to pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| maximum_count | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_read_available | prototype | Supplies maximum_count to pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| payload | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_commit | prototype | Supplies payload to pstvnc_transport_rfb_channel_commit. | A001 Transport |
| payload_length | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_commit | prototype | Supplies payload_length to pstvnc_transport_rfb_channel_commit. | A001 Transport |
| pstvnc_transport_rfb_channel | structure | src/transport/rfb_channel.h | Transport | public | Defines pstvnc_transport_rfb_channel type. | A001 Transport |
| pstvnc_transport_rfb_channel_activity_generation | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_activity_generation. | A001 Transport |
| pstvnc_transport_rfb_channel_available | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_available. | A001 Transport |
| pstvnc_transport_rfb_channel_commit | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_commit. | A001 Transport |
| pstvnc_transport_rfb_channel_discard_residual | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_discard_residual. | A001 Transport |
| pstvnc_transport_rfb_channel_initialize | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| pstvnc_transport_rfb_channel_read_available | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_read_available. | A001 Transport |
| pstvnc_transport_rfb_channel_read_exact | function declaration | src/transport/rfb_channel.h | Transport | public | Declares pstvnc_transport_rfb_channel_read_exact. | A001 Transport |
| pstvnc_transport_rfb_channel_t | type | src/transport/rfb_channel.h | Transport | public | Defines pstvnc_transport_rfb_channel_t type. | A001 Transport |
| read_offset | field | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel | public | Stores read_offset in pstvnc_transport_rfb_channel. | A001 Transport |
| storage | field | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel | public | Stores storage in pstvnc_transport_rfb_channel. | A001 Transport |
| storage | prototype parameter | src/transport/rfb_channel.h | pstvnc_transport_rfb_channel_initialize | prototype | Supplies storage to pstvnc_transport_rfb_channel_initialize. | A001 Transport |
| accepted | variable | src/transport/runtime.c | pstvnc_transport_runtime_accept_rfb_frame | local | Stores accepted for pstvnc_transport_runtime_accept_rfb_frame. | A001 Transport |
| activity_sequence | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_activity_snapshot | local | Supplies activity_sequence to pstvnc_transport_runtime_rfb_activity_snapshot. | A001 Transport |
| activity_sequence | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_wait_activity | local | Supplies activity_sequence to pstvnc_transport_runtime_rfb_wait_activity. | A001 Transport |
| activity_sequence | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Stores activity_sequence for pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| aligned_address | variable | src/transport/runtime.c | pstvnc_transport_runtime_allocate_aligned16 | local | Stores aligned_address for pstvnc_transport_runtime_allocate_aligned16. | A001 Transport |
| allocation | parameter | src/transport/runtime.c | pstvnc_transport_runtime_allocate_aligned16 | local | Supplies allocation to pstvnc_transport_runtime_allocate_aligned16. | A001 Transport |
| amount | parameter | src/transport/runtime.c | pstvnc_transport_runtime_send_credit | local | Supplies amount to pstvnc_transport_runtime_send_credit. | A001 Transport |
| argument | parameter | src/transport/runtime.c | pstvnc_transport_runtime_receiver_thread | local | Supplies argument to pstvnc_transport_runtime_receiver_thread. | A001 Transport |
| available | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_poll_receive | local | Stores available for pstvnc_transport_runtime_rfb_poll_receive. | A001 Transport |
| buffer | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Supplies buffer to pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| buffer | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Supplies buffer to pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| byte_count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_allocate_aligned16 | local | Supplies byte_count to pstvnc_transport_runtime_allocate_aligned16. | A001 Transport |
| bytes | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Stores bytes for pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_config_valid | local | Supplies config to pstvnc_transport_runtime_config_valid. | A001 Transport |
| config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize | local | Supplies config to pstvnc_transport_runtime_initialize. | A001 Transport |
| count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Supplies count to pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Supplies count to pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| destination | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Stores destination for pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| done | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Stores done for pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| fragment | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Stores fragment for pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| header | parameter | src/transport/runtime.c | pstvnc_transport_runtime_accept_rfb_frame | local | Supplies header to pstvnc_transport_runtime_accept_rfb_frame. | A001 Transport |
| header | variable | src/transport/runtime.c | pstvnc_transport_runtime_receiver_thread | local | Stores header for pstvnc_transport_runtime_receiver_thread. | A001 Transport |
| initial_count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_create_semaphore | local | Supplies initial_count to pstvnc_transport_runtime_create_semaphore. | A001 Transport |
| maximum_count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_create_semaphore | local | Supplies maximum_count to pstvnc_transport_runtime_create_semaphore. | A001 Transport |
| offset | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Stores offset for pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| payload | variable | src/transport/runtime.c | pstvnc_transport_runtime_send_credit | local | Stores payload for pstvnc_transport_runtime_send_credit. | A001 Transport |
| pstvnc_transport_runtime_accept_quiesce_marker_locked | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_accept_quiesce_marker_locked. | A001 Transport |
| pstvnc_transport_runtime_accept_rfb_frame | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_accept_rfb_frame. | A001 Transport |
| pstvnc_transport_runtime_allocate_aligned16 | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_allocate_aligned16. | A001 Transport |
| pstvnc_transport_runtime_config_valid | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_config_valid. | A001 Transport |
| pstvnc_transport_runtime_create_semaphore | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_create_semaphore. | A001 Transport |
| pstvnc_transport_runtime_initialize | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_initialize. | A001 Transport |
| pstvnc_transport_runtime_receiver_thread | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_receiver_thread. | A001 Transport |
| pstvnc_transport_runtime_release | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_release. | A001 Transport |
| pstvnc_transport_runtime_request_stop | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_request_stop. | A001 Transport |
| pstvnc_transport_runtime_rfb_activity_snapshot | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_rfb_activity_snapshot. | A001 Transport |
| pstvnc_transport_runtime_rfb_poll_receive | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_rfb_poll_receive. | A001 Transport |
| pstvnc_transport_runtime_rfb_read_exact | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| pstvnc_transport_runtime_rfb_wait_activity | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_rfb_wait_activity. | A001 Transport |
| pstvnc_transport_runtime_rfb_write_exact | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| pstvnc_transport_runtime_send_credit | function | src/transport/runtime.c | Transport | file | Defines pstvnc_transport_runtime_send_credit. | A001 Transport |
| pstvnc_transport_runtime_start_receiver | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_start_receiver. | A001 Transport |
| pstvnc_transport_runtime_wait_receiver_done | function | src/transport/runtime.c | Transport | public | Defines pstvnc_transport_runtime_wait_receiver_done. | A001 Transport |
| queue_empty | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Stores queue_empty for pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| raw | variable | src/transport/runtime.c | pstvnc_transport_runtime_allocate_aligned16 | local | Stores raw for pstvnc_transport_runtime_allocate_aligned16. | A001 Transport |
| remaining | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Stores remaining for pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| result | variable | src/transport/runtime.c | pstvnc_transport_runtime_release | local | Stores result for pstvnc_transport_runtime_release. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_accept_quiesce_marker_locked | local | Supplies runtime to pstvnc_transport_runtime_accept_quiesce_marker_locked. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_accept_rfb_frame | local | Supplies runtime to pstvnc_transport_runtime_accept_rfb_frame. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize | local | Supplies runtime to pstvnc_transport_runtime_initialize. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_release | local | Supplies runtime to pstvnc_transport_runtime_release. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_request_stop | local | Supplies runtime to pstvnc_transport_runtime_request_stop. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_activity_snapshot | local | Supplies runtime to pstvnc_transport_runtime_rfb_activity_snapshot. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_poll_receive | local | Supplies runtime to pstvnc_transport_runtime_rfb_poll_receive. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Supplies runtime to pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_wait_activity | local | Supplies runtime to pstvnc_transport_runtime_rfb_wait_activity. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_rfb_write_exact | local | Supplies runtime to pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_send_credit | local | Supplies runtime to pstvnc_transport_runtime_send_credit. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_start_receiver | local | Supplies runtime to pstvnc_transport_runtime_start_receiver. | A001 Transport |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_wait_receiver_done | local | Supplies runtime to pstvnc_transport_runtime_wait_receiver_done. | A001 Transport |
| runtime | variable | src/transport/runtime.c | pstvnc_transport_runtime_receiver_thread | local | Stores runtime for pstvnc_transport_runtime_receiver_thread. | A001 Transport |
| semaphore | variable | src/transport/runtime.c | pstvnc_transport_runtime_create_semaphore | local | Stores semaphore for pstvnc_transport_runtime_create_semaphore. | A001 Transport |
| signal_waiter | variable | src/transport/runtime.c | pstvnc_transport_runtime_accept_rfb_frame | local | Stores signal_waiter for pstvnc_transport_runtime_accept_rfb_frame. | A001 Transport |
| signal_waiter | variable | src/transport/runtime.c | pstvnc_transport_runtime_receiver_thread | local | Stores signal_waiter for pstvnc_transport_runtime_receiver_thread. | A001 Transport |
| socket_fd | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize | local | Supplies socket_fd to pstvnc_transport_runtime_initialize. | A001 Transport |
| status | variable | src/transport/runtime.c | pstvnc_transport_runtime_release | local | Stores status for pstvnc_transport_runtime_release. | A001 Transport |
| taken | variable | src/transport/runtime.c | pstvnc_transport_runtime_rfb_read_exact | local | Stores taken for pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| thread | variable | src/transport/runtime.c | pstvnc_transport_runtime_start_receiver | local | Stores thread for pstvnc_transport_runtime_start_receiver. | A001 Transport |
| PSTVNC_TRANSPORT_RUNTIME_H | include guard | src/transport/runtime.h | Transport | file | Defines PSTVNC_TRANSPORT_RUNTIME_H type. | A001 Transport |
| activity_sequence | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores activity_sequence in pstvnc_transport_runtime. | A001 Transport |
| activity_sequence | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_activity_snapshot | prototype | Supplies activity_sequence to pstvnc_transport_runtime_rfb_activity_snapshot. | A001 Transport |
| activity_sequence | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_wait_activity | prototype | Supplies activity_sequence to pstvnc_transport_runtime_rfb_wait_activity. | A001 Transport |
| activity_wait_armed | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores activity_wait_armed in pstvnc_transport_runtime. | A001 Transport |
| buffer | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_read_exact | prototype | Supplies buffer to pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| buffer | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_write_exact | prototype | Supplies buffer to pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| config | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize | prototype | Supplies config to pstvnc_transport_runtime_initialize. | A001 Transport |
| count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_read_exact | prototype | Supplies count to pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_write_exact | prototype | Supplies count to pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| discarded_count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_discard_quiesce_residual | prototype | Supplies discarded_count to pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| expected_count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_discard_quiesce_residual | prototype | Supplies expected_count to pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| failed | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores failed in pstvnc_transport_runtime. | A001 Transport |
| initialized | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores initialized in pstvnc_transport_runtime. | A001 Transport |
| max_data_payload | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores max_data_payload in pstvnc_transport_runtime. | A001 Transport |
| physical_stream | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores physical_stream in pstvnc_transport_runtime. | A001 Transport |
| pstvnc_transport_runtime | structure | src/transport/runtime.h | Transport | public | Defines pstvnc_transport_runtime type. | A001 Transport |
| pstvnc_transport_runtime_config_t | type | src/transport/runtime.h | Transport | public | Defines pstvnc_transport_runtime_config_t type. | A001 Transport |
| pstvnc_transport_runtime_initialize | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_initialize. | A001 Transport |
| pstvnc_transport_runtime_release | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_release. | A001 Transport |
| pstvnc_transport_runtime_request_stop | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_request_stop. | A001 Transport |
| pstvnc_transport_runtime_rfb_activity_snapshot | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_activity_snapshot. | A001 Transport |
| pstvnc_transport_runtime_rfb_discard_quiesce_residual | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| pstvnc_transport_runtime_rfb_poll_receive | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_poll_receive. | A001 Transport |
| pstvnc_transport_runtime_rfb_quiesce_requested | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_quiesce_requested. | A001 Transport |
| pstvnc_transport_runtime_rfb_read_exact | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| pstvnc_transport_runtime_rfb_send_quiesce_boundary | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_send_quiesce_boundary. | A001 Transport |
| pstvnc_transport_runtime_rfb_send_quiesce_complete | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_send_quiesce_complete. | A001 Transport |
| pstvnc_transport_runtime_rfb_snapshot_residual | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_snapshot_residual. | A001 Transport |
| pstvnc_transport_runtime_rfb_wait_activity | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_wait_activity. | A001 Transport |
| pstvnc_transport_runtime_rfb_wait_quiesce_commit | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| pstvnc_transport_runtime_rfb_write_exact | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| pstvnc_transport_runtime_start_receiver | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_start_receiver. | A001 Transport |
| pstvnc_transport_runtime_t | type | src/transport/runtime.h | Transport | public | Defines pstvnc_transport_runtime_t type. | A001 Transport |
| pstvnc_transport_runtime_wait_receiver_done | function declaration | src/transport/runtime.h | Transport | public | Declares pstvnc_transport_runtime_wait_receiver_done. | A001 Transport |
| receiver_done | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_done in pstvnc_transport_runtime. | A001 Transport |
| receiver_done_semaphore_id | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_done_semaphore_id in pstvnc_transport_runtime. | A001 Transport |
| receiver_payload | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_payload in pstvnc_transport_runtime. | A001 Transport |
| receiver_stack | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_stack in pstvnc_transport_runtime. | A001 Transport |
| receiver_stack_allocation | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_stack_allocation in pstvnc_transport_runtime. | A001 Transport |
| receiver_thread_id | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_thread_id in pstvnc_transport_runtime. | A001 Transport |
| receiver_thread_priority | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_thread_priority in pstvnc_transport_runtime. | A001 Transport |
| receiver_thread_stack_size | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_thread_stack_size in pstvnc_transport_runtime. | A001 Transport |
| receiver_thread_started | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores receiver_thread_started in pstvnc_transport_runtime. | A001 Transport |
| residual_count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_snapshot_residual | prototype | Supplies residual_count to pstvnc_transport_runtime_rfb_snapshot_residual. | A001 Transport |
| rfb_activity_semaphore_id | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_activity_semaphore_id in pstvnc_transport_runtime. | A001 Transport |
| rfb_channel | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_channel in pstvnc_transport_runtime. | A001 Transport |
| rfb_credit_batch_bytes | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_credit_batch_bytes in pstvnc_transport_runtime. | A001 Transport |
| rfb_credit_flush_on_empty | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_credit_flush_on_empty in pstvnc_transport_runtime. | A001 Transport |
| rfb_credit_pending | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_credit_pending in pstvnc_transport_runtime. | A001 Transport |
| rfb_credit_return_enabled | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_credit_return_enabled in pstvnc_transport_runtime. | A001 Transport |
| rfb_initial_credit_bytes | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_initial_credit_bytes in pstvnc_transport_runtime. | A001 Transport |
| rfb_queue_semaphore_id | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_queue_semaphore_id in pstvnc_transport_runtime. | A001 Transport |
| rfb_queue_storage | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_queue_storage in pstvnc_transport_runtime. | A001 Transport |
| rfb_quiesce_boundary_sent | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_quiesce_boundary_sent in pstvnc_transport_runtime. | A001 Transport |
| rfb_quiesce_commit_received | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_quiesce_commit_received in pstvnc_transport_runtime. | A001 Transport |
| rfb_quiesce_complete_sent | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_quiesce_complete_sent in pstvnc_transport_runtime. | A001 Transport |
| rfb_quiesce_request_received | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores rfb_quiesce_request_received in pstvnc_transport_runtime. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize | prototype | Supplies runtime to pstvnc_transport_runtime_initialize. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_release | prototype | Supplies runtime to pstvnc_transport_runtime_release. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_request_stop | prototype | Supplies runtime to pstvnc_transport_runtime_request_stop. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_activity_snapshot | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_activity_snapshot. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_discard_quiesce_residual | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_discard_quiesce_residual. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_poll_receive | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_poll_receive. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_quiesce_requested | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_quiesce_requested. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_read_exact | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_read_exact. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_send_quiesce_boundary | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_send_quiesce_boundary. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_send_quiesce_complete | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_send_quiesce_complete. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_snapshot_residual | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_snapshot_residual. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_wait_activity | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_wait_activity. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_wait_quiesce_commit | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_wait_quiesce_commit. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_rfb_write_exact | prototype | Supplies runtime to pstvnc_transport_runtime_rfb_write_exact. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_start_receiver | prototype | Supplies runtime to pstvnc_transport_runtime_start_receiver. | A001 Transport |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_wait_receiver_done | prototype | Supplies runtime to pstvnc_transport_runtime_wait_receiver_done. | A001 Transport |
| socket_fd | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize | prototype | Supplies socket_fd to pstvnc_transport_runtime_initialize. | A001 Transport |
| stop_requested | field | src/transport/runtime.h | pstvnc_transport_runtime | public | Stores stop_requested in pstvnc_transport_runtime. | A001 Transport |
| PSTVNC_TRANSPORT_CLOSED | enum value | src/transport/transport.h | pstvnc_transport_result | public | Defines PSTVNC_TRANSPORT_CLOSED value. | A001 Transport |
| PSTVNC_TRANSPORT_FAILED | enum value | src/transport/transport.h | pstvnc_transport_result | public | Defines PSTVNC_TRANSPORT_FAILED value. | A001 Transport |
| PSTVNC_TRANSPORT_H | include guard | src/transport/transport.h | Transport | file | Defines PSTVNC_TRANSPORT_H type. | A001 Transport |
| PSTVNC_TRANSPORT_INVALID | enum value | src/transport/transport.h | pstvnc_transport_result | public | Defines PSTVNC_TRANSPORT_INVALID value. | A001 Transport |
| PSTVNC_TRANSPORT_OK | enum value | src/transport/transport.h | pstvnc_transport_result | public | Defines PSTVNC_TRANSPORT_OK value. | A001 Transport |
| PSTVNC_TRANSPORT_WOULD_BLOCK | enum value | src/transport/transport.h | pstvnc_transport_result | public | Defines PSTVNC_TRANSPORT_WOULD_BLOCK value. | A001 Transport |
| max_data_payload | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores max_data_payload in pstvnc_transport_session_config. | A001 Transport |
| pstvnc_transport_result | enum | src/transport/transport.h | Transport | public | Defines pstvnc_transport_result type. | A001 Transport |
| pstvnc_transport_result_t | type | src/transport/transport.h | Transport | public | Defines pstvnc_transport_result_t type. | A001 Transport |
| pstvnc_transport_session_config | structure | src/transport/transport.h | Transport | public | Defines pstvnc_transport_session_config type. | A001 Transport |
| pstvnc_transport_session_config_t | type | src/transport/transport.h | Transport | public | Defines pstvnc_transport_session_config_t type. | A001 Transport |
| receiver_thread_priority | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores receiver_thread_priority in pstvnc_transport_session_config. | A001 Transport |
| receiver_thread_stack_size | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores receiver_thread_stack_size in pstvnc_transport_session_config. | A001 Transport |
| rfb_credit_batch_bytes | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores rfb_credit_batch_bytes in pstvnc_transport_session_config. | A001 Transport |
| rfb_credit_flush_on_empty | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores rfb_credit_flush_on_empty in pstvnc_transport_session_config. | A001 Transport |
| rfb_credit_return_enabled | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores rfb_credit_return_enabled in pstvnc_transport_session_config. | A001 Transport |
| rfb_initial_credit_bytes | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores rfb_initial_credit_bytes in pstvnc_transport_session_config. | A001 Transport |
| rfb_queue_capacity | field | src/transport/transport.h | pstvnc_transport_session_config | public | Stores rfb_queue_capacity in pstvnc_transport_session_config. | A001 Transport |
| pstvnc_transport_audio_channel_available | function | src/transport/audio_channel.c | audio_channel | file | Defines pstvnc_transport_audio_channel_available as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_commit_data | function | src/transport/audio_channel.c | audio_channel | file | Defines pstvnc_transport_audio_channel_commit_data as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_initialize | function | src/transport/audio_channel.c | audio_channel | file | Defines pstvnc_transport_audio_channel_initialize as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_mark_producer_done | function | src/transport/audio_channel.c | audio_channel | file | Defines pstvnc_transport_audio_channel_mark_producer_done as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_producer_done | function | src/transport/audio_channel.c | audio_channel | file | Defines pstvnc_transport_audio_channel_producer_done as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_read_available | function | src/transport/audio_channel.c | audio_channel | file | Defines pstvnc_transport_audio_channel_read_available as a current clean-source function. | mechanically reconciled current clean source |
| channel | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_available | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| channel | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_commit_data | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| payload | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_commit_data | local | Defines payload as a current clean-source parameter. | mechanically reconciled current clean source |
| payload_length | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_commit_data | local | Defines payload_length as a current clean-source parameter. | mechanically reconciled current clean source |
| first_count | variable | src/transport/audio_channel.c | pstvnc_transport_audio_channel_commit_data | local | Defines first_count as a current clean-source variable. | mechanically reconciled current clean source |
| write_offset | variable | src/transport/audio_channel.c | pstvnc_transport_audio_channel_commit_data | local | Defines write_offset as a current clean-source variable. | mechanically reconciled current clean source |
| capacity | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_initialize | local | Defines capacity as a current clean-source parameter. | mechanically reconciled current clean source |
| channel | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_initialize | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| storage | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_initialize | local | Defines storage as a current clean-source parameter. | mechanically reconciled current clean source |
| channel | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_mark_producer_done | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| channel | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_producer_done | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| channel | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_read_available | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| destination | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_read_available | local | Defines destination as a current clean-source parameter. | mechanically reconciled current clean source |
| maximum_count | parameter | src/transport/audio_channel.c | pstvnc_transport_audio_channel_read_available | local | Defines maximum_count as a current clean-source parameter. | mechanically reconciled current clean source |
| count | variable | src/transport/audio_channel.c | pstvnc_transport_audio_channel_read_available | local | Defines count as a current clean-source variable. | mechanically reconciled current clean source |
| first_count | variable | src/transport/audio_channel.c | pstvnc_transport_audio_channel_read_available | local | Defines first_count as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_available | function declaration | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_available as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_commit_data | function declaration | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_commit_data as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_initialize | function declaration | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_initialize as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_mark_producer_done | function declaration | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_mark_producer_done as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_producer_done | function declaration | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_producer_done as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_read_available | function declaration | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_read_available as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_TRANSPORT_AUDIO_CHANNEL_H | macro | src/transport/audio_channel.h | audio_channel interface | public | Defines PSTVNC_TRANSPORT_AUDIO_CHANNEL_H as a current clean-source macro. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel | structure | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel as a current clean-source structure. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_t | type | src/transport/audio_channel.h | audio_channel interface | public | Defines pstvnc_transport_audio_channel_t as a current clean-source type. | mechanically reconciled current clean source |
| byte_count | field | src/transport/audio_channel.h | pstvnc_transport_audio_channel | local | Defines byte_count as a current clean-source field. | mechanically reconciled current clean source |
| capacity | field | src/transport/audio_channel.h | pstvnc_transport_audio_channel | local | Defines capacity as a current clean-source field. | mechanically reconciled current clean source |
| producer_done | field | src/transport/audio_channel.h | pstvnc_transport_audio_channel | local | Defines producer_done as a current clean-source field. | mechanically reconciled current clean source |
| read_offset | field | src/transport/audio_channel.h | pstvnc_transport_audio_channel | local | Defines read_offset as a current clean-source field. | mechanically reconciled current clean source |
| storage | field | src/transport/audio_channel.h | pstvnc_transport_audio_channel | local | Defines storage as a current clean-source field. | mechanically reconciled current clean source |
| channel | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_available | local | Defines channel as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| channel | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_commit_data | local | Defines channel as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| payload | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_commit_data | local | Defines payload as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| payload_length | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_commit_data | local | Defines payload_length as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| capacity | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_initialize | local | Defines capacity as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| channel | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_initialize | local | Defines channel as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| storage | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_initialize | local | Defines storage as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| channel | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_mark_producer_done | local | Defines channel as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| channel | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_producer_done | local | Defines channel as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| channel | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_read_available | local | Defines channel as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| destination | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_read_available | local | Defines destination as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| maximum_count | prototype parameter | src/transport/audio_channel.h | pstvnc_transport_audio_channel_read_available | local | Defines maximum_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_transport_audio_activity_snapshot | function | src/transport/bridge.c | bridge | file | Defines pstvnc_transport_audio_activity_snapshot as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_read_available | function | src/transport/bridge.c | bridge | file | Defines pstvnc_transport_audio_read_available as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_status | function | src/transport/bridge.c | bridge | file | Defines pstvnc_transport_audio_status as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_audio_wait_activity | function | src/transport/bridge.c | bridge | file | Defines pstvnc_transport_audio_wait_activity as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_session_open_internal | function | src/transport/bridge.c | bridge | file | Defines pstvnc_transport_session_open_internal as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_session_open_with_audio | function | src/transport/bridge.c | bridge | file | Defines pstvnc_transport_session_open_with_audio as a current clean-source function. | mechanically reconciled current clean source |
| activity_sequence | parameter | src/transport/bridge.c | pstvnc_transport_audio_activity_snapshot | local | Defines activity_sequence as a current clean-source parameter. | mechanically reconciled current clean source |
| buffer | parameter | src/transport/bridge.c | pstvnc_transport_audio_read_available | local | Defines buffer as a current clean-source parameter. | mechanically reconciled current clean source |
| maximum_count | parameter | src/transport/bridge.c | pstvnc_transport_audio_read_available | local | Defines maximum_count as a current clean-source parameter. | mechanically reconciled current clean source |
| read_count | parameter | src/transport/bridge.c | pstvnc_transport_audio_read_available | local | Defines read_count as a current clean-source parameter. | mechanically reconciled current clean source |
| available_count | parameter | src/transport/bridge.c | pstvnc_transport_audio_status | local | Defines available_count as a current clean-source parameter. | mechanically reconciled current clean source |
| producer_done | parameter | src/transport/bridge.c | pstvnc_transport_audio_status | local | Defines producer_done as a current clean-source parameter. | mechanically reconciled current clean source |
| activity_sequence | parameter | src/transport/bridge.c | pstvnc_transport_audio_wait_activity | local | Defines activity_sequence as a current clean-source parameter. | mechanically reconciled current clean source |
| audio_config | parameter | src/transport/bridge.c | pstvnc_transport_session_open_internal | local | Defines audio_config as a current clean-source parameter. | mechanically reconciled current clean source |
| config | parameter | src/transport/bridge.c | pstvnc_transport_session_open_internal | local | Defines config as a current clean-source parameter. | mechanically reconciled current clean source |
| socket_fd | parameter | src/transport/bridge.c | pstvnc_transport_session_open_internal | local | Defines socket_fd as a current clean-source parameter. | mechanically reconciled current clean source |
| initialized | variable | src/transport/bridge.c | pstvnc_transport_session_open_internal | local | Defines initialized as a current clean-source variable. | mechanically reconciled current clean source |
| audio_config | parameter | src/transport/bridge.c | pstvnc_transport_session_open_with_audio | local | Defines audio_config as a current clean-source parameter. | mechanically reconciled current clean source |
| config | parameter | src/transport/bridge.c | pstvnc_transport_session_open_with_audio | local | Defines config as a current clean-source parameter. | mechanically reconciled current clean source |
| socket_fd | parameter | src/transport/bridge.c | pstvnc_transport_session_open_with_audio | local | Defines socket_fd as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_transport_audio_activity_snapshot | function declaration | src/transport/bridge.h | bridge interface | public | Defines pstvnc_transport_audio_activity_snapshot as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_read_available | function declaration | src/transport/bridge.h | bridge interface | public | Defines pstvnc_transport_audio_read_available as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_status | function declaration | src/transport/bridge.h | bridge interface | public | Defines pstvnc_transport_audio_status as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_audio_wait_activity | function declaration | src/transport/bridge.h | bridge interface | public | Defines pstvnc_transport_audio_wait_activity as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_session_open_with_audio | function declaration | src/transport/bridge.h | bridge interface | public | Defines pstvnc_transport_session_open_with_audio as a current clean-source function declaration. | mechanically reconciled current clean source |
| activity_sequence | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_activity_snapshot | local | Defines activity_sequence as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| buffer | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_read_available | local | Defines buffer as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| maximum_count | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_read_available | local | Defines maximum_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| read_count | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_read_available | local | Defines read_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| available_count | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_status | local | Defines available_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| producer_done | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_status | local | Defines producer_done as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| activity_sequence | prototype parameter | src/transport/bridge.h | pstvnc_transport_audio_wait_activity | local | Defines activity_sequence as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| audio_config | prototype parameter | src/transport/bridge.h | pstvnc_transport_session_open_with_audio | local | Defines audio_config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| config | prototype parameter | src/transport/bridge.h | pstvnc_transport_session_open_with_audio | local | Defines config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| socket_fd | prototype parameter | src/transport/bridge.h | pstvnc_transport_session_open_with_audio | local | Defines socket_fd as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_accept_audio_frame | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_accept_audio_frame as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_activity_snapshot | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_audio_activity_snapshot as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_config_valid | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_audio_config_valid as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_read_available | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_audio_read_available as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_status | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_audio_status as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_wait_activity | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_audio_wait_activity as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_initialize_internal | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_initialize_internal as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_initialize_with_audio | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_initialize_with_audio as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_publish_audio_activity_locked | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_publish_audio_activity_locked as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_publish_audio_terminal | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_publish_audio_terminal as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_publish_rfb_activity_locked | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_publish_rfb_activity_locked as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_reset_identifiers | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_reset_identifiers as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_return_audio_credit | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_return_audio_credit as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_return_rfb_credit | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_return_rfb_credit as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_signal_audio_activity | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_signal_audio_activity as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_signal_rfb_activity | function | src/transport/runtime.c | runtime | file | Defines pstvnc_transport_runtime_signal_rfb_activity as a current clean-source function. | mechanically reconciled current clean source |
| header | parameter | src/transport/runtime.c | pstvnc_transport_runtime_accept_audio_frame | local | Defines header as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_accept_audio_frame | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| accepted | variable | src/transport/runtime.c | pstvnc_transport_runtime_accept_audio_frame | local | Defines accepted as a current clean-source variable. | mechanically reconciled current clean source |
| signal_waiter | variable | src/transport/runtime.c | pstvnc_transport_runtime_accept_audio_frame | local | Defines signal_waiter as a current clean-source variable. | mechanically reconciled current clean source |
| activity_sequence | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_activity_snapshot | local | Defines activity_sequence as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_activity_snapshot | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| audio_config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_config_valid | local | Defines audio_config as a current clean-source parameter. | mechanically reconciled current clean source |
| config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_config_valid | local | Defines config as a current clean-source parameter. | mechanically reconciled current clean source |
| buffer | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines buffer as a current clean-source parameter. | mechanically reconciled current clean source |
| maximum_count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines maximum_count as a current clean-source parameter. | mechanically reconciled current clean source |
| read_count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines read_count as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| producer_done | variable | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines producer_done as a current clean-source variable. | mechanically reconciled current clean source |
| queue_empty | variable | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines queue_empty as a current clean-source variable. | mechanically reconciled current clean source |
| taken | variable | src/transport/runtime.c | pstvnc_transport_runtime_audio_read_available | local | Defines taken as a current clean-source variable. | mechanically reconciled current clean source |
| available_count | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_status | local | Defines available_count as a current clean-source parameter. | mechanically reconciled current clean source |
| producer_done | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_status | local | Defines producer_done as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_status | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| activity_sequence | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_wait_activity | local | Defines activity_sequence as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_audio_wait_activity | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| audio_config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_internal | local | Defines audio_config as a current clean-source parameter. | mechanically reconciled current clean source |
| config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_internal | local | Defines config as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_internal | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| socket_fd | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_internal | local | Defines socket_fd as a current clean-source parameter. | mechanically reconciled current clean source |
| audio_config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_with_audio | local | Defines audio_config as a current clean-source parameter. | mechanically reconciled current clean source |
| config | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_with_audio | local | Defines config as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_with_audio | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| socket_fd | parameter | src/transport/runtime.c | pstvnc_transport_runtime_initialize_with_audio | local | Defines socket_fd as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_publish_audio_activity_locked | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| signal_waiter | variable | src/transport/runtime.c | pstvnc_transport_runtime_publish_audio_activity_locked | local | Defines signal_waiter as a current clean-source variable. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_publish_audio_terminal | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| signal_waiter | variable | src/transport/runtime.c | pstvnc_transport_runtime_publish_audio_terminal | local | Defines signal_waiter as a current clean-source variable. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_publish_rfb_activity_locked | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| signal_waiter | variable | src/transport/runtime.c | pstvnc_transport_runtime_publish_rfb_activity_locked | local | Defines signal_waiter as a current clean-source variable. | mechanically reconciled current clean source |
| accepted | variable | src/transport/runtime.c | pstvnc_transport_runtime_receiver_thread | local | Defines accepted as a current clean-source variable. | mechanically reconciled current clean source |
| waiter_live | variable | src/transport/runtime.c | pstvnc_transport_runtime_release | local | Defines waiter_live as a current clean-source variable. | mechanically reconciled current clean source |
| shutdown_ok | variable | src/transport/runtime.c | pstvnc_transport_runtime_request_stop | local | Defines shutdown_ok as a current clean-source variable. | mechanically reconciled current clean source |
| wake_ok | variable | src/transport/runtime.c | pstvnc_transport_runtime_request_stop | local | Defines wake_ok as a current clean-source variable. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_reset_identifiers | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| consumed | parameter | src/transport/runtime.c | pstvnc_transport_runtime_return_audio_credit | local | Defines consumed as a current clean-source parameter. | mechanically reconciled current clean source |
| queue_empty | parameter | src/transport/runtime.c | pstvnc_transport_runtime_return_audio_credit | local | Defines queue_empty as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_return_audio_credit | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| amount | variable | src/transport/runtime.c | pstvnc_transport_runtime_return_audio_credit | local | Defines amount as a current clean-source variable. | mechanically reconciled current clean source |
| consumed | parameter | src/transport/runtime.c | pstvnc_transport_runtime_return_rfb_credit | local | Defines consumed as a current clean-source parameter. | mechanically reconciled current clean source |
| queue_empty | parameter | src/transport/runtime.c | pstvnc_transport_runtime_return_rfb_credit | local | Defines queue_empty as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_return_rfb_credit | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| amount | variable | src/transport/runtime.c | pstvnc_transport_runtime_return_rfb_credit | local | Defines amount as a current clean-source variable. | mechanically reconciled current clean source |
| channel | parameter | src/transport/runtime.c | pstvnc_transport_runtime_send_credit | local | Defines channel as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_signal_audio_activity | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| signal_waiter | parameter | src/transport/runtime.c | pstvnc_transport_runtime_signal_audio_activity | local | Defines signal_waiter as a current clean-source parameter. | mechanically reconciled current clean source |
| runtime | parameter | src/transport/runtime.c | pstvnc_transport_runtime_signal_rfb_activity | local | Defines runtime as a current clean-source parameter. | mechanically reconciled current clean source |
| signal_waiter | parameter | src/transport/runtime.c | pstvnc_transport_runtime_signal_rfb_activity | local | Defines signal_waiter as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_activity_snapshot | function declaration | src/transport/runtime.h | runtime interface | public | Defines pstvnc_transport_runtime_audio_activity_snapshot as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_read_available | function declaration | src/transport/runtime.h | runtime interface | public | Defines pstvnc_transport_runtime_audio_read_available as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_status | function declaration | src/transport/runtime.h | runtime interface | public | Defines pstvnc_transport_runtime_audio_status as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_audio_wait_activity | function declaration | src/transport/runtime.h | runtime interface | public | Defines pstvnc_transport_runtime_audio_wait_activity as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_transport_runtime_initialize_with_audio | function declaration | src/transport/runtime.h | runtime interface | public | Defines pstvnc_transport_runtime_initialize_with_audio as a current clean-source function declaration. | mechanically reconciled current clean source |
| audio_activity_semaphore_id | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_activity_semaphore_id as a current clean-source field. | mechanically reconciled current clean source |
| audio_activity_sequence | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_activity_sequence as a current clean-source field. | mechanically reconciled current clean source |
| audio_activity_wait_armed | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_activity_wait_armed as a current clean-source field. | mechanically reconciled current clean source |
| audio_channel | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_channel as a current clean-source field. | mechanically reconciled current clean source |
| audio_credit_batch_bytes | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_credit_batch_bytes as a current clean-source field. | mechanically reconciled current clean source |
| audio_credit_flush_on_empty | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_credit_flush_on_empty as a current clean-source field. | mechanically reconciled current clean source |
| audio_credit_pending | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_credit_pending as a current clean-source field. | mechanically reconciled current clean source |
| audio_credit_return_enabled | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_credit_return_enabled as a current clean-source field. | mechanically reconciled current clean source |
| audio_enabled | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_enabled as a current clean-source field. | mechanically reconciled current clean source |
| audio_initial_credit_bytes | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_initial_credit_bytes as a current clean-source field. | mechanically reconciled current clean source |
| audio_queue_semaphore_id | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_queue_semaphore_id as a current clean-source field. | mechanically reconciled current clean source |
| audio_queue_storage | field | src/transport/runtime.h | pstvnc_transport_runtime | local | Defines audio_queue_storage as a current clean-source field. | mechanically reconciled current clean source |
| activity_sequence | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_activity_snapshot | local | Defines activity_sequence as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_activity_snapshot | local | Defines runtime as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| buffer | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_read_available | local | Defines buffer as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| maximum_count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_read_available | local | Defines maximum_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| read_count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_read_available | local | Defines read_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_read_available | local | Defines runtime as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| available_count | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_status | local | Defines available_count as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| producer_done | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_status | local | Defines producer_done as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_status | local | Defines runtime as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| activity_sequence | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_wait_activity | local | Defines activity_sequence as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_audio_wait_activity | local | Defines runtime as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| audio_config | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize_with_audio | local | Defines audio_config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| config | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize_with_audio | local | Defines config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| runtime | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize_with_audio | local | Defines runtime as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| socket_fd | prototype parameter | src/transport/runtime.h | pstvnc_transport_runtime_initialize_with_audio | local | Defines socket_fd as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_config | structure | src/transport/transport.h | transport interface | public | Defines pstvnc_transport_audio_channel_config as a current clean-source structure. | mechanically reconciled current clean source |
| pstvnc_transport_audio_channel_config_t | type | src/transport/transport.h | transport interface | public | Defines pstvnc_transport_audio_channel_config_t as a current clean-source type. | mechanically reconciled current clean source |
| credit_batch_bytes | field | src/transport/transport.h | pstvnc_transport_audio_channel_config | local | Defines credit_batch_bytes as a current clean-source field. | mechanically reconciled current clean source |
| credit_flush_on_empty | field | src/transport/transport.h | pstvnc_transport_audio_channel_config | local | Defines credit_flush_on_empty as a current clean-source field. | mechanically reconciled current clean source |
| credit_return_enabled | field | src/transport/transport.h | pstvnc_transport_audio_channel_config | local | Defines credit_return_enabled as a current clean-source field. | mechanically reconciled current clean source |
| initial_credit_bytes | field | src/transport/transport.h | pstvnc_transport_audio_channel_config | local | Defines initial_credit_bytes as a current clean-source field. | mechanically reconciled current clean source |
| queue_capacity | field | src/transport/transport.h | pstvnc_transport_audio_channel_config | local | Defines queue_capacity as a current clean-source field. | mechanically reconciled current clean source |
| PSTVNC_TRANSPORT_EXHAUSTED | enum value | src/transport/transport.h | pstvnc_transport_result | local | Defines PSTVNC_TRANSPORT_EXHAUSTED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_TRANSPORT_STOPPED | enum value | src/transport/transport.h | pstvnc_transport_result | local | Defines PSTVNC_TRANSPORT_STOPPED as a current clean-source enum value. | mechanically reconciled current clean source |
