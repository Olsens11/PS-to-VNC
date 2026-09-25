#ifndef PSTVNC_INPUT_RUNTIME_PRODUCT_ACTION_TEST_LIBPAD_H
#define PSTVNC_INPUT_RUNTIME_PRODUCT_ACTION_TEST_LIBPAD_H

#define PAD_SELECT    0x0001u
#define PAD_L3        0x0002u
#define PAD_R3        0x0004u
#define PAD_START     0x0008u
#define PAD_UP        0x0010u
#define PAD_RIGHT     0x0020u
#define PAD_DOWN      0x0040u
#define PAD_LEFT      0x0080u
#define PAD_L2        0x0100u
#define PAD_R2        0x0200u
#define PAD_L1        0x0400u
#define PAD_R1        0x0800u
#define PAD_TRIANGLE  0x1000u
#define PAD_CIRCLE    0x2000u
#define PAD_CROSS     0x4000u
#define PAD_SQUARE    0x8000u

#define PAD_TYPE_DUALSHOCK 7u

struct padButtonStatus {
    unsigned char mode;
    unsigned char ljoy_h;
    unsigned char ljoy_v;
};

#endif
