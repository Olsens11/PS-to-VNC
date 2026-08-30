#ifndef PS2VNC_SERVICES_IMPORTS_H
#define PS2VNC_SERVICES_IMPORTS_H

/* Symbols owned by the main/runtime TU and consumed by services. */

extern ps2vnc_backend_t active_display_backend;
extern const ps2vnc_video_mode_t *active_video_mode;
extern volatile int controller_pad_pause_ack;
extern volatile int controller_pad_pause_requested;
extern volatile int controller_thread_error;
extern int desktop_geometry_locked;
extern unsigned int desktop_height;
extern unsigned int desktop_output_height;
extern unsigned int desktop_output_width;
extern unsigned int desktop_output_x;
extern unsigned int desktop_output_y;
extern unsigned int desktop_width;
extern volatile int display_hide_locked_modes;
extern volatile unsigned char display_mode_locked[];
extern volatile int display_policy_loaded;
extern volatile int exit_requested;
extern int force_screen_fit_calibration;
extern unsigned short *hires_bg_back_buffer;
extern unsigned short *hires_bg_buffer;
extern int hires_bg_configured;
extern unsigned short *hires_mapped_source_framebuffer;
extern unsigned short *live_direct_present_buffer;
extern int live_direct_present_safe;
extern int live_linear_framebuffer_stale;
extern volatile unsigned int osk_generation;
extern unsigned short osk_pixels[] __attribute__((aligned(128)));
extern volatile int rfb_connected;
extern int rfb_queue_sema;
extern volatile int runtime_action_request;
extern ps2vnc_display_profile_t startup_authoritative_display_profile;
extern int startup_authoritative_display_profile_loaded;
extern ps2vnc_display_profile_t startup_confirmed_display_profile;
extern int startup_confirmed_display_profile_loaded;
extern int startup_display_commit_finish_required;
extern int startup_display_restore_ack_required;
extern int startup_display_restore_required;
extern int startup_display_restored_mark_required;
extern ps2vnc_display_transaction_t startup_display_transaction;
extern int startup_display_transaction_loaded;
extern volatile int system_menu_countdown_action;
extern volatile unsigned int system_menu_countdown_seconds;
extern volatile int system_menu_selection;
extern volatile int system_menu_status;
extern volatile int system_menu_visible;
extern volatile u64 system_refresh_cooldown_started_tick;
extern volatile int system_refresh_state;

const ps2vnc_video_mode_t *display_menu_mode_at_selection( int selection);
int display_menu_selection_for_mode( const ps2vnc_video_mode_t *mode);
int ethGetNetIFLinkStatus(void);
int flush_rfb_outgoing(int sock);
unsigned short osk_color( unsigned int r5, unsigned int g5, unsigned int b5);
void osk_draw_text_centered( int x, int y, int w, int h, const char *s, int scale, unsigned short color);
void osk_fill_rect( int x, int y, int w, int h, unsigned short color);
const unsigned char *osk_find_glyph(char c);
int probe_extended_desktop_size( int sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer);
unsigned int profile_ticks_us(u64 ticks);
void ps2vnc_display_direct_write_cancel(void);
float ps2vnc_display_output_x(float x);
float ps2vnc_display_output_y(float y);
int ps2vnc_display_publish_full_desktop( GSGLOBAL *gsGlobal, GSTEXTURE *tex, unsigned short *gs_framebuffer);
unsigned int ps2vnc_hires_effective_width(void);
int ps2vnc_hires_requires_raster_mapping(void);
unsigned int ps2vnc_hires_storage_bytes(void);
unsigned int ps2vnc_hires_storage_height(void);
unsigned int ps2vnc_hires_storage_width(void);
const ps2vnc_video_mode_t *pstvnc_video_mode_by_name( const char *name);
ps2vnc_backend_t pstvnc_video_mode_default_backend( const ps2vnc_video_mode_t *mode);
int publish_hires_background( GSGLOBAL *gsGlobal, GSTEXTURE *bg_tex, const unsigned short *gs_framebuffer);
int receive_framebuffer_update( int sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer, int verbose, int require_full);
int region_fits_u32( unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int bounds_width, unsigned int bounds_height);
int request_and_confirm_desktop_size( int sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer, unsigned int width, unsigned int height);
int request_framebuffer(int sock, int incremental);
void reset_rfb_transport_state(void);
int rfb_connect_and_handshake(void);
int rfb_connect_and_handshake_limited( unsigned int max_width, unsigned int max_height);
void screenf(const char *fmt, ...);
int send_exact(int sock, const void *data, int len);
int set_gs555_pixel_format(int sock);
int set_live_encoding(int sock);
int set_raw_encoding(int sock);
int set_raw_extended_desktop_encoding(int sock);


/* M3E actual-source call closure. */
const ps2vnc_video_mode_t *display_menu_mode_at_selection( int selection);
const unsigned char *osk_find_glyph(char c);
const ps2vnc_video_mode_t *pstvnc_video_mode_by_name( const char *name);


/* M3E R3 direct source-resolution repair. */
int pstvnc_video_mode_supports_backend( const ps2vnc_video_mode_t *mode, ps2vnc_backend_t backend);

#endif
