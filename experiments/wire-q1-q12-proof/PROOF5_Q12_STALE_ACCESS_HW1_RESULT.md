# Proof 5 — Q12 post-reconnect stale-access hardware result

## Classification

This run is a successful real-PS2 hardware proof of the **post-reconnect stale
access admission fence** in the product Transport bridge.

It is deliberately **not** classified as complete Q12 architectural closure,
because the stronger Q12 contract also requires an operation already admitted
under Session A to be prevented from surviving Transport retirement/reuse into
Session B. This HW1 transaction did not deliberately hold such an already-
admitted A operation across the A->B replacement boundary.

Therefore:

    POST_RECONNECT_STALE_A_OUTBOUND_FENCING_HARDWARE_PROVEN=YES
    POST_RECONNECT_STALE_A_INBOUND_NONCONSUMPTION_HARDWARE_PROVEN=YES
    FRESH_B_ACCESS_HARDWARE_PROVEN=YES

    IN_FLIGHT_A_OPERATION_DRAIN_HARDWARE_PROVEN=NO
    FULL_Q12_ARCHITECTURAL_FENCE_HARDWARE_PROVEN=NO

This narrower classification corrects the broader
`Q12_HARDWARE_PROVEN=YES` label printed by the collection packet. The machine
and operator evidence remain valid; only the claim boundary is tightened to
match the governing Q12 lifecycle contract.

## Authority

    SOURCE_HEAD=7bf69c79bf95bcade12787c7eb076f91bb78fb35
    RUN_ID=Q12-STALE-HW1-20260920T011710Z
    TEST_ID=WIRE-Q12-STALE-HW1

    PRODUCT_TRANSPORT_BRIDGE_SHA256=aa7edfc7343f9395b0e949f435abb554b8a267f786ca509e0c7e2338291887d1
    PRODUCT_TRANSPORT_RUNTIME_SHA256=4af9b45b561974e95340d0e1151948011293e5f72e94b27543aa4c978c5dd465

The DUT linked the current product Transport bridge/runtime unchanged. Q4 Wire
establishment remained proof-local apparatus.

## Exact DUT

Pristine build:

    ELF_SHA256=c687d1a37eee90482b90dbeafbbe57682b5d329af0183693b7145492e5c6299e
    ELF_BYTES=2457908
    PT_LOAD_SHA256=477048d82e711efdc557133408b594c724195b4ab1b795ba317dbc53082252e4
    PT_LOAD_BYTES=390152

Identity-stamped hardware DUT:

    ELF_SHA256=efec6f6b997483c380bc2ffd4ebf2b8c1a433e8d89f008845edd96145bddf614
    ELF_BYTES=2457908
    PT_LOAD_SHA256=b85f5a5df0b9b41dabaaee4eb80d36e9d422ae64b37a4b74e5d0ea3f47d782aa
    PT_LOAD_BYTES=390152

Both the unique FTP destination and rolling `/mass/0/PS2VNC.ELF` read back
with the exact stamped DUT SHA-256.

## Machine evidence

Session A established with proof identity `1780350440`, advertised 16 bytes
of initial RFB credit, and accepted a valid A-side RFB write.

The Pi then forcibly terminated A.

The same PS2 process established Session B with a distinct proof identity
`2791894676`. B advertised its own initial RFB credit and the Pi injected the
known four-byte B payload `b1b2b3b4`.

The decisive observations were:

    Q12_STALE_A_OUTBOUND_ABSENT=PASS
    Q12_STALE_A_RETURN_PATH_PRESERVED_B_PAYLOAD=PASS
    Q12_B_FRESH_READ_CREDIT=PASS amount=4 sequence=2
    Q12_FRESH_B_ACK=PASS payload=BOK!
    Q12_STALE_ACCESS_HARDWARE_PROOF=PASS

This proves that a **new operation attempted through stale access A after B was
active** did not become B outbound traffic, and that a stale A read attempted
after known B data was present did not consume that B payload. Fresh access B
then consumed the payload and transmitted normally.

    MACHINE_EVIDENCE=PASS

## Operator observation

The operator launched the stamped DUT exactly once and reported the final PS2
screen result as:

    PASS

The operator did not report a reset or relaunch between Sessions A and B.

    OPERATOR_OBSERVED=PASS

## What this hardware run proves

Within this bounded two-session transaction:

1. access A was valid while Session A was active;
2. Session A was physically terminated by the Pi;
3. the same PS2 process established a distinct Session B;
4. stale access A could not emit the deliberate `OLD!` outbound marker through
   B;
5. stale access A could not consume a known inbound B payload;
6. fresh access B still consumed that exact payload;
7. fresh access B emitted the expected `BOK!` acknowledgement;
8. the product Transport bridge/runtime, not a proof-local replacement bridge,
   enforced the stale-access admission behavior.

## Remaining Q12 gap

The governing Q12 contract is stronger than post-reconnect stale admission.

It also requires that work **already admitted while A was valid** cannot remain
inside the reusable Transport runtime while A is retired and then complete,
consume, publish, or otherwise interact with B.

The current bridge uses one reusable singleton runtime. Its opaque ticket is
checked before entering each bridge operation, but this HW1 proof did not hold
an A operation inside that runtime across loss/replacement.

The next Q12 boundary is therefore:

- add a Transport-owned admission/drain lifetime fence so runtime retirement
  cannot reclaim/reinitialize while admitted A calls remain;
- host-test the race deterministically;
- hardware-prove an A call admitted before loss remains a retirement blocker,
  then prove B can open only after that old call has returned.

Only after that boundary is proven should the repository claim full Q12
architectural fencing.

## Product boundary

Q4 establishment remains proof-local.

Accordingly:

    PRODUCT_Q4_ESTABLISHMENT_IMPLEMENTED=NO
    EXACT_PRODUCT_ELF_HARDWARE_PROVEN=NO

## Preserved evidence

Tracked text evidence:

    experiments/wire-q1-q12-proof/evidence/Q12-STALE-HW1-20260920T011710Z/authority.env
    experiments/wire-q1-q12-proof/evidence/Q12-STALE-HW1-20260920T011710Z/wire-server.log
    experiments/wire-q1-q12-proof/evidence/Q12-STALE-HW1-20260920T011710Z/operator-observation.txt

The canonical deployment and hardware-preparation files remain in the Pi run
directory and are identified here by their captured SHA-256 values:

    deployment.json=c17bdc622daf58ef67ba8b37c221ad8fdb6d5f308c23d66d657e70dbcfb6eab1
    prepare-hardware.env=e2576af554c4e0011e77c2c72c6608d5b8337b394de5ff5a559657e133d2c6a5

No repository copy of those two Pi-local files is fabricated from summary
output.
