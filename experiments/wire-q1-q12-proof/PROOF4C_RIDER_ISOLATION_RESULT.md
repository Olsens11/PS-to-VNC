# Proof 4C — RFB/PCM rider isolation with one physical-I/O owner

## Authority

    SOURCE_HEAD=a8377f04803476d3486b1badfd6e57416196866c
    RUN_ID=P4C-HW1-20260919T093503Z
    SESSION_ID=2416923489

    HARDWARE_ELF_SHA256=d34a7b35d7699854fba298d184457e81a3c85f2ecb446e28577ba310a5e48ff1
    HARDWARE_PT_LOAD_SHA256=cc800a1fbd486549d8a0aa40bb1eb0ce3cddde3697c7922b8e33ebd6ac484518

## Scheduling control

    TRANSPORT_IO_EE_PRIORITY=63
    RFB_EE_PRIORITY=64
    PCM_EE_PRIORITY=65
    MPEG_EE_PRIORITY=67
    MPEG_THREAD_ACTIVE=NO

    PRIORITY_POLICY=BEST_KNOWN_TESTED_BASELINE_NOT_FINAL
    PRIORITIES_CONSIDERED_FINAL=NO
    PRIORITY_OPTIMIZATION_DEFERRED=YES

The priority tuple is held fixed as the best-known tested scheduling baseline.
This proof does not establish that these values are globally optimal or final.

## Architecture mechanism under test

Proof 4C preserves separate EE execution contexts for the active domain workers:

- Transport physical-I/O owner;
- RFB domain worker;
- PCM domain worker.

MPEG is intentionally inactive in this bounded proof.

Exactly one Transport-owned EE execution context performs physical Wire socket
send and receive operations.

RFB and PCM interact with bounded logical channel state and a proof-local
bounded outbound-work rendezvous rather than touching the physical socket.

The clean product Transport runtime is not linked in this discriminator because
its current receiver-thread / caller-send implementation is the split physical
I/O behavior being characterized.

The one-item outbound rendezvous and deterministic transaction state machine are
development apparatus, not a promoted final product ABI or final runtime design.

## Machine evidence

The Pi observed one accepted Wire Session:

    WIRE_SESSION=ACTIVE session_id=2416923489

The machine transaction proved:

    INITIAL_CREDITS=PASS
    RFB_RELAY_FILLED=32
    AUDIO_PROGRESS_WHILE_RFB_FULL=PASS
    AUDIO_LOCAL_FAILURE_AND_FULL_RELAY_MARKER=PASS
    RFB_FIRST_DRAIN_AFTER_AUDIO_FAILURE=PASS
    RFB_PROGRESS_WHILE_AUDIO_FAILED_AND_FULL=PASS
    RFB_OUTBOUND_AFTER_AUDIO_FAILURE=PASS
    WIRE_SESSION=INACTIVE
    WIRE_PROOF4_RIDER_ISOLATION=PASS

The injected PCM service-play failure completed with cleanup before the
post-failure marker. The failed AUDIO rider then remained full while the sibling
RFB rider drained, accepted another complete 32-byte window, returned the
formerly failing sequence-7 credit, and transmitted 37 outbound RFB bytes.

The physical Wire therefore remained usable after the PCM-domain failure.

The all-port capture recorded:

    NON_5902_TCP_PACKET_COUNT=0
    PARALLEL_TCP_STREAM_CONFOUNDER=NOT_OBSERVED

## Operator observation

The operator supplied a photograph of the P4C PS2 screen from the same run.

The photograph visibly identifies:

    WIRE Q8/Q11 PROOF 4C
    PRIORITY_BASELINE=63/64/65/67
    PRIORITIES_FINAL=NO
    MPEG_PRIORITY=67 INACTIVE
    TRANSPORT_IO_PRIORITY=63
    RFB_PRIORITY=64
    PCM_PRIORITY=65
    PHYSICAL_IO_OWNER=TRANSPORT_THREAD_ONLY
    WIRE ACTIVE id=2416923489
    INITIAL_CREDITS=32+16
    RFB_RELAY_FULL=32
    AUDIO_PROGRESS_WHILE_RFB_FULL=YES
    AUDIO_FAILURE=SERVICE_PLAY
    AUDIO_CLEANUP=PASS
    AUDIO_FAILURE_LOCAL=YES
    AUDIO_RELAY_FULL_AFTER_FAILURE=16
    AUDIO_FAILURE_MARKER_SENT=YES
    RFB_FIRST_DRAIN_AFTER_AUDIO_FAIL=32
    RFB_PROGRESS_WITH_FAILED_AUDIO=YES
    SEQ7_CREDIT_COMPLETE=YES
    RFB_OUTBOUND_AFTER_AUDIO_FAIL=37
    RFB_DOMAIN_THREAD=PASS
    TRANSPORT_FAILED=NO

The photograph's session ID matches the machine evidence.

## Hardware-supported conclusion

For the bounded RFB/PCM scope exercised by Proof 4C:

1. separate RFB and PCM EE domain threads can coexist with one Transport-owned
   physical-I/O EE thread;
2. a PCM service-play failure can remain local to PCM while cleanup completes;
3. a failed/full AUDIO rider does not prevent subsequent RFB receive progress,
   RFB credit return, or RFB outbound traffic;
4. the Wire Session can remain healthy through that local rider failure and
   retire normally;
5. the split-thread P4 stall is not reproduced when the same rider-isolation
   transaction is executed behind single physical-I/O ownership.

Therefore:

    MACHINE_EVIDENCE=PASS
    OPERATOR_OBSERVED=PASS
    P4C_HARDWARE_PROVEN=YES

    Q8_PCM_FAILURE_CONTAINMENT_P4C_SCOPE_HARDWARE_PROVEN=YES
    Q11_RFB_PCM_INDEPENDENCE_P4C_SCOPE_HARDWARE_PROVEN=YES

## Limits

This proof does not establish:

- that PS2IP thread safety is the singular root cause of the earlier P4 stall;
- final production Transport implementation details;
- final outbound queue depth or scheduling policy;
- optimal/final EE priorities;
- MPEG participation or MPEG failure containment;
- every future rider combination;
- Q12 stale-runtime/session fencing;
- long-duration production endurance.

Accordingly:

    ROOT_CAUSE_PS2IP_THREAD_SAFETY=NOT_PROVEN
    PRIORITIES_CONSIDERED_FINAL=NO
    FULL_MULTI_RIDER_PRODUCT_INTEGRATION_HARDWARE_PROVEN=NO
    MPEG_INCLUDED_IN_P4C=NO
    Q12_TESTED=NO

## Next engineering boundary

The next implementation step may carry the now-hardware-supported invariant
into the clean Transport runtime:

    one Transport-owned physical-I/O EE execution context owns all physical
    socket send and receive operations

while preserving independent RFB, PCM, and eventually MPEG domain EE threads.

Productionization must not simply serialize blocking recv/send with one mutex.
The Transport owner needs an explicit bounded outbound-work mechanism and an I/O
service policy that allows receive and outbound work to make progress.

The current 63/64/65/67 tuple should remain the development baseline during that
productionization unless a deliberate later scheduling experiment changes it.

## Raw evidence identities

    authority=7de501e3b58b11ed2aa9a7db6eae4cde0ee7cf540ea2ca8132cb9969a34a8f6e
    wire_server=d95eb6afd05008c5fb490be3deaaf74579ba9bbf39ce5ffda5962295c2663183
    server_pass_fail=9155616ba78675188af8e90261758442ed83423604a952152226a980ad17ad4b
    pcap=41d01c84d645add68dd185528a6fd22d501f2ce349109260082df6c8c9b8b46e
    pcap_text=fffb87162c1e4255ee9b39237ea7b380fa45e25e35fdb1af91cee0c63b586926
    deployment=785d89647e38d6a8f9dbd47c7cc1d0711390fefa7e3884a52d0d09d22ceca0ce
    preparation=30558411701de1ff7c2e76f305b1dcd0696ed8d2c134b8f1fdfa440f789f6352
