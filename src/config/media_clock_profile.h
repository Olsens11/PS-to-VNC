/*
 * File synopsis:
 * Declares Configuration's immutable selected product media-clock profile.
 * R26 fixes the clean A002 clock inputs to the evidence-grounded qualified H1
 * lineage values: zero epoch lead, zero audio presentation offset, and zero
 * video presentation offset. This is internal product profile authority, not a
 * mutable user/network tuning surface and not a PSTV CONFIG-wire extension.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 * docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26.
 */

#ifndef PSTVNC_CONFIG_MEDIA_CLOCK_PROFILE_H
#define PSTVNC_CONFIG_MEDIA_CLOCK_PROFILE_H

#include "profile.h"

/*
 * Return one caller-owned copy of the selected profile.
 *
 * Returning by value prevents a caller from mutating Configuration-owned
 * authority. A later call always reconstructs the exact selected 0/0/0 value.
 */
pstvnc_config_media_clock_profile_t
pstvnc_config_media_clock_profile_selected(void);

#endif /* PSTVNC_CONFIG_MEDIA_CLOCK_PROFILE_H */
