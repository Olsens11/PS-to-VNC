#ifndef PS2VNC_CONFIG_TEXT_H
#define PS2VNC_CONFIG_TEXT_H

char *ps2vnc_config_trim_left(char *text);
void ps2vnc_config_trim_right(char *text);


int ps2vnc_config_parse_int(
    const char *text,
    int allow_negative,
    int require_positive,
    int *out_value);

int ps2vnc_config_parse_bool(
    const char *value,
    int *out_value);

#endif
