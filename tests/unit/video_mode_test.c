#include <stdio.h>
#include <string.h>

#include "video/mode.h"

static int failures;

#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            fprintf(stderr,                                             \
                    "video_mode_test:%d: CHECK failed: %s\n",           \
                    __LINE__, #expr);                                   \
            failures++;                                                 \
        }                                                               \
    } while (0)

static const char *const expected_mode_names[
    PSTVNC_VIDEO_MODE_COUNT
] = {
    "480i",
    "480p",
    "480p-hires",
    "576i",
    "576p",
    "720p",
    "1080i",
    "640x480-60",
    "800x600-60",
    "1024x768-60",
    "1280x1024-60",
    "640x480-72",
    "640x480-75",
    "640x480-85",
    "800x600-56",
    "800x600-72",
    "800x600-75",
    "800x600-85",
    "1024x768-70",
    "1024x768-75",
    "1024x768-85",
    "1280x1024-75"
};

static const pstvnc_video_mode_t *must_find(
    const char *name)
{
    const pstvnc_video_mode_t *mode =
        pstvnc_video_mode_by_name(name);

    CHECK(mode != NULL);
    return mode;
}

int main(void)
{
    unsigned int i;
    unsigned int j;

    CHECK(PSTVNC_VIDEO_MODE_COUNT == 22u);

    CHECK(
        pstvnc_video_mode_by_name(NULL)
        == NULL);

    CHECK(
        pstvnc_video_mode_by_name("")
        == NULL);

    CHECK(
        pstvnc_video_mode_by_name("does-not-exist")
        == NULL);

    CHECK(
        pstvnc_video_mode_supports_backend(
            NULL,
            PSTVNC_VIDEO_BACKEND_STANDARD)
        == 0);

    CHECK(
        pstvnc_video_mode_default_backend(NULL)
        == PSTVNC_VIDEO_BACKEND_NONE);

    for (i = 0; i < PSTVNC_VIDEO_MODE_COUNT; i++) {
        const pstvnc_video_mode_t *mode =
            pstvnc_video_modes[i];

        CHECK(mode != NULL);

        if (mode == NULL)
            continue;

        CHECK(
            strcmp(
                mode->name,
                expected_mode_names[i])
            == 0);

        CHECK(mode->name[0] != '\0');
        CHECK(mode->raster_width > 0);
        CHECK(mode->raster_height > 0);

        CHECK(
            mode->menu_group
            < PSTVNC_VIDEO_MODE_GROUP_COUNT);

        CHECK(
            mode->allowed_backends
            != PSTVNC_VIDEO_BACKEND_NONE);

        CHECK(
            pstvnc_video_mode_supports_backend(
                mode,
                PSTVNC_VIDEO_BACKEND_NONE)
            == 0);

        CHECK(
            pstvnc_video_mode_supports_backend(
                mode,
                mode->recommended_backend)
            != 0);

        CHECK(
            pstvnc_video_mode_default_backend(mode)
            == mode->recommended_backend);

        CHECK(
            pstvnc_video_mode_by_name(mode->name)
            == mode);

        if (
            pstvnc_video_mode_supports_backend(
                mode,
                PSTVNC_VIDEO_BACKEND_STANDARD)
        ) {
            CHECK(mode->standard_fb_width > 0);
            CHECK(mode->standard_fb_height > 0);
            CHECK(mode->standard_logical_width > 0);
            CHECK(mode->standard_logical_height > 0);
        }

        if (
            pstvnc_video_mode_supports_backend(
                mode,
                PSTVNC_VIDEO_BACKEND_HIRES)
        ) {
            CHECK(mode->hires_width > 0);
            CHECK(mode->hires_height > 0);
            CHECK(mode->hires_passes > 0);
        }

        for (
            j = i + 1;
            j < PSTVNC_VIDEO_MODE_COUNT;
            j++
        ) {
            CHECK(
                strcmp(
                    mode->name,
                    pstvnc_video_modes[j]->name)
                != 0);
        }
    }

    {
        const pstvnc_video_mode_t *mode =
            must_find("480i");

        if (mode != NULL) {
            CHECK(
                pstvnc_video_mode_supports_backend(
                    mode,
                    PSTVNC_VIDEO_BACKEND_STANDARD));

            CHECK(
                !pstvnc_video_mode_supports_backend(
                    mode,
                    PSTVNC_VIDEO_BACKEND_HIRES));
        }
    }

    {
        const pstvnc_video_mode_t *mode =
            must_find("480p-hires");

        if (mode != NULL) {
            CHECK(
                !pstvnc_video_mode_supports_backend(
                    mode,
                    PSTVNC_VIDEO_BACKEND_STANDARD));

            CHECK(
                pstvnc_video_mode_supports_backend(
                    mode,
                    PSTVNC_VIDEO_BACKEND_HIRES));
        }
    }

    {
        const pstvnc_video_mode_t *mode =
            must_find("720p");

        if (mode != NULL) {
            CHECK(mode->raster_width == 1280u);
            CHECK(mode->raster_height == 720u);
        }
    }

    {
        const pstvnc_video_mode_t *mode =
            must_find("1080i");

        if (mode != NULL) {
            CHECK(mode->raster_width == 1920u);
            CHECK(mode->raster_height == 1080u);
            CHECK(mode->hires_height == 540u);
        }
    }

    {
        pstvnc_video_mode_t invalid =
            *pstvnc_video_modes[0];

        invalid.recommended_backend =
            PSTVNC_VIDEO_BACKEND_HIRES;

        invalid.allowed_backends =
            PSTVNC_VIDEO_BACKEND_STANDARD;

        CHECK(
            pstvnc_video_mode_default_backend(
                &invalid)
            == PSTVNC_VIDEO_BACKEND_NONE);
    }

    if (failures != 0) {
        fprintf(
            stderr,
            "video_mode_test: FAIL (%d)\n",
            failures);

        return 1;
    }

    puts("video_mode_test: PASS");
    return 0;
}
