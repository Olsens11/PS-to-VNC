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
| `pstvnc_transport_physical_stream_t` | struct type | `src/transport/physical_stream.h` | transport | internal | Owns the adopted physical socket, send lock, and next outbound PSTV sequence. |
| `pstvnc_transport_physical_stream_adopt` | function | `src/transport/physical_stream.c` | transport | internal | Accepts ownership of one physical socket only after the send mutex is created. |
| `pstvnc_transport_physical_stream_send_frame` | function | `src/transport/physical_stream.c` | transport | internal | Serializes one complete PSTV header/payload transaction and advances sequence only after complete send. |
| `pstvnc_transport_physical_stream_release` | function | `src/transport/physical_stream.c` | transport | internal | Releases the adopted socket and send mutex after higher-level quiescence is proven. |
| `pstvnc_transport_physical_stream_send_exact` | function | `src/transport/physical_stream.c` | transport | file-local | Repeats socket send until the requested bytes are committed or a send fails. |
