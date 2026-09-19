# Proof 4I — clean Transport single-I/O owner with cooperative idle yield

## Authority

    SOURCE_HEAD=325cf8332212042bc78170fc8d5776a442985006
    PASS_RUN_ID=P4I-HW2-RETRY-20260919T125705Z
    SESSION_ID=1357469748

    DUT_REUSED_FROM_RUN=P4I-HW1-20260919T123629Z
    HARDWARE_ELF_SHA256=d8775e025886699935c42fab90e526ef53cabcb537efe85d0e6612dafe12172f
    HARDWARE_PT_LOAD_SHA256=aa49d91d08d50557561501df1a0b4308dbac3ffff73a040f9e42c8984d77a68e

The passing retry reused the exact previously stamped P4I hardware ELF. It was not
rebuilt, restamped, or modified for this run.

## Architecture mechanism under test

P4I exercises the clean product Transport runtime with:

- one Transport-owned physical framed-I/O EE execution context;
- separate RFB and PCM domain execution;
- the real bounded product outbound rendezvous;
- the real PollSema/select Transport event loop;
- one cooperative EE scheduling delay after an idle readiness timeout.

The tested development values were:

    TRANSPORT_IO_EE_PRIORITY=63
    SELECT_TIMEOUT_US=1000
    COOPERATIVE_IDLE_YIELD_US=1000

These values are not declared final product policy.

MPEG is intentionally inactive in this bounded proof.

## Development apparatus

The proof still uses:

- proof-local Wire establishment ownership;
- the deterministic P4 RFB/PCM transaction;
- deterministic PCM SERVICE_PLAY failure injection;
- the Pi proof server;
- a corrected cumulative-credit proof oracle.

These remain development apparatus rather than promoted product ABI.

## Corrected cumulative-credit oracle

P4I HW1 completed the PS2-side transaction but the original P4 server assumed the
second 32-byte RFB capacity return had to arrive in one CREDIT frame.

The real product Transport permits the returned capacity to be split while
flush-on-empty is active.

The passing hardware run produced:

    first RFB drain:
        ps2_sequences=6
        credit_amounts=32

    second RFB drain:
        ps2_sequences=7,8
        credit_amounts=16+16

The corrected oracle requires one or more positive RFB CREDIT frames summing
exactly to 32 and derives subsequent expected sequence numbers from the actual
number of CREDIT frames.

It changes only proof validation. It does not change the PS2 ELF, Wire protocol,
Transport implementation, or P4 transaction.

## Machine evidence

The Pi proof server observed:

    WIRE_SESSION=ACTIVE session_id=1357469748

The full transaction reached:

    INITIAL_CREDITS=PASS rfb=32 audio=16
    RFB_RELAY_FILLED bytes=32 fragments=16+16
    AUDIO_PROGRESS_WHILE_RFB_FULL=PASS
    AUDIO_LOCAL_FAILURE_AND_FULL_RELAY_MARKER=PASS
    RFB_FIRST_DRAIN_AFTER_AUDIO_FAILURE=PASS credit_amounts=32
    RFB_PROGRESS_WHILE_AUDIO_FAILED_AND_FULL=PASS credit_amounts=16+16
    RFB_OUTBOUND_AFTER_AUDIO_FAILURE=PASS bytes=37
    WIRE_SESSION=INACTIVE
    WIRE_PROOF4_RIDER_ISOLATION=PASS

Because the second returned capacity used two CREDIT frames, the 37-byte outbound
RFB write followed on PS2 Wire sequences:

    9,10,11

The intentional PCM service-play failure remained local while RFB resumed receive
progress, returned capacity, transmitted outbound data, and the Wire Session
retired normally.

## Operator observation

The operator supplied a photograph of the completion screen from the same retry.

Visible markers include:

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
machine artifact.

## Hardware-supported conclusion

For the bounded RFB/PCM transaction exercised by P4I:

1. the clean single-physical-I/O-owner Transport can complete the full P4
   rider-isolation workload when its idle polling loop cooperatively yields EE
   scheduling time;
2. initial credits, returned credits, and outbound RFB traffic make progress
   through the same Transport I/O owner;
3. the injected PCM failure remains local and cleanup completes;
4. a failed/full AUDIO rider does not prevent subsequent RFB receive, credit-return,
   or outbound progress;
5. the Wire Session remains healthy and retires normally;
6. a 1000-us cooperative delay after an idle 1000-us readiness timeout is
   sufficient for this exact hardware workload.

Therefore:

    MACHINE_EVIDENCE=PASS
    OPERATOR_OBSERVED=PASS
    P4I_HARDWARE_PROVEN=YES

    COOPERATIVE_IDLE_YIELD_P4_SCOPE_HARDWARE_PROVEN=YES
    CLEAN_RUNTIME_SINGLE_IO_OWNER_P4_SCOPE_HARDWARE_PROVEN=YES

    Q8_PCM_FAILURE_CONTAINMENT_P4I_SCOPE_HARDWARE_PROVEN=YES
    Q11_RFB_PCM_INDEPENDENCE_P4I_SCOPE_HARDWARE_PROVEN=YES

## Independent packet-capture limitation

The independent tcpdump apparatus did not preserve packet records from the passing
retry.

    PCAP_BYTES=24
    PCAP_SHA256=704e5e5b3234433c01fcfd1b20a306e77e985038120492dc53965c3edd38a4ea
    INDEPENDENT_PCAP_EVIDENCE=UNAVAILABLE

The preserved PCAP contains only its global header.

The hardware classification therefore relies on the real Pi server's decoded TCP
Wire transaction together with the operator-observed PS2 completion screen. The
empty PCAP is not represented as packet evidence.

## Evidence classification

    SOURCE_COMPLETE=YES
    HOST_TESTED=NO
    PS2_COMPILE_PASS=YES
    PS2_LINK_PASS=YES
    MACHINE_EVIDENCE=PASS
    OPERATOR_OBSERVED=PASS
    HARDWARE_PROVEN=YES

The P4I scheduling derivative was not separately host-tested. Its relevant behavior
is PS2 EE scheduling, and it was compiled, linked, and physically exercised on the
PS2.

## Limits

This proof does not establish:

- that 1000 microseconds is the final or optimal product yield interval;
- that an explicit delay is the final product wake/wait mechanism;
- that PS2IP thread safety is the singular root cause of earlier failures;
- final production scheduling policy;
- optimal/final EE priorities;
- partial-frame timeout hardening;
- MPEG participation or failure containment;
- Q12 stale-runtime/session fencing;
- every future rider combination;
- long-duration production endurance.

Accordingly:

    PRODUCT_ABI_COMMITMENT_TO_1000US_YIELD=NO
    FINAL_WAKE_MECHANISM_SELECTED=NO
    ROOT_CAUSE_PS2IP_THREAD_SAFETY=NOT_PROVEN
    PRIORITIES_CONSIDERED_FINAL=NO
    MPEG_HARDWARE_PARTICIPATION=NOT_TESTED
    Q12_STALE_RUNTIME_FENCING_TESTED=NO
    LONG_DURATION_ENDURANCE_TESTED=NO

## Next engineering boundary

The hardware evidence establishes that the clean single-I/O-owner Transport needs
a scheduling point that actually relinquishes EE execution often enough for the
PS2 networking stack to make progress.

Productionization should preserve that invariant without prematurely freezing
1000 microseconds as a permanent architectural/API constant.

The smallest production candidate should retain:

    one Transport-owned physical framed-I/O execution context
    separate RFB / PCM / MPEG domain execution contexts
    bounded Transport-owned outbound work
    cooperative network-stack scheduling opportunity

and then be requalified before moving to the remaining Q12 and MPEG hardware
boundaries.

## Raw evidence identities

The raw evidence directory contains its canonical SHA256SUMS.txt manifest.
