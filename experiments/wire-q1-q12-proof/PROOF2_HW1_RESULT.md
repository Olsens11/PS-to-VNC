# Wire Q9/Q12 Proof 2 — HW1 Loss, Reconnect, and New Session Identity

## Classification

- Result: `HARDWARE_PROVEN`
- Scope: one active Wire Session is deliberately killed by the Pi, the same
  still-running PS2 process survives the loss, performs a fresh provisional
  establishment, and receives a different Pi-authoritative Wire Session ID.
- Source commit:
  `f9820f6d8a8233a097a1d19cc138ea244a800b03`
- Hardware test ID: `WIRE-Q1Q12-P2-HW1`
- Run ID: `P2-HW1-20260919T031651Z`

This proof deliberately starts no RFB, audio, MPEG, configuration, or desktop
rider. It isolates Wire Session lifetime and reconnect behavior from rider
lifecycle behavior.

The proof-local establishment representation remains development apparatus and
is not promoted into the final product Wire ABI by this result.

## Exact hardware DUT

Pristine committed-source build:

    ELF_SHA256=788a5b8c090a5958722ce0da2d1897d46e65365e2543c909c3dc2b5e69748b3e
    ELF_BYTES=2301824
    PT_LOAD_SHA256=58bc46021669891ee3f2b5c308788f865b1c4d96875bff2a520e7e66abf23ae1
    PT_LOAD_BYTES=366216

Identity-stamped hardware DUT:

    TEST_ID=WIRE-Q1Q12-P2-HW1
    ELF_SHA256=97abb713ddecba2678d56b42f8f47bb4a1b8b58706ebb18890ea47b14fb5ff6b
    ELF_BYTES=2301824
    PT_LOAD_SHA256=7a0c62e6f32fb48ba813766088217c183109074849db52ce1e228aca99afe06e
    PT_LOAD_BYTES=366216

The generic deployment tool wrote both its unique test path and rolling
`/mass/0/PS2VNC.ELF`, read both back, and verified the exact stamped SHA-256.

The previous rolling ELF was preserved in the build/run directory before
deployment. That binary backup is not committed as evidence because its
identity is sufficient for this proof and it is a generated binary artifact.

## Pi development apparatus

The foreground two-session proof server was:

    experiments/wire-q1-q12-proof/proof2_reconnect_server.py

Its source SHA-256 was:

    2ea35f833a1f533dfa5b7bb17c7bf05570770bd68d554171fbbc02b44a720a92

It listened on:

    0.0.0.0:5902

No Pi systemd service or network configuration was changed.

The server deliberately retained its listener while terminating only the
physical connection for Session A.

## Machine evidence

Session A:

    WIRE_PROOF_A_PROVISIONAL peer=192.168.50.2:52432
    WIRE_PROOF_A_HELLO wire_version=1 product_version=1
    WIRE_SESSION_A=ACTIVE session_id=2897272434
    WIRE_RIDERS_A=0 rfb=0 audio=0 mpeg=0 config=0 desktop=0

The Pi then deliberately terminated that physical connection:

    WIRE_PROOF_FORCED_LOSS session=A session_id=2897272434 after_seconds=3.000000
    WIRE_SESSION_A=INACTIVE former_session_id=2897272434 reason=PI_FORCED_CLOSE
    WIRE_PROOF_WAITING_FOR_RECONNECT=YES

Without a new PS2 ELF launch, the Pi then observed a new TCP peer port and a
fresh provisional handshake:

    WIRE_PROOF_B_PROVISIONAL peer=192.168.50.2:52433
    WIRE_PROOF_B_HELLO wire_version=1 product_version=1
    WIRE_SESSION_B=ACTIVE session_id=4227936578
    WIRE_RIDERS_B=0 rfb=0 audio=0 mpeg=0 config=0 desktop=0

The server explicitly verified:

    WIRE_SESSION_IDS_DISTINCT=YES session_a=2897272434 session_b=4227936578
    WIRE_RECONNECT_PEER session_a=192.168.50.2:52432 session_b=192.168.50.2:52433
    WIRE_PROOF_SESSION_B=PASS elapsed=3.004191 ordinary_post_establishment_bytes=0
    WIRE_PROOF2_RECONNECT=PASS

## Operator observation

The operator launched the PS2 ELF once and did not reset or relaunch it between
Session A and Session B.

The final visible PS2 screen showed:

    WIRE Q9/Q12 PROOF 2
    state=BOOT
    state=NETWORK_INIT_ONCE
    state=SESSION_A_TCP_CONNECT
    state=SESSION_A_PROVISIONAL
    SESSION_A ACTIVE id=2897272434
    riders=NONE
    state=WAIT_FOR_PI_FORCED_LOSS
    SESSION_A LOST retained_id=2897272434
    PROCESS_SURVIVED_WIRE_LOSS=YES
    state=SESSION_B_TCP_CONNECT
    state=SESSION_B_PROVISIONAL
    SESSION_B ACTIVE id=4227936578
    SESSION_A_RETAINED id=2897272434
    SESSION_IDS_DISTINCT=YES
    riders=NONE
    session_b_idle=3_seconds
    SESSION_B INACTIVE
    PROOF2 RECONNECT COMPLETE

The PS2's Session A and Session B IDs exactly matched the two authoritative
session IDs independently recorded by the Pi.

## Physically established

Within this no-rider experiment:

1. Session A was genuinely ACTIVE with a non-zero Pi-authoritative ID.
2. The Pi could kill the physical connection for Session A without killing the
   PS2 application process.
3. The PS2 retained the old Session A identity as historical local state after
   the physical session died.
4. The same PS2 process created a new TCP connection without reinitializing the
   whole network stack.
5. The new TCP connection began as a new provisional establishment.
6. A fresh HELLO/ACCEPT exchange created Session B.
7. Session B received a new non-zero Pi-authoritative ID.
8. Session B's ID was different from Session A's ID.
9. No rider was required for either session.
10. No ordinary post-establishment traffic was required for Session B to remain
    active during the bounded idle window.

This establishes the key Q9 session-lifetime behavior that reconnect creates a
new Wire Session rather than resuming the dead one.

It also establishes the prerequisite session-identity replacement needed by
Q12, but it does not by itself prove Q12's rider/runtime stale-work fencing.

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

- stale rider/runtime work being fenced after Wire Session replacement;
- a session-bound RFB runtime being destroyed and recreated;
- PCM runtime replacement;
- MPEG generation teardown or restart across Wire loss;
- queued old-session payload contamination prevention;
- continuously-running/systemd Pi Wire-server packaging;
- long-duration repeated reconnect endurance;
- physical Ethernet carrier loss as distinct from TCP/session loss;
- final product reconnect retry/backoff policy;
- final product numeric representation of proof-local establishment frames.

## Preserved raw evidence

    evidence/P2-HW1-20260919T031651Z/authority.env
    evidence/P2-HW1-20260919T031651Z/deployment.json
    evidence/P2-HW1-20260919T031651Z/wire-server.log

SHA-256:

    authority.env=a40afb0a5623a4b2abcc7d0bfe791cd8f0c392cab24d82631e897b0fcb0c519c
    deployment.json=ae22f8f11663da2d1cd36365ab71493f41539e5872587651086e7d472bf9a9cc
    wire-server.log=9c69cf27cd1da704c77b82c622317c4212e483aa1e532156d1a5d4b145c9c783
