# CP2D — dormant RFB resources owned by H1 transport lifecycle

Status: **COMPLETE — SOURCE/BUILD VERIFIED, RFB STILL OFF-GATED, NOT HARDWARE QUALIFIED**

This is chronological checkpoint evidence under the governing authority in
`RFB_MUX_INTEGRATION_PREP.md`. It does not replace that plan and does not begin
Issue #40.

## Commits

    7ea24933840bc49f8d448a5bbfbc86d8c26498af  h1: embed dormant RFB resources in transport runtime
    962fea417388a0dd0cb95f2e0da9d69121381502  h1: wire dormant RFB resources into lifecycle
    3264db6b4660aa070a06aaa38a8a6b44e06e4f50  h1: route cumulative transport lifecycle through RFB wrapper
    9bfcf6fe290aba0f8e8fde8cb4bb568a24ed02bf  h1: verify RFB lifecycle wrapper ownership
    5c90e563a02c8301bd1c53838702756de0cd7683  h1: compare exact symbols in RFB lifecycle seam check

## What changed

- `pstvnc_h1_transport_runtime_t` now contains the already-prepared
  `pstvnc_h1_rfb_runtime_resources_t` bundle, making the one-socket H1 transport
  runtime the explicit owner of logical channel-1 resources.
- Only the cumulative H1 RFB-preparation build mechanically renames the original
  transport lifecycle definitions to:

      pstvnc_h1_transport_start_inner
      pstvnc_h1_transport_shutdown_inner

- A small experiment-owned lifecycle wrapper retains the public
  `pstvnc_h1_transport_start` / `pstvnc_h1_transport_shutdown` API.
- Public start first executes the original H1 start unchanged, then initializes
  the embedded RFB bundle and calls its activation function using the accepted
  CONFIG `rfb_mode`.
- The current CONFIG validator still accepts only `PSTVNC_H1_RFB_OFF`, therefore
  every currently valid start takes the resource bundle's explicit OFF no-op:
  no RFB queue allocation, no RFB semaphore creation, and no channel-1 runtime
  activity occurs.
- Public shutdown stops the original sole physical socket/receiver owner first,
  then releases the RFB resource bundle. That ordering is deliberate future
  preparation for a later live channel-1 path.
- Other H1 targets are unaffected; the lifecycle symbol remapping is scoped to
  the cumulative preparation target.
- The clean through-Issue-39 RFB parser/session source remains unchanged.

This checkpoint does not advertise RFB in HELLO, relax CONFIG validation, route
channel-1 DATA, send RFB CREDIT/DATA, activate the mux I/O adapter, create a Pi
VNC bridge, or start RFB presentation/input.

## Verification

The first CP2D verification run exposed a checker-only false positive: substring
matching treated `pstvnc_h1_transport_start_inner` as the public
`pstvnc_h1_transport_start` symbol. The PS2 ELF had already compiled and linked.
The checker was corrected to parse exact `nm` symbol tokens; no runtime source
behavior was changed by that correction.

Final passing evidence:

    workflow=H1 RFB mux preparation checks
    run=34266624134
    result=PASS
    host_preflight=PASS
    logical_rfb_channel_test=PASS
    source_seam=PASS
    object_seam=PASS
    pinned_ps2_build=PASS
    ELF_SHA256=5c729e68a5262ea68404d75ccee21f09baa3b24d9e8060e88abc7132c85349bd
    PT_LOAD_SHA256=428279043a0607d4cabee2d535f26cd6f0aad319f277dad5260473bc724135fe
    PT_LOAD_BYTES=481172
    ARTIFACT=h1-rfb-mux-prep-unqualified-elf

Object inspection proves:

- the cumulative transport object defines the renamed inner start/shutdown
  symbols rather than the public lifecycle names;
- the lifecycle wrapper defines the public start/shutdown symbols and references
  the inner implementations;
- the lifecycle wrapper references all three RFB-resource lifecycle operations;
- the RFB session remains bound to H1 mux-I/O adapter names rather than direct
  RFB socket I/O;
- the adapter remains fail closed.

The ELF and PT_LOAD above are **unqualified build evidence only**. No deployment
or real-PS2 behavior is claimed.

## What remains unproven

- any RFB resource allocation under an accepted RFB-ON configuration;
- channel-1 DATA dispatch from the sole H1 receiver thread;
- actual channel-1 CREDIT frames;
- actual outbound channel-1 DATA through H1's existing send semaphore;
- mux-adapter exact-read/poll/write operation;
- HELLO RFB capability advertisement;
- Pi VNC byte-stream bridge behavior;
- `h1_tool.py --rfb on` operational behavior;
- RFB parser/session execution over the mux;
- any graphics/input integration;
- any real-PS2 behavior of this changed PT_LOAD.

## Deliberate stop point

The previously identified RFB credit-policy gap remains unresolved:

    H1_RFB_MUX_RFB_CREDIT_POLICY=UNDECIDED_DO_NOT_GUESS

AUDIO/MPEG have explicit CONFIG fields for initial credit, batch size, empty
flush, and credit-return enablement. RFB currently has no equivalent explicit
policy. CP2D does not infer or silently copy one of those policies.

## Next recommended checkpoint

Before enabling receiver channel-1 DATA or relaxing `rfb_mode=ON`, make one
explicit repository-level RFB credit-policy decision. The decision should state
initial credit, return batching/flush semantics, whether those values become
CONFIG fields or fixed first-milestone constants, and why. Only after that
contract is explicit should a later checkpoint wire DATA dispatch, CREDIT
return, outbound DATA serialization, and the mux adapter.
