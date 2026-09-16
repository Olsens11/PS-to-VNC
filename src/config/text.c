/*
 * File synopsis:
 * Implements bounded configuration-text whitespace, integer, and boolean
 * helpers. The integer parser preserves the PS2-libc overflow workaround; this
 * file is utility code and does not decode or own the production session
 * profile.
 *
 * Context: docs/development/source-topology.md and src/config/profile.h.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "text.h"

char *pstvnc_config_trim_left(char *text)
{
    while (*text == ' ' || *text == '\t')
        text++;

    return text;
}

void pstvnc_config_trim_right(char *text)
{
    size_t len = strlen(text);

    while (len > 0 &&
           (text[len - 1] == ' ' ||
            text[len - 1] == '\t')) {

        text[len - 1] = '\0';
        len--;
    }
}

int pstvnc_config_parse_int(
    const char *text,
    int allow_negative,
    int require_positive,
    int *out_value)
{
    const char *p;
    const char *digits;

    char *endptr;

    unsigned long magnitude = 0;
    unsigned long limit;

    int negative = 0;

    long value;

    if (text == NULL ||
        out_value == NULL ||
        text[0] == '\0')
        return 0;

    p = text;

    if (*p == '-') {

        if (!allow_negative)
            return 0;

        negative = 1;
        p++;

        if (*p == '\0')
            return 0;

    } else if (*p == '+') {

        /*
         * Keep one normalized representation.
         * A leading plus sign is unnecessary and rejected.
         */
        return 0;
    }

    digits = p;

    /*
     * Preflight the decimal magnitude ourselves.
     *
     * The PS2 libc strtol() observed in D4-A2 can saturate on overflow
     * without reliably reporting ERANGE.  Therefore errno alone is not a
     * sufficient range boundary on this target.
     *
     * Accumulate only while the next decimal digit provably fits inside
     * the destination int range.  This avoids arithmetic overflow in the
     * checker itself.
     */
    if (negative) {
        limit = (unsigned long)INT_MAX + 1UL;
    } else {
        limit = (unsigned long)INT_MAX;
    }

    while (*p != '\0') {
        unsigned int digit;

        if (*p < '0' || *p > '9')
            return 0;

        digit = (unsigned int)(*p - '0');

        if (magnitude >
                (limit - (unsigned long)digit) / 10UL) {

            return 0;
        }

        magnitude =
            magnitude * 10UL +
            (unsigned long)digit;

        p++;
    }

    if (p == digits)
        return 0;

    /*
     * Keep strtol() as the final conversion and complete-string check,
     * but do not depend on libc to be our only overflow detector.
     */
    errno = 0;
    endptr = NULL;

    value = strtol(
        text,
        &endptr,
        10
    );

    if (errno == ERANGE)
        return 0;

    if (endptr == NULL ||
        endptr == text ||
        *endptr != '\0')
        return 0;

    if (value < INT_MIN ||
        value > INT_MAX)
        return 0;

    if (require_positive &&
        value <= 0)
        return 0;

    *out_value = (int)value;

    return 1;
}

int pstvnc_config_parse_bool(
    const char *value,
    int *out_value)
{
    if (value == NULL ||
        out_value == NULL)
        return 0;

    if (strcmp(value, "true") == 0 ||
        strcmp(value, "1") == 0) {

        *out_value = 1;
        return 1;
    }

    if (strcmp(value, "false") == 0 ||
        strcmp(value, "0") == 0) {

        *out_value = 0;
        return 1;
    }

    return 0;
}
