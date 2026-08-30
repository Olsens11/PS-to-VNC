#ifndef PS2VNC_RUNTIME_CORE_TO_TAIL_H
#define PS2VNC_RUNTIME_CORE_TO_TAIL_H

/*
 * M3O migration-private interface:
 * core state/constants/helpers consumed by runtime tail.
 */

#define DISPLAY_CONFIRM_TIMEOUT_SECONDS 30u

#define DISPLAY_MODE_REQUEST_NONE 0

#define OSK_HEIGHT   178

#define OSK_ROWS     5

#define OSK_WIDTH    600

#define OUTPUT_HEIGHT  (active_video_mode->raster_height)

#define OUTPUT_WIDTH   (active_video_mode->raster_width)

#define PS2VNC_BOOTSTRAP_PI_IP      "192.168.50.1"

#define SYSTEM_REFRESH_COOLDOWN_SECONDS 3u

#define VNC_HEIGHT     RFB_CAPACITY_HEIGHT

#define VNC_PORT                    5900

#define VNC_WIDTH      RFB_CAPACITY_WIDTH

#define XK_ALT_L     0xFFE9

#define XK_CAPS_LOCK  0xFFE5

#define XK_CONTROL_L 0xFFE3

#define XK_DOWN       0xFF54

#define XK_END        0xFF57

#define XK_F1         0xFFBE

#define XK_F10        0xFFC7

#define XK_F11        0xFFC8

#define XK_F12        0xFFC9

#define XK_F2         0xFFBF

#define XK_F3         0xFFC0

#define XK_F4         0xFFC1

#define XK_F5         0xFFC2

#define XK_F6         0xFFC3

#define XK_F7         0xFFC4

#define XK_F8         0xFFC5

#define XK_F9         0xFFC6

#define XK_HOME       0xFF50

#define XK_INSERT     0xFF63

#define XK_LEFT       0xFF51

#define XK_PAGE_DOWN  0xFF56

#define XK_PAGE_UP    0xFF55

#define XK_PAUSE      0xFF13

#define XK_PRINT      0xFF61

#define XK_RIGHT      0xFF53

#define XK_UP         0xFF52

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

typedef enum {
    RUNTIME_ACTION_NONE = 0,
    RUNTIME_ACTION_REFRESH_RFB,
    RUNTIME_ACTION_DISPLAY_SETTINGS,
    RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU,
    RUNTIME_ACTION_SEND_TAB,
    RUNTIME_ACTION_SEND_ENTER
} ps2vnc_runtime_action_t;

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
extern unsigned int debug_idle_poll_counter ;
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
extern volatile u64 display_confirm_started_tick ;
extern volatile int display_confirm_visible ;
extern volatile int display_menu_visible ;
extern volatile int display_mode_request ;
extern volatile int display_restore_ack_visible ;
extern volatile int display_transition_switch_in_progress ;
extern volatile unsigned int live_encoding ;
extern volatile int live_error_code ;
extern volatile unsigned int live_hdr_end_end ;
extern volatile unsigned int live_hdr_end_pos ;
extern volatile unsigned char live_hdr_raw[12];
extern volatile unsigned int live_hdr_recv_call ;
extern volatile int live_hdr_recv_n ;
extern volatile unsigned int live_hdr_start_avail ;
extern volatile unsigned int live_hdr_start_end ;
extern volatile unsigned int live_hdr_start_pos ;
extern volatile unsigned int live_message_type ;
extern volatile unsigned int live_rect_count ;
extern volatile unsigned int live_rect_h ;
extern volatile unsigned int live_rect_index ;
extern volatile unsigned int live_rect_w ;
extern volatile unsigned int live_rect_x ;
extern volatile unsigned int live_rect_y ;
extern volatile int osk_alt ;
extern volatile int osk_col ;
extern volatile int osk_ctrl ;
extern volatile int osk_page ;
extern volatile int osk_row ;
extern volatile int osk_shift ;
extern volatile int osk_visible ;
extern unsigned int profile_changed_bytes ;
extern unsigned int profile_dirty_updates ;
extern u64 profile_dirty_wait_ticks ;
extern u64 profile_dirty_work_ticks ;
extern u64 profile_flip_ticks ;
extern unsigned int profile_hextile_rects ;
extern u64 profile_hextile_rx_wait_ticks ;
extern u64 profile_hextile_ticks ;
extern u64 profile_present_sync_ticks ;
extern unsigned int profile_raw_rects ;
extern u64 profile_rfb_ticks ;
extern unsigned int profile_rfb_updates ;
extern u64 profile_rx_wait_ticks ;
extern volatile int
    remote_display_suppress_next_active_menu_return ;
extern volatile unsigned int rfb_out_count ;

extern void display_menu_open_for_active (int);
extern unsigned int read_be16 (const unsigned char *p);
extern unsigned int read_be32 (const unsigned char *p);
extern int recv_exact (int sock, void *data, int len);
extern void sanitize_text (char *s);
extern int send_rfb_message (int sock, const void *data, int len);

#endif /* PS2VNC_RUNTIME_CORE_TO_TAIL_H */
