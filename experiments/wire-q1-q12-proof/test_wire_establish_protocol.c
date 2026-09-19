/*
 * File synopsis:
 * Host-tests exact proof-local Wire establishment payload representation.
 */

#include "wire_establish_protocol.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    uint8_t hello_bytes[PSTVNC_WIRE_PROOF_HELLO_BYTES];
    uint8_t accept_bytes[PSTVNC_WIRE_PROOF_ACCEPT_BYTES];
    uint8_t reject_bytes[PSTVNC_WIRE_PROOF_NOT_ACCEPTED_BYTES];

    pstvnc_wire_proof_hello_t hello = {
        PSTVNC_WIRE_PROOF_WIRE_VERSION,
        PSTVNC_WIRE_PROOF_PRODUCT_VERSION
    };
    pstvnc_wire_proof_hello_t decoded_hello;

    pstvnc_wire_proof_accept_t acceptance = { 0x12345678u };
    pstvnc_wire_proof_accept_t decoded_acceptance;

    pstvnc_wire_proof_not_accepted_t rejection = {
        PSTVNC_WIRE_PROOF_REJECT_PRODUCT_VERSION
    };
    pstvnc_wire_proof_not_accepted_t decoded_rejection;

    memset(&decoded_hello, 0, sizeof(decoded_hello));
    memset(&decoded_acceptance, 0, sizeof(decoded_acceptance));
    memset(&decoded_rejection, 0, sizeof(decoded_rejection));

    assert(pstvnc_wire_proof_hello_encode(hello_bytes, &hello));
    assert(pstvnc_wire_proof_hello_decode(
        &decoded_hello, hello_bytes, sizeof(hello_bytes)));
    assert(decoded_hello.wire_version == PSTVNC_WIRE_PROOF_WIRE_VERSION);
    assert(decoded_hello.product_version == PSTVNC_WIRE_PROOF_PRODUCT_VERSION);

    assert(!pstvnc_wire_proof_hello_decode(
        &decoded_hello, hello_bytes, sizeof(hello_bytes) - 1u));

    assert(pstvnc_wire_proof_accept_encode(accept_bytes, &acceptance));
    assert(pstvnc_wire_proof_accept_decode(
        &decoded_acceptance, accept_bytes, sizeof(accept_bytes)));
    assert(decoded_acceptance.session_id == acceptance.session_id);

    acceptance.session_id = 0u;
    assert(!pstvnc_wire_proof_accept_encode(accept_bytes, &acceptance));

    assert(pstvnc_wire_proof_not_accepted_encode(
        reject_bytes, &rejection));
    assert(pstvnc_wire_proof_not_accepted_decode(
        &decoded_rejection, reject_bytes, sizeof(reject_bytes)));
    assert(decoded_rejection.reason == rejection.reason);

    printf("WIRE_PROOF_ESTABLISH_CODEC_TEST=PASS\n");
    return 0;
}
