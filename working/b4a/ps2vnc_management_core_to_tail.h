#ifndef PS2VNC_MANAGEMENT_CORE_TO_TAIL_H
#define PS2VNC_MANAGEMENT_CORE_TO_TAIL_H

/*
 * M3Z management-core -> management-tail interface.
 *
 * Function declarations are derived from the qualified post-M3X
 * GCC aux-info. Macro definitions are copied exactly from the
 * qualified management/config parent.
 */

#define PS2VNC_CONFIG_MAX_BYTES  16384
#define PS2VNC_CONFIG_MAX_LINE     512

extern int ps2vnc_http_find_header_end(
    const unsigned char *data,
    unsigned int len);

extern int ps2vnc_http_status_is_200(
    const unsigned char *header,
    unsigned int header_len);

extern const char *ps2vnc_backend_transaction_name(
    ps2vnc_backend_t backend);

extern ps2vnc_backend_t ps2vnc_backend_from_transaction_name(
    const char *name);

#endif
