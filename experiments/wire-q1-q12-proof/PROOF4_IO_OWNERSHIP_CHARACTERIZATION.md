# Proof 4 physical I/O ownership characterization

## Scope

This records a bounded real-hardware characterization of the PS2 Wire
physical-I/O ownership problem discovered during Proof 4.

It does not claim that Q8, Q11, or Q12 are complete.

## Split-thread negative evidence

The exact P4 hardware DUT was:

    SHA256=5b842b481941020ed283f1a8731ca9077d327d7a916d2a47c6120eb97171469f

The clean Transport runtime used a receiver EE thread for physical receive
while Transport output could be emitted by another EE execution context on the
same TCP socket.

Two complementary diagnostic captures preserve the negative result.

### All-port control

Session:

    session_id=993404672

This run captured every Pi/PS2 Ethernet packet rather than only 5902.

It reproduced:

    AUDIO_PROGRESS_WHILE_RFB_FULL=PASS
    AUDIO_LOCAL_FAILURE_AND_FULL_RELAY_MARKER=PASS
    RFB_FIRST_DRAIN_AFTER_AUDIO_FAILURE=PASS
    WIRE_PROOF4_RIDER_ISOLATION=FAIL detail=timed out

The all-port capture found:

    NON_5902_TCP_PACKET_COUNT=0

Therefore no independent resident Pi TCP stream was competing for the PS2
network path during this reproduced failure.

The TCP trace ends useful PS2 transmit progress at sequence range 6630:6646,
the Pi ACKs through 6646, and the Pi repeatedly retransmits the same outstanding
second-RFB range while no further useful PS2 TCP progress occurs.

### Raw packet-level TX diagnostic

Session:

    session_id=2666041766

This reused the exact same P4 DUT and changed only the Pi socket receive timeout
from 15 seconds to 60 seconds.

The raw -X packet decode shows the PS2 transmitting:

    PSTV magic=PSTV
    kind=CREDIT
    channel=RFB
    sequence=7
    payload_length=4

as one 16-byte TCP payload:

    TCP seq 6630:6646

The Pi ACKs through 6646.

The expected following four-byte CREDIT payload would occupy TCP sequence
6646:6650. No such PS2-to-Pi payload packet appears in the capture.

Meanwhile the Pi repeatedly retransmits the outstanding second-RFB segment:

    TCP seq 2293371915:2293371947
    ack 6646

The 60-second server still ends:

    WIRE_PROOF4_RIDER_ISOLATION=FAIL detail=timed out

This is direct packet-level evidence of a reproducible physical-I/O liveness
failure in the split-thread P4 path.

It does not by itself establish that PS2IP thread safety is the sole root cause.

## Single-owner positive evidence

P4B source commit:

    c2150c5e7f19512dac725051e7acd068235ee743

Stamped DUT:

    TEST_ID=WIRE-Q1Q12-P4B-HW1
    SHA256=e70584006f34f3fcd7c0efa7f4c0ab06a734f522b928991ddce52788fd4d749f

One EE execution context performed every physical socket send and receive.

The real Wire physical-stream/framing and real PS2IP path were retained.

Pi machine evidence:

    SECOND_RFB_WINDOW_SENT=PASS pi_sequences=6,7
    SEQ7_CREDIT_HEADER_AND_PAYLOAD=PASS amount=32 ps2_sequence=7
    OUTBOUND_RFB=PASS bytes=37 fragments=16+16+5 ps2_sequences=8,9,10
    WIRE_SESSION=INACTIVE former_session_id=4284470743
    WIRE_PROOF4B_SINGLE_IO_OWNER=PASS

The all-port P4B capture independently contains the complete sequence-7 CREDIT
and post-sequence-7 RFB DATA frames.

It also observed no competing non-5902 PS2 TCP stream.

Operator observation showed:

    PHYSICAL_IO_OWNER=ONE_EE_THREAD
    SECOND_RFB_WINDOW=32
    SEQ7_CREDIT_COMPLETE=YES
    OUTBOUND_RFB=16+16+5
    SINGLE_IO_TRANSACTION=PASS
    WIRE INACTIVE
    PROOF4B IO OWNER COMPLETE

## Hardware-supported conclusion

The evidence supports these bounded statements:

1. The current split-thread P4 physical-I/O path reproducibly enters a TCP/Wire
   liveness failure during this transaction.
2. The failure is reproduced without a parallel resident Pi TCP stream.
3. Packet evidence shows the split-thread failure reaching the sequence-7
   CREDIT header but not its declared four-byte payload.
4. The same critical Wire transaction completes through the real
   physical-stream and PS2IP path when one EE execution context owns physical
   socket I/O.
5. Single physical-I/O ownership is therefore a credible hardware-safe
   Transport mechanism that should be carried into the next integration proof.

## Limitation

This is not treated as a perfect one-variable A/B.

P4B also bypasses the full clean Transport runtime, bounded Relay queues, and
domain behavior.

Therefore:

    ROOT_CAUSE_PS2IP_THREAD_SAFETY=NOT_PROVEN

and the result does not yet prove:

- Q8 PCM failure containment under the single-I/O-owner mechanism;
- Q11 independent rider behavior under that mechanism;
- Q12 stale-runtime fencing;
- final product Transport integration;
- endurance under production traffic.

## Candidate Transport invariant

Hardware evidence now justifies directly testing:

    exactly one Transport-owned physical-I/O execution context performs
    both socket receive and socket send operations for one Wire Session

Rider/domain code should communicate with that owner through bounded logical
channel state and outbound work rather than touching the socket.

## Next proof

Proof 4C should restore the real bounded RFB/AUDIO Relay semantics and real PCM
playback-core failure semantics while retaining one physical-I/O owner.

That is the next discriminator for Q8/Q11.

## Classification

    P4_SPLIT_THREAD_STALL_MACHINE_EVIDENCE=PASS
    P4_PARALLEL_TCP_STREAM_CONFOUNDER=NOT_OBSERVED
    P4_PARTIAL_SEQ7_PACKET_EVIDENCE=PASS

    P4B_SINGLE_IO_OWNER_MACHINE_EVIDENCE=PASS
    P4B_OPERATOR_OBSERVED=PASS
    P4B_DISCRIMINATOR_HARDWARE_PROVEN=YES

    ROOT_CAUSE_PS2IP_THREAD_SAFETY=NOT_PROVEN
    SINGLE_IO_OWNER_HARDWARE_SAFE_CANDIDATE=YES

    Q8_HARDWARE_PROVEN=NO
    Q11_HARDWARE_PROVEN=NO
    Q12_TESTED=NO

## Raw evidence identities

All-port split-thread control:

    server=4c9de8ac5569fc3ca248907818f32e1ca0a23c03b4013bf588bd39a93fdc0c6a
    pcap=948bb330d2e614340a8732f002e5fe9b0718e7b274d9f177ca7d09bb196e1894
    text=2c9708152e946d68cecfcab8c6defa92082fb0edcf2ded4b164af74fd94a8dc1
    flow_summary=c6af00a35e3c4f9f7636477acbf257faa07cfacb48c5ef291cc8d0674db1e3d5
    tcpdump=c0971d0e87d4fae17a33f30a8716f393c75e4b159a0ae2b312fe60c602d0b4b8
    server_copy=5942109a7b65570bdec8a9738dab8e2e5f49e43899b55af53a3702f6619f867d

Raw split-thread TX diagnostic:

    authority=ee35284da1197c6f1043db3440553909feb28b581e6f7cc8167583d81d22c26e
    server=f1a40a06db98daaaa8bf05100ede5a336b75ca71904764f77d99b7570234c00e
    pcap=b717de99c21d095b0397761221088051bf6e6617585678bf04860e363429446f
    decoded=49f4ad54a5e12a5de0a233828d971b0b8b850abd09bfdefed9fd0b8bd65b04b8
    payload_summary=3f647ea52a983647596e0e71c682237dbda2b1054fe46913d9de37faf1ef313a
    tcpdump=2fdb2c849aa7169d6796747ca2cee1329e19b66c60ce21f718ff463ecc249ec3
    server_copy=5942109a7b65570bdec8a9738dab8e2e5f49e43899b55af53a3702f6619f867d

Single-owner P4B:

    authority=5a92a08d266d48759b13d3625d1e1f398a40e59e0e8bc7ce068f74f7239c4a3e
    server=86acc1360f1fc9866c6a179fd49965f7124135c90361d1bef36fb90dc09b5d35
    pcap=5b01e30538c8c014f091c72a0d93d7678a163c816bed992b9829a7fce3efb192
    text=e2979b661a0122d895c9640eb2a34151b69a18fd2204ab8d15cafb4cf0fc2a57
    deployment=53d831b3cb437234da3e4e37cf1e5fc3c84f9c40decf3fce4885fd76d2346b82
    preparation=b7164d13ba5c997cb7bca8652ff7128e7b26c20707bd3bf89024ae6ea8b34af9
    tcpdump=9f7f752d79ab7aeaf04a3aa0d630c1e255a1537bb26de5d1d7cf9abaeab02f80
