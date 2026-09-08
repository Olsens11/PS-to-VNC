# H1 RFB channel-1 credit policy decision packet

Status: **DECISION REQUIRED BEFORE LIVE CHANNEL-1 DISPATCH**

This record resolves the evidence and choices around the only policy ambiguity
left after CP2D. It deliberately does **not** activate RFB, change CONFIG wire
format, alter PT_LOAD, or select a policy on David's behalf.

## Invariants already decided

The following are not part of this decision:

- one PS2-facing PSTV TCP connection only;
- RFB uses logical channel 1;
- the H1 receiver thread remains the sole physical `recv()` owner;
- outbound RFB bytes use H1's existing serialized send owner;
- through-Issue-39 RFB parser/session semantics remain unchanged;
- RFB queue storage is independent from AUDIO and MPEG;
- first controlled RFB queue capacity remains 32768 bytes, matching the
  through-Issue-39 direct-RFB receive-prefetch precedent;
- queue exhaustion is an observable failure, not permission to silently grow
  the queue;
- first hardware milestone is RFB-only, with AUDIO and MPEG disabled;
- Issue #40 is outside this experiment.

## Existing H1 credit precedent

AUDIO and MPEG already use receiver credit with four independent profile
choices per enabled channel:

1. initial credit bytes;
2. credit-return enabled/disabled;
3. returned-credit batch size;
4. flush pending credit when the logical queue becomes empty.

The qualified P11-compatible H1 profiles currently grant initial credit equal
to the full queue capacity. MPEG uses an 8192-byte return batch and flushes on
empty. PCM uses a 4096-byte return batch and flushes on empty. Both return
credit as bytes are consumed rather than merely as DATA frames arrive.

RFB has no equivalent CONFIG fields today. CP2D therefore stopped before live
channel-1 dispatch rather than inheriting AUDIO or MPEG policy implicitly.

## Candidate A — explicit RFB CONFIG knobs

Add RFB counterparts to the existing H1 credit vocabulary:

    rfb_queue_capacity
    rfb_credit_batch_bytes
    rfb_credit_flush_on_empty
    rfb_credit_return_enabled
    rfb_initial_credit_bytes

Recommended first RFB-only profile values if this candidate is selected:

    rfb_queue_capacity=32768
    rfb_initial_credit_bytes=32768
    rfb_credit_return_enabled=1
    rfb_credit_batch_bytes=8192
    rfb_credit_flush_on_empty=1

Why 8192 is the leading starting value rather than a claim of an optimum:

- it equals H1's current `max_data_payload` in the known profiles;
- it is one quarter of the 32768-byte RFB queue;
- it matches the current MPEG return batch;
- flush-on-empty prevents a small interactive RFB update from waiting forever
  merely because it did not accumulate a full batch;
- it avoids returning one CREDIT frame for every tiny parser read.

Advantages:

- matches H1's laboratory/configuration philosophy;
- makes queue/credit behavior visible in `h1_tool.py knobs` and sweeps;
- lets hardware evidence change batching without another ELF;
- avoids hidden RFB-specific constants.

Costs:

- changes CONFIG wire version/field count and both Pi/PS2 profile codecs;
- expands validation and test surface before the first RFB-only run;
- the first candidate values remain hypotheses until hardware evidence exists.

## Candidate B — fixed first-milestone credit policy

Keep CONFIG unchanged for the first RFB-only milestone and compile the same
starting values into the H1 RFB experiment runtime:

    queue=32768
    initial_credit=32768
    return_enabled=1
    batch=8192
    flush_on_empty=1

Advantages:

- smallest wire-protocol change surface;
- fastest route to testing the mux adapter itself;
- isolates RFB transport integration from CONFIG-vocabulary expansion.

Costs:

- H1's RFB credit policy becomes less observable/configurable than AUDIO/MPEG;
- tuning requires another ELF/source change;
- risks turning provisional laboratory values into hidden accidental policy.

## Candidate C — no RFB receiver credit

Do not select this for the first controlled integration without contrary
evidence. The Pi could otherwise outrun the finite 32768-byte PS2 logical queue,
and queue-full behavior would become dependent on TCP/socket backpressure at a
layer that no longer directly represents RFB consumption.

This would weaken the reason the mux has per-channel credit in the first place.

## Recommendation awaiting operator decision

**Candidate A is the stronger long-term H1 design.** It keeps RFB symmetric with
the existing independently tunable AUDIO/MPEG logical channels and gives later
hardware tests a clean way to distinguish a bad credit policy from a transport
or parser defect.

For the first values, use the evidence-based 32768-byte queue, full-queue
initial credit, enabled return, 8192-byte batching, and flush-on-empty. These
are controlled starting values, not claims about the PS2's required queue or
optimal batching.

If minimizing the number of variables in the very first RFB-over-mux hardware
run is more important than tunability, Candidate B is defensible, but it should
be explicitly labeled temporary and graduated to CONFIG knobs before hybrid
RFB+media optimization begins.

## What remains blocked until the decision

Do not yet:

- accept `rfb_mode=ON` in CONFIG;
- advertise live RFB capability in HELLO;
- dispatch channel-1 DATA into the resident RFB queue;
- emit channel-1 CREDIT;
- activate the mux-backed RFB adapter;
- create the Pi VNC bridge as an operationally enabled path.

Repository work that does not depend on the selected policy may continue, but
those activation steps must not infer a credit contract implicitly.
