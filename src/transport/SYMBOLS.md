# Clean symbols — `src/transport`

DIRECTORY=src/transport
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This directory owns the one physical PSTV stream, sole receive owner, transport
framing/sequence state, logical RFB storage/flow control, explicit quiescence,
and the single process-organized cross-component bridge authorized by the ledge
all-guns architecture overlay. Coverage remains `IN_PROGRESS` until the strict
definition-level inventory and generated portal are proven; this file must not
be read as V004 closure.

Context: `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` and
`docs/ledge/LEDGE_AUDIT_A001_TRANSPORT_RFB.md`.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| pstvnc_transport_result_t | enum type | src/transport/transport.h | transport public values | public | Names the common result vocabulary returned by cross-component Transport operations. | session/logical-stream bridge |
| PSTVNC_TRANSPORT_OK | enum value | src/transport/transport.h | pstvnc_transport_result_t | public | Reports successful completion of the requested operation. | bridge result |
| PSTVNC_TRANSPORT_WOULD_BLOCK | enum value | src/transport/transport.h | pstvnc_transport_result_t | public | Reports that no progress is currently available without asserting terminal failure. | bridge scheduling result |
| PSTVNC_TRANSPORT_CLOSED | enum value | src/transport/transport.h | pstvnc_transport_result_t | public | Reports proven terminal receiver/session state. | bridge terminal result |
| PSTVNC_TRANSPORT_INVALID | enum value | src/transport/transport.h | pstvnc_transport_result_t | public | Reports an argument or lifecycle-state contract violation. | bridge contract |
| PSTVNC_TRANSPORT_FAILED | enum value | src/transport/transport.h | pstvnc_transport_result_t | public | Reports a non-benign Transport mechanism failure. | bridge failure |
| pstvnc_transport_session_config_t | struct type | src/transport/transport.h | validated transport session value | public | Carries the already-validated H1-derived queue, credit, receiver-thread, and payload values needed by the A001 logical-RFB session without defaults. | application/config to Transport |
| pstvnc_transport_runtime_config_t | type alias | src/transport/runtime.h | transport runtime | internal | Transitional internal spelling that aliases the stable session config exactly and creates no second value/default authority. | runtime migration |
| pstvnc_transport_bridge_runtime | variable | src/transport/bridge.c | Transport bridge | file | Stores the one active product Transport runtime behind the cross-component bridge. | single-session bridge |
| pstvnc_transport_bridge_session_active | variable | src/transport/bridge.c | Transport bridge | file | Records whether the bridge currently owns an active session runtime. | session lifecycle |
| pstvnc_transport_bridge_terminal_result | function | src/transport/bridge.c | Transport bridge | file | Maps private runtime failed/completed state into the public result vocabulary without exposing runtime internals. | bridge error mapping |
| pstvnc_transport_session_open | function | src/transport/bridge.c | application lifecycle process | public bridge | Adopts the caller socket only after successful runtime initialization, clears caller ownership, starts the sole receiver, and never invents CONFIG defaults. | application -> Transport |
| pstvnc_transport_session_wait_receiver_done | function | src/transport/bridge.c | application lifecycle process | public bridge | Blocks on explicit receiver completion and proves only receiver termination, leaving product failure classification to the application. | shutdown lifecycle |
| pstvnc_transport_session_close | function | src/transport/bridge.c | application lifecycle process | public bridge | Refuses live-receiver reclamation and releases the private runtime only after receiver completion. | shutdown lifecycle |
| pstvnc_transport_rfb_read_exact | function | src/transport/bridge.c | logical RFB delivery process | public bridge | Reads exact protocol bytes from Transport-owned logical RFB state without exposing the physical socket. | RFB -> Transport logical stream |
| pstvnc_transport_rfb_poll_receive | function | src/transport/bridge.c | logical RFB delivery process | public bridge | Reports logical RFB readiness/idle/terminal state without independently receiving from the physical socket. | RFB -> Transport logical stream |
| pstvnc_transport_rfb_write_exact | function | src/transport/bridge.c | logical RFB delivery process | public bridge | Sends exact logical RFB bytes through Transport's fragmented serialized physical send path. | RFB -> Transport logical stream |
| pstvnc_transport_rfb_quiesce_requested | function | src/transport/bridge.c | RFB quiesce process | public bridge | Reports whether ordered Pi REQUEST has arrived while leaving safe-boundary choice outside Transport. | finite RFB shutdown |
| pstvnc_transport_rfb_send_quiesce_boundary | function | src/transport/bridge.c | RFB quiesce process | public bridge | Sends PS2 BOUNDARY only when the external application/RFB owner has selected the complete-message boundary. | finite RFB shutdown |
| pstvnc_transport_rfb_wait_quiesce_commit | function | src/transport/bridge.c | RFB quiesce process | public bridge | Waits through Transport activity synchronization for the ordered Pi COMMIT marker. | finite RFB shutdown |
| pstvnc_transport_rfb_snapshot_quiesce_residual | function | src/transport/bridge.c | RFB quiesce process | public bridge | Captures the post-COMMIT residual logical-RFB byte count for exact ownership checking. | finite RFB shutdown |
| pstvnc_transport_rfb_discard_quiesce_residual | function | src/transport/bridge.c | RFB quiesce process | public bridge | Discards only the exact residual snapshot without awarding parser-consumption credit. | finite RFB shutdown |
| pstvnc_transport_rfb_send_quiesce_complete | function | src/transport/bridge.c | RFB quiesce process | public bridge | Sends COMPLETE only after the Transport-owned ordered prerequisites and empty residual queue are proven. | finite RFB shutdown |
| pstvnc_transport_frame_kind_t | enum type | src/transport/protocol.h | PSTV framing | shared wire value | Names the stable PSTV v1 frame-kind vocabulary. | physical framing |
| pstvnc_transport_channel_t | enum type | src/transport/protocol.h | PSTV framing | shared wire value | Names stable logical channel identities carried by the shared physical PSTV stream. | logical-channel framing |
| pstvnc_transport_header_t | struct type | src/transport/protocol.h | PSTV framing | internal | Represents one decoded fixed PSTV header. | physical framing |
| pstvnc_transport_header_encode | function | src/transport/protocol.c | PSTV framing | internal | Validates and encodes one fixed PSTV v1 wire header. | physical framing |
| pstvnc_transport_header_decode | function | src/transport/protocol.c | PSTV framing | internal | Validates magic/version/payload ceiling and decodes one PSTV v1 wire header. | physical framing |
| pstvnc_transport_read_be32 | function | src/transport/protocol.c | PSTV framing | internal | Reads one big-endian 32-bit protocol value. | wire helper |
| pstvnc_transport_write_be32 | function | src/transport/protocol.c | PSTV framing | internal | Writes one big-endian 32-bit protocol value. | wire helper |
| pstvnc_transport_physical_stream_t | struct type | src/transport/physical_stream.h | physical PSTV stream | internal | Owns the adopted physical socket, send lock, and direction-local PSTV sequence state. | sole physical stream |
| pstvnc_transport_physical_stream_adopt | function | src/transport/physical_stream.c | physical PSTV stream | internal | Creates serialized-send synchronization before accepting ownership of one physical socket and initializes both sequence directions. | socket adoption |
| pstvnc_transport_physical_stream_send_frame | function | src/transport/physical_stream.c | physical PSTV stream | internal | Serializes one complete PSTV header/payload transaction and advances outbound sequence only after complete send. | serialized send |
| pstvnc_transport_physical_stream_receive_frame | function | src/transport/physical_stream.c | physical PSTV stream | internal | Sole physical receive primitive that reads, validates, and sequence-commits one complete inbound frame. | sole receive |
| pstvnc_transport_physical_stream_release | function | src/transport/physical_stream.c | physical PSTV stream | internal | Closes the adopted physical socket and releases send synchronization after higher-level lifetime rules permit reclamation. | physical lifecycle |
| pstvnc_transport_physical_stream_send_exact | function | src/transport/physical_stream.c | physical PSTV stream | file | Repeats socket send until all requested bytes reach the socket API or an error occurs. | physical send helper |
| pstvnc_transport_physical_stream_receive_exact | function | src/transport/physical_stream.c | physical PSTV stream | file | Repeats socket receive until all requested bytes are present or an error occurs. | physical receive helper |
| pstvnc_transport_rfb_channel_t | struct type | src/transport/rfb_channel.h | logical RFB storage | internal | Owns committed logical RFB bytes, circular-buffer accounting, and producer activity generation. | logical channel 1 |
| pstvnc_transport_rfb_channel_initialize | function | src/transport/rfb_channel.c | logical RFB storage | internal | Binds caller-owned session storage and initializes an empty logical RFB stream. | logical channel 1 |
| pstvnc_transport_rfb_channel_commit | function | src/transport/rfb_channel.c | logical RFB storage | internal | Atomically commits one complete received channel-1 DATA payload when capacity is available. | inbound RFB DATA |
| pstvnc_transport_rfb_channel_read_exact | function | src/transport/rfb_channel.c | logical RFB storage | internal | Consumes exactly the requested committed bytes or leaves channel state unchanged. | logical RFB consumption |
| pstvnc_transport_rfb_channel_read_available | function | src/transport/rfb_channel.c | logical RFB storage | internal | Consumes up to the requested committed bytes so parser credit can return incrementally. | logical RFB consumption |
| pstvnc_transport_rfb_channel_discard_residual | function | src/transport/rfb_channel.c | logical RFB storage | internal | Discards an exact terminal residual snapshot without classifying bytes as parser consumption/activity. | terminal RFB residual |
| pstvnc_transport_rfb_channel_available | function | src/transport/rfb_channel.c | logical RFB storage | internal | Reports currently committed logical RFB bytes. | logical RFB storage |
| pstvnc_transport_rfb_channel_activity_generation | function | src/transport/rfb_channel.c | logical RFB storage | internal | Reports the monotonically advancing producer generation used for activity evidence. | logical RFB activity |
| pstvnc_transport_runtime_t | struct type | src/transport/runtime.h | Transport session runtime | internal | Owns one physical stream, sole receiver thread, synchronized RFB resources, flow control, activity rendezvous, and receiver completion. | A001 runtime |
| pstvnc_transport_runtime_initialize | function | src/transport/runtime.c | Transport session runtime | internal | Validates supplied session values, allocates receiver-visible RFB resources/stack, and adopts the one physical socket as the final successful initialization step. | runtime lifecycle |
| pstvnc_transport_runtime_start_receiver | function | src/transport/runtime.c | Transport session runtime | internal | Sends initial RFB credit and starts the sole receiver after all receiver-visible resources exist. | runtime lifecycle |
| pstvnc_transport_runtime_rfb_activity_snapshot | function | src/transport/runtime.c | Transport session runtime | internal | Snapshots protected producer activity for race-free timerless consumer rendezvous. | activity rendezvous |
| pstvnc_transport_runtime_rfb_wait_activity | function | src/transport/runtime.c | Transport session runtime | internal | Arms and waits for one producer event only when no newer protected activity is already visible. | activity rendezvous |
| pstvnc_transport_runtime_rfb_read_exact | function | src/transport/runtime.c | Transport session runtime | internal | Completes an exact RFB read by incrementally consuming committed bytes, returning earned credit, and waiting only when empty. | parser consumption/credit |
| pstvnc_transport_runtime_rfb_poll_receive | function | src/transport/runtime.c | Transport session runtime | internal | Reports logical RFB availability or terminal receiver state without touching the physical receive path. | logical RFB scheduling |
| pstvnc_transport_runtime_rfb_write_exact | function | src/transport/runtime.c | Transport session runtime | internal | Fragments outbound RFB bytes at the configured DATA payload ceiling through the serialized physical send path. | outbound logical RFB |
| pstvnc_transport_runtime_wait_receiver_done | function | src/transport/runtime.c | Transport session runtime | internal | Blocks on the explicit receiver completion event instead of diagnostic counters or timer polling. | receiver lifetime |
| pstvnc_transport_runtime_release | function | src/transport/runtime.c | Transport session runtime | internal | Refuses receiver-visible reclamation while a started receiver is live, then retires Transport-owned resources. | receiver lifetime |
| pstvnc_transport_runtime_rfb_quiesce_requested | function | src/transport/quiesce.c | Transport quiesce state | internal | Reports whether ordered Pi REQUEST has been accepted. | finite RFB shutdown |
| pstvnc_transport_runtime_rfb_send_quiesce_boundary | function | src/transport/quiesce.c | Transport quiesce state | internal | Publishes/sends PS2 BOUNDARY after an external safe-boundary decision. | finite RFB shutdown |
| pstvnc_transport_runtime_rfb_wait_quiesce_commit | function | src/transport/quiesce.c | Transport quiesce state | internal | Waits for ordered Pi COMMIT through the producer activity rendezvous. | finite RFB shutdown |
| pstvnc_transport_runtime_rfb_snapshot_residual | function | src/transport/quiesce.c | Transport quiesce state | internal | Captures exact remaining logical RFB bytes after COMMIT. | terminal RFB residual |
| pstvnc_transport_runtime_rfb_discard_quiesce_residual | function | src/transport/quiesce.c | Transport quiesce state | internal | Discards only the exact snapshotted terminal residual without false parser credit. | terminal RFB residual |
| pstvnc_transport_runtime_rfb_send_quiesce_complete | function | src/transport/quiesce.c | Transport quiesce state | internal | Sends COMPLETE only after REQUEST, BOUNDARY, COMMIT, and an empty residual queue. | finite RFB shutdown |
