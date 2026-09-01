/*
 * File synopsis:
 * Formats caller-owned debug, performance, and geometry snapshots into
 * diagnostic datagrams without becoming authoritative for their values.
 */

#include <stdio.h>

#include "diagnostics/report.h"

void pstvnc_diagnostics_report_debug(
    const pstvnc_diagnostics_debug_report_t *report)
{
    char msg[640];
    int len;

    if (!report || !pstvnc_debug_is_ready())
        return;

    len = snprintf(
        msg,
        sizeof(msg),
        "DBG frame=%u stage=%s(%u) bytes=%d msg=%u "
        "rect=%u/%u xywh=%u,%u,%u,%u enc=%08x ctl=%u cerr=%d osk=%d "
        "hdr=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x "
        "hs=%u/%u hav=%u he=%u/%u rn=%d rc=%u "
        "hkm=%04x hks=%u/%u hka=%d hkt=%d hhp=%u hcd=%u "
        "hkl=%d hq=%04x sm=%d dm=%d rar=%d "
        "b4=%u ppr=%d ppa=%d q=%u",
        report->frame,
        pstvnc_debug_stage_name(report->stage),
        (unsigned int)report->stage,
        report->changed_bytes,
        report->message_type,
        report->rect_index,
        report->rect_count,
        report->rect_x,
        report->rect_y,
        report->rect_w,
        report->rect_h,
        report->encoding,
        report->controller_loop,
        report->controller_error,
        report->osk_visible,
        report->header_raw[0],
        report->header_raw[1],
        report->header_raw[2],
        report->header_raw[3],
        report->header_raw[4],
        report->header_raw[5],
        report->header_raw[6],
        report->header_raw[7],
        report->header_raw[8],
        report->header_raw[9],
        report->header_raw[10],
        report->header_raw[11],
        report->header_start_pos,
        report->header_start_end,
        report->header_start_avail,
        report->header_end_pos,
        report->header_end_end,
        report->header_recv_n,
        report->header_recv_call,
        report->hotkey_mask,
        report->hotkey_stable_polls,
        report->hotkey_settle_target,
        report->hotkey_action,
        report->hotkey_trigger,
        report->hotkey_hold_polls,
        report->system_menu_countdown_seconds,
        report->hotkey_latched,
        report->hotkey_quarantine,
        report->system_menu_visible,
        report->display_menu_visible,
        report->runtime_action_request,
        report->transition_phase,
        report->controller_pad_pause_requested,
        report->controller_pad_pause_ack,
        report->rfb_out_count);

    if (len <= 0)
        return;

    if (len >= (int)sizeof(msg))
        len = sizeof(msg) - 1;

    (void)pstvnc_debug_send(
        msg,
        (size_t)len);
}

void pstvnc_diagnostics_report_profile(
    const pstvnc_diagnostics_profile_report_t *report)
{
    char msg[256];
    int len;

    if (!report || !pstvnc_debug_is_ready())
        return;

    len = snprintf(
        msg,
        sizeof(msg),
        "PRF frame=%u rn=%u dn=%u bytes=%u raw=%u hx=%u "
        "ps=%u fl=%u rfb=%u rx=%u hd=%u hr=%u dw=%u cp=%u",
        report->frame,
        report->rfb_updates,
        report->dirty_updates,
        report->changed_bytes,
        report->raw_rects,
        report->hextile_rects,
        report->present_sync_us,
        report->flip_us,
        report->rfb_us,
        report->rx_wait_us,
        report->hextile_us,
        report->hextile_rx_wait_us,
        report->dirty_wait_us,
        report->dirty_work_us);

    if (len <= 0)
        return;

    if (len >= (int)sizeof(msg))
        len = sizeof(msg) - 1;

    (void)pstvnc_debug_send(
        msg,
        (size_t)len);
}

void pstvnc_diagnostics_report_geometry(
    const pstvnc_diagnostics_geometry_report_t *report)
{
    char msg[192];
    int len;

    if (!report || !pstvnc_debug_is_ready())
        return;

    len = snprintf(
        msg,
        sizeof(msg),
        "GEOM fit=%dx%d@%d,%d full=%d "
        "desk=%ux%u out=%u,%u cfg=%d force=%d",
        report->fit_width,
        report->fit_height,
        report->fit_offset_x,
        report->fit_offset_y,
        report->full_bypass,
        report->desktop_width,
        report->desktop_height,
        report->output_x,
        report->output_y,
        report->display_config_loaded,
        report->force_calibration);

    if (len <= 0)
        return;

    if (len >= (int)sizeof(msg))
        len = sizeof(msg) - 1;

    (void)pstvnc_debug_send(
        msg,
        (size_t)len);
}
