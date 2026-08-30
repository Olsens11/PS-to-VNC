#ifndef PSTVNC_TEST_GSKIT_H
#define PSTVNC_TEST_GSKIT_H

/*
 * Host-test-only PS2 video constant seam.
 *
 * video/mode.c stores these identifiers as opaque platform values.
 * Host tests exercise catalog/model semantics, not GS timing values.
 */

#define GS_NONINTERLACED 0
#define GS_INTERLACED 1
#define GS_FRAME 1

#define GS_MODE_DTV_1080I 100
#define GS_MODE_DTV_480P 101
#define GS_MODE_DTV_576P 102
#define GS_MODE_DTV_720P 103
#define GS_MODE_NTSC 104
#define GS_MODE_PAL 105
#define GS_MODE_VGA_1024_60 106
#define GS_MODE_VGA_1024_70 107
#define GS_MODE_VGA_1024_75 108
#define GS_MODE_VGA_1024_85 109
#define GS_MODE_VGA_1280_60 110
#define GS_MODE_VGA_1280_75 111
#define GS_MODE_VGA_640_60 112
#define GS_MODE_VGA_640_72 113
#define GS_MODE_VGA_640_75 114
#define GS_MODE_VGA_640_85 115
#define GS_MODE_VGA_800_56 116
#define GS_MODE_VGA_800_60 117
#define GS_MODE_VGA_800_72 118
#define GS_MODE_VGA_800_75 119
#define GS_MODE_VGA_800_85 120

#endif /* PSTVNC_TEST_GSKIT_H */
