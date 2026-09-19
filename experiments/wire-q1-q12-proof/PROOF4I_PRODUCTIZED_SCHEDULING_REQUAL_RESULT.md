# Proof 4I — productized cooperative-idle-yield requalification

## Purpose

The original Proof 4I established on physical PS2 hardware that the clean
single-physical-I/O-owner Transport needs a real EE scheduling opportunity after
an otherwise idle readiness-poll cycle.

Commit `d226b59ba54ef2e3f114906564289dada132b361` then productized only that
scheduling invariant:

- one Transport-owned physical framed-I/O execution context remains;
- outbound work is still checked before socket readiness waiting;
- readable inbound Wire data still bypasses the cooperative delay;
- an idle readiness timeout cooperatively yields EE execution;
- the currently tested implementation value remains 1000 microseconds;
- that value is implementation policy, not Wire ABI and not a declared final
  optimum.

This requalification physically exercises that committed productized scheduling
implementation before moving to the remaining Q12 and MPEG hardware boundaries.

## Authority

    PRODUCT_COMMIT=d226b59ba54ef2e3f114906564289dada132b361
    RUN_ID=P4I-PROD-REQUAL-HW1-20260919T220651Z
    SESSION_ID=2983844695

    PRODUCT_RUNTIME_SHA256=4af9b45b561974e95340d0e1151948011293e5f72e94b27543aa4c978c5dd465

    DUT_ELF_SHA256=3eee608ef7b7956b703c6a049276aeacfc2cc34ec13f9e9391c139884fd01b8c
    DUT_PT_LOAD_SHA256=ffa20b2aab8b1000a69cd3139e478e16492e08cd03fe1051c8f05922ca448aba

## Product source versus development apparatus

Q4 Wire establishment has not yet been promoted into the clean product runtime.

Therefore the requalification DUT is not represented as an exact standalone
product ELF. The architecture under test is the exact committed productized
Transport scheduling source plus the minimum previously proven proof-local P4E
establishment bootstrap.

The construction was bounded and checked:

    PRODUCT_RUNTIME_SHA256=4af9b45b561974e95340d0e1151948011293e5f72e94b27543aa4c978c5dd465
    P4I_PROOF_APPARATUS_SHA256=65d386e8e9de0e3237af3ec1769a603fb9e1b48d997c1598de2143a7d9a706ee
    P4E_ESTABLISHMENT_BLOCK_SHA256=f1c39ce971035a8334b9d427413a0fe0aa1d93312f9c50a6210b3d436bda4982

Removing the injected proof-local establishment block recovers the committed
`src/transport/runtime.c` byte-for-byte.

The P4 transaction harness is byte-identical to the original hardware-proven
P4I harness. The corrected cumulative-credit validator is also byte-identical to
the previously sealed P4I validator.

## Executable continuity

The productized candidate's pristine executable file differs from the original
P4I pristine ELF in non-loaded ELF material, but its complete loadable program
image reproduced exactly:

    PRODUCTIZED_PRISTINE_ELF_SHA256=3d1c2e426eb0ac9baa081d708530ea1c466bea400dbb67c7d63816cd917fa250
    ORIGINAL_P4I_PRISTINE_ELF_SHA256=e6ff306b7a7774783ff55b2947c41dce2a7540e4d44cc4b80ebaa9b5ac96ef73

    PRODUCTIZED_PRISTINE_PT_LOAD_SHA256=8b9490d6956025c2f573d9660edb4e3b1fb4be2fd27c62fe6b83e654c20997fa
    ORIGINAL_P4I_PRISTINE_PT_LOAD_SHA256=8b9490d6956025c2f573d9660edb4e3b1fb4be2fd27c62fe6b83e654c20997fa

Because identity stamping embeds an ELF-derived digest into the loaded identity
blob, the freshly stamped productized DUT had a different PT_LOAD identity from
the historical P4I DUT. It was therefore physically re-run rather than treating
the prior hardware result as sufficient.

## Deployment authority

The freshly stamped DUT was deployed to the PS2 by FTP with exact readback
verification.

    UNIQUE_READBACK_SHA256=3eee608ef7b7956b703c6a049276aeacfc2cc34ec13f9e9391c139884fd01b8c
    ROLLING_READBACK_SHA256=3eee608ef7b7956b703c6a049276aeacfc2cc34ec13f9e9391c139884fd01b8c

The rolling launch target was:

    /mass/0/PS2VNC.ELF

The corrected proof server was listening before the DUT was launched.

## Machine evidence

The Pi proof server observed exactly one complete accepted session:

    WIRE_SESSION=ACTIVE session_id=2983844695
    INITIAL_CREDITS=PASS rfb=32 audio=16 ps2_sequences=2,3
    RFB_RELAY_FILLED bytes=32 fragments=16+16 pi_sequences=2,3
    AUDIO_PROGRESS_WHILE_RFB_FULL=PASS bytes=4 returned_credit=4 ps2_sequence=4
    AUDIO_RELAY_REFILLED_AFTER_PLAYBACK_READ bytes=16 pi_sequence=5
    AUDIO_LOCAL_FAILURE_AND_FULL_RELAY_MARKER=PASS ps2_sequence=5
    RFB_FIRST_DRAIN_AFTER_AUDIO_FAILURE=PASS credit=32 ps2_sequences=6 credit_amounts=32
    RFB_PROGRESS_WHILE_AUDIO_FAILED_AND_FULL=PASS bytes=32 pi_sequences=6,7 ps2_credit_sequences=7,8 credit_amounts=16+16
    RFB_OUTBOUND_AFTER_AUDIO_FAILURE=PASS bytes=37 fragments=16+16+5 ps2_sequences=9,10,11
    WIRE_SESSION=INACTIVE former_session_id=2983844695
    WIRE_PROOF4_RIDER_ISOLATION=PASS

The second RFB capacity return again arrived as two valid CREDIT frames totaling
32 bytes. The intentional PCM SERVICE_PLAY failure remained local while RFB
continued receive progress, returned capacity, transmitted outbound traffic, and
the Wire Session retired normally.

    MACHINE_EVIDENCE=PASS

## Operator observation

The operator supplied a physical PS2 television photograph immediately after the
same run.

Visible final-state evidence included:

    ESTABLISHMENT_OWNER=TRANSPORT_IO_THREAD
    OWNER_ESTABLISHMENT_AND_INITIAL_CREDITS=PASS
    RFB_CREDIT=32 AUDIO_CREDIT=16
    state=RFB_STALLED_FULL
    AUDIO_PROGRESS_WHILE_RFB_FULL=YES
    AUDIO_FAILURE=SERVICE_PLAY
    AUDIO_CLEANUP=PASS
    AUDIO_FAILURE_LOCAL=YES
    WIRE_AFTER_AUDIO_FAILURE=ACTIVE
    AUDIO_RELAY_FULL_AFTER_FAILURE=16
    AUDIO_FAILURE_MARKER_SENT=YES
    RFB_FIRST_DRAIN_AFTER_AUDIO_FAIL=32
    RFB_PROGRESS_WITH_FAILED_AUDIO=YES
    RFB_OUTBOUND_AFTER_AUDIO_FAIL=37
    TRANSPORT_FAILED=NO
    WIRE_HEALTHY_WITH_AUDIO_FAILED=YES
    WIRE INACTIVE
    PROOF4 RIDER ISOLATION COMPLETE

The photograph remains conversational operator evidence rather than a repository
binary artifact.

    OPERATOR_OBSERVED=PASS

## Hardware-supported conclusion

For the bounded P4 RFB/PCM workload, the committed productized Transport
scheduling mechanism is now hardware-qualified:

1. the single Transport physical framed-I/O owner completes the full workload;
2. the product outbound rendezvous continues to make progress;
3. the PollSema/select owner loop remains viable when its idle path cooperatively
   relinquishes EE execution;
4. the intentional PCM failure remains local;
5. RFB continues both receive and transmit progress after that PCM failure;
6. the Wire Session remains healthy and retires normally.

Therefore:

    SOURCE_COMPLETE=YES
    HOST_TESTED=YES
    PS2_COMPILE_PASS=YES
    PS2_LINK_PASS=YES
    MACHINE_EVIDENCE=PASS
    OPERATOR_OBSERVED=PASS
    HARDWARE_PROVEN=YES

    PRODUCTIZED_COOPERATIVE_IDLE_YIELD_P4_SCOPE_HARDWARE_PROVEN=YES
    CLEAN_RUNTIME_SINGLE_IO_OWNER_P4_SCOPE_HARDWARE_PROVEN=YES
    Q8_PCM_FAILURE_CONTAINMENT_P4I_SCOPE_HARDWARE_PROVEN=YES
    Q11_RFB_PCM_INDEPENDENCE_P4I_SCOPE_HARDWARE_PROVEN=YES

## Important boundary on the claim

This is a hardware qualification of the productized Transport scheduling
mechanism, not a claim that the current standalone product ELF is already fully
hardware-qualified.

Q4 establishment remains proof-local in this run.

Accordingly:

    PRODUCT_Q4_ESTABLISHMENT_IMPLEMENTED=NO
    EXACT_PRODUCT_ELF_HARDWARE_PROVEN=NO

## Still unproven

This requalification does not establish:

- that 1000 microseconds is the final or optimal cooperative-yield interval;
- that an explicit delay is the final wake/wait mechanism;
- that PS2IP thread safety is the singular root cause of the earlier failures;
- partial-frame timeout hardening;
- MPEG participation or MPEG failure containment;
- Q12 stale-runtime/session fencing;
- final thread-priority optimization;
- long-duration production endurance.

Accordingly:

    PRODUCT_ABI_COMMITMENT_TO_1000US_YIELD=NO
    IDLE_YIELD_US_FINAL_OPTIMUM=NO
    FINAL_WAKE_MECHANISM_SELECTED=NO
    ROOT_CAUSE_PS2IP_THREAD_SAFETY=NOT_PROVEN
    PARTIAL_FRAME_TIMEOUT_HARDENING=NOT_INCLUDED
    MPEG_HARDWARE_PARTICIPATION=NOT_TESTED
    Q12_STALE_RUNTIME_FENCING_TESTED=NO
    PRIORITIES_CONSIDERED_FINAL=NO
    LONG_DURATION_ENDURANCE_TESTED=NO

## Next engineering boundary

The productized P4I scheduling mechanism no longer blocks subsequent Wire
hardware work.

The remaining explicitly unqualified architectural boundaries include Q12
session-bound stale-runtime fencing and MPEG participation/lifecycle behavior.
Those should be tested as separate bounded experiments rather than extending
this already-qualified P4I transaction.

## Raw evidence

The tracked evidence directory for this run is:

    experiments/wire-q1-q12-proof/evidence/P4I-PROD-REQUAL-HW1-20260919T220651Z/

`SHA256SUMS.txt` records the exact preserved evidence identities.
