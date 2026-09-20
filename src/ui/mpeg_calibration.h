/*
 * File synopsis:
 * Defines the platform-neutral MPEG CALIBRATION value and foreground-state
 * owner used by A004 reconstruction.
 *
 * This module owns MPEG CALIBRATION base geometry, presentation-local matte
 * extents, edit/review/accept/cancel state, and release quarantine. It does not
 * own DESKTOP CALIBRATION, controller acquisition, RFB/Transport, MPEG decode,
 * Pi producer behavior, GS composition, media-clock arm, or MPEG visual
 * ownership.
 *
 * Context:
 *   docs/ledge/LEDGE_AUDIT_A004_PRESENTATION_CALIBRATION.md;
 *   docs/ledge/LEDGE_A004_CALIBRATION_SEPARATION_INVARIANT.md.
 */

#ifndef PSTVNC_MPEG_CALIBRATION_H
#define PSTVNC_MPEG_CALIBRATION_H

#include <stdint.h>

enum pstvnc_mpeg_calibration_action {
    PSTVNC_MPEG_CALIBRATION_UP = 1u << 0,
    PSTVNC_MPEG_CALIBRATION_RIGHT = 1u << 1,
    PSTVNC_MPEG_CALIBRATION_DOWN = 1u << 2,
    PSTVNC_MPEG_CALIBRATION_LEFT = 1u << 3,
    PSTVNC_MPEG_CALIBRATION_MOVE = 1u << 4,
    PSTVNC_MPEG_CALIBRATION_INNER_MATTE = 1u << 5,
    PSTVNC_MPEG_CALIBRATION_OUTER_MATTE = 1u << 6,
    PSTVNC_MPEG_CALIBRATION_RESET = 1u << 7,
    PSTVNC_MPEG_CALIBRATION_REVIEW_ACCEPT = 1u << 8,
    PSTVNC_MPEG_CALIBRATION_CANCEL = 1u << 9
};

#define PSTVNC_MPEG_CALIBRATION_ALL_ACTIONS ((1u << 10) - 1u)

typedef struct pstvnc_mpeg_calibration_rect {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} pstvnc_mpeg_calibration_rect_t;

typedef struct pstvnc_mpeg_calibration_region {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t inner_matte_x;
    int32_t inner_matte_y;
    int32_t outer_matte_x;
    int32_t outer_matte_y;
} pstvnc_mpeg_calibration_region_t;

typedef struct pstvnc_mpeg_calibration_geometry {
    pstvnc_mpeg_calibration_rect_t base;
    pstvnc_mpeg_calibration_rect_t inner_content;
    pstvnc_mpeg_calibration_rect_t suppression;
} pstvnc_mpeg_calibration_geometry_t;

typedef enum pstvnc_mpeg_calibration_screen {
    PSTVNC_MPEG_CALIBRATION_INACTIVE = 0,
    PSTVNC_MPEG_CALIBRATION_EDIT,
    PSTVNC_MPEG_CALIBRATION_REVIEW
} pstvnc_mpeg_calibration_screen_t;

typedef struct pstvnc_mpeg_calibration_input {
    uint32_t down;
    uint32_t pressed;
} pstvnc_mpeg_calibration_input_t;

typedef struct pstvnc_mpeg_calibration_effects {
    unsigned accepted : 1;
    unsigned cancelled : 1;
    unsigned release_quarantine_complete : 1;
    pstvnc_mpeg_calibration_region_t accepted_region;
} pstvnc_mpeg_calibration_effects_t;

typedef struct pstvnc_mpeg_calibration {
    int32_t canvas_width;
    int32_t canvas_height;
    pstvnc_mpeg_calibration_region_t defaults;
    pstvnc_mpeg_calibration_region_t current;
    pstvnc_mpeg_calibration_region_t committed;
    pstvnc_mpeg_calibration_screen_t screen;
    unsigned has_committed : 1;
    unsigned review_accept_armed : 1;
    unsigned release_quarantine_pending : 1;
} pstvnc_mpeg_calibration_t;

/*
 * Initialize one MPEG CALIBRATION owner for a caller-supplied local canvas.
 * The canvas is only a calibration/presentation bound; it is not Pi desktop
 * geometry authority. Both dimensions must be at least one 16-pixel MPEG
 * macroblock.
 */
int pstvnc_mpeg_calibration_init(
    pstvnc_mpeg_calibration_t *calibration,
    int32_t canvas_width,
    int32_t canvas_height);

/*
 * Enter MPEG CALIBRATION edit foreground explicitly.
 *
 * Entry gesture timing/chord policy belongs to a later input/orchestration
 * owner. A previous committed value, when present, becomes the new candidate;
 * otherwise the centered default candidate is used.
 */
int pstvnc_mpeg_calibration_begin(
    pstvnc_mpeg_calibration_t *calibration);

/*
 * Apply one already-normalized MPEG CALIBRATION semantic-input sample.
 *
 * REVIEW_ACCEPT first opens review. Review can accept only after that control
 * has been observed released and then freshly pressed again. This preserves the
 * historical held-X safety invariant without owning a physical controller.
 */
int pstvnc_mpeg_calibration_update(
    pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_input_t *input,
    pstvnc_mpeg_calibration_effects_t *effects);

/*
 * Report whether MPEG CALIBRATION still owns local foreground semantics.
 * Ownership includes the post-accept/cancel release quarantine.
 */
int pstvnc_mpeg_calibration_owns_foreground(
    const pstvnc_mpeg_calibration_t *calibration);

/*
 * Resolve the three distinct geometry meanings for a valid MPEG CALIBRATION
 * region: exact base rectangle, base content after the inner matte, and the
 * outer/suppression footprint clipped to the calibration canvas.
 */
int pstvnc_mpeg_calibration_resolve_geometry(
    const pstvnc_mpeg_calibration_t *calibration,
    const pstvnc_mpeg_calibration_region_t *region,
    pstvnc_mpeg_calibration_geometry_t *geometry);

#endif /* PSTVNC_MPEG_CALIBRATION_H */
