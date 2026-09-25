/*
 * File synopsis:
 * Publishes Configuration's selected common-media-clock product values without
 * taking timing mechanism or runtime lifecycle ownership.
 *
 * A002 classifies the common media epoch as a clean media-owner mechanism and
 * selected profile values as Configuration authority. R26 selects the common
 * zero-lead/zero-offset values recovered from the qualified/all-guns H1 profile
 * lineage at forensic commit
 * 3426f28b93de9519ca93e5f0e0aaf8b67cfca845.
 *
 * This file contains values only. It does not arm a media clock, observe a PS2
 * timer, lock synchronization state, delay a thread, start AUDIO/MPEG, or add
 * PSTV CONFIG fields.
 *
 * Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md;
 * docs/ledge/LEDGE_FOREMAN_STATE.md,
 * A002-PS2-MEDIA-CLOCK-PRODUCT-BINDING-R26.
 */

#include "media_clock_profile.h"

static const pstvnc_config_media_clock_profile_t
    pstvnc_config_selected_media_clock_profile = {
        .epoch_lead_us = 0u,
        .audio_presentation_offset_us = 0,
        .video_presentation_offset_us = 0
    };

pstvnc_config_media_clock_profile_t
pstvnc_config_media_clock_profile_selected(void)
{
    return pstvnc_config_selected_media_clock_profile;
}
