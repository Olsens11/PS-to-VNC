# CP2F — CONFIG v4 carries tunable RFB queue and credit policy

Status: **COMPLETE — SOURCE/HOST/BUILD VERIFIED, RFB STILL OFF-GATED, NOT HARDWARE QUALIFIED**

This checkpoint implements the operator decision recorded in
`RFB_CREDIT_POLICY_DECISION.md`: logical RFB channel 1 is not exempt from H1's
per-session flow-control knobs. Queue and credit policy must be tunable without
building a new ELF for each optimization trial.

## Operator decision

David selected the explicit-config policy on 2026-09-08. The first controlled
RFB-only values are:

    rfb_queue_capacity=32768
    rfb_initial_credit_bytes=32768
    rfb_credit_return_enabled=1
    rfb_credit_batch_bytes=8192
    rfb_credit_flush_on_empty=1

These are starting hypotheses, not claimed hardware optima. The 32768-byte queue
comes from the qualified direct-RFB receive-prefetch precedent; the 8192-byte
batch matches current max_data_payload and the existing MPEG batch. No arbitrary
upper queue ceiling was introduced.

## CONFIG/profile change

H1 CONFIG is now version 4 with 61 complete wire fields. New field IDs are:

    57 rfb_queue_capacity
    58 rfb_credit_batch_bytes
    59 rfb_credit_flush_on_empty
    60 rfb_credit_return_enabled
    61 rfb_initial_credit_bytes

Existing RFB-OFF media profiles explicitly serialize zero for all five fields.
This keeps the OFF path genuinely inert rather than relying on implicit/default
state.

`h1_profiles.py` can now represent the selected future RFB-only policy and the
operator tool automatically discovers all five raw fields for `knobs`, `--set`,
and `sweep --vary`. Detailed usage is recorded in `H1_TOOL_GUIDE.md`.

## Resource flexibility

The logical RFB queue component no longer hard-codes 32768 bytes as its only
accepted capacity. Caller-owned storage may use any nonzero uint32-representable
capacity; live CONFIG validation will require the queue to be at least one
`max_data_payload` when RFB ON is eventually accepted.

The resource bundle now allocates exactly the session-selected RFB queue
capacity and records that capacity for release/diagnostics. OFF activation
requires capacity zero and allocates nothing.

The cumulative transport lifecycle passes the CONFIG-selected queue capacity to
that bundle. Because the current authoritative validator still rejects RFB ON,
all currently accepted sessions continue to take the allocation-free OFF path.

## Deliberate activation gate

This checkpoint does **not** yet:

- accept `rfb_mode=1` on the PS2;
- advertise CAP_RFB;
- dispatch channel-1 DATA from the sole physical receiver;
- emit initial or returned channel-1 CREDIT;
- activate the mux-backed RFB I/O adapter;
- bridge a Pi VNC socket to channel 1;
- execute the through-Issue-39 RFB session over H1;
- start RFB presentation/input;
- combine RFB and MPEG graphics ownership.

The point of CP2F is to establish and verify the complete tunable policy
vocabulary before those live paths depend on it.

## Verification

The first source attempt correctly exposed a stale seam-check assumption that
32768 had to remain a compile-time allocation constant. The checker was updated
to require 32768 as the evidence-based reference/default while explicitly
verifying caller-selected allocation. No queue size was silently increased.

Final current-branch verification:

    workflow=H1 RFB mux preparation checks
    run=34280357913
    result=PASS
    head=22c5de87cbc920ea8f7c7d28fb4edc79dbc2dd4b
    H1_CONFIG_V4_PROFILE_SELF_TEST=PASS
    host_preflight=PASS
    logical_rfb_channel_test=PASS
    source_seam=PASS
    object_seam=PASS
    pinned_ps2_build=PASS
    ELF_SHA256=6c721a2d877f8d3d5c56413dc70214fb507045054118912b8122d5514cf6d541
    PT_LOAD_SHA256=897a1e7b3a7d85764f305dd4651f6707545a915a7160442fe3a0748080d22b8d
    PT_LOAD_BYTES=482068
    ARTIFACT=h1-rfb-mux-prep-unqualified-elf
    ARTIFACT_ID=10077357821

The exact ELF remains **unqualified build evidence only**. This checkpoint does
not authorize a claim that RFB works on hardware.

## Next checkpoint

Wire the selected policy into live channel-1 mechanics while the CONFIG RFB-ON
gate remains closed:

1. sole receiver dispatches channel-1 DATA only through the RFB queue owner;
2. initial/returned CREDIT uses the five CONFIG v4 values;
3. returned credit is based on parser-consumed bytes, with configurable batch
   and flush-on-empty behavior;
4. outbound logical RFB bytes are fragmented and serialized through H1's existing
   send semaphore;
5. mux adapter read/poll/write binds to those live mechanics;
6. host/build checks prove ownership before the activation gate is relaxed.

After that, add the Pi raw-byte VNC bridge, advertise RFB only when truly
supported, and make the first hardware test RFB-only with AUDIO/MPEG disabled.
