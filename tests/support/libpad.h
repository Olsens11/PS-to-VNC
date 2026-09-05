/*
 * File synopsis:
 * Supplies only the native libpad structure declaration required while host
 * tests compile production headers that contain pstvnc_pad_t by value.
 *
 * This is not a libpad behavioral mock. Application coordinator tests stub the
 * input-runtime API itself and therefore never interpret this opaque storage.
 */

#ifndef PSTVNC_TEST_SUPPORT_LIBPAD_H
#define PSTVNC_TEST_SUPPORT_LIBPAD_H

struct padButtonStatus {
    unsigned char opaque[32];
};

#endif
