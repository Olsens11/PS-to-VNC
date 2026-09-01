#ifndef PSTVNC_DIAGNOSTICS_H
#define PSTVNC_DIAGNOSTICS_H

#include <stddef.h>

/*
 * Minimal optional diagnostics transport for the first clean milestone.
 * Product state remains owned by the application/subsystems; diagnostics only
 * transports caller-supplied records to the Pi UDP endpoint.
 */
int pstvnc_diagnostics_init(void);
int pstvnc_diagnostics_is_ready(void);
int pstvnc_diagnostics_send(const void *data, size_t length);
void pstvnc_diagnostics_shutdown(void);

#endif
