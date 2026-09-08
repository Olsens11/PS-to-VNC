# CP2C — dormant RFB runtime resources resident

Status: **COMPLETE — SOURCE/BUILD VERIFIED, NOT LIVE-RUNTIME-WIRED, NOT HARDWARE QUALIFIED**

This is chronological checkpoint evidence under the governing authority in
`RFB_MUX_INTEGRATION_PREP.md`. It does not replace that plan.

## Commits

    5d39d219d6a0e92f2f017ddb36762c38e32a0cfd  h1: define dormant RFB runtime resources
    2e76ecacd87de1edbe30e841bac7a516e4473ec9  h1: implement dormant RFB runtime resources
    2bb67fd46f43f94b963be36b39a907a219e804cb  h1: link dormant RFB runtime resources
    b409c083e5e8ed93f4261235fd912c77d06f1285  h1: verify dormant RFB runtime resources

## What changed

- Added an H1 experiment-owned resource bundle containing exactly:
  - one logical RFB channel state object;
  - one RFB queue-storage pointer;
  - one RFB queue mutex semaphore id;
  - one active flag.
- Activation with `enabled == 0` is a successful no-op and requires the bundle
  to remain completely inactive/unallocated.
- The prepared ON path allocates exactly the existing evidence-based
  `PSTVNC_H1_RFB_QUEUE_REFERENCE_BYTES` (32768 bytes), creates one mutex, and
  initializes the already host-tested logical RFB channel over that storage.
- Release deletes only that semaphore, frees only that storage, and returns the
  bundle to the explicit inactive state.
- The cumulative pinned PS2 build now compiles and links this dormant resource
  implementation, and the saved seam checker verifies its symbols in the PS2
  object.
- `h1_config.c` still rejects every `rfb_mode` except `PSTVNC_H1_RFB_OFF`.
- `h1_rfb_mux_io.c` remains fail closed. The new resource functions are not
  called from H1 startup, receiver dispatch, credit return, or send paths.

Therefore CP2C changes resident PT_LOAD/source population but does **not**
activate channel 1 or alter the one-physical-PSTV-stream ownership model.

## Verification

    workflow=H1 RFB mux preparation checks
    run=34255172496
    result=PASS
    host_preflight=PASS
    existing_logical_rfb_channel_test=PASS
    source_seam=PASS
    object_seam=PASS
    pinned_ps2_build=PASS
    ELF_SHA256=38b886beb22a1d094d06ae2cc55eea0df23cd83623f705f25eef23ff26124662
    PT_LOAD_SHA256=9ff9c69fa9a701737d30cb657137fa2ec859e6bf554ea0c386f6c7ece02e95d7
    PT_LOAD_BYTES=480916
    ARTIFACT=h1-rfb-mux-prep-unqualified-elf

The ELF is an **unqualified build artifact only**. No deployment or physical
PS2 claim is authorized by this checkpoint.

## What remains unproven

- embedding/calling the resource bundle from `h1_transport_runtime` lifecycle;
- sole-receiver dispatch of PSTV channel-1 DATA into the RFB queue;
- actual channel-1 CREDIT frames;
- actual outbound channel-1 DATA through H1's existing send semaphore;
- mux-adapter exact-read/poll/write operation;
- Pi VNC byte-stream bridge behavior;
- any `RFB=ON` runtime behavior;
- any real-PS2 behavior of this changed PT_LOAD.

## Policy gap deliberately not guessed in CP2C

The current H1 CONFIG schema has AUDIO/MPEG initial-credit, batching, flush, and
return-policy fields, but no corresponding RFB credit-policy fields. CP2C does
not invent an RFB initial-credit or batching contract. That choice must be made
explicitly before live channel-1 credit transmission is enabled.

## Next recommended checkpoint

Embed the already-resident RFB resource bundle into `pstvnc_h1_transport_runtime_t`
and initialize/release it along the transport lifecycle while **continuing to
reject `rfb_mode=ON`**. This establishes concrete ownership in the actual H1
runtime without yet choosing RFB credit wire policy or enabling receiver/send
behavior.

Only after that lifecycle wiring is build-verified should a later checkpoint
choose and implement channel-1 initial-credit/return semantics, receiver DATA
dispatch, and outbound DATA serialization through the existing H1 send lock.
