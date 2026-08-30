#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "config/text.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                    __FILE__, __LINE__, #expr);                         \
            failures++;                                                 \
        }                                                               \
    } while (0)

static void test_trim_left(void)
{
    char empty[] = "";
    char plain[] = "alpha";
    char spaced[] = " \t  alpha";

    CHECK(strcmp(pstvnc_config_trim_left(empty), "") == 0);
    CHECK(strcmp(pstvnc_config_trim_left(plain), "alpha") == 0);
    CHECK(strcmp(pstvnc_config_trim_left(spaced), "alpha") == 0);
}

static void test_trim_right(void)
{
    char empty[] = "";
    char plain[] = "alpha";
    char spaced[] = "alpha \t  ";
    char all_space[] = " \t ";

    pstvnc_config_trim_right(empty);
    pstvnc_config_trim_right(plain);
    pstvnc_config_trim_right(spaced);
    pstvnc_config_trim_right(all_space);

    CHECK(strcmp(empty, "") == 0);
    CHECK(strcmp(plain, "alpha") == 0);
    CHECK(strcmp(spaced, "alpha") == 0);
    CHECK(strcmp(all_space, "") == 0);
}

static void test_parse_int(void)
{
    int value = 777;
    char int_max_text[64];
    char int_min_text[64];
    char above_max_text[64];
    char below_min_text[64];

    snprintf(int_max_text, sizeof(int_max_text), "%d", INT_MAX);
    snprintf(int_min_text, sizeof(int_min_text), "%d", INT_MIN);

    snprintf(
        above_max_text,
        sizeof(above_max_text),
        "%lld",
        (long long)INT_MAX + 1LL);

    snprintf(
        below_min_text,
        sizeof(below_min_text),
        "%lld",
        (long long)INT_MIN - 1LL);

    CHECK(!pstvnc_config_parse_int(NULL, 1, 0, &value));
    CHECK(!pstvnc_config_parse_int("", 1, 0, &value));
    CHECK(!pstvnc_config_parse_int("1", 1, 0, NULL));

    CHECK(pstvnc_config_parse_int("0", 0, 0, &value));
    CHECK(value == 0);

    CHECK(pstvnc_config_parse_int("42", 0, 0, &value));
    CHECK(value == 42);

    CHECK(!pstvnc_config_parse_int("-1", 0, 0, &value));

    CHECK(pstvnc_config_parse_int("-1", 1, 0, &value));
    CHECK(value == -1);

    CHECK(pstvnc_config_parse_int("-0", 1, 0, &value));
    CHECK(value == 0);

    CHECK(!pstvnc_config_parse_int("+1", 1, 0, &value));
    CHECK(!pstvnc_config_parse_int(" 1", 1, 0, &value));
    CHECK(!pstvnc_config_parse_int("1 ", 1, 0, &value));
    CHECK(!pstvnc_config_parse_int("1x", 1, 0, &value));
    CHECK(!pstvnc_config_parse_int("-", 1, 0, &value));

    CHECK(!pstvnc_config_parse_int("0", 1, 1, &value));
    CHECK(!pstvnc_config_parse_int("-1", 1, 1, &value));

    CHECK(pstvnc_config_parse_int("1", 1, 1, &value));
    CHECK(value == 1);

    CHECK(pstvnc_config_parse_int(
        int_max_text, 0, 0, &value));
    CHECK(value == INT_MAX);

    CHECK(pstvnc_config_parse_int(
        int_min_text, 1, 0, &value));
    CHECK(value == INT_MIN);

    CHECK(!pstvnc_config_parse_int(
        above_max_text, 1, 0, &value));

    CHECK(!pstvnc_config_parse_int(
        below_min_text, 1, 0, &value));
}

static void test_parse_bool(void)
{
    int value = 777;

    CHECK(!pstvnc_config_parse_bool(NULL, &value));
    CHECK(!pstvnc_config_parse_bool("true", NULL));

    CHECK(pstvnc_config_parse_bool("true", &value));
    CHECK(value == 1);

    CHECK(pstvnc_config_parse_bool("1", &value));
    CHECK(value == 1);

    CHECK(pstvnc_config_parse_bool("false", &value));
    CHECK(value == 0);

    CHECK(pstvnc_config_parse_bool("0", &value));
    CHECK(value == 0);

    value = 777;

    CHECK(!pstvnc_config_parse_bool("TRUE", &value));
    CHECK(value == 777);

    CHECK(!pstvnc_config_parse_bool("", &value));
    CHECK(value == 777);

    CHECK(!pstvnc_config_parse_bool("yes", &value));
    CHECK(value == 777);
}

int main(void)
{
    test_trim_left();
    test_trim_right();
    test_parse_int();
    test_parse_bool();

    if (failures != 0) {
        fprintf(
            stderr,
            "config_text_test: %d failure(s)\n",
            failures);

        return 1;
    }

    puts("config_text_test: PASS");
    return 0;
}
