#ifndef PSTVNC_DIAGNOSTICS_REPORT_H
#define PSTVNC_DIAGNOSTICS_REPORT_H

#include "diagnostics/debug.h"

typedef struct pstvnc_diagnostics_debug_report
{
    unsigned int frame;
    pstvnc_debug_stage_t stage;
    int changed_bytes;

    unsigned int message_type;
    unsigned int rect_index;
    unsigned int rect_count;
    unsigned int rect_x;
    unsigned int rect_y;
    unsigned int rect_w;
    unsigned int rect_h;
    unsigned int encoding;

    unsigned int controller_loop;
    int controller_error;
    int osk_visible;

    unsigned char header_raw[12];

    unsigned int header_start_pos;
    unsigned int header_start_end;
    unsigned int header_start_avail;
    unsigned int header_end_pos;
    unsigned int header_end_end;
    int header_recv_n;
    unsigned int header_recv_call;

    unsigned int hotkey_mask;
    unsigned int hotkey_stable_polls;
    unsigned int hotkey_settle_target;
    int hotkey_action;
    int hotkey_trigger;
    unsigned int hotkey_hold_polls;
    unsigned int system_menu_countdown_seconds;
    int hotkey_latched;
    unsigned int hotkey_quarantine;

    int system_menu_visible;
    int display_menu_visible;
    int runtime_action_request;

    unsigned int transition_phase;
    int controller_pad_pause_requested;
    int controller_pad_pause_ack;

    unsigned int rfb_out_count;
} pstvnc_diagnostics_debug_report_t;

void pstvnc_diagnostics_report_debug(
    const pstvnc_diagnostics_debug_report_t *report);

typedef struct pstvnc_diagnostics_profile_report
{
    unsigned int frame;
    unsigned int rfb_updates;
    unsigned int dirty_updates;
    unsigned int changed_bytes;
    unsigned int raw_rects;
    unsigned int hextile_rects;

    unsigned int present_sync_us;
    unsigned int flip_us;
    unsigned int rfb_us;
    unsigned int rx_wait_us;
    unsigned int hextile_us;
    unsigned int hextile_rx_wait_us;
    unsigned int dirty_wait_us;
    unsigned int dirty_work_us;
} pstvnc_diagnostics_profile_report_t;

void pstvnc_diagnostics_report_profile(
    const pstvnc_diagnostics_profile_report_t *report);

typedef struct pstvnc_diagnostics_geometry_report
{
    int fit_width;
    int fit_height;
    int fit_offset_x;
    int fit_offset_y;
    int full_bypass;

    unsigned int desktop_width;
    unsigned int desktop_height;
    unsigned int output_x;
    unsigned int output_y;

    int display_config_loaded;
    int force_calibration;
} pstvnc_diagnostics_geometry_report_t;

void pstvnc_diagnostics_report_geometry(
    const pstvnc_diagnostics_geometry_report_t *report);

#endif
