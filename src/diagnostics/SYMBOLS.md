# Clean symbols — `src/diagnostics`

DIRECTORY=src/diagnostics
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns clean-generation diagnostic transport and exact runtime identity. `diagnostics.c/.h` and `identity.c/.h` participate in clean product source. Retained pre-refresh `debug.*` and `report.*` remain outside clean-generation scope because they do not carry the required `File synopsis:` marker.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| PSTVNC_DIAGNOSTICS_HOST_IPV4 | macro | src/diagnostics/diagnostics.c | diagnostics transport | file | Names the fixed private-link Pi address that receives observer datagrams. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| PSTVNC_DIAGNOSTICS_UDP_PORT | macro | src/diagnostics/diagnostics.c | diagnostics transport | file | Names the fixed UDP port used by the qualification observer stream. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| diagnostics_socket | variable | src/diagnostics/diagnostics.c | diagnostics transport | file | Owns the optional UDP socket or records that no diagnostics socket exists. | clean diagnostics ownership |
| diagnostics_address | variable | src/diagnostics/diagnostics.c | diagnostics transport | file | Stores the fixed sockaddr used for best-effort diagnostic datagrams. | clean diagnostics ownership |
| pstvnc_diagnostics_init | function | src/diagnostics/diagnostics.c | diagnostics transport | public | Opens and configures the optional UDP diagnostics transport to the Pi. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| pstvnc_diagnostics_is_ready | function | src/diagnostics/diagnostics.c | diagnostics transport | public | Reports whether the optional diagnostics socket is currently owned. | clean diagnostics ownership |
| pstvnc_diagnostics_send | function | src/diagnostics/diagnostics.c | diagnostics transport | public | Sends one caller-owned diagnostic payload as one UDP observer datagram. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| data | parameter | src/diagnostics/diagnostics.c | pstvnc_diagnostics_send | local | Points to caller-owned diagnostic bytes without transferring state ownership. | clean diagnostics |
| length | parameter | src/diagnostics/diagnostics.c | pstvnc_diagnostics_send | local | Gives the exact payload byte count for one diagnostic datagram. | clean diagnostics |
| pstvnc_diagnostics_shutdown | function | src/diagnostics/diagnostics.c | diagnostics transport | public | Releases optional diagnostics resources and restores the uninitialized baseline. | application coordinator cleanup |
| PSTVNC_DIAGNOSTICS_H | include guard | src/diagnostics/diagnostics.h | diagnostics transport interface | file | Prevents repeated inclusion of the diagnostics transport declarations. | clean source interface |
| pstvnc_diagnostics_init | function declaration | src/diagnostics/diagnostics.h | diagnostics transport interface | public | Declares optional diagnostics transport initialization. | clean diagnostics |
| pstvnc_diagnostics_is_ready | function declaration | src/diagnostics/diagnostics.h | diagnostics transport interface | public | Declares readiness inspection for the optional diagnostics transport. | clean diagnostics |
| pstvnc_diagnostics_send | function declaration | src/diagnostics/diagnostics.h | diagnostics transport interface | public | Declares best-effort transmission of one caller-owned diagnostic datagram. | clean diagnostics |
| data | parameter | src/diagnostics/diagnostics.h | pstvnc_diagnostics_send | public | Declares the diagnostic payload pointer accepted by the transport seam. | clean diagnostics |
| length | parameter | src/diagnostics/diagnostics.h | pstvnc_diagnostics_send | public | Declares the diagnostic payload byte count accepted by the transport seam. | clean diagnostics |
| pstvnc_diagnostics_shutdown | function declaration | src/diagnostics/diagnostics.h | diagnostics transport interface | public | Declares release of optional diagnostics transport resources. | application coordinator cleanup |
| pstvnc_identity_bounded_length | function | src/diagnostics/identity.c | runtime identity formatter | file | Returns a bounded visible string length or a sentinel when no terminator exists in range. | deterministic runtime identity |
| text | parameter | src/diagnostics/identity.c | pstvnc_identity_bounded_length | function | Supplies the candidate NUL-terminated text whose bounded length is being validated. | deterministic runtime identity |
| limit | parameter | src/diagnostics/identity.c | pstvnc_identity_bounded_length | function | Sets the maximum number of visible bytes accepted before reporting overflow. | deterministic runtime identity |
| length | variable | src/diagnostics/identity.c | pstvnc_identity_bounded_length | local | Tracks the current character offset while searching for the terminating NUL. | deterministic runtime identity |
| pstvnc_identity_append | function | src/diagnostics/identity.c | runtime identity formatter | file | Appends an exact byte span to the bounded identity message without printf-family formatting. | deterministic runtime identity |
| message | parameter | src/diagnostics/identity.c | pstvnc_identity_append | function | Points at the destination identity-message buffer receiving the appended bytes. | deterministic runtime identity |
| capacity | parameter | src/diagnostics/identity.c | pstvnc_identity_append | function | Gives the total writable byte capacity of the identity-message buffer. | deterministic runtime identity |
| length | parameter | src/diagnostics/identity.c | pstvnc_identity_append | function | Points at the caller-owned current message length that is advanced after a successful append. | deterministic runtime identity |
| source | parameter | src/diagnostics/identity.c | pstvnc_identity_append | function | Points at the source byte sequence to append to the identity message. | deterministic runtime identity |
| source_length | parameter | src/diagnostics/identity.c | pstvnc_identity_append | function | Gives the exact number of source bytes that must fit and be copied. | deterministic runtime identity |
| index | variable | src/diagnostics/identity.c | pstvnc_identity_append | local | Iterates through the source span during the exact bounded copy. | deterministic runtime identity |
| pstvnc_identity_is_hex | function | src/diagnostics/identity.c | runtime identity formatter | file | Recognizes one hexadecimal digit accepted by the 64-character identity digest contract. | deterministic runtime identity |
| value | parameter | src/diagnostics/identity.c | pstvnc_identity_is_hex | function | Supplies the candidate digest character being checked for hexadecimal form. | deterministic runtime identity |
| pstvnc_diagnostics_identity_format_message | function | src/diagnostics/identity.c | runtime identity formatter | public | Serializes the exact bounded TestKit-compatible runtime identity datagram and returns its payload length. | ISSUE7_MINIMAL_CORE: deterministic runtime identity |
| message | parameter | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | function | Points at caller-owned output storage for the serialized identity datagram. | deterministic runtime identity |
| capacity | parameter | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | function | Gives the total output-buffer capacity including the local terminating NUL. | deterministic runtime identity |
| test_id | parameter | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | function | Supplies the stamped test identifier that must fit the fixed 64-byte blob field. | deterministic runtime identity |
| digest | parameter | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | function | Supplies the exact 64-character hexadecimal DUT identity digest. | deterministic runtime identity |
| prefix | constant | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | local static | Holds the fixed protocol prefix emitted before the stamped test identifier. | deterministic runtime identity |
| separator | constant | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | local static | Holds the fixed delimiter emitted between the test identifier and digest. | deterministic runtime identity |
| test_id_length | variable | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | local | Stores the validated visible byte length of the stamped test identifier. | deterministic runtime identity |
| digest_length | variable | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | local | Stores the validated visible digest length, which must be exactly 64 bytes. | deterministic runtime identity |
| length | variable | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | local | Accumulates the serialized identity payload length as exact spans are appended. | deterministic runtime identity |
| index | variable | src/diagnostics/identity.c | pstvnc_diagnostics_identity_format_message | local | Iterates over every digest character during hexadecimal validation. | deterministic runtime identity |
| PS2VNC_IDENTITY_ZERO64 | macro | src/diagnostics/identity.c | stampable identity blob | file | Supplies the all-zero 64-character placeholder digest used before successor tooling stamps the ELF. | ISSUE7_DUT_MANIFEST |
| ps2vnc_identity_blob_v1 | structure | src/diagnostics/identity.c | stampable identity blob | file | Defines the packed fixed-size ELF blob layout consumed by deterministic identity-stamping tooling. | ISSUE7_DUT_MANIFEST |
| magic | field | src/diagnostics/identity.c | ps2vnc_identity_blob_v1 | structure | Stores the fixed blob magic used to locate and validate the stampable identity record. | ISSUE7_DUT_MANIFEST |
| test_id | field | src/diagnostics/identity.c | ps2vnc_identity_blob_v1 | structure | Stores up to 63 visible test-identifier bytes plus terminating NUL in the stampable blob. | ISSUE7_DUT_MANIFEST |
| digest | field | src/diagnostics/identity.c | ps2vnc_identity_blob_v1 | structure | Stores exactly 64 hexadecimal digest characters plus terminating NUL in the stampable blob. | ISSUE7_DUT_MANIFEST |
| ps2vnc_identity_blob | variable | src/diagnostics/identity.c | stampable identity blob | file static | Instantiates the retained stampable ELF identity record with explicit unstamped defaults. | ISSUE7_DUT_MANIFEST |
| ps2vnc_identity_sent | variable | src/diagnostics/identity.c | sendto identity wrapper | file static | Records whether the runtime identity datagram has already been emitted for this process. | deterministic runtime identity |
| __wrap_sendto | function | src/diagnostics/identity.c | sendto identity wrapper | linker seam | Emits one identity datagram before the first UDP diagnostics send to port 5999, then forwards the caller payload. | deterministic runtime identity |
| sock | parameter | src/diagnostics/identity.c | __wrap_sendto | function | Carries the caller socket descriptor through both identity and original datagram sends. | deterministic runtime identity |
| data | parameter | src/diagnostics/identity.c | __wrap_sendto | function | Points at the original caller datagram that must still be forwarded unchanged. | deterministic runtime identity |
| len | parameter | src/diagnostics/identity.c | __wrap_sendto | function | Gives the byte length of the original caller datagram. | deterministic runtime identity |
| flags | parameter | src/diagnostics/identity.c | __wrap_sendto | function | Preserves the caller's send flags for identity emission and original-datagram forwarding. | deterministic runtime identity |
| to | parameter | src/diagnostics/identity.c | __wrap_sendto | function | Points at the caller destination used to determine whether this is the diagnostics endpoint. | deterministic runtime identity |
| tolen | parameter | src/diagnostics/identity.c | __wrap_sendto | function | Gives the destination-address length used for safe IPv4 endpoint inspection and forwarding. | deterministic runtime identity |
| in | variable | src/diagnostics/identity.c | __wrap_sendto | local | Views a validated IPv4 destination as sockaddr_in so its UDP port can be inspected. | deterministic runtime identity |
| message | variable | src/diagnostics/identity.c | __wrap_sendto | local | Provides bounded stack storage for the deterministic runtime identity datagram. | deterministic runtime identity |
| message_length | variable | src/diagnostics/identity.c | __wrap_sendto | local | Stores the exact formatted identity payload length before the injected send. | deterministic runtime identity |
| rc | variable | src/diagnostics/identity.c | __wrap_sendto | local | Captures the injected identity send result before marking identity as emitted. | deterministic runtime identity |
| PSTVNC_DIAGNOSTICS_IDENTITY_H | include-guard macro | src/diagnostics/identity.h | runtime identity interface | header | Prevents duplicate inclusion of the clean runtime-identity interface declaration. | deterministic runtime identity |
| PSTVNC_DIAGNOSTICS_IDENTITY_UDP_PORT | macro | src/diagnostics/identity.h | runtime identity interface | public | Names UDP port 5999 as the qualifying diagnostics endpoint for identity injection. | deterministic runtime identity |
| PSTVNC_DIAGNOSTICS_IDENTITY_MAGIC | macro | src/diagnostics/identity.h | runtime identity interface | public | Names the fixed stampable ELF identity-blob magic string. | ISSUE7_DUT_MANIFEST |
| pstvnc_diagnostics_identity_format_message | function declaration | src/diagnostics/identity.h | runtime identity interface | public | Declares deterministic identity-message serialization independently of the PS2 send wrapper. | deterministic runtime identity |
| message | prototype parameter | src/diagnostics/identity.h | pstvnc_diagnostics_identity_format_message | prototype | Names the caller-owned output buffer in the public formatter contract. | deterministic runtime identity |
| capacity | prototype parameter | src/diagnostics/identity.h | pstvnc_diagnostics_identity_format_message | prototype | Names the output-buffer capacity in the public formatter contract. | deterministic runtime identity |
| test_id | prototype parameter | src/diagnostics/identity.h | pstvnc_diagnostics_identity_format_message | prototype | Names the stamped test identifier input in the public formatter contract. | deterministic runtime identity |
| digest | prototype parameter | src/diagnostics/identity.h | pstvnc_diagnostics_identity_format_message | prototype | Names the exact hexadecimal DUT digest input in the public formatter contract. | deterministic runtime identity |
