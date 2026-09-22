#!/usr/bin/env python3
"""File synopsis:\nProjects the canonical current RFB profile onto the Pi R13 flow type.\n
Selected numeric values are generated from src/config/rfb_runtime_profile.json.
This module only constructs an RfbFlowConfig value when semantic RFB is ON. OFF
returns no projection, so it cannot silently create an active attachment from a
zero-valued fake running profile.

The module does not open the provider, create RfbAttachment, establish Wire,
choose Application policy, retry failures, or alter the default Wire service.

Context: docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-RFB-SHARED-RUNTIME-PROFILE-R14.
"""

from __future__ import annotations

import rfb_attachment
import rfb_runtime_profile_generated as generated


def project_rfb_flow_config(mode: str) -> rfb_attachment.RfbFlowConfig | None:
    """Return one R13 flow projection for ON, or no projection for OFF."""

    if mode == "OFF":
        return None
    if mode != "ON":
        raise ValueError("RFB mode must be semantic ON or OFF")

    return rfb_attachment.RfbFlowConfig(
        provider_read_credit_limit=generated.RFB_WINDOW_BYTES,
        provider_write_capacity=generated.RFB_WINDOW_BYTES,
        max_data_payload=generated.MAX_DATA_PAYLOAD,
    )


def selected_rfb_flow_config() -> rfb_attachment.RfbFlowConfig | None:
    """Project the one canonical current semantic selection."""

    return project_rfb_flow_config(generated.RFB_MODE)
