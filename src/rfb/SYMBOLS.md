# Clean symbols — `src/rfb`

DIRECTORY=src/rfb
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the RFB protocol mechanism family: native wire encoding, exact stream/session framing, synchronized session state, framebuffer update processing, and remote pointer/key serialization.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| bytes | parameter | src/rfb/rfb.c | write_be32 | local | Supplies bytes to write_be32. | Issue #39: keyboard, OSK and local UI foreground model |
| value | parameter | src/rfb/rfb.c | write_be32 | local | Supplies value to write_be32. | Issue #39: keyboard, OSK and local UI foreground model |
| write_be32 | function | src/rfb/rfb.c | RFB wire encoding | file | Writes one 32-bit value in big-endian RFB wire order. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_build_key_event | function | src/rfb/rfb.c | RFB wire encoding | file | Serializes one exact eight-byte RFB 3.x KeyEvent from explicit down/up state and a caller-resolved 32-bit X11 keysym without interpreting keyboard meaning. | Issue #39: keyboard, OSK and local UI foreground model |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_build_key_event | local | Supplies out to pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | parameter | src/rfb/rfb.c | pstvnc_rfb_build_key_event | local | Supplies down to pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/rfb/rfb.c | pstvnc_rfb_build_key_event | local | Supplies keysym to pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_RFB_KEY_EVENT_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the exact eight-byte size of an RFB 3.x client-to-server KeyEvent. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_build_key_event | function declaration | src/rfb/rfb.h | RFB wire interface | public | Serializes one exact eight-byte RFB 3.x KeyEvent from explicit down/up state and a caller-resolved 32-bit X11 keysym without interpreting keyboard meaning. | Issue #39: keyboard, OSK and local UI foreground model |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_key_event | prototype | Declares the out argument accepted by pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_key_event | prototype | Declares the down argument accepted by pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_key_event | prototype | Declares the keysym argument accepted by pstvnc_rfb_build_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_session_send_key_event | function | src/rfb/rfb_session.c | RFB session | file | Requires a READY main-thread-owned RFB session, serializes one native KeyEvent, and sends it exactly while leaving keysym, modifier, OSK, and controller meaning with the caller. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Supplies session to pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Supplies down to pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Supplies keysym to pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| message | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_send_key_event | local | Stores message while pstvnc_rfb_session_send_key_event runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_rfb_session_send_key_event | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Requires a READY main-thread-owned RFB session, serializes one native KeyEvent, and sends it exactly while leaving keysym, modifier, OSK, and controller meaning with the caller. | Issue #39: keyboard, OSK and local UI foreground model |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_key_event | prototype | Declares the session argument accepted by pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| down | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_key_event | prototype | Declares the down argument accepted by pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_key_event | prototype | Declares the keysym argument accepted by pstvnc_rfb_session_send_key_event. | Issue #39: keyboard, OSK and local UI foreground model |
| read_be16 | function | src/rfb/rfb.c | RFB wire format | file | Decodes one unsigned 16-bit big-endian RFB field without side effects. | RFB wire contract |
| bytes | parameter | src/rfb/rfb.c | read_be16 | local | Points to the two network-order bytes to decode. | RFB wire contract |
| read_be32 | function | src/rfb/rfb.c | RFB wire format | file | Decodes one unsigned 32-bit big-endian RFB field without side effects. | RFB wire contract |
| bytes | parameter | src/rfb/rfb.c | read_be32 | local | Points to the four network-order bytes to decode. | RFB wire contract |
| write_be16 | function | src/rfb/rfb.c | RFB wire format | file | Encodes one unsigned 16-bit value into network byte order. | RFB wire contract |
| bytes | parameter | src/rfb/rfb.c | write_be16 | local | Points to the two output bytes receiving network-order data. | RFB wire contract |
| value | parameter | src/rfb/rfb.c | write_be16 | local | Supplies the host-order value to encode. | RFB wire contract |
| pstvnc_rfb_parse_protocol_version | function | src/rfb/rfb.c | RFB wire format | public | Validates and decodes the fixed 12-byte RFB protocol banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| banner | parameter | src/rfb/rfb.c | pstvnc_rfb_parse_protocol_version | local | Supplies the exact server protocol banner bytes to validate. | RFB wire contract |
| major | parameter | src/rfb/rfb.c | pstvnc_rfb_parse_protocol_version | local | Receives the parsed protocol major version. | RFB wire contract |
| minor | parameter | src/rfb/rfb.c | pstvnc_rfb_parse_protocol_version | local | Receives the parsed protocol minor version. | RFB wire contract |
| i | variable | src/rfb/rfb.c | pstvnc_rfb_parse_protocol_version | local | Iterates over fixed decimal version digits in the wire banner. | RFB wire contract |
| parsed_major | variable | src/rfb/rfb.c | pstvnc_rfb_parse_protocol_version | local | Accumulates the validated three-digit server major version. | RFB wire contract |
| parsed_minor | variable | src/rfb/rfb.c | pstvnc_rfb_parse_protocol_version | local | Accumulates the validated three-digit server minor version. | RFB wire contract |
| pstvnc_rfb_build_client_version | function | src/rfb/rfb.c | RFB wire format | public | Writes the fixed RFB 3.8 client banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_build_client_version | local | Receives the exact fixed-length client protocol banner. | RFB wire contract |
| version | variable | src/rfb/rfb.c | pstvnc_rfb_build_client_version | local | Stores the immutable RFB 3.8 client banner bytes. | RFB wire contract |
| pstvnc_rfb_choose_security_none | function | src/rfb/rfb.c | RFB wire format | public | Selects SecurityType None only when the server explicitly offers it. | Issue #7 security baseline |
| types | parameter | src/rfb/rfb.c | pstvnc_rfb_choose_security_none | local | Supplies the server-advertised security type list. | RFB wire contract |
| count | parameter | src/rfb/rfb.c | pstvnc_rfb_choose_security_none | local | Gives the exact number of advertised security type bytes. | RFB wire contract |
| choice | parameter | src/rfb/rfb.c | pstvnc_rfb_choose_security_none | local | Receives the selected None security type on success. | Issue #7 security baseline |
| i | variable | src/rfb/rfb.c | pstvnc_rfb_choose_security_none | local | Iterates across advertised security types. | RFB wire contract |
| pstvnc_rfb_security_result_ok | function | src/rfb/rfb.c | RFB wire format | public | Accepts only a zero RFB SecurityResult status. | Issue #7 security baseline |
| result | parameter | src/rfb/rfb.c | pstvnc_rfb_security_result_ok | local | Supplies the four network-order SecurityResult bytes. | RFB wire contract |
| pstvnc_rfb_client_init_shared | function | src/rfb/rfb.c | RFB wire format | public | Returns the fixed shared-session ClientInit flag. | Issue #7 RFB baseline |
| pstvnc_rfb_parse_server_init | function | src/rfb/rfb.c | RFB wire format | public | Parses fixed ServerInit fields and rejects zero geometry. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| bytes | parameter | src/rfb/rfb.c | pstvnc_rfb_parse_server_init | local | Supplies the fixed-size ServerInit header bytes. | RFB wire contract |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_parse_server_init | local | Receives parsed server geometry and pixel-format metadata. | RFB wire contract |
| pstvnc_rfb_build_set_pixel_format_gs555 | function | src/rfb/rfb.c | RFB wire format | public | Builds the fixed little-endian B5:G5:R5 SetPixelFormat request. | fixed GS-friendly pixel format |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_build_set_pixel_format_gs555 | local | Receives the fixed SetPixelFormat wire message. | fixed GS-friendly pixel format |
| message | variable | src/rfb/rfb.c | pstvnc_rfb_build_set_pixel_format_gs555 | local | Stores the immutable GS-friendly SetPixelFormat message bytes. | fixed GS-friendly pixel format |
| pstvnc_rfb_build_set_encodings_raw | function | src/rfb/rfb.c | RFB wire format | public | Builds the one-entry SetEncodings request advertising Raw only. | Raw-only Issue #7 baseline |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_build_set_encodings_raw | local | Receives the fixed Raw-only SetEncodings wire message. | Raw-only Issue #7 baseline |
| message | variable | src/rfb/rfb.c | pstvnc_rfb_build_set_encodings_raw | local | Stores the immutable Raw-only SetEncodings message bytes. | Raw-only Issue #7 baseline |
| pstvnc_rfb_build_framebuffer_update_request | function | src/rfb/rfb.c | RFB wire format | public | Serializes one full-coordinate FramebufferUpdateRequest. | RFB wire contract |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Receives the fixed-size FramebufferUpdateRequest bytes. | RFB wire contract |
| incremental | parameter | src/rfb/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Selects full versus incremental server update semantics. | RFB update requests |
| x | parameter | src/rfb/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle left coordinate. | RFB update requests |
| y | parameter | src/rfb/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle top coordinate. | RFB update requests |
| width | parameter | src/rfb/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle width. | RFB update requests |
| height | parameter | src/rfb/rfb.c | pstvnc_rfb_build_framebuffer_update_request | local | Gives requested rectangle height. | RFB update requests |
| pstvnc_rfb_build_pointer_event | function | src/rfb/rfb.c | RFB wire format | public | Builds one exact six-byte RFB PointerEvent from an already-mapped native RFB button mask and logical coordinates. | Issue #38 pointer publication |
| out | parameter | src/rfb/rfb.c | pstvnc_rfb_build_pointer_event | local | Receives the exact six-byte PointerEvent wire message. | Issue #38 pointer publication |
| button_mask | parameter | src/rfb/rfb.c | pstvnc_rfb_build_pointer_event | local | Supplies native RFB button-mask vocabulary without semantic input interpretation. | Issue #38 pointer publication |
| x | parameter | src/rfb/rfb.c | pstvnc_rfb_build_pointer_event | local | Gives the logical remote pointer X coordinate encoded big-endian on the wire. | Issue #38 pointer publication |
| y | parameter | src/rfb/rfb.c | pstvnc_rfb_build_pointer_event | local | Gives the logical remote pointer Y coordinate encoded big-endian on the wire. | Issue #38 pointer publication |
| PSTVNC_RFB_H | include guard | src/rfb/rfb.h | RFB wire interface | file | Prevents repeated inclusion of pure RFB wire declarations. | clean source interface |
| PSTVNC_RFB_PROTOCOL_VERSION_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the exact byte length of an RFB version banner. | RFB wire contract |
| PSTVNC_RFB_SERVER_INIT_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the fixed byte length of the ServerInit header before desktop name. | RFB wire contract |
| PSTVNC_RFB_SET_PIXEL_FORMAT_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the exact SetPixelFormat client-message byte length. | RFB wire contract |
| PSTVNC_RFB_SET_ENCODINGS_RAW_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the exact one-entry Raw SetEncodings byte length. | Raw-only Issue #7 baseline |
| PSTVNC_RFB_FRAMEBUFFER_REQUEST_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the exact FramebufferUpdateRequest byte length. | RFB wire contract |
| PSTVNC_RFB_SECURITY_NONE | macro | src/rfb/rfb.h | RFB wire interface | public | Names RFB SecurityType None required by the minimal baseline. | Issue #7 security baseline |
| PSTVNC_RFB_ENCODING_RAW | macro | src/rfb/rfb.h | RFB wire interface | public | Names the Raw framebuffer encoding and no other encoding. | Raw-only Issue #7 baseline |
| pstvnc_rfb_server_init | structure | src/rfb/rfb.h | RFB wire interface | public | Defines parsed fixed ServerInit geometry and pixel-format metadata. | RFB wire contract |
| width | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised framebuffer width. | RFB ServerInit |
| height | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised framebuffer height. | RFB ServerInit |
| bits_per_pixel | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised bits per pixel. | RFB ServerInit |
| depth | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised color depth. | RFB ServerInit |
| big_endian | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised pixel byte-order flag. | RFB ServerInit |
| true_color | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised true-color flag. | RFB ServerInit |
| red_max | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised maximum red component value. | RFB ServerInit |
| green_max | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised maximum green component value. | RFB ServerInit |
| blue_max | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised maximum blue component value. | RFB ServerInit |
| red_shift | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised red component bit shift. | RFB ServerInit |
| green_shift | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised green component bit shift. | RFB ServerInit |
| blue_shift | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores server-advertised blue component bit shift. | RFB ServerInit |
| name_length | field | src/rfb/rfb.h | pstvnc_rfb_server_init | public | Stores the declared byte length of the following desktop name. | RFB ServerInit |
| pstvnc_rfb_server_init_t | type | src/rfb/rfb.h | RFB wire interface | public | Names parsed ServerInit metadata used by synchronized session state. | RFB wire contract |
| pstvnc_rfb_build_client_version | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares construction of the fixed client RFB protocol-version banner. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_build_framebuffer_update_request | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares construction of a framebuffer-update request for one explicit rectangle and incremental/full mode. | RFB wire contract |
| pstvnc_rfb_build_set_encodings_raw | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares construction of the SetEncodings request selecting Raw encoding only. | Raw-only Issue #7 baseline |
| pstvnc_rfb_build_set_pixel_format_gs555 | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares construction of the SetPixelFormat request for the clean GS555 framebuffer contract. | fixed GS-friendly pixel format |
| pstvnc_rfb_choose_security_none | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares selection of RFB None security from the server-advertised security-type list. | Issue #7 security baseline |
| pstvnc_rfb_client_init_shared | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares the ClientInit shared-desktop flag used by the clean RFB connection. | Issue #7 RFB baseline |
| pstvnc_rfb_parse_protocol_version | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares validation and parsing of one RFB protocol-version banner into its major and minor components. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_parse_server_init | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares decoding of the fixed ServerInit header into typed server framebuffer metadata. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| pstvnc_rfb_security_result_ok | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares interpretation of the four-byte RFB SecurityResult field as success or failure. | Issue #7 security baseline |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_client_version | prototype | Receives the exact fixed-length client protocol banner. | RFB wire contract |
| height | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle height. | RFB update requests |
| incremental | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Selects full versus incremental server update semantics. | RFB update requests |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Receives the fixed-size FramebufferUpdateRequest bytes. | RFB wire contract |
| width | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle width. | RFB update requests |
| x | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle left coordinate. | RFB update requests |
| y | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_framebuffer_update_request | prototype | Gives requested rectangle top coordinate. | RFB update requests |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_set_encodings_raw | prototype | Receives the fixed Raw-only SetEncodings wire message. | Raw-only Issue #7 baseline |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_set_pixel_format_gs555 | prototype | Receives the fixed SetPixelFormat wire message. | fixed GS-friendly pixel format |
| choice | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_choose_security_none | prototype | Receives the selected None security type on success. | Issue #7 security baseline |
| count | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_choose_security_none | prototype | Gives the exact number of advertised security type bytes. | RFB wire contract |
| types | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_choose_security_none | prototype | Supplies the server-advertised security type list. | RFB wire contract |
| banner | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_parse_protocol_version | prototype | Supplies the exact server protocol banner bytes to validate. | RFB wire contract |
| major | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_parse_protocol_version | prototype | Receives the parsed protocol major version. | RFB wire contract |
| minor | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_parse_protocol_version | prototype | Receives the parsed protocol minor version. | RFB wire contract |
| bytes | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_parse_server_init | prototype | Supplies the fixed-size ServerInit header bytes. | RFB wire contract |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_parse_server_init | prototype | Receives parsed server geometry and pixel-format metadata. | RFB wire contract |
| result | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_security_result_ok | prototype | Supplies the four network-order SecurityResult bytes. | RFB wire contract |
| PSTVNC_RFB_POINTER_EVENT_SIZE | macro | src/rfb/rfb.h | RFB wire interface | public | Names the exact six-byte RFB PointerEvent message size. | Issue #38 pointer publication |
| PSTVNC_RFB_POINTER_BUTTON_LEFT | macro | src/rfb/rfb.h | RFB wire interface | public | Names native RFB button 1, conventionally the left pointer button. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_BUTTON_MIDDLE | macro | src/rfb/rfb.h | RFB wire interface | public | Names native RFB button 2, conventionally the middle pointer button. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_BUTTON_RIGHT | macro | src/rfb/rfb.h | RFB wire interface | public | Names native RFB button 3, conventionally the right pointer button. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_UP | macro | src/rfb/rfb.h | RFB wire interface | public | Names the adopted native RFB button-4 mask used for one wheel-up press. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_DOWN | macro | src/rfb/rfb.h | RFB wire interface | public | Names the adopted native RFB button-5 mask used for one wheel-down press. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_LEFT | macro | src/rfb/rfb.h | RFB wire interface | public | Names the adopted native RFB button-6 mask used for one wheel-left press. | RFB PointerEvent vocabulary |
| PSTVNC_RFB_POINTER_WHEEL_RIGHT | macro | src/rfb/rfb.h | RFB wire interface | public | Names the adopted native RFB button-7 mask used for one wheel-right press. | RFB PointerEvent vocabulary |
| pstvnc_rfb_build_pointer_event | function declaration | src/rfb/rfb.h | RFB wire interface | public | Declares pure construction of one already-mapped native RFB PointerEvent. | Issue #38 pointer publication |
| out | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares destination storage for the exact six-byte PointerEvent. | Issue #38 pointer publication |
| button_mask | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares the already-mapped native RFB button mask accepted by the wire helper. | Issue #38 pointer publication |
| x | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares the logical remote pointer X coordinate. | Issue #38 pointer publication |
| y | prototype parameter | src/rfb/rfb.h | pstvnc_rfb_build_pointer_event | public | Declares the logical remote pointer Y coordinate. | Issue #38 pointer publication |
| PSTVNC_RFB_IO_H | include guard | src/rfb/rfb_io.h | RFB transport interface | file | Prevents repeated inclusion of exact I/O seam declarations. | clean source interface |
| pstvnc_rfb_io_read_exact | function declaration | src/rfb/rfb_io.h | RFB transport seam | platform | Requires the transport to deliver an exact protocol byte count or fail. | RFB framing across TCP |
| socket_fd | parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Identifies the connected socket from which exact bytes are required. | RFB framing across TCP |
| buffer | parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Receives exactly the requested number of protocol bytes. | RFB framing across TCP |
| count | parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_read_exact | platform | Gives the exact byte count whose transfer defines success. | RFB framing across TCP |
| pstvnc_rfb_io_write_exact | function declaration | src/rfb/rfb_io.h | RFB transport seam | platform | Requires the transport to send an exact protocol byte count or fail. | RFB framing across TCP |
| socket_fd | parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Identifies the connected socket to which exact bytes are written. | RFB framing across TCP |
| buffer | parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Supplies exactly the protocol bytes that must be written. | RFB framing across TCP |
| count | parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_write_exact | platform | Gives the exact byte count whose transfer defines success. | RFB framing across TCP |
| pstvnc_rfb_io_poll_receive | function declaration | src/rfb/rfb_io.h | RFB transport seam | platform | Declares a nonblocking receive-prefetch check that reports whether at least one server byte is ready without advancing protocol parsing. | Issue #38 responsive RFB scheduling |
| socket_fd | prototype parameter | src/rfb/rfb_io.h | pstvnc_rfb_io_poll_receive | platform | Declares the synchronized RFB socket whose receive readiness is checked without blocking. | RFB exact I/O seam |
| PSTVNC_RFB_INITIAL_COVERAGE_BYTES | macro | src/rfb/rfb_session.c | RFB session | file | Sizes the bitmap used to prove unique initial-frame pixel coverage. | initial authoritative Raw frame proof |
| initial_frame_coverage | variable | src/rfb/rfb_session.c | RFB session | file | Stores temporary per-pixel initial-frame coverage bits for the single owned session. | initial authoritative Raw frame proof |
| read_be32 | function | src/rfb/rfb_session.c | RFB session parser | file | Decodes one four-byte big-endian field used by streamed server messages. | RFB server-message framing |
| bytes | parameter | src/rfb/rfb_session.c | read_be32 | local | Supplies four network-order bytes to decode. | RFB server-message framing |
| read_be16 | function | src/rfb/rfb_session.c | RFB session parser | file | Decodes one two-byte big-endian field used by streamed server messages. | RFB server-message framing |
| bytes | parameter | src/rfb/rfb_session.c | read_be16 | local | Supplies two network-order bytes to decode. | RFB server-message framing |
| read_exact | function | src/rfb/rfb_session.c | RFB session parser | file | Converts the platform exact-read convention into a boolean session helper. | RFB framing across TCP |
| socket_fd | parameter | src/rfb/rfb_session.c | read_exact | local | Identifies the synchronized session socket to read. | RFB framing across TCP |
| buffer | parameter | src/rfb/rfb_session.c | read_exact | local | Receives the exact requested bytes. | RFB framing across TCP |
| count | parameter | src/rfb/rfb_session.c | read_exact | local | Gives the exact protocol byte count required. | RFB framing across TCP |
| write_exact | function | src/rfb/rfb_session.c | RFB session parser | file | Converts the platform exact-write convention into a boolean session helper. | RFB framing across TCP |
| socket_fd | parameter | src/rfb/rfb_session.c | write_exact | local | Identifies the synchronized session socket to write. | RFB framing across TCP |
| buffer | parameter | src/rfb/rfb_session.c | write_exact | local | Supplies the exact protocol bytes to write. | RFB framing across TCP |
| count | parameter | src/rfb/rfb_session.c | write_exact | local | Gives the exact protocol byte count required. | RFB framing across TCP |
| read_bounded_text | function | src/rfb/rfb_session.c | RFB session parser | file | Retains bounded diagnostic text while consuming the server field to its exact boundary. | RFB server-message framing |
| socket_fd | parameter | src/rfb/rfb_session.c | read_bounded_text | local | Identifies the socket carrying the declared text field. | RFB server-message framing |
| length | parameter | src/rfb/rfb_session.c | read_bounded_text | local | Gives the complete server-declared field length that must be consumed. | RFB server-message framing |
| out | parameter | src/rfb/rfb_session.c | read_bounded_text | local | Receives the bounded retained prefix plus terminator. | RFB server-message framing |
| discard | variable | src/rfb/rfb_session.c | read_bounded_text | local | Provides bounded scratch for consuming text beyond retained storage. | RFB server-message framing |
| take | variable | src/rfb/rfb_session.c | read_bounded_text | local | Stores the number of declared text bytes retained locally. | RFB server-message framing |
| remaining | variable | src/rfb/rfb_session.c | read_bounded_text | local | Tracks declared text bytes still requiring stream consumption. | RFB server-message framing |
| chunk | variable | src/rfb/rfb_session.c | read_bounded_text | local | Bounds each discard read to scratch capacity. | RFB server-message framing |
| discard_exact | function | src/rfb/rfb_session.c | RFB session parser | file | Consumes exactly a declared server payload length without retaining its bytes. | RFB server-message framing |
| socket_fd | parameter | src/rfb/rfb_session.c | discard_exact | local | Identifies the synchronized session socket to consume. | RFB server-message framing |
| count | parameter | src/rfb/rfb_session.c | discard_exact | local | Tracks the exact number of bytes still requiring discard. | RFB server-message framing |
| discard | variable | src/rfb/rfb_session.c | discard_exact | local | Provides fixed scratch for bounded exact payload consumption. | RFB server-message framing |
| chunk | variable | src/rfb/rfb_session.c | discard_exact | local | Bounds each discard operation to scratch capacity. | RFB server-message framing |
| fail | function | src/rfb/rfb_session.c | RFB session failure policy | file | Publishes terminal session failure and its precise classification. | fail-closed session semantics |
| session | parameter | src/rfb/rfb_session.c | fail | local | Supplies the session whose terminal state is published. | fail-closed session semantics |
| error | parameter | src/rfb/rfb_session.c | fail | local | Supplies the precise terminal error classification. | fail-closed session semantics |
| fail_frame | function | src/rfb/rfb_session.c | RFB session failure policy | file | Revokes framebuffer authority before publishing a terminal decode/session failure. | authoritative framebuffer failure semantics |
| session | parameter | src/rfb/rfb_session.c | fail_frame | local | Supplies the session whose failure state is published. | fail-closed session semantics |
| framebuffer | parameter | src/rfb/rfb_session.c | fail_frame | local | Supplies framebuffer authority that must be revoked after possible partial mutation. | authoritative framebuffer failure semantics |
| error | parameter | src/rfb/rfb_session.c | fail_frame | local | Supplies the precise terminal error classification. | fail-closed session semantics |
| framebuffer_matches_session | function | src/rfb/rfb_session.c | RFB session parser | file | Verifies socket ownership and exact framebuffer/server geometry agreement. | RFB session authority boundary |
| session | parameter | src/rfb/rfb_session.c | framebuffer_matches_session | local | Supplies synchronized session state to compare against framebuffer state. | RFB session authority boundary |
| framebuffer | parameter | src/rfb/rfb_session.c | framebuffer_matches_session | local | Supplies framebuffer geometry and initialization state for agreement checks. | RFB session authority boundary |
| region_fits | function | src/rfb/rfb_session.c | RFB session parser | file | Validates server rectangle geometry using widened arithmetic. | RFB rectangle bounds |
| x | parameter | src/rfb/rfb_session.c | region_fits | local | Gives server rectangle left coordinate. | RFB rectangle bounds |
| y | parameter | src/rfb/rfb_session.c | region_fits | local | Gives server rectangle top coordinate. | RFB rectangle bounds |
| width | parameter | src/rfb/rfb_session.c | region_fits | local | Gives server rectangle width. | RFB rectangle bounds |
| height | parameter | src/rfb/rfb_session.c | region_fits | local | Gives server rectangle height. | RFB rectangle bounds |
| framebuffer_width | parameter | src/rfb/rfb_session.c | region_fits | local | Gives authoritative logical framebuffer width. | RFB rectangle bounds |
| framebuffer_height | parameter | src/rfb/rfb_session.c | region_fits | local | Gives authoritative logical framebuffer height. | RFB rectangle bounds |
| mark_initial_frame_coverage | function | src/rfb/rfb_session.c | initial frame proof | file | Rejects duplicate pixels and counts unique authority for one initial Raw rectangle. | initial authoritative Raw frame proof |
| framebuffer | parameter | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Supplies logical row stride used to map rectangles to pixel identities. | initial authoritative Raw frame proof |
| x | parameter | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle left coordinate. | initial authoritative Raw frame proof |
| y | parameter | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle top coordinate. | initial authoritative Raw frame proof |
| width | parameter | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle width. | initial authoritative Raw frame proof |
| height | parameter | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Gives initial rectangle height. | initial authoritative Raw frame proof |
| covered_pixels | parameter | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Receives the cumulative count of uniquely covered logical pixels. | initial authoritative Raw frame proof |
| row | variable | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Iterates over logical rows covered by the rectangle. | initial authoritative Raw frame proof |
| column | variable | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Iterates over logical columns covered by the rectangle. | initial authoritative Raw frame proof |
| pixel_index | variable | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Maps one logical pixel coordinate to its unique linear identity. | initial authoritative Raw frame proof |
| byte_index | variable | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Locates the coverage byte containing the current pixel bit. | initial authoritative Raw frame proof |
| bit | variable | src/rfb/rfb_session.c | mark_initial_frame_coverage | local | Selects the coverage bit representing the current logical pixel. | initial authoritative Raw frame proof |
| read_raw_row | function | src/rfb/rfb_session.c | Raw decoder | file | Reads and converts exactly one little-endian Raw pixel row into bounded scratch. | Raw-only Issue #7 baseline |
| session | parameter | src/rfb/rfb_session.c | read_raw_row | local | Supplies synchronized socket state and row scratch storage. | Raw decoder |
| width | parameter | src/rfb/rfb_session.c | read_raw_row | local | Gives the number of 16-bit Raw pixels expected in the row. | Raw decoder |
| bytes | variable | src/rfb/rfb_session.c | read_raw_row | local | Aliases row scratch as bytes for exact network receive. | Raw decoder |
| column | variable | src/rfb/rfb_session.c | read_raw_row | local | Iterates across received Raw row pixels during endian conversion. | Raw decoder |
| byte_count | variable | src/rfb/rfb_session.c | read_raw_row | local | Stores the exact two-bytes-per-pixel row payload length. | Raw decoder |
| receive_framebuffer_update | function | src/rfb/rfb_session.c | RFB session parser | file | Consumes asynchronous server messages until one complete Raw framebuffer update is resolved. | shared Raw server-message parser |
| session | parameter | src/rfb/rfb_session.c | receive_framebuffer_update | local | Supplies synchronized session state and transport ownership. | shared Raw server-message parser |
| framebuffer | parameter | src/rfb/rfb_session.c | receive_framebuffer_update | local | Supplies mutable authoritative framebuffer state. | shared Raw server-message parser |
| require_full | parameter | src/rfb/rfb_session.c | receive_framebuffer_update | local | Selects strict initial full-frame proof versus ordinary incremental semantics. | initial authoritative Raw frame proof |
| message_type | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Holds the next server-to-client message discriminator. | RFB server-message framing |
| cut_header | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Holds the fixed remainder of a ServerCutText header. | RFB server-message framing |
| text_length | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores declared ServerCutText payload length for exact discard. | RFB server-message framing |
| color_header | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Holds the fixed remainder of a SetColorMapEntries header. | RFB server-message framing |
| color_count | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the declared color-map entry count. | RFB server-message framing |
| payload_length | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the exact color-map payload byte length to consume. | RFB server-message framing |
| update_header | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Holds the fixed remainder of a FramebufferUpdate header. | RFB server-message framing |
| rectangle_count | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the number of rectangles declared by one framebuffer update. | Raw framebuffer update |
| rectangle_index | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Iterates through declared framebuffer rectangles. | Raw framebuffer update |
| covered_pixels | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Counts unique initial-frame pixels proven by the coverage bitmap. | initial authoritative Raw frame proof |
| total_pixel_bytes | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Counts decoded Raw payload bytes for independent full-frame size proof. | initial authoritative Raw frame proof |
| framebuffer_pixels | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the exact logical framebuffer pixel count under test. | initial authoritative Raw frame proof |
| required_pixel_bytes | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the exact Raw byte total required for the complete logical desktop. | initial authoritative Raw frame proof |
| coverage_bytes | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the active initial-coverage bitmap length for current geometry. | initial authoritative Raw frame proof |
| rectangle_header | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Holds one fixed-size RFB rectangle header. | Raw framebuffer update |
| x | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle left coordinate. | Raw framebuffer update |
| y | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle top coordinate. | Raw framebuffer update |
| width | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle width. | Raw framebuffer update |
| height | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle height. | Raw framebuffer update |
| encoding | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores decoded rectangle encoding and must equal Raw. | Raw-only Issue #7 baseline |
| row | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Iterates through Raw rectangle rows written into framebuffer storage. | Raw framebuffer update |
| pstvnc_rfb_session_init | function | src/rfb/rfb_session.c | RFB session | public | Establishes the new disconnected baseline for one synchronized session. | CLEAN_ARCHITECTURE: RFB client/session |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_init | local | Supplies the session object to reset to NEW state. | RFB session lifecycle |
| pstvnc_rfb_session_start | function | src/rfb/rfb_session.c | RFB session | public | Performs exact RFB 3.8/None handshake, validates geometry, configures Raw, and requests the first full desktop. | ISSUE7_MINIMAL_CORE: RFB connection and wire contract |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Supplies session storage that becomes owner of synchronized protocol state. | RFB session lifecycle |
| socket_fd | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Supplies the already-connected VNC socket descriptor. | RFB session lifecycle |
| expected_width | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Gives the only acceptable server framebuffer width for this milestone. | fixed 704x462 baseline |
| expected_height | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Gives the only acceptable server framebuffer height for this milestone. | fixed 704x462 baseline |
| banner | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the exact server and client protocol version banner. | RFB handshake |
| security_count | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Stores the server-advertised number of security types. | RFB handshake |
| security_types | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the complete one-byte security-type list allowed by RFB 3.8. | RFB handshake |
| security_choice | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Stores SecurityType None selected from the advertised list. | RFB handshake |
| security_result | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the exact four-byte server SecurityResult. | RFB handshake |
| shared_flag | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Stores the fixed shared ClientInit flag sent to the server. | RFB handshake |
| server_init_bytes | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the fixed ServerInit header before desktop-name consumption. | RFB handshake |
| message | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Provides bounded client-message scratch sized for the largest startup message. | RFB handshake |
| reason_length_bytes | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Buffers the four-byte rejection-reason length when no security types are offered. | RFB handshake failure |
| reason_length | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_start | local | Stores decoded rejection-reason byte length for bounded exact consumption. | RFB handshake failure |
| pstvnc_rfb_session_request_update | function | src/rfb/rfb_session.c | RFB session | public | Sends one full-desktop update request only from a synchronized READY session. | Issue #7 live loop |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_request_update | local | Supplies the READY session and validated server geometry. | Issue #7 live loop |
| incremental | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_request_update | local | Selects ordinary incremental service versus a nonincremental request. | Issue #7 live loop |
| message | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_request_update | local | Buffers the exact serialized FramebufferUpdateRequest. | Issue #7 live loop |
| pstvnc_rfb_session_receive_initial_frame | function | src/rfb/rfb_session.c | RFB session | public | Receives and proves the requested full initial Raw desktop before publishing READY. | initial authoritative Raw frame proof |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_receive_initial_frame | local | Supplies the session awaiting its first complete framebuffer. | initial authoritative Raw frame proof |
| framebuffer | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_receive_initial_frame | local | Supplies framebuffer storage whose authority remains revoked until proof succeeds. | initial authoritative Raw frame proof |
| pstvnc_rfb_session_receive_update | function | src/rfb/rfb_session.c | RFB session | public | Receives one ordinary update only when session and framebuffer authority are already valid. | Issue #7 live loop |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_receive_update | local | Supplies the synchronized READY session. | Issue #7 live loop |
| framebuffer | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_receive_update | local | Supplies the currently authoritative framebuffer to update or invalidate on failure. | authoritative framebuffer semantics |
| pstvnc_rfb_session_send_pointer_event | function | src/rfb/rfb_session.c | RFB session | public | Serializes and sends one already-mapped PointerEvent only through a valid READY session and in-bounds negotiated geometry. | Issue #38 main-thread RFB pointer publication |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Supplies the synchronized READY RFB session that exclusively owns socket transmission. | Issue #38 main-thread RFB pointer publication |
| button_mask | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Supplies an already-mapped native RFB button mask; semantic input mapping remains outside the session. | Issue #38 main-thread RFB pointer publication |
| x | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Gives the pointer X coordinate validated against negotiated server width before transmission. | Issue #38 main-thread RFB pointer publication |
| y | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Gives the pointer Y coordinate validated against negotiated server height before transmission. | Issue #38 main-thread RFB pointer publication |
| message | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_send_pointer_event | local | Buffers one exact serialized PointerEvent before the session's exact write. | Issue #38 main-thread RFB pointer publication |
| pstvnc_rfb_receive_update_result | enum | src/rfb/rfb_session.c | RFB session receive parser | file | Defines internal complete, idle, and failed outcomes for one framebuffer-update parser service attempt. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_RECEIVE_UPDATE_FAILED | enum value | src/rfb/rfb_session.c | pstvnc_rfb_receive_update_result | file | Marks an internal receive/parser failure that has already applied fail-closed session semantics. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_RECEIVE_UPDATE_COMPLETE | enum value | src/rfb/rfb_session.c | pstvnc_rfb_receive_update_result | file | Marks completion of one whole server FramebufferUpdate message. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_RECEIVE_UPDATE_IDLE | enum value | src/rfb/rfb_session.c | pstvnc_rfb_receive_update_result | file | Marks a benign yield reached only before the next server-message byte is consumed. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_receive_update_result_t | type | src/rfb/rfb_session.c | RFB session receive parser | file | Names the internal tri-state result used by blocking and responsive receive wrappers. | Issue #38 responsive RFB scheduling |
| allow_idle | parameter | src/rfb/rfb_session.c | receive_framebuffer_update | local | Selects whether the parser may return a benign idle result at a complete server-message boundary. | Issue #38 responsive RFB scheduling |
| receive_ready | variable | src/rfb/rfb_session.c | receive_framebuffer_update | local | Stores the transport readiness result checked before consuming the next server-message byte. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_session_try_receive_update | function | src/rfb/rfb_session.c | RFB session | public | Services live server input without blocking while idle, but never yields after beginning a server message. | Issue #38 responsive RFB scheduling |
| session | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_try_receive_update | local | Supplies the synchronized READY session whose server input is serviced. | Issue #38 responsive RFB scheduling |
| framebuffer | parameter | src/rfb/rfb_session.c | pstvnc_rfb_session_try_receive_update | local | Supplies valid authoritative framebuffer storage for any completed update. | Issue #38 responsive RFB scheduling |
| result | variable | src/rfb/rfb_session.c | pstvnc_rfb_session_try_receive_update | local | Holds the internal parser outcome translated into the public receive-service result. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_H | include guard | src/rfb/rfb_session.h | RFB session interface | file | Prevents repeated inclusion of RFB session state and lifecycle declarations. | clean source interface |
| PSTVNC_RFB_SESSION_TEXT_MAX | macro | src/rfb/rfb_session.h | RFB session interface | public | Bounds retained desktop and server-rejection diagnostic text. | bounded RFB session storage |
| PSTVNC_RFB_SESSION_MAX_ROW_PIXELS | macro | src/rfb/rfb_session.h | RFB session interface | public | Bounds the reusable Raw decode row scratch capacity. | bounded RFB session storage |
| PSTVNC_RFB_SESSION_MAX_FRAME_PIXELS | macro | src/rfb/rfb_session.h | RFB session interface | public | Bounds the fixed Issue #7 initial-frame coverage proof to 704x462 pixels. | fixed 704x462 baseline |
| pstvnc_rfb_session_state | enum | src/rfb/rfb_session.h | RFB session interface | public | Defines the only lifecycle states of the synchronized clean session. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_NEW | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_state | public | Identifies an initialized session with no completed startup handshake. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_AWAITING_FULL_FRAME | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_state | public | Identifies a session whose handshake succeeded but initial authority is unproven. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_READY | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_state | public | Identifies a synchronized session with an authoritative initial framebuffer. | RFB session lifecycle |
| PSTVNC_RFB_SESSION_FAILED | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_state | public | Identifies terminal session failure requiring coordinator convergence. | fail-closed session semantics |
| pstvnc_rfb_session_state_t | type | src/rfb/rfb_session.h | RFB session interface | public | Names the synchronized session lifecycle-state enum. | RFB session lifecycle |
| pstvnc_rfb_session_error | enum | src/rfb/rfb_session.h | RFB session interface | public | Defines precise fail-closed classifications for startup and framebuffer service. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_NONE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Indicates no currently published session error. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_IO | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies failure to transfer an exact required protocol byte count. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_PROTOCOL_VERSION | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies malformed or unsupported server protocol version negotiation. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SERVER_REJECTED | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an RFB 3.8 server rejection before security selection. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SECURITY_NONE_UNAVAILABLE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies absence of the required None security type. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SECURITY_RESULT | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies a nonzero SecurityResult after selecting None. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_SERVER_INIT | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies invalid fixed ServerInit fields. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_GEOMETRY | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies server geometry that differs from the fixed expected desktop. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_SERVER_MESSAGE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies a server message type not supported by the synchronized baseline parser. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_EMPTY_UPDATE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an empty update where the initial full desktop is mandatory. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_UNSUPPORTED_ENCODING | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies any framebuffer rectangle encoding other than Raw. | Raw-only Issue #7 baseline |
| PSTVNC_RFB_SESSION_ERROR_RECTANGLE_BOUNDS | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an out-of-bounds or unwritable framebuffer rectangle. | failure classification |
| PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_SIZE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies an initial Raw byte total that differs from the exact desktop size. | initial authoritative Raw frame proof |
| PSTVNC_RFB_SESSION_ERROR_FULL_FRAME_COVERAGE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies duplicate, missing, excessive, or otherwise unproven initial pixel coverage. | initial authoritative Raw frame proof |
| PSTVNC_RFB_SESSION_ERROR_ROW_WIDTH | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_error | public | Classifies framebuffer width that exceeds bounded Raw row scratch. | bounded RFB session storage |
| pstvnc_rfb_session_error_t | type | src/rfb/rfb_session.h | RFB session interface | public | Names the precise synchronized-session error enum. | failure classification |
| pstvnc_rfb_session | structure | src/rfb/rfb_session.h | RFB session interface | public | Defines owned handshake state, errors, parsed server data, and bounded scratch. | CLEAN_ARCHITECTURE: RFB client/session |
| socket_fd | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores the connected socket descriptor owned by the synchronized session. | RFB session ownership |
| state | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores current synchronized lifecycle state. | RFB session lifecycle |
| error | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores the current precise failure classification. | failure classification |
| server_major | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores parsed server protocol major version. | RFB handshake |
| server_minor | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores parsed server protocol minor version. | RFB handshake |
| server_init | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores parsed server geometry and pixel-format metadata. | RFB handshake |
| desktop_name | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores a bounded retained server desktop name for diagnostics. | bounded RFB session storage |
| server_rejection | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores a bounded retained RFB server rejection reason. | bounded RFB session storage |
| row_scratch | field | src/rfb/rfb_session.h | pstvnc_rfb_session | public | Stores one bounded Raw row used during exact stream decoding. | Raw decoder |
| pstvnc_rfb_session_t | type | src/rfb/rfb_session.h | RFB session interface | public | Names the complete synchronized RFB session value. | CLEAN_ARCHITECTURE: RFB client/session |
| pstvnc_rfb_session_init | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares reset to the NEW disconnected session baseline. | RFB session lifecycle |
| pstvnc_rfb_session_start | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares fixed RFB startup and initial full-frame request. | RFB session lifecycle |
| pstvnc_rfb_session_request_update | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares synchronized full-desktop update requests. | Issue #7 live loop |
| pstvnc_rfb_session_receive_initial_frame | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares strict initial-frame receive and authority publication. | initial authoritative Raw frame proof |
| pstvnc_rfb_session_receive_update | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares ordinary update receive against an already authoritative framebuffer. | Issue #7 live loop |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_init | prototype | Supplies the session object to reset to NEW state. | RFB session lifecycle |
| framebuffer | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_initial_frame | prototype | Supplies framebuffer storage whose authority remains revoked until proof succeeds. | initial authoritative Raw frame proof |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_initial_frame | prototype | Supplies the session awaiting its first complete framebuffer. | initial authoritative Raw frame proof |
| framebuffer | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_update | prototype | Supplies the currently authoritative framebuffer to update or invalidate on failure. | authoritative framebuffer semantics |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_update | prototype | Supplies the synchronized READY session. | Issue #7 live loop |
| incremental | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_request_update | prototype | Selects ordinary incremental service versus a nonincremental request. | Issue #7 live loop |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_request_update | prototype | Supplies the READY session and validated server geometry. | Issue #7 live loop |
| expected_height | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_start | prototype | Gives the only acceptable server framebuffer height for this milestone. | fixed 704x462 baseline |
| expected_width | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_start | prototype | Gives the only acceptable server framebuffer width for this milestone. | fixed 704x462 baseline |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_start | prototype | Supplies session storage that becomes owner of synchronized protocol state. | RFB session lifecycle |
| socket_fd | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_start | prototype | Supplies the already-connected VNC socket descriptor. | RFB session lifecycle |
| pstvnc_rfb_session_send_pointer_event | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares READY-session transmission of one already-mapped native RFB PointerEvent. | Issue #38 main-thread RFB pointer publication |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the synchronized READY session that owns PointerEvent transmission. | Issue #38 main-thread RFB pointer publication |
| button_mask | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the already-mapped native RFB button mask supplied by application routing. | Issue #38 main-thread RFB pointer publication |
| x | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the logical remote pointer X coordinate to validate and publish. | Issue #38 main-thread RFB pointer publication |
| y | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_send_pointer_event | public | Declares the logical remote pointer Y coordinate to validate and publish. | Issue #38 main-thread RFB pointer publication |
| pstvnc_rfb_session_receive_result | enum | src/rfb/rfb_session.h | RFB session interface | public | Defines failed, idle, and completed-update outcomes for responsive live receive service. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_RECEIVE_FAILED | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_result | public | Reports that responsive receive service failed and ordinary session failure handling applies. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_RECEIVE_IDLE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_result | public | Reports that no next server-message byte is currently available and the READY session remains valid. | Issue #38 responsive RFB scheduling |
| PSTVNC_RFB_SESSION_RECEIVE_UPDATE | enum value | src/rfb/rfb_session.h | pstvnc_rfb_session_receive_result | public | Reports successful consumption of one complete server FramebufferUpdate. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_session_receive_result_t | type | src/rfb/rfb_session.h | RFB session interface | public | Names the public tri-state outcome of one responsive receive-service attempt. | Issue #38 responsive RFB scheduling |
| pstvnc_rfb_session_try_receive_update | function declaration | src/rfb/rfb_session.h | RFB session interface | public | Declares nonblocking-at-idle live receive service with server-message-boundary yield semantics. | Issue #38 responsive RFB scheduling |
| session | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_try_receive_update | public | Declares the READY synchronized RFB session to service. | Issue #38 responsive RFB scheduling |
| framebuffer | prototype parameter | src/rfb/rfb_session.h | pstvnc_rfb_session_try_receive_update | public | Declares authoritative framebuffer storage for a completed server update. | Issue #38 responsive RFB scheduling |
