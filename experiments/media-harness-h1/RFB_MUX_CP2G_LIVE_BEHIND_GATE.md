# CP2G — live RFB channel-1 mechanics wired behind the closed activation gate

Status: **COMPLETE — SOURCE/HOST/OBJECT/BUILD VERIFIED, RFB STILL OFF-GATED, NOT HARDWARE QUALIFIED**

CP2G wires the CONFIG-v4 RFB queue and credit policy into the cumulative H1
transport without yet permitting an RFB session to start. The purpose is to make
the logical channel complete enough to inspect mechanically before the public
activation gate or Pi-side VNC bridge can expose it to hardware.

## Preserved architecture

- One physical PS2-facing PSTV TCP connection remains authoritative.
- H1's existing receiver thread remains the sole owner of physical receives.
- H1's existing send semaphore and sequence counter serialize every outbound
  frame, including logical RFB DATA/CREDIT.
- RFB is logical channel 1; AUDIO remains 2 and MPEG2 remains 4.
- No direct RFB socket path was introduced.
- The through-Issue-39 RFB parser source remains unchanged and is still
  mechanically compiled against the H1 experiment-owned I/O adapter names.
- Issue #40 remains out of scope.

## Startup race fixed before activation

The previous lifecycle preparation activated the dormant RFB resource bundle
after the inner H1 start had already launched the sole receiver thread. That
would have been unsafe once channel 1 became legal: DATA could race the queue
allocation.

The cumulative H1 transport now prepares the RFB resource bundle after CONFIG
has been decoded/validated but **before CONFIG ACK, initial RFB credit, and the
receiver-thread start**. Normal shutdown stops the physical receiver/socket
owner first, then unbinds/releases logical RFB resources.

## Live mechanics now present behind the gate

`h1_rfb_transport_live.c` now provides:

- exact CONFIG-selected queue allocation through `h1_rfb_runtime_resources`;
- sole-receiver dispatch of channel-1 DATA into that queue;
- parser-facing exact-read and poll behavior over the logical queue;
- configurable initial RFB receiver credit;
- returned receiver credit based on bytes actually consumed by the RFB parser;
- configurable credit batch and flush-on-empty policy;
- outbound logical RFB fragmentation at `max_data_payload`;
- outbound DATA serialization through H1's existing send semaphore/sequence
  path;
- session-scoped adapter bind/unbind using the already-owned H1 socket descriptor
  only as a fail-closed identity token.

A pure `h1_rfb_credit_policy` component keeps the batch/flush decision
host-testable without PS2 kernel dependencies.

## Deliberately still closed

CP2G does **not**:

- accept `rfb_mode=1` in the authoritative PS2 CONFIG validator;
- advertise `PSTVNC_TRANSPORT_CAP_RFB` in HELLO;
- connect the Pi mux server to a VNC server;
- run the RFB parser/session on PS2;
- allocate/present an RFB framebuffer in the H1 coordinator;
- start pointer/keyboard/OSK behavior;
- combine RFB and MPEG graphics ownership.

Thus ordinary accepted media sessions still use `rfb_mode=0`, serialize zero for
all five RFB queue/credit fields, and allocate no RFB queue.

## Verification

Final CP2G source/build verification:

    workflow=H1 RFB mux preparation checks
    run=34301679740
    result=PASS
    head=53c6fbf82b5cc4ab171d7ab2fdb482b629fd98d8
    H1_PI_PROFILES_SELF_TEST=PASS
    host_preflight=PASS
    logical_rfb_channel_test=PASS
    logical_rfb_credit_policy_test=PASS
    source_seam=PASS
    object_seam=PASS
    pinned_ps2_build=PASS
    ELF_SHA256=1ee11891c6f60a793230555ae87c99e319fbc593492f4447b68fcef4d9832c98
    PT_LOAD_SHA256=a3f90c558083087ebdb522b334cd47944562c2ae70c7ad3ae90d0240b0e0639d
    PT_LOAD_BYTES=484628
    ARTIFACT=h1-rfb-mux-prep-unqualified-elf
    ARTIFACT_ID=10085146497

The exact ELF is **unqualified build evidence only**. RFB has not yet been
activated on hardware.

## Next checkpoint

Build the Pi-side raw-byte VNC bridge around the same H1 session object while
keeping the PS2 RFB gate closed:

1. add channel-1 state and CREDIT handling to the Pi H1 reader;
2. connect one Pi-local/upstream VNC socket only when the selected profile asks
   for RFB;
3. forward VNC-server bytes to PS2 only within PS2-granted RFB credit;
4. forward PS2 channel-1 DATA back to the VNC socket;
5. keep this bridge independent of AUDIO/MPEG producers and scheduling policy;
6. add host tests using a fake VNC endpoint so framing, credit exhaustion,
   replenishment, and bidirectional byte fidelity are proved before the PS2
   activation gate changes.

Only after that bridge is verified should CONFIG accept RFB ON and HELLO begin
advertising RFB capability for the first controlled RFB-only hardware test.
