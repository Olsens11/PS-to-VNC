#ifndef PS2VNC_RUNTIME_TO_UI_DISPLAY_H
#define PS2VNC_RUNTIME_TO_UI_DISPLAY_H

/*
 * runtime exports consumed by UI/display
 *
 * M3I migration interface scaffold.
 * Functions: GCC aux-info authority.
 */

enum {
    DBG_STAGE_STARTUP = 0,
    DBG_STAGE_NET_READY,
    DBG_STAGE_GS_INIT,
    DBG_STAGE_INITIAL_TEXTURE,
    DBG_STAGE_CONTROLLER_START,
    DBG_STAGE_LOOP_BEGIN,
    DBG_STAGE_PRE_DRAW,
    DBG_STAGE_POST_DRAW,
    DBG_STAGE_PRE_FLIP,
    DBG_STAGE_POST_FLIP,
    DBG_STAGE_PRE_QUEUE,
    DBG_STAGE_POST_QUEUE,
    DBG_STAGE_PRE_REQUEST,
    DBG_STAGE_POST_REQUEST,
    DBG_STAGE_WAIT_RFB,
    DBG_STAGE_POST_RECEIVE,
    DBG_STAGE_PRE_TEXTURE,
    DBG_STAGE_POST_TEXTURE,
    DBG_STAGE_CONTROLLER_ERROR,
    DBG_STAGE_REQUEST_ERROR,
    DBG_STAGE_RECEIVE_ERROR
};

enum {
    DISPLAY_CONFIRM_ACTION_NONE = 0,
    DISPLAY_CONFIRM_ACTION_KEEP,
    DISPLAY_CONFIRM_ACTION_ROLLBACK,
    DISPLAY_CONFIRM_ACTION_ROLLBACK_IN_PROGRESS
};

enum {
    DISPLAY_CONFIRM_GO_BACK = 0,
    DISPLAY_CONFIRM_KEEP,
    DISPLAY_CONFIRM_ITEM_COUNT
};

#define DISPLAY_MODE_REQUEST_NONE 0

enum {
    DISPLAY_POLICY_REQUEST_NONE = 0,
    DISPLAY_POLICY_REQUEST_REFRESH,
    DISPLAY_POLICY_REQUEST_LOCK,
    DISPLAY_POLICY_REQUEST_HIDE_LOCKED
};

enum {
    DISPLAY_TAB_TV_DTV = 0,
    DISPLAY_TAB_VGA_60,
    DISPLAY_TAB_VGA_OTHER,
    DISPLAY_TAB_ADVANCED,
    DISPLAY_TAB_COUNT
};

(PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2 | PAD_START)

enum {
    HOTKEY_TRIGGER_SETTLE = 1,
    HOTKEY_TRIGGER_RELEASE,
    HOTKEY_TRIGGER_HOLD
};

extern unsigned char NETMAN_irx[];

#define OSK_HEIGHT   178

enum {
    OSK_PAGE_ABC = 0,
    OSK_PAGE_FUNC = 1
};

#define OSK_ROWS     5

#define OSK_WIDTH    600

#define OUTPUT_HEIGHT  (active_video_mode->raster_height)

#define OUTPUT_WIDTH   (active_video_mode->raster_width)

#define PS2VNC_DISPLAY_CONTROL_MAX_BYTES 768

#define PS2VNC_DISPLAY_CONTROL_PENDING_PATH \
    "/display-control/pending"

#define PS2VNC_DISPLAY_CONTROL_RESULT_PATH \
    "/display-control/result"

#define PS2VNC_REMOTE_CONTROL_GET_BUDGET_US 100000u

#define PS2VNC_REMOTE_CONTROL_POLL_US     500000u

enum {
    REMOTE_DISPLAY_IDLE = 0,
    REMOTE_DISPLAY_SWITCH_REQUESTED,
    REMOTE_DISPLAY_SWITCHING,
    REMOTE_DISPLAY_ACTIVE_TEMP,
    REMOTE_DISPLAY_RESTORING
};

#define RFB_RECEIVE_LOCAL_REDRAW (-2)

#define RUNTIME_HOTKEY_BUTTON_MASK \
    (PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2 | PAD_START)

#define RUNTIME_HOTKEY_EXTENDABLE_SETTLE_POLLS 18u

#define RUNTIME_HOTKEY_SETTLE_POLLS 8u

extern unsigned char SMAP_irx[];

#define SYSTEM_CHORD_HOLD_POLLS 120u

enum {
    SYSTEM_MENU_REFRESH = 0,
    SYSTEM_MENU_DISPLAY,
    SYSTEM_MENU_EXIT,
    SYSTEM_MENU_ITEM_COUNT
};

enum {
    SYSTEM_MENU_STATUS_NONE = 0,
    SYSTEM_MENU_STATUS_REFRESHING,
    SYSTEM_MENU_STATUS_REFRESH_COMPLETE
};

enum {
    SYSTEM_REFRESH_READY = 0,
    SYSTEM_REFRESH_REFRESHING,
    SYSTEM_REFRESH_COOLDOWN
};

#define TEST29_DISCARD_LIVE_PIXELS 0

#define VIDEO_MODE_COUNT \
    (sizeof(video_modes) / sizeof(video_modes[0]))

#define VNC_HEIGHT     RFB_CAPACITY_HEIGHT

#define VNC_WIDTH      RFB_CAPACITY_WIDTH

#define XK_BACKSPACE 0xFF08

#define XK_DELETE    0xFFFF

#define XK_DOWN       0xFF54

#define XK_ESCAPE    0xFF1B

#define XK_LEFT       0xFF51

#define XK_RETURN    0xFF0D

#define XK_RIGHT      0xFF53

#define XK_SHIFT_L    0xFFE1

#define XK_TAB       0xFF09

#define XK_UP         0xFF52

extern unsigned int size_DEV9_irx;

extern unsigned int size_NETMAN_irx;

extern unsigned int size_SMAP_irx;

extern "/display-transaction"
    "/display-transaction/begin"
    "/display-transaction/commit"
    "/display-transaction/restore"
    "/display-transaction/restored"
    "/display-transaction/ack"

    "/display-control/pending"
    "/display-control/result"




 

 















 

extern unsigned char DEV9_irx[];

extern const ps2vnc_hotkey_binding_t runtime_hotkeys[] ;

extern volatile int system_menu_countdown_action ;

extern volatile unsigned int controller_loop_counter ;

extern volatile int debug_changed_bytes ;

extern volatile unsigned int debug_frame_counter ;

extern volatile int debug_hotkey_action ;

extern volatile unsigned int debug_hotkey_hold_polls ;

extern volatile int debug_hotkey_latched ;

extern volatile unsigned int debug_hotkey_mask ;

extern volatile unsigned int debug_hotkey_quarantine ;

extern volatile unsigned int debug_hotkey_settle_target ;

extern volatile unsigned int debug_hotkey_stable_polls ;

extern volatile int debug_hotkey_trigger ;

extern volatile unsigned int debug_transition_phase ;

extern volatile int display_calibration_request ;

extern volatile int display_confirm_action ;

extern volatile int display_confirm_pending ;

extern ps2vnc_backend_t display_confirm_previous_backend ;

extern unsigned int display_confirm_previous_desktop_h ;

extern unsigned int display_confirm_previous_desktop_w ;

extern unsigned int display_confirm_previous_h ;

extern const ps2vnc_video_mode_t *display_confirm_previous_mode ;

extern int display_confirm_previous_screen_fit_full_bypass ;

extern int display_confirm_previous_screen_fit_height ;

extern int display_confirm_previous_screen_fit_offset_x ;

extern int display_confirm_previous_screen_fit_offset_y ;

extern int display_confirm_previous_screen_fit_width ;

extern unsigned int display_confirm_previous_w ;

extern unsigned int display_confirm_previous_x ;

extern unsigned int display_confirm_previous_y ;

extern volatile unsigned int display_confirm_seconds ;

extern volatile int display_confirm_selection ;

extern volatile int display_confirm_visible ;

extern volatile int display_menu_error ;

extern volatile int display_menu_parent_system ;

extern volatile int display_menu_selection ;

extern volatile int display_menu_tab ;

extern volatile int display_menu_tab_selection[3] ;

extern volatile int display_menu_visible ;

extern volatile int display_mode_request ;

extern volatile int display_mode_request_remote ;

extern volatile int display_policy_error ;

extern volatile int display_policy_request ;

extern volatile int display_policy_request_selection ;

extern volatile int display_policy_request_value ;

extern const ps2vnc_video_mode_t *display_restore_ack_mode ;

extern volatile int display_restore_ack_request ;

extern volatile int display_restore_ack_visible ;

extern volatile unsigned int display_switch_failure_stage ;

extern volatile int display_transition_switch_in_progress ;

extern volatile unsigned int live_encoding ;

extern volatile int live_error_code ;

extern volatile unsigned char live_hdr_raw[12];

extern volatile int live_hdr_recv_n ;

extern volatile unsigned int live_hdr_start_avail ;

extern volatile unsigned int live_message_type ;

extern volatile unsigned int live_rect_count ;

extern volatile unsigned int live_rect_h ;

extern volatile unsigned int live_rect_index ;

extern volatile unsigned int live_rect_w ;

extern volatile unsigned int live_rect_x ;

extern volatile unsigned int live_rect_y ;

extern volatile unsigned char live_update_raw[4];

extern volatile int osk_alt ;

extern volatile int osk_col ;

extern volatile int osk_ctrl ;

extern const unsigned int osk_func_keysyms[4][12] ;

extern volatile int osk_page ;

extern volatile int osk_row ;

extern volatile int osk_shift ;

extern volatile int osk_visible ;

extern unsigned char pad_area[256] __attribute__((aligned(64)));

extern unsigned int profile_changed_bytes ;

extern u64 profile_dirty_wait_ticks ;

extern u64 profile_flip_ticks ;

extern u64 profile_present_sync_ticks ;

extern u64 profile_rfb_start_tick ;

extern u64 profile_rfb_ticks ;

extern unsigned int profile_rfb_updates ;

extern ps2vnc_hotkey_binding_t;

extern const ps2vnc_video_mode_t *
    remote_display_baseline_mode ;

extern unsigned int remote_display_error_code ;

extern u64 remote_display_last_poll_tick ;

extern unsigned int remote_display_lease_seconds ;

extern u64 remote_display_lease_started_tick ;

extern const ps2vnc_video_mode_t *
    remote_display_requested_mode ;

extern volatile int remote_display_state ;

extern volatile int
    remote_display_suppress_next_active_menu_return ;

extern unsigned int remote_display_txid ;

extern volatile int rfb_confirm_live_yield_enabled ;

extern volatile unsigned int rfb_confirm_wait_generation ;

extern volatile unsigned int rfb_last_recv_call ;

extern volatile int rfb_last_recv_n ;

extern unsigned int rfb_rx_end ;

extern unsigned int rfb_rx_pos ;

extern volatile unsigned int rfb_rx_syscalls ;

extern int screen_row ;

extern const ps2vnc_video_mode_t *const video_modes[] ;

extern void debug_emit_now (void);

extern int debug_udp_init (void);

extern void display_confirm_arm_transaction (const ps2vnc_video_mode_t *previous_mode, unsigned int previous_x, unsigned int previous_y, unsigned int previous_w, unsigned int previous_h, int previous_screen_fit_width, int previous_screen_fit_height, int previous_screen_fit_offset_x, int previous_screen_fit_offset_y, int previous_screen_fit_full_bypass);

extern void display_confirm_disarm_transaction (void);

extern unsigned int display_confirm_remaining_seconds (void);

extern void display_confirm_return_to_active_menu (void);

extern int display_mode_request_for_selection (int selection);

extern const ps2vnc_video_mode_t *display_mode_request_target (int request);

extern int ethWaitValidNetIFLinkState (void);

extern void osk_clear_one_shot_modifiers (void);

extern char osk_display_char (int row, int col);

extern void osk_render_bitmap (void);

extern int osk_row_length (int row);

extern void profile_emit_now (void);

extern int ps2vnc_discard_controller_rfb_queue (void);

extern int ps2vnc_display_begin_update (void);

extern int ps2vnc_display_commit_update (GSGLOBAL *gsGlobal, GSTEXTURE *tex, int direct_present_complete);

extern int ps2vnc_display_finish_update_decode (void);

extern int ps2vnc_display_logical_desktop_size (unsigned int output_width, unsigned int output_height, unsigned int *width, unsigned int *height);

extern int ps2vnc_system_exit_accept_request (void);

extern int ps2vnc_system_overlay_allowed (void);

extern int ps2vnc_system_refresh_accept_request (void);

extern int ps2vnc_system_refresh_update_state (void);

extern int runtime_hotkey_has_hold_extension (unsigned int mask, int desktop_context);

extern const ps2vnc_hotkey_binding_t *runtime_hotkey_lookup (unsigned int mask, int desktop_context);

extern int send_key_event (int sock, unsigned char down, unsigned int keysym);

extern int send_key_tap (int sock, unsigned int keysym);

extern int send_modified_key_tap (int sock, unsigned int keysym, int use_ctrl, int use_alt);

extern int send_pointer_event (int sock, unsigned char button_mask, unsigned int x, unsigned int y);

extern int start_controller_thread (int sock);

extern int startup_force_calibration_requested (void);

extern int wait_pad_ready_after_mode_change (void);

#endif /* PS2VNC_RUNTIME_TO_UI_DISPLAY_H */
