# Wire Q3 Proof 3 — HW1 Representative RFB Channel Relay

## Classification

- Result: `HARDWARE_PROVEN`
- Scope: representative Wire Channel Relay behavior for the logical RFB
  channel on real PS2 hardware.
- Source commit:
  `1bd5abbb8ba444cc1aaeebe0f2ff13be3a8484c2`
- Hardware test ID: `WIRE-Q1Q12-P3-HW1`
- Run ID: `P3-HW1-20260919T033824Z`

This is a bounded Q3 hardware proof. It proves important Relay mechanics and
ownership boundaries, but does not claim every part of Q3 is complete.

In particular, it does not yet prove that a second ordinary rider continues
while RFB is stalled. The continuing traffic in this proof is Wire CONTROL
traffic.

The experiment deliberately does not use the real RFB parser or a VNC
provider. Channel-1 payload is opaque test data so the Relay boundary itself
can be physically characterized without parser, framebuffer, provider, media,
or presentation behavior confounding the result.

## Architecture mechanism versus development apparatus

Reused clean product mechanisms:

- `src/transport/physical_stream.c`
- `src/transport/rfb_channel.c`
- `src/rfb/bridge.c`

The real physical stream owns the socket and one continuous PSTV sequence
space from establishment through the Relay traffic.

The real bounded logical RFB storage is reused directly.

The real RFB-facing public bridge supplies the domain exact-read/exact-write
boundary.

Development apparatus:

- proof-local Transport backing for the RFB bridge;
- foreground Pi proof server;
- fixed 32-byte Relay capacity;
- fixed 16-byte maximum DATA payload;
- proof-local establishment ACCEPT representation.

The clean product Transport runtime and clean product Transport bridge are not
linked into this proof. No final product Wire ABI is promoted by this result.

## Exact hardware DUT

Pristine committed-source build:

    ELF_SHA256=2100c8b8801912a00ad7449d3bfc11f17d5930ab3f377f8a02258b7ebe3edff8
    ELF_BYTES=2323840
    PT_LOAD_SHA256=6b6b4e1da05ec49ec993f9c3063b8d692d42819d2e60b7329b71ee8ae7bbfdfe
    PT_LOAD_BYTES=369544

Identity-stamped hardware DUT:

    TEST_ID=WIRE-Q1Q12-P3-HW1
    ELF_SHA256=6440bb2409e1bfdb57fcb983944a9a08ec0e1579a51d49750dac14c2c939aaff
    ELF_BYTES=2323840
    PT_LOAD_SHA256=9ffa271d17130842e1bad2ab53e782b05982184246a8f1415c13aea2caa63878
    PT_LOAD_BYTES=369544

The deployment tool wrote the unique test path and rolling
`/mass/0/PS2VNC.ELF`, read both back, and verified the exact stamped
whole-ELF identity.

The previous rolling Proof 2 ELF was preserved before deployment.

## Pi development apparatus

The foreground server was:

    experiments/wire-q1-q12-proof/proof3_rfb_relay_server.py

Source SHA-256:

    b79e38e9f627aea6cc4e58b6a4cd04e6b8065f04c5a218e00fbf099e848db27c

It listened temporarily on:

    0.0.0.0:5902

No Pi systemd service or network configuration was changed.

## One continuous Wire sequence

The proof intentionally adopts the physical socket into the real Transport
physical-stream owner before the establishment HELLO.

PS2-to-Pi sequence:

    1 = HELLO
    2 = initial RFB CREDIT
    3 = CONTROL heartbeat while RFB Relay is full
    4 = returned RFB CREDIT after domain acceptance
    5 = outbound RFB DATA, 16 bytes
    6 = outbound RFB DATA, 16 bytes
    7 = outbound RFB DATA, 5 bytes

Pi-to-PS2 sequence:

    1 = ACCEPT
    2 = inbound RFB DATA, 16 bytes
    3 = inbound RFB DATA, 16 bytes
    4 = CONTROL heartbeat while RFB Relay is full
    5 = final CONTROL heartbeat

There is no Transport handoff or sequence reset between establishment and
Relay activity.

## Machine evidence

The Pi recorded:

    WIRE_PROOF3_PROVISIONAL peer=192.168.50.2:52432
    WIRE_PROOF3_HELLO wire_version=1 product_version=1 ps2_sequence=1
    WIRE_SESSION=ACTIVE session_id=3058085999

The PS2 advertised exactly 32 bytes of channel-1 capacity:

    RFB_INITIAL_CREDIT=PASS amount=32 ps2_sequence=2

The Pi filled that entire advertised window with two 16-byte opaque DATA
frames:

    RFB_WINDOW_FILLED bytes=32 fragments=16+16 pi_sequences=2,3

Before the RFB domain accepted any queued bytes, Wire CONTROL traffic still
made round-trip progress:

    WIRE_PROGRESS_WHILE_RFB_STALLED=PASS ps2_sequence=3

The Pi then deliberately watched the connection while the Relay remained full
and the domain remained stalled. No frame, including no returned RFB CREDIT,
arrived during that window:

    RFB_NO_EARLY_RETURN_CREDIT=PASS window_seconds=2.000

Only after the PS2 accepted the queued bytes through the RFB public boundary
did the Pi receive the 32-byte returned credit:

    RFB_RETURN_CREDIT_AFTER_DOMAIN_ACCEPT=PASS amount=32 ps2_sequence=4

The opposite-direction opaque byte stream then crossed the same RFB public
boundary. Transport delivered the 37 bytes as three channel-1 DATA frames and
the Pi reassembled the exact expected bytes:

    RFB_OUTBOUND_REASSEMBLY=PASS bytes=37 fragments=16+16+5 ps2_sequences=5,6,7

The Pi then sent a final CONTROL heartbeat on the same Wire Session and the
session shut down cleanly:

    WIRE_FINAL_HEARTBEAT_SENT pi_sequence=5
    WIRE_SESSION=INACTIVE former_session_id=3058085999
    WIRE_PROOF3_RFB_RELAY=PASS

## Operator observation

The operator launched the stamped PS2 ELF once.

The final visible PS2 screen showed:

    WIRE Q3 PROOF 3 RFB RELAY
    state=BOOT
    state=NETWORK_INIT
    state=TCP_CONNECT
    state=PROVISIONAL
    WIRE ACTIVE session_id=3058085999
    RFB_RELAY capacity=32 max_payload=16
    RFB_INITIAL_CREDIT=32
    WIRE_RECEIVED_RFB_BYTES=32
    RFB_RELAY_AVAILABLE=32
    RFB_DOMAIN_ACCEPTED=0
    state=RFB_DOMAIN_STALLED
    WIRE_PROGRESS_WHILE_RFB_STALLED=YES
    RFB_DOMAIN_ACCEPTED=32
    RFB_DOMAIN_PROCESSED=32
    RFB_RETURN_CREDIT=32
    RFB_OUTBOUND_BYTES=37
    RFB_OUTBOUND_FRAGMENTS=16+16+5
    WIRE_HEALTHY_AFTER_RELAY=YES
    WIRE INACTIVE
    PROOF3 RFB RELAY COMPLETE

The visible session ID exactly matched the Pi-authoritative session ID.

The operator-provided photograph is conversational evidence; the repository
preserves this textual observation rather than pretending the photograph is a
machine-generated repository artifact.

## Physically established

Within this bounded representative Relay experiment:

1. One established Wire Session remained the physical owner from HELLO through
   all Relay traffic.
2. Transport advertised finite RFB capacity through credit.
3. The Pi could fill that advertised Relay capacity exactly.
4. Wire receipt and domain acceptance were physically distinct events.
5. Filling the RFB Relay did not inherently stop Wire CONTROL progress.
6. Merely receiving/queuing the bytes did not return capacity credit.
7. Credit was returned only after the RFB public boundary accepted the bytes.
8. The accepted inbound bytes remained byte-for-byte intact.
9. Opaque outbound bytes crossed the RFB public boundary without RFB parsing.
10. Transport fragmented the 37 outbound bytes as 16 + 16 + 5 DATA payloads.
11. The Pi reassembled the exact expected outbound byte string.
12. The same Wire Session remained healthy after the Relay transaction.

These observations establish the core representative Q3 Wire Channel Relay
mechanism on real hardware.

## Evidence classification

    SOURCE_COMPLETE=YES
    HOST_TESTED=YES
    PS2_COMPILE_PASS=YES
    PS2_LINK_PASS=YES
    MACHINE_EVIDENCE=PASS
    OPERATOR_OBSERVED=PASS
    HARDWARE_PROVEN=YES

## Still unproven

This result does not establish:

- actual RFB protocol parsing over this Relay;
- a real Pi-local VNC/RFB provider adapter;
- framebuffer decode or rendering;
- a second ordinary rider continuing while RFB is stalled;
- rider-specific failure/restart containment;
- Q12 stale runtime or stale asynchronous-work fencing;
- product integration of Q4 establishment with the clean Transport runtime;
- final product reconnect/backoff policy;
- PCM or MPEG channel behavior;
- MPEG generation identity or retirement;
- final product Wire ABI numbering for proof-local establishment frames;
- long-duration Relay endurance or pathological backpressure timing.

## Preserved raw evidence

    evidence/P3-HW1-20260919T033824Z/authority.env
    evidence/P3-HW1-20260919T033824Z/deployment.json
    evidence/P3-HW1-20260919T033824Z/prepare-hardware.env
    evidence/P3-HW1-20260919T033824Z/wire-server.log

SHA-256:

    authority.env=1dee3366f6c25c58c361064ef988c719af9dfbb1bd65f05cab3b8c90ef8402a5
    deployment.json=dbaf242636887d5767ad96eb43ecf825685f8084f05204463071b463668b7d40
    prepare-hardware.env=8dd83de9a4c0ebfaa7f35b08f2640a98dd32bbb60d046283f2fa815032c0b12a
    wire-server.log=6e0098db7b4fd0c9fbce137d4e1b2a4775237c2520a6f39b7107dc84d907a780
