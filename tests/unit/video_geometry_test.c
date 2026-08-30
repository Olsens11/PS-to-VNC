#include <stdio.h>

#include "video/geometry.h"
#include "video/mode.h"

static int failures = 0;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(                                                    \
                stderr,                                                 \
                "FAIL:%s:%d: %s\n",                                     \
                __FILE__,                                               \
                __LINE__,                                               \
                #expr                                                   \
            );                                                          \
            failures++;                                                 \
        }                                                               \
    } while (0)

static pstvnc_video_mode_t synthetic_mode(
    unsigned int width,
    unsigned int height
)
{
    pstvnc_video_mode_t mode = {0};

    mode.name = "synthetic";
    mode.raster_width = width;
    mode.raster_height = height;

    return mode;
}

static int historical_safe_area_valid(
    const pstvnc_video_mode_t *mode,
    int width,
    int height,
    int offset_x,
    int offset_y
)
{
    int output_width;
    int output_height;
    int remaining_x;
    int remaining_y;
    int centered_x;
    int centered_y;
    int max_positive_x;
    int max_positive_y;

    if (mode == NULL)
        return 0;

    output_width = (int)mode->raster_width;
    output_height = (int)mode->raster_height;

    if (width <= 0 || height <= 0)
        return 0;

    if (width > output_width ||
        height > output_height)
        return 0;

    remaining_x = output_width - width;
    remaining_y = output_height - height;

    centered_x = remaining_x / 2;
    centered_y = remaining_y / 2;

    max_positive_x = remaining_x - centered_x;
    max_positive_y = remaining_y - centered_y;

    if (offset_x < -centered_x ||
        offset_x > max_positive_x)
        return 0;

    if (offset_y < -centered_y ||
        offset_y > max_positive_y)
        return 0;

    return 1;
}

static void check_validity_contract(void)
{
    pstvnc_video_mode_t mode =
        synthetic_mode(100u, 80u);

    int width;
    int height;
    int x;
    int y;

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            NULL, 80, 60, 0, 0
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 0, 60, 0, 0
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 0, 0, 0
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 101, 60, 0, 0
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 81, 0, 0
        ) == 0
    );

    /*
     * Compare a representative grid directly against the exact historical
     * acceptance arithmetic.
     */
    for (width = 79; width <= 100; width += 7) {
        for (height = 59; height <= 80; height += 7) {
            for (x = -15; x <= 15; x++) {
                for (y = -15; y <= 15; y++) {
                    CHECK(
                        pstvnc_video_geometry_safe_area_valid(
                            &mode,
                            width,
                            height,
                            x,
                            y
                        )
                        ==
                        historical_safe_area_valid(
                            &mode,
                            width,
                            height,
                            x,
                            y
                        )
                    );
                }
            }
        }
    }
}

static void check_offset_boundaries(void)
{
    pstvnc_video_mode_t mode =
        synthetic_mode(100u, 80u);

    /*
     * Even remainder:
     * 100 - 80 = 20, so X legal range is -10..+10.
     */
    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, -10, 0
        ) == 1
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, 10, 0
        ) == 1
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, -11, 0
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, 11, 0
        ) == 0
    );

    /*
     * Odd remainder:
     * 100 - 79 = 21.
     * Historical integer centering yields -10..+11.
     */
    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 79, 60, -10, 0
        ) == 1
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 79, 60, 11, 0
        ) == 1
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 79, 60, -11, 0
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 79, 60, 12, 0
        ) == 0
    );

    /*
     * Equivalent Y boundary:
     * 80 - 60 = 20, so -10..+10.
     */
    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, 0, -10
        ) == 1
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, 0, 10
        ) == 1
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, 0, -11
        ) == 0
    );

    CHECK(
        pstvnc_video_geometry_safe_area_valid(
            &mode, 80, 60, 0, 11
        ) == 0
    );
}

static void check_safe_area_rect(void)
{
    pstvnc_video_mode_t mode =
        synthetic_mode(100u, 80u);

    pstvnc_video_rect_t rect;

    CHECK(
        pstvnc_video_geometry_safe_area_rect(
            &mode,
            80,
            60,
            0,
            0,
            &rect
        ) == 0
    );

    CHECK(rect.x == 10u);
    CHECK(rect.y == 10u);
    CHECK(rect.width == 80u);
    CHECK(rect.height == 60u);

    CHECK(
        pstvnc_video_geometry_safe_area_rect(
            &mode,
            80,
            60,
            -10,
            -10,
            &rect
        ) == 0
    );

    CHECK(rect.x == 0u);
    CHECK(rect.y == 0u);

    CHECK(
        pstvnc_video_geometry_safe_area_rect(
            &mode,
            100,
            80,
            0,
            0,
            &rect
        ) == 0
    );

    CHECK(rect.x == 0u);
    CHECK(rect.y == 0u);
    CHECK(rect.width == 100u);
    CHECK(rect.height == 80u);

    CHECK(
        pstvnc_video_geometry_safe_area_rect(
            &mode,
            80,
            60,
            0,
            0,
            NULL
        ) == -1
    );
}

static void check_presented_rect(void)
{
    pstvnc_video_mode_t mode =
        synthetic_mode(100u, 80u);

    pstvnc_video_rect_t safe = {
        10u,
        10u,
        80u,
        60u,
    };

    pstvnc_video_rect_t presented;

    /*
     * Safe area is smaller than capacity: preserve it exactly.
     */
    CHECK(
        pstvnc_video_geometry_presented_rect(
            &mode,
            &safe,
            100u,
            100u,
            &presented
        ) == 0
    );

    CHECK(presented.x == 10u);
    CHECK(presented.y == 10u);
    CHECK(presented.width == 80u);
    CHECK(presented.height == 60u);

    /*
     * Both dimensions constrained by desktop capacity.
     */
    CHECK(
        pstvnc_video_geometry_presented_rect(
            &mode,
            &safe,
            40u,
            20u,
            &presented
        ) == 0
    );

    CHECK(presented.x == 30u);
    CHECK(presented.y == 30u);
    CHECK(presented.width == 40u);
    CHECK(presented.height == 20u);

    /*
     * Width-only capacity clamp.
     */
    CHECK(
        pstvnc_video_geometry_presented_rect(
            &mode,
            &safe,
            40u,
            100u,
            &presented
        ) == 0
    );

    CHECK(presented.x == 30u);
    CHECK(presented.y == 10u);
    CHECK(presented.width == 40u);
    CHECK(presented.height == 60u);

    /*
     * Height-only capacity clamp.
     */
    CHECK(
        pstvnc_video_geometry_presented_rect(
            &mode,
            &safe,
            100u,
            20u,
            &presented
        ) == 0
    );

    CHECK(presented.x == 10u);
    CHECK(presented.y == 30u);
    CHECK(presented.width == 80u);
    CHECK(presented.height == 20u);

    CHECK(
        pstvnc_video_geometry_presented_rect(
            &mode,
            &safe,
            0u,
            20u,
            &presented
        ) == -1
    );
}

static void check_catalog_modes(void)
{
    static const char *names[] = {
        "480p",
        "720p",
        "1080i",
    };

    unsigned int i;

    for (i = 0u;
         i < sizeof(names) / sizeof(names[0]);
         i++) {

        const pstvnc_video_mode_t *mode =
            pstvnc_video_mode_by_name(names[i]);

        pstvnc_video_rect_t rect;

        CHECK(mode != NULL);

        if (mode == NULL)
            continue;

        CHECK(
            pstvnc_video_geometry_safe_area_valid(
                mode,
                (int)mode->raster_width,
                (int)mode->raster_height,
                0,
                0
            ) == 1
        );

        CHECK(
            pstvnc_video_geometry_safe_area_rect(
                mode,
                (int)mode->raster_width,
                (int)mode->raster_height,
                0,
                0,
                &rect
            ) == 0
        );

        CHECK(rect.x == 0u);
        CHECK(rect.y == 0u);
        CHECK(rect.width == mode->raster_width);
        CHECK(rect.height == mode->raster_height);
    }
}

int main(void)
{
    check_validity_contract();
    check_offset_boundaries();
    check_safe_area_rect();
    check_presented_rect();
    check_catalog_modes();

    if (failures != 0) {
        fprintf(
            stderr,
            "video_geometry_test: FAIL (%d)\n",
            failures
        );

        return 1;
    }

    puts("video_geometry_test: PASS");
    return 0;
}
