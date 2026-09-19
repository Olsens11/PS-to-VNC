# Wire Q1-Q12 Proof 1A — HW1 Result

## Classification

- Result: `HARDWARE_PROVEN`
- Scope: accepted establishment and idle Wire Session behavior only.
- Source branch: `wire/q1-q12-hardware-proof`
- Source commit: `9943af768f9844505753078a30b9eeb7dd7a8573`
- Hardware test ID: `WIRE-Q1Q12-P1A-HW1`

This result does not promote the proof-local ACCEPT/NOT_ACCEPTED numeric frame
identities into the product Wire ABI.

## Exact DUT

    ELF=PS2VNC-Wire-Q1Q12-Proof1A-HW1.ELF
    ELF_SHA256=de22b52f1c2c2b5895a2efe0c7c5faa689a78792c4b5259c7b9385c8dcdf730c
    ELF_BYTES=2297184
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=3b43585e1dd4a26650c132c33010698cd556cbbbade9c925a9de3e3227b5ab3a
    PT_LOAD_BYTES=365064

## Pi development apparatus

The one-shot foreground server was:

    experiments/wire-q1-q12-proof/proof1_wire_server.py

    SHA256=2ec01484bed4f7d487f37859efd2a27538d0dea242ace23bf2151865280a53f1
    listen=0.0.0.0:5902

No Pi service was installed, removed, enabled, disabled, or reconfigured.

## Deployment

The generic TestKit deployer wrote and read back the exact stamped ELF at:

    /mass/0/PS2VNC-WIRE-Q1Q12-P1A-HW1-de22b52f.ELF
    /mass/0/PS2VNC.ELF

Both readbacks matched:

    SHA256=de22b52f1c2c2b5895a2efe0c7c5faa689a78792c4b5259c7b9385c8dcdf730c
    BYTES=2297184

The prior rolling ELF was preserved before replacement:

    SHA256=2e7bcdf5542c0568be16fc7b3e433b40e2ab1f90b3a47c20087cf8a4f1868b9a
    BYTES=3106560

## Machine evidence

The real PS2 connected from `192.168.50.2:52432`.

The Pi observed in order:

    WIRE_PROOF_PROVISIONAL peer=192.168.50.2:52432
    WIRE_PROOF_HELLO wire_version=1 product_version=1
    WIRE_SESSION=ACTIVE session_id=2738949150
    WIRE_RIDERS_STARTED=0 rfb=0 audio=0 mpeg=0 config=0 desktop=0
    WIRE_PROOF_IDLE=PASS elapsed=10.003628 ordinary_post_establishment_bytes=0
    WIRE_SESSION=INACTIVE former_session_id=2738949150

This physically establishes for the accepted path:

1. the console initiates TCP to the Pi Wire endpoint;
2. the connection is provisional before establishment;
3. Wire framing is used from the first application traffic;
4. the console initiates establishment with Wire/product versions;
5. the Pi validates before activation;
6. the Pi assigns an authoritative non-zero Wire Session ID;
7. both peers agree on session ID `2738949150`;
8. the active Wire Session remains healthy for the proof interval with no
   RFB, AUDIO, MPEG, config, desktop, or other ordinary rider traffic;
9. ordinary close transitions that session to INACTIVE.

## Operator observation

The operator physically observed the real PS2 display.

The final visible screen showed:

    WIRE Q1-Q12 PROOF 1A
    state=BOOT
    state=NETWORK_INIT
    state=TCP_CONNECT
    state=PROVISIONAL
    WIRE ACTIVE session_id=2738949150
    riders=NONE
    idle_window=10_seconds
    WIRE INACTIVE
    PROOF1A CLIENT COMPLETE

The PS2-displayed session ID exactly matched the Pi machine evidence.

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

- `NOT_ACCEPTED` on real hardware;
- version-mismatch rejection;
- final product numeric representation of ACCEPT/NOT_ACCEPTED;
- persistent/systemd Pi Wire-server packaging;
- repeated sessions through one resident server;
- physical Wire loss with the console program remaining alive;
- reconnect and assignment of a different Wire Session ID;
- stale old-session work fencing;
- RFB, AUDIO, or MPEG Wire Channel Relay behavior;
- independent rider failure containment;
- MPEG generation behavior.

## Preserved evidence

    evidence/HW1-20260919T025049Z/authority.env
    evidence/HW1-20260919T025049Z/wire-server.log
    evidence/HW1-20260919T025049Z/deployment.json

SHA-256:

    authority.env=3add5302d53b88e2ce3d06bc1beec313b69bd401d4e9971c33c880979f26731a
    wire-server.log=c05606102e5021ca461af65e5b66a095e53b39775db5db9eac7147f4043e4e86
    deployment.json=c65f68381d9c5233d491d6d3a6ce155bd1ca9c9e8b8d7e20f6c8ac2efb3492f3
