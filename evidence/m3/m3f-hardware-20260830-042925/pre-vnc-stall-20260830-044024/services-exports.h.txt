#ifndef PS2VNC_SERVICES_EXPORTS_H
#define PS2VNC_SERVICES_EXPORTS_H

/* Symbols owned by the services TU and consumed by the main TU. */

extern int ps2vnc_display_config_loaded;
extern int screen_fit_full_bypass;
extern int screen_fit_height;
extern int screen_fit_offset_x;
extern int screen_fit_offset_y;
extern int screen_fit_width;

int calculate_presented_desktop_geometry( unsigned int *out_x, unsigned int *out_y, unsigned int *out_w, unsigned int *out_h);
void debug_emit_geometry_snapshot(void);
int draw_recovery_status( GSGLOBAL *gsGlobal, GSTEXTURE *osk_tex, const char *line1, const char *line2, int seconds_left);
int perform_manual_rfb_recovery( int old_sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer, GSGLOBAL *gsGlobal, GSTEXTURE *tex, GSTEXTURE *osk_tex);
int ps2vnc_config_apply_display( const ps2vnc_display_config_t *config);
int ps2vnc_config_get_display_for_mode( const ps2vnc_video_mode_t *mode, ps2vnc_display_config_t *out_config);
int ps2vnc_config_load_from_pi(void);
int ps2vnc_config_presented_geometry_for_mode( const ps2vnc_video_mode_t *mode, const ps2vnc_display_config_t *config, unsigned int *out_x, unsigned int *out_y, unsigned int *out_w, unsigned int *out_h);
int ps2vnc_controller_pad_acquire(void);
int ps2vnc_controller_pad_release(void);
GSGLOBAL *ps2vnc_display_create(void);
int ps2vnc_display_destroy(GSGLOBAL **display);
int ps2vnc_display_draw_desktop( GSGLOBAL *gsGlobal, GSTEXTURE *tex, u64 clear_color, u64 tex_color);
int ps2vnc_display_finish_draw(GSGLOBAL *gsGlobal);
int ps2vnc_display_flip(GSGLOBAL *gsGlobal);
int ps2vnc_display_policy_hide_to_pi( int hide_locked);
int ps2vnc_display_policy_lock_to_pi( int selection, int locked);
int ps2vnc_display_policy_refresh_from_pi(void);
int ps2vnc_display_prepare_desktop( GSGLOBAL *gsGlobal, GSTEXTURE *tex, unsigned short *gs_framebuffer);
void ps2vnc_display_prepare_osk( GSGLOBAL *gsGlobal, GSTEXTURE *osk_tex);
int ps2vnc_display_profile_capture_active( ps2vnc_display_profile_t *profile);
int ps2vnc_display_profile_resolve_candidate( const ps2vnc_video_mode_t *mode, unsigned int out_x, unsigned int out_y, unsigned int out_w, unsigned int out_h, ps2vnc_display_profile_t *profile);
int ps2vnc_display_transaction_ack_to_pi(void);
int ps2vnc_display_transaction_begin_to_pi( const ps2vnc_display_profile_t *previous, const ps2vnc_display_profile_t *candidate);
int ps2vnc_display_transaction_commit_to_pi(void);
int ps2vnc_display_transaction_restore_to_pi(void);
int ps2vnc_display_transaction_restored_to_pi(void);
int ps2vnc_display_transaction_startup_preflight(void);
int ps2vnc_display_tx_profiles_equal( const ps2vnc_display_profile_t *a, const ps2vnc_display_profile_t *b);
int ps2vnc_display_wait_presentation_memory_idle( GSGLOBAL *gsGlobal);
int ps2vnc_display_wait_texture_idle( GSGLOBAL *gsGlobal);
int ps2vnc_management_get_bounded_quiet( const char *path, unsigned char *out_body, unsigned int out_capacity, unsigned int *out_len, unsigned int budget_us);
int ps2vnc_management_post_text( const char *path, const char *body);
int ps2vnc_run_runtime_calibration( int sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer, GSGLOBAL *gsGlobal, GSTEXTURE *tex, u64 black);
int ps2vnc_startup_apply_authoritative_profile(void);
int ps2vnc_startup_connect_reconciled( unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer);
int ps2vnc_startup_prepare_display_transaction(void);
int recover_rfb_connection( int old_sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer, GSGLOBAL *gsGlobal, GSTEXTURE *tex, GSTEXTURE *osk_tex);
int recover_rfb_transport_for_display_rollback( int old_sock, unsigned short *rfb_framebuffer, unsigned short *gs_framebuffer, const ps2vnc_video_mode_t *handshake_mode);
int run_screen_fit_calibration( GSGLOBAL *gsGlobal, u64 black, int save_on_accept, int *accepted_out);
int screen_fit_apply_preset( int width, int height, int offset_x, int offset_y, int full_bypass);


/* M3E actual-source call closure. */
GSGLOBAL *ps2vnc_display_create(void);

#endif
