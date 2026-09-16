/*
 * File synopsis:
 * Declares small configuration-text normalization helpers retained as clean
 * configuration utilities. These helpers parse individual textual values only;
 * they do not own the production session profile or PSTV CONFIG wire format.
 *
 * Context: docs/development/source-topology.md and src/config/profile.h.
 */

#ifndef PSTVNC_CONFIG_TEXT_H
#define PSTVNC_CONFIG_TEXT_H

char *pstvnc_config_trim_left(char *text);
void pstvnc_config_trim_right(char *text);

int pstvnc_config_parse_int(
    const char *text,
    int allow_negative,
    int require_positive,
    int *out_value);

int pstvnc_config_parse_bool(
    const char *value,
    int *out_value);

#endif
