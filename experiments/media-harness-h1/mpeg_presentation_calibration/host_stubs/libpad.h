/*
 * File synopsis:
 * Host-only PS2SDK libpad compatibility shim for the MPEG calibration binding
 * contract test.
 *
 * The production binding still includes the real src/input/input_runtime.h and
 * therefore compiles against the real PS2SDK <libpad.h> in pinned PS2 builds.
 * Ordinary host CI has no PS2SDK headers, and this test needs only enough of
 * libpad's type surface for src/input/pad.h to define pstvnc_pad_t. No libpad
 * function or controller semantic is emulated here.
 */
#ifndef PSTVNC_MPEG_CALIBRATION_HOST_STUB_LIBPAD_H
#define PSTVNC_MPEG_CALIBRATION_HOST_STUB_LIBPAD_H

struct padButtonStatus {
    unsigned char bytes[32];
};

#endif /* PSTVNC_MPEG_CALIBRATION_HOST_STUB_LIBPAD_H */
