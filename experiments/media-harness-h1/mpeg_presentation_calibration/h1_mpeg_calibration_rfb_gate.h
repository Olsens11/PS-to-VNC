/*
 * File synopsis:
 * Defines the experiment-local RFB visual/request gate used to translate MPEG
 * calibration foreground ownership into safe-boundary RFB scheduling policy.
 *
 * This module owns no socket, parser, framebuffer, or renderer state. The H1 RFB
 * worker evaluates it only at the worker's already-existing complete-server-
 * message scheduling boundary.
 */

#ifndef PSTVNC_H1_MPEG_CALIBRATION_RFB_GATE_H
#define PSTVNC_H1_MPEG_CALIBRATION_RFB_GATE_H

typedef struct pstvnc_h1_mpeg_calibration_rfb_gate {
    unsigned frozen : 1;
    unsigned full_refresh_required : 1;
} pstvnc_h1_mpeg_calibration_rfb_gate_t;

void pstvnc_h1_mpeg_calibration_rfb_gate_init(
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate);

/*
 * Apply the foreground bridge's freeze fact at a safe RFB scheduling boundary.
 * A frozen->thawed transition records one required full refresh so the retained
 * authoritative desktop can be resynchronized without depending on incremental
 * dirty-region history accumulated while visuals were frozen.
 */
int pstvnc_h1_mpeg_calibration_rfb_gate_set_frozen(
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate,
    int frozen);

int pstvnc_h1_mpeg_calibration_rfb_gate_allows_request(
    const pstvnc_h1_mpeg_calibration_rfb_gate_t *gate);

int pstvnc_h1_mpeg_calibration_rfb_gate_allows_remote_present(
    const pstvnc_h1_mpeg_calibration_rfb_gate_t *gate);

/*
 * Consume the one-shot thaw refresh requirement. Returns 1 exactly once after
 * each frozen->thawed transition, otherwise 0. Never returns 1 while frozen.
 */
int pstvnc_h1_mpeg_calibration_rfb_gate_take_full_refresh(
    pstvnc_h1_mpeg_calibration_rfb_gate_t *gate);

#endif /* PSTVNC_H1_MPEG_CALIBRATION_RFB_GATE_H */
