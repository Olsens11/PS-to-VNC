#ifndef PS2VNC_MANAGEMENT_TAIL_TO_SERVICES_H
#define PS2VNC_MANAGEMENT_TAIL_TO_SERVICES_H

/*
 * M3Z management-tail -> services/calibration-core interface.
 *
 * Declarations are derived from the qualified post-M3X
 * GCC aux-info.
 */

extern int ps2vnc_display_profile_valid(
    const ps2vnc_display_profile_t *profile);

extern int ps2vnc_management_get(
    const char *path,
    unsigned char *out_body,
    unsigned int out_capacity,
    unsigned int *out_len);

extern int ps2vnc_display_tx_take_line(
    char **cursor,
    char *end,
    const char *key,
    char **out_value);

extern int ps2vnc_display_tx_parse_profile(
    char **cursor,
    char *end,
    const char *prefix,
    ps2vnc_display_profile_t *profile);

#endif
