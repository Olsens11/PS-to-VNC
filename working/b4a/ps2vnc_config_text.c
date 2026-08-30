#include <string.h>

#include "ps2vnc_config_text.h"

char *ps2vnc_config_trim_left(char *text)
{
    while (*text == ' ' || *text == '\t')
        text++;

    return text;
}


void ps2vnc_config_trim_right(char *text)
{
    size_t len = strlen(text);

    while (len > 0 &&
           (text[len - 1] == ' ' ||
            text[len - 1] == '\t')) {

        text[len - 1] = '\0';
        len--;
    }
}
