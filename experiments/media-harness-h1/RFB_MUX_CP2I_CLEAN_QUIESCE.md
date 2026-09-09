# CP2I — Clean finite RFB quiesce behind the closed activation gate

Status: **COMPLETE — HOST/CI/PINNED-BUILD VERIFIED, NOT HARDWARE QUALIFIED**

CP2I solves the finite-session termination problem for raw RFB-over-H1 without
teaching the Pi bridge to parse RFB or allowing an arbitrary timer to cut a
server message in the middle.

## Why MEDIA_END alone was unsafe

The Pi bridge intentionally transports an ordinary VNC byte stream without
understanding RFB headers, rectangles, or message boundaries. If a finite test
simply emitted H1 MEDIA_END at the duration deadline, the bridge could already
have forwarded only part of an RFB server message. The through-Issue-39 parser
would then fail an exact read and a healthy run could be misclassified as a
parser/transport failure.

RFB protocol-boundary authority therefore remains where it already exists: the
unchanged PS2 RFB parser.

## Ordered quiesce handshake

Zero-length PSTV DATA on logical channel 1 is reserved in this experiment for
lifecycle control. Non-empty channel-1 DATA remains raw RFB bytes.

Direction plus strict state ordering gives the four zero-length markers their
meaning:

    Pi REQUEST
        -> PS2 continues parsing the currently outstanding complete RFB message
        -> PS2 does not issue another framebuffer request

    PS2 BOUNDARY
        -> proves parser is between complete RFB server messages
        -> Pi stops/shuts down/joins the upstream VNC-reader worker

    Pi COMMIT
        -> is sent only after that worker has joined
        -> all Pi->PS2 raw DATA read before shutdown is therefore serialized
           before COMMIT on the same H1 send lock and sequence stream

    PS2 COMPLETE
        -> sent only after COMMIT is received and a semaphore-consistent RFB
           queue snapshot proves queue_current == 0

Only after COMPLETE does the Pi send ordinary H1 MEDIA_END. The PS2 then emits
the ordinary H1 SESSION_RESULT, preserving existing result/integrity ordering.

## Conservative fail-closed property

BOUNDARY proves where the PS2 parser stopped requesting new work, but the raw Pi
reader may already have read bytes belonging to a following unsolicited or
already-in-flight server message. Stopping the upstream reader alone cannot
pretend those bytes never existed.

COMMIT solves the transport-ordering half: because it is sent after the bridge
worker has joined and through the same serialized PSTV sender, every final raw
DATA frame must precede COMMIT.

The PS2 then requires its channel-1 queue to be empty. If residual bytes remain,
the finite test fails instead of claiming a clean boundary. This is deliberate;
hardware evidence can later show whether a more elaborate drain mechanism is
needed.

## Ownership preserved

CP2I does not add:

- a second PS2-facing socket;
- a second PSTV receive thread;
- an RFB parser on the Pi;
- a second channel-1 staging reservoir;
- graphics presentation;
- controller, pointer, keyboard, OSK, or local UI;
- AUDIO/MPEG+RFB hybrid operation;
- Issue #40 work.

`H1Session.reader()` remains the sole Pi PSTV receive owner. The H1 EE receiver
remains the sole PS2 PSTV recv() owner. The clean through-Issue-39 RFB session
remains the only RFB protocol-boundary authority.

## Host proof

The Pi adapter socketpair test proves:

- normal HELLO/CONFIG and AUDIO/MPEG frames still interleave with channel 1;
- REQUEST is a zero-length channel-1 DATA frame;
- BOUNDARY is consumed by the same global receive-sequence authority;
- `bridge.stop()` shuts down and joins the upstream reader before COMMIT;
- the upstream fake VNC peer observes EOF before COMMIT is emitted;
- COMMIT uses the next globally serialized Pi PSTV sequence;
- COMPLETE closes the ordered state machine;
- zero-length lifecycle markers never enter the raw RFB byte stream.

The strengthened source/object verifier additionally proves that the headless
PS2 coordinator:

- checks for REQUEST only at complete parser-message boundaries;
- stops issuing incremental framebuffer requests before BOUNDARY;
- waits for COMMIT;
- takes the locked RFB queue snapshot;
- requires an empty queue before COMPLETE;
- waits for ordinary MEDIA_END after COMPLETE before SESSION_RESULT;
- references no graphics/input/UI owner and opens no socket.

## Final CP2I evidence

    workflow=H1 RFB mux preparation checks
    run=34305837804
    head=6f94ce88812849e4e9b5ccc730d474dbfa10225d
    conclusion=PASS

    H1_RFB_MUX_SEAM=PASS
    H1_RFB_SESSION_COORDINATOR_SOURCE=PASS
    H1_RFB_SESSION_COORDINATOR_OBJECTS=PASS
    H1_RFB_SESSION_COORDINATOR=PASS
    H1_RFB_SESSION_ADAPTER_TEST=PASS
    H1_CUMULATIVE39_RFB_BRIDGE_RUNNER_TEST=PASS

    ELF_SHA256=817bb073e5699bd1d936ff7069bdd943285d98865fb9067170ef201b9e7544c1
    PT_LOAD_SHA256=b33d82763a481e33b79ff3a6799059cc1a90d06399265a15225738fc0e9d3151
    PT_LOAD_BYTES=487572
    ARTIFACT_ID=10086607011

The public PS2 CONFIG validator still rejects RFB ON and HELLO still omits
CAP_RFB at CP2I. Therefore none of this evidence is a hardware RFB qualification
claim.

## Next checkpoint

The next change may deliberately open only the first hardware test path:

- RFB ON;
- AUDIO OFF;
- MPEG OFF;
- candidate-A RFB policy: queue 32768, initial credit 32768, returned-credit
  batch 8192, flush-on-empty 1, return enabled 1;
- CAP_RFB advertised only by the cumulative RFB-prep build;
- no RFB presentation/input yet.

That first hardware run should qualify the one-socket transport, unchanged RFB
parser, incremental-update loop, credit behavior, and CP2I quiesce handshake as
one end-to-end chain before any GS-compositor work begins.
