# Wire Q1-Q12 Proof 1B — HW1 Product-Version Rejection

## Classification

- Result: `HARDWARE_PROVEN`
- Scope: product-version incompatibility rejection during provisional Wire
  establishment.
- PS2 DUT source commit:
  `9943af768f9844505753078a30b9eeb7dd7a8573`
- Pi proof-apparatus commit:
  `b78cec834d2aeb065b55e3a964087684b92d4381`
- PS2 hardware test ID: `WIRE-Q1Q12-P1A-HW1`
- Run ID: `P1B-20260919T030341Z`

This proof uses the same PS2 DUT that passed Proof 1A. The PS2 ELF was neither
rebuilt nor redeployed for Proof 1B.

The proof-local ACCEPT/NOT_ACCEPTED numeric frame identities remain experiment
apparatus and are not promoted into the product Wire ABI by this result.

## Exact unchanged PS2 DUT

    SOURCE_HEAD=9943af768f9844505753078a30b9eeb7dd7a8573
    TEST_ID=WIRE-Q1Q12-P1A-HW1
    ELF_SHA256=de22b52f1c2c2b5895a2efe0c7c5faa689a78792c4b5259c7b9385c8dcdf730c
    ELF_BYTES=2297184
    PT_LOAD_SHA256=3b43585e1dd4a26650c132c33010698cd556cbbbade9c925a9de3e3227b5ab3a
    PT_LOAD_BYTES=365064
    PS2_REDEPLOYED=NO

Immediately before the hardware run, the rolling `/mass/0/PS2VNC.ELF` was read
back over FTP and matched the exact Proof 1A HW1 ELF identity above.

## Pi development apparatus

Proof 1B used:

    experiments/wire-q1-q12-proof/proof1_wire_server.py

Apparatus source identity:

    COMMIT=b78cec834d2aeb065b55e3a964087684b92d4381
    SHA256=78277ce2a5b90e50df5e96cb55232f96f1f0791cb2f7fbd0c8796768e059ad7c

The server remained one-shot foreground development apparatus on:

    0.0.0.0:5902

No Pi systemd service or network configuration was changed.

For this run only, the compatibility policy was:

    accepted_wire_version=1
    accepted_product_version=2

The unchanged PS2 DUT truthfully sent:

    wire_version=1
    product_version=1

This therefore exercised an actual product-version incompatibility rather than
an unconditional artificial rejection.

## Machine evidence

The Pi observed:

    WIRE_PROOF_ACCEPT_POLICY wire_version=1 product_version=2
    WIRE_PROOF_PROVISIONAL peer=192.168.50.2:52432
    WIRE_PROOF_HELLO wire_version=1 product_version=1
    WIRE_PROOF_NOT_ACCEPTED reason=PRODUCT_VERSION received_wire_version=1 received_product_version=1 accepted_wire_version=1 accepted_product_version=2
    WIRE_SESSION=INACTIVE session_id=NONE

The machine-evidence evaluation confirmed:

    POLICY_COUNT=1
    PROVISIONAL_COUNT=1
    HELLO_COUNT=1
    REJECT_COUNT=1
    INACTIVE_COUNT=1
    ACTIVE_COUNT=0
    RIDER_COUNT=0
    PROOF1B_MACHINE_EVIDENCE=PASS

Therefore the incompatible provisional connection was rejected without ever
becoming an active Wire Session and without starting a rider.

## Operator observation

The operator physically observed the real PS2 display after launching the exact
unchanged HW1 ELF.

The final visible screen showed:

    WIRE Q1-Q12 PROOF 1A
    state=BOOT
    state=NETWORK_INIT
    state=TCP_CONNECT
    state=PROVISIONAL
    NOT_ACCEPTED reason=2

There was no `WIRE ACTIVE` line and no rider-start indication.

The operator observation agrees with the Pi evidence: rejection reason `2` is
the proof-local PRODUCT_VERSION rejection used by this experiment.

## Physically established by Proof 1B

For the product-version mismatch case:

1. the console reaches PROVISIONAL state;
2. the console sends its actual Wire/product compatibility identity;
3. the Pi evaluates that identity before creating an active session;
4. incompatible product versions produce NOT_ACCEPTED;
5. the console receives and reports that rejection;
6. no authoritative Wire Session ID is created;
7. the connection returns to INACTIVE;
8. no rider begins merely because TCP connected.

Together with Proof 1A, this physically distinguishes provisional TCP
connectivity from an accepted ACTIVE Wire Session.

## Evidence classification

    SOURCE_COMPLETE=YES
    HOST_TESTED=YES
    PS2_COMPILE_PASS=YES
    PS2_LINK_PASS=YES
    MACHINE_EVIDENCE=PASS
    OPERATOR_OBSERVED=PASS
    HARDWARE_PROVEN=YES

The PS2 compile/link labels are inherited from the exact unchanged HW1 DUT
already proven in Proof 1A; Proof 1B did not rebuild it.

## Still unproven

This result does not establish:

- Wire-version mismatch rejection on real hardware;
- malformed-establishment rejection on real hardware;
- cryptographic peer authentication;
- final product numeric representation of ACCEPT/NOT_ACCEPTED;
- continuously-running/systemd Pi Wire-server packaging;
- multiple sequential sessions accepted by one resident server;
- physical Wire loss while the console application remains alive;
- reconnect after loss and assignment of a new Wire Session ID;
- stale-session fencing;
- RFB, AUDIO, or MPEG Wire Channel Relay behavior;
- independent rider failure containment;
- MPEG generation lifecycle behavior.

## Preserved raw evidence

    evidence/P1B-20260919T030341Z/authority.env
    evidence/P1B-20260919T030341Z/wire-server.log

SHA-256:

    authority.env=89183db814b238e2b8163df23a199767053c0855006cf31021fbabd0a7fe031e
    wire-server.log=456fecb93c3be7317f5e7eaa156c71c49f54d6fcfa7f3f5736f874ad6dae3d83
