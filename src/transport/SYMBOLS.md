# `src/transport` Symbol Dictionary

Context: `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` and `docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md`.

| Symbol | Kind | File | Owner | Scope | Meaning |
|---|---|---|---|---|---|
| `pstvnc_transport_result_t` | enum type | `src/transport/transport.h` | transport | public | Common result vocabulary for logical transport operations. |
| `PSTVNC_TRANSPORT_OK` | enum value | `src/transport/transport.h` | transport | public | Requested transport operation completed. |
| `PSTVNC_TRANSPORT_WOULD_BLOCK` | enum value | `src/transport/transport.h` | transport | public | Logical stream currently has no complete requested progress available. |
| `PSTVNC_TRANSPORT_CLOSED` | enum value | `src/transport/transport.h` | transport | public | Session/physical stream is closed. |
| `PSTVNC_TRANSPORT_INVALID` | enum value | `src/transport/transport.h` | transport | public | Call violates transport state or argument contract. |
| `pstvnc_transport_rfb_read_exact` | function | `src/transport/transport.h` | transport | public | Reads exactly the requested bytes from transport-dispatched logical RFB state. |
| `pstvnc_transport_rfb_poll_receive` | function | `src/transport/transport.h` | transport | public | Reports whether logical RFB bytes are ready without advancing the physical socket independently. |
| `pstvnc_transport_rfb_write_exact` | function | `src/transport/transport.h` | transport | public | Submits RFB bytes to transport-owned serialized physical framing. |
| `pstvnc_transport_session_open` | function | `src/transport/transport.h` | transport | public | Adopts the application-established physical socket and allocates only enabled logical channels. |
| `pstvnc_transport_session_quiesce` | function | `src/transport/transport.h` | transport | public | Waits for explicit receiver-dispatch quiescence before channel resources may retire. |
| `pstvnc_transport_session_close` | function | `src/transport/transport.h` | transport | public | Retires transport-owned session/channel state after quiescence. |
| `pstvnc_transport_frame_kind_t` | enum type | `src/transport/protocol.h` | transport | internal/public wire value | Stable PSTV v1 frame-kind vocabulary. |
| `pstvnc_transport_channel_t` | enum type | `src/transport/protocol.h` | transport | internal/public wire value | Stable logical channel identities for control, RFB, audio, telemetry, and MPEG2. |
| `pstvnc_transport_header_t` | struct type | `src/transport/protocol.h` | transport | internal | Decoded fixed PSTV frame-header representation. |
| `pstvnc_transport_header_encode` | function | `src/transport/protocol.c` | transport | internal | Validates and encodes one fixed PSTV v1 wire header. |
| `pstvnc_transport_header_decode` | function | `src/transport/protocol.c` | transport | internal | Validates magic/version/payload ceiling and decodes one PSTV v1 wire header. |
| `pstvnc_transport_read_be32` | function | `src/transport/protocol.c` | transport | internal | Reads one big-endian 32-bit wire value. |
| `pstvnc_transport_write_be32` | function | `src/transport/protocol.c` | transport | internal | Writes one big-endian 32-bit wire value. |
| `pstvnc_transport_physical_stream_t` | struct type | `src/transport/physical_stream.h` | transport | internal | Owns the adopted physical socket, send lock, and direction-local PSTV sequence state. |
| `pstvnc_transport_physical_stream_adopt` | function | `src/transport/physical_stream.c` | transport | internal | Accepts ownership of one physical socket only after the send mutex is created and initializes both sequence directions. |
| `pstvnc_transport_physical_stream_send_frame` | function | `src/transport/physical_stream.c` | transport | internal | Serializes one complete PSTV header/payload transaction and advances outbound sequence only after complete send. |
| `pstvnc_transport_physical_stream_receive_frame` | function | `src/transport/physical_stream.c` | transport | internal | Sole physical receive primitive; reads one complete frame, validates inbound sequence and capacity, and advances receive sequence only after complete payload receipt. |
| `pstvnc_transport_physical_stream_release` | function | `src/transport/physical_stream.c` | transport | internal | Releases the adopted socket and send mutex after higher-level quiescence is proven and resets direction-local sequences. |
| `pstvnc_transport_physical_stream_send_exact` | function | `src/transport/physical_stream.c` | transport | file-local | Repeats socket send until the requested bytes are committed or a send fails. |
| `pstvnc_transport_physical_stream_receive_exact` | function | `src/transport/physical_stream.c` | transport | file-local | Repeats socket receive until the requested bytes are complete or receive fails. |
| `pstvnc_transport_rfb_channel_t` | struct type | `src/transport/rfb_channel.h` | transport | internal | Owns committed logical RFB bytes, circular-buffer accounting, and producer activity generation for one session. |
| `pstvnc_transport_rfb_channel_initialize` | function | `src/transport/rfb_channel.c` | transport | internal | Binds caller-owned session storage and initializes an empty logical RFB stream. |
| `pstvnc_transport_rfb_channel_commit` | function | `src/transport/rfb_channel.c` | transport | internal | Atomically commits one complete received channel-1 DATA payload when sufficient capacity exists. |
| `pstvnc_transport_rfb_channel_read_exact` | function | `src/transport/rfb_channel.c` | transport | internal | Consumes exactly the requested committed logical RFB bytes or leaves stream state unchanged. |
| `pstvnc_transport_rfb_channel_read_available` | function | `src/transport/rfb_channel.c` | transport | internal | Consumes up to the requested committed bytes so parser credit can return incrementally instead of requiring one whole exact read to fit in the queue. |
| `pstvnc_transport_rfb_channel_discard_residual` | function | `src/transport/rfb_channel.c` | transport | internal | Discards an exact terminal residual snapshot without classifying bytes as parser consumption or activity. |
| `pstvnc_transport_rfb_channel_available` | function | `src/transport/rfb_channel.c` | transport | internal | Reports the number of committed logical RFB bytes currently available. |
| `pstvnc_transport_rfb_channel_activity_generation` | function | `src/transport/rfb_channel.c` | transport | internal | Reports the monotonically advancing producer generation used by the higher runtime for activity rendezvous. |
| `pstvnc_transport_runtime_config_t` | struct type | `src/transport/runtime.h` | transport | internal | Carries validated session queue, credit, payload, and receiver-thread parameters into the transport runtime. |
| `pstvnc_transport_runtime_t` | struct type | `src/transport/runtime.h` | transport | internal | Owns one physical stream, sole receiver thread, synchronized logical RFB resources, flow-control state, activity rendezvous, and receiver completion state. |
| `pstvnc_transport_runtime_initialize` | function | `src/transport/runtime.c` | transport | internal | Allocates session RFB resources and receiver stack and adopts the one physical stream from validated runtime configuration. |
| `pstvnc_transport_runtime_start_receiver` | function | `src/transport/runtime.c` | transport | internal | Sends initial RFB credit and starts the sole physical receiver only after all receiver-visible resources exist. |
| `pstvnc_transport_runtime_rfb_activity_snapshot` | function | `src/transport/runtime.c` | transport | internal | Snapshots protected producer activity for race-free timerless consumer rendezvous. |
| `pstvnc_transport_runtime_rfb_wait_activity` | function | `src/transport/runtime.c` | transport | internal | Arms and waits for exactly one producer event only when no newer protected activity is already visible. |
| `pstvnc_transport_runtime_rfb_read_exact` | function | `src/transport/runtime.c` | transport | internal | Completes a logical exact read by incrementally consuming committed bytes, returning parser-earned credit, and sleeping only on producer activity. |
| `pstvnc_transport_runtime_rfb_poll_receive` | function | `src/transport/runtime.c` | transport | internal | Reports logical RFB availability or terminal receiver state without reading the physical socket. |
| `pstvnc_transport_runtime_rfb_write_exact` | function | `src/transport/runtime.c` | transport | internal | Fragments outbound logical RFB bytes at the configured payload ceiling through the shared serialized physical send path. |
| `pstvnc_transport_runtime_wait_receiver_done` | function | `src/transport/runtime.c` | transport | internal | Blocks on the receiver completion event rather than polling diagnostic counters or timers. |
| `pstvnc_transport_runtime_release` | function | `src/transport/runtime.c` | transport | internal | Refuses receiver-visible resource reclamation until explicit receiver completion and then releases transport-owned resources. |
| `pstvnc_transport_runtime_rfb_quiesce_requested` | function | `src/transport/quiesce.c` | transport | internal | Reports whether the ordered Pi REQUEST marker has been accepted. |
| `pstvnc_transport_runtime_rfb_send_quiesce_boundary` | function | `src/transport/quiesce.c` | transport | internal | Publishes and sends the PS2 BOUNDARY phase after the caller reaches its safe RFB-message boundary. |
| `pstvnc_transport_runtime_rfb_wait_quiesce_commit` | function | `src/transport/quiesce.c` | transport | internal | Waits for the ordered Pi COMMIT phase via the same producer activity rendezvous. |
| `pstvnc_transport_runtime_rfb_snapshot_residual` | function | `src/transport/quiesce.c` | transport | internal | Captures the exact remaining logical RFB queue size after COMMIT. |
| `pstvnc_transport_runtime_rfb_discard_quiesce_residual` | function | `src/transport/quiesce.c` | transport | internal | Discards only the exact snapshotted terminal residual without returning false parser credit. |
| `pstvnc_transport_runtime_rfb_send_quiesce_complete` | function | `src/transport/quiesce.c` | transport | internal | Sends COMPLETE only after REQUEST, BOUNDARY, COMMIT, and an empty terminal residual queue. |
