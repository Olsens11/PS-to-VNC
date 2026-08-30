/*
 * PS-to-VNC M3G shared startup-state translation unit.
 *
 * Runtime implementation moved intact to ps2vnc_runtime.c.
 */

#include "ps2vnc_runtime_compat.h"
#include "ps2vnc_cross_types.h"

/*
 * H4C startup preflight result.
 *
 * Later reconciliation stages consume this exact durable state rather than
 * re-querying authority after RFB startup has begun.
 */
ps2vnc_display_transaction_t startup_display_transaction;

int startup_display_transaction_loaded = 0;

/*
 * H4C3C1:
 * The confirmed profile is derived from the SAME configuration body that
 * startup applied. Do not re-fetch configuration later to reconstruct it.
 */
ps2vnc_display_profile_t startup_confirmed_display_profile;

int startup_confirmed_display_profile_loaded = 0;

ps2vnc_display_profile_t startup_authoritative_display_profile;

int startup_authoritative_display_profile_loaded = 0;

/*
 * Durable-state obligations retained for the later reconciliation stage.
 *
 * provisional: RESTORE -> repair A -> RESTORED -> ACK
 * restoring:              repair A -> RESTORED -> ACK
 * restored:                          verify A -> ACK
 */
int startup_display_commit_finish_required = 0;
int startup_display_restore_required = 0;
int startup_display_restored_mark_required = 0;
int startup_display_restore_ack_required = 0;
