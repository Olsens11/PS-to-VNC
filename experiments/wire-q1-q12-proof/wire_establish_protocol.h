/*
 * File synopsis:
 * Defines the experiment-scoped Q1-Q12 Wire-establishment payloads used only
 * by the first real-hardware proof.
 *
 * The fixed 16-byte Wire header remains owned by src/transport/protocol.*.
 * This proof deliberately does not promote ACCEPT/NOT_ACCEPTED numeric frame
 * identities into the clean product ABI before hardware behavior is proven.
 *
 * Architecture authority:
 *   docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md Q4, Q9, Q10.
 */

#ifndef PSTVNC_WIRE_ESTABLISH_PROOF_PROTOCOL_H
#define PSTVNC_WIRE_ESTABLISH_PROOF_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

/*
 * HELLO itself uses the existing Wire HELLO frame kind on CONTROL channel 0.
 *
 * Kinds 12/13 are intentionally proof-local. Historical H1 used 8/9 for
 * MEDIA_END/SESSION_RESULT and mature Wire already preserves 10/11 for
 * MPEG RETIRE/START, so this proof does not collide with those identities.
 */
#define PSTVNC_WIRE_PROOF_FRAME_ACCEPT       12u
#define PSTVNC_WIRE_PROOF_FRAME_NOT_ACCEPTED 13u

#define PSTVNC_WIRE_PROOF_WIRE_VERSION       1u
#define PSTVNC_WIRE_PROOF_PRODUCT_VERSION    1u

#define PSTVNC_WIRE_PROOF_HELLO_BYTES         8u
#define PSTVNC_WIRE_PROOF_ACCEPT_BYTES        4u
#define PSTVNC_WIRE_PROOF_NOT_ACCEPTED_BYTES  4u

#define PSTVNC_WIRE_PROOF_REJECT_WIRE_VERSION    1u
#define PSTVNC_WIRE_PROOF_REJECT_PRODUCT_VERSION 2u
#define PSTVNC_WIRE_PROOF_REJECT_MALFORMED       3u

typedef struct pstvnc_wire_proof_hello {
    uint32_t wire_version;
    uint32_t product_version;
} pstvnc_wire_proof_hello_t;

typedef struct pstvnc_wire_proof_accept {
    uint32_t session_id;
} pstvnc_wire_proof_accept_t;

typedef struct pstvnc_wire_proof_not_accepted {
    uint32_t reason;
} pstvnc_wire_proof_not_accepted_t;

int pstvnc_wire_proof_hello_encode(
    uint8_t output[PSTVNC_WIRE_PROOF_HELLO_BYTES],
    const pstvnc_wire_proof_hello_t *hello);

int pstvnc_wire_proof_hello_decode(
    pstvnc_wire_proof_hello_t *hello,
    const uint8_t *input,
    size_t input_size);

int pstvnc_wire_proof_accept_encode(
    uint8_t output[PSTVNC_WIRE_PROOF_ACCEPT_BYTES],
    const pstvnc_wire_proof_accept_t *acceptance);

int pstvnc_wire_proof_accept_decode(
    pstvnc_wire_proof_accept_t *acceptance,
    const uint8_t *input,
    size_t input_size);

int pstvnc_wire_proof_not_accepted_encode(
    uint8_t output[PSTVNC_WIRE_PROOF_NOT_ACCEPTED_BYTES],
    const pstvnc_wire_proof_not_accepted_t *rejection);

int pstvnc_wire_proof_not_accepted_decode(
    pstvnc_wire_proof_not_accepted_t *rejection,
    const uint8_t *input,
    size_t input_size);

#endif
