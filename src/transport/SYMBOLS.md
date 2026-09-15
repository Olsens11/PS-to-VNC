# `src/transport` Symbol Dictionary

Context: `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md`.

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
