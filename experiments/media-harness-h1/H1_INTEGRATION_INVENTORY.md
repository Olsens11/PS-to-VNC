# H1 integration inventory: clean product vs shared data transport

## Status

This is a read-only architectural inventory of the current code relationships.
It does **not** create a production transport directory, move runtime code, begin
Issue #40, or promote any unqualified H1 behavior into product authority.

The purpose is to answer the question required by `H1_INTEGRATION_INTENT.md`:
which H1 mechanisms are genuine new product capabilities, which existing clean
modules should remain authoritative, which experiment adapters should disappear,
and which qualification tools should remain outside the production ELF.

Current hardware authority remains CP2L visible RFB plus PS2 mouse input. CP2N
real Issue-39 local-controller/OSK/keyboard composition is CI-qualified and
awaits its real-PS2 gate.

## Architectural conclusion before file movement

The repository now has enough independent evidence to justify a **shared data
transport responsibility** above the PS2 socket/platform primitives and below
RFB/audio/MPEG consumers.

That conclusion is about responsibility, not yet about a directory name.
`src/transport/`, `src/data/`, or any other path is intentionally **not** created
or canonized here. Source-topology policy still requires the eventual directory
choice, local symbol dictionary, build/test/doc updates, continuity check, and
hardware gate to be handled as a deliberate topology change.

The responsibility is earned rather than speculative because the current H1
already has all of these facts simultaneously:

- one physical PSTV TCP connection;
- independent CONTROL, RFB, AUDIO, TELEMETRY and MPEG2 logical channels;
- one physical receive owner and one serialized send sequence;
- independent consumer queue/credit/backpressure requirements;
- three current bulk/stream consumers whose queue/credit mechanics already show
  duplication and specialization;
- an unchanged RFB parser that works correctly when its byte stream is supplied
  by a logical channel instead of a physical VNC TCP socket; and
- a stated future-compatibility requirement that bulk-data activity must not
  force redesign of RFB/audio/MPEG or the common delivery mechanism.

Future file transfer itself remains explicitly out of H1 scope. This inventory
only preserves the architectural property that a future higher-level feature may
quiesce/deprioritize consumers or use a bulk path without forcing the present
transport/domain boundaries to be rewritten.

## Clean product authority map

| Existing clean module | Responsibility / public seam | Current assumption exposed by H1 | H1 impact | Intended disposition |
|---|---|---|---|---|
| `src/app.c` | Application coordinator: ordered startup, main-thread semantic input routing, RFB pointer/key publication, local foreground ownership, presentation and cleanup | Startup currently obtains a direct VNC TCP socket from `ps2_network`; otherwise its domain composition is already the desired product composition | **Thin integration change** at connection/transport startup and session wiring | Keep as product coordinator authority; H1 coordinator copies should disappear rather than replace it |
| `src/rfb/rfb.c` | RFB wire structures/decoding helpers | None relevant to mux | **Unchanged** | Keep unchanged unless an independently justified RFB feature requires work |
| `src/rfb/rfb_session.c/.h` | RFB session/protocol state, exact message parsing, framebuffer updates, pointer/key serialization, safe complete-message scheduling boundary | Session stores an `int socket_fd`, although all actual byte transfer is already delegated through `rfb_io` | **Protocol unchanged; handle/interface shape may need a narrow evolution** | Keep RFB parser/session authoritative and mux-unaware |
| `src/rfb/rfb_io.h` | Three-operation byte-stream seam: exact read, safe-boundary poll, exact write | API names describe transport correctly, but the handle is still a physical-socket-shaped integer | **Thin adapter boundary; likely explicit endpoint/context evolution** | Preserve this responsibility as the RFB-to-byte-stream boundary; do not make RFB know PSTV framing/channels |
| `src/input/*` | Controller worker, semantic event queue, mouse interpreter, keyboard sequence construction, pad ownership/handoff | None created by mux | **Unchanged** | Reuse directly; delete experiment-specific substitutes |
| `src/ui/*` | Local-controller routing, foreground/quarantine state, OSK semantics/rendering/local presentation | None created by mux | **Unchanged** | Reuse directly; CP2N is the qualification bridge back to this authority |
| `src/framebuffer/*` | Authoritative CPU-side remote desktop | None created by mux | **Unchanged** | Keep authoritative |
| `src/display/*` | CPU framebuffer to GS-ready presentation conversion | None created by mux | **Unchanged for RFB** | Keep authoritative; later media composition is a separate responsibility and remains deferred |
| `src/platform/ps2_network.*` | Qualified PS2 Ethernet stack startup, link wait, low-level TCP/socket mechanism | Currently also hard-codes VNC endpoint `192.168.50.1:5900` and owns a private 32 KiB RFB receive buffer plus the three `rfb_io` implementations | **Boundary must narrow** | Keep PS2 link/socket primitives here; direct-VNC connection policy and RFB-specific buffering should no longer be the platform's long-term owner once shared transport is integrated |
| `src/platform/ps2_graphics.*` | PS2 GS presentation mechanism | None created by mux | **Unchanged for current RFB/OSK composition** | Keep platform mechanism authority |

### Important concurrency reinterpretation

The clean architecture says application/main is the sole owner of VNC socket
operations and cross-domain product side effects. H1 demonstrates an earned
change in what the physical socket *is*.

Under the shared-transport design, the physical TCP socket is no longer an RFB
/VNC socket. It is the PSTV data-transport connection. Therefore the durable
ownership model should become:

- one data-transport receive owner reads/fragments/dispatches the physical PSTV
  connection;
- transport-internal credit/sequence/framing work remains inside that owner/domain;
- application/main remains the sole RFB protocol/session owner and the sole owner
  of user-visible cross-domain side effects;
- the RFB session sees only its ordered logical byte-stream endpoint and remains
  responsible for safe RFB message boundaries.

This preserves the reason for the original main-thread rule while allowing the
new multiplexed physical mechanism to have exactly one independent receive
owner.

## H1 mechanism inventory

| Current H1 mechanism | Actual responsibility | Durable or checkpoint-specific? | Naming/readiness finding | Likely long-term owner / disposition |
|---|---|---|---|---|
| `experiments/audio-transport/common/transport_protocol.*` | Portable PSTV v1 header encoding/decoding, frame kinds, channel IDs, endian helpers | **Durable production candidate** | Existing names already describe generic transport responsibility; file comments correctly exclude TCP/RFB/audio/scheduling/threading | Shared data-transport domain above platform socket primitives |
| `experiments/audio-transport/common/transport_queue.*` | Caller-storage bounded byte ring with no allocation/synchronization | **Durable reusable mechanism** | Already consumer-neutral and host-testable | Shared transport internal utility or another narrowly justified reusable owner |
| `h1_transport_runtime.*` | Physical PSTV socket creation, handshake/config, sole recv thread, serialized send sequence, dispatch, media queues/credits, RFB resource hook, telemetry, media-end metadata and errors | **Mixed: durable core plus H1/media scaffolding** | `h1_transport_runtime` hides several owners in one experiment object | Split rather than migrate wholesale: extract physical connection/framing/dispatch ownership; leave H1 media/session-result policy in harness or proper media owners |
| `h1_rfb_channel.*` | One logical byte channel's inbound ring accounting, consumed-byte credit accounting and outbound fragmentation | **Mechanism is durable; RFB specialization is accidental** | `rfb_channel` is now too consumer-specific for mechanics that are not RFB syntax | Candidate for a generic logical byte-channel mechanism; RFB becomes one consumer |
| `h1_rfb_credit_policy.*` | Pure decision rule for when accumulated consumed bytes become returned credit | **Durable, host-testable policy candidate** | Policy itself is not RFB syntax | Candidate per-channel credit/backpressure policy in shared transport; actual policy values remain profile/config decisions |
| `h1_rfb_runtime_resources.*` | PS2 queue storage + semaphore + logical-channel state bundle | **Durable pattern, current implementation RFB-specific** | Resource ownership is clear, but vocabulary should describe logical channel rather than RFB if generalized | PS2-side channel-runtime wrapper above portable queue/channel core; exact domain/path still undecided |
| `h1_rfb_transport_live.*` | Bridges RFB exact read/poll/write to channel-1 queue/send mechanics and also owns H1 zero-length quiesce-marker state | **Mixed** | Name conflates RFB consumer adapter, generic channel transport and H1 qualification lifecycle | Split: durable RFB stream adapter + generic channel endpoint; retain finite-session quiesce protocol with H1 qualification unless product lifecycle independently earns it |
| `h1_rfb_mux_io.*` | Global experiment binding that makes the legacy integer socket handle resolve to one H1 runtime | **Experimental substitute to delete** | The fake-socket identity/global bound runtime is a compatibility trick, not desired product vocabulary | Remove after `rfb_io`/session has an explicit logical-stream endpoint/context; do not promote global binding into product |
| `h1_transport_runtime_rfb_lifecycle.c` | Mechanical wrapper around renamed H1 start/shutdown for RFB resource cleanup | **Checkpoint/build scaffolding** | Exists because the experiment patches an existing build without reshaping owners | Delete after proper product lifecycle integration |
| `h1_config_rfb_activation_gate.c` | Checkpoint gate that opens RFB-only mode while preserving older H1 validator and forbidding RFB+media | **Checkpoint scaffolding; validation facts partly reusable** | RFB-only prohibition is an experiment qualification rule, not transport law | Delete gate after product configuration owns consumer activation; preserve valid queue/credit relationship checks in proper config/channel policy |
| `h1_rfb_session_adapter.py` | Attaches Pi upstream VNC to channel 1 by registry + receive-frame monkeypatch; owns H1 quiesce state | **Mixed** | The one-PSTV-reader invariant is correct; monkeypatch/global registry are harness integration techniques | Keep equivalent Pi-side RFB-to-logical-stream responsibility, but replace harness patching with an explicit production transport interface when Pi product architecture is adopted |
| `h1_rfb_pi_bridge.py` | Credit-driven raw-byte bridge between upstream VNC socket and logical RFB channel; stops upstream reads at zero credit and relies on TCP backpressure | **Strong production-mechanism candidate** | Responsibility is already well isolated and RFB-syntax-free despite RFB-facing name | Pi-side RFB upstream bridge consuming a shared logical channel endpoint; no PS2-facing socket ownership |
| `h1_rfb_session_runtime*`, CP2J/K/L/N mains/coordinators | Finite qualification orchestration around parser, presentation/input checkpoints and quiesce | **Engineering/qualification scaffolding** | Checkpoint names are appropriate here | Retain only while useful for reproducible qualification; do not migrate as product coordinator |
| `h1_tool.py`, `h1_profiles.py`, checkers, sweeps, candidate/result/runbook docs | Parameter exploration, test selection, evidence, reproducible qualification | **Permanent engineering infrastructure** | H1/checkpoint vocabulary is appropriate because these are test artifacts | Retain outside production ELF; allow future qualified profile subsets to inform product defaults/modes |

## Duplication that proves generalization is earned

The current experiment has reached a useful consolidation point: AUDIO, MPEG and
RFB all need bounded receiver capacity and credit/backpressure, but they do not
currently share one complete channel-runtime abstraction.

`h1_transport_runtime` owns audio and MPEG queues, semaphores, pending credit and
return logic directly. RFB later introduced its own clearer resource bundle,
logical-channel object and pure credit policy. That is no longer hypothetical
abstraction work: the same transport responsibility is visibly repeated for
three real consumers.

The production integration should therefore generalize **only the shared
mechanism**:

    domain producer/consumer
        -> semantic/domain byte stream
        -> logical transport channel endpoint
        -> common PSTV framing / sequencing / credit / dispatch
        -> one physical data-transport TCP connection

The consumer keeps everything that gives those bytes meaning. The transport
must not parse RFB messages, decode MPEG, interpret PCM, understand OSK/input,
or own future file/storage semantics.

## RFB boundary recommendation

The strongest current seam is already `rfb_io`: RFB needs exact read, complete-
message-boundary poll, and exact write. The weakest part is only the historical
`int socket_fd` identity.

The durable integration should avoid teaching `rfb_session` about channel IDs or
PSTV frames and avoid preserving the H1 global `h1_rfb_bound_runtime` trick.
Instead, the eventual clean change should make the RFB I/O seam refer explicitly
to one byte-stream endpoint/context while preserving the same three behavioral
operations and the same parser scheduling rules.

This is an interface-shape change, not a parser rewrite. Because it will alter
clean product source/PT_LOAD, it must receive the normal host tests, symbol-
dictionary updates and real-PS2 hardware gate when implemented.

## H1 quiesce disposition

CP2I and later checkpoints use zero-length channel-1 DATA as the deterministic
finite-test handshake:

    Pi REQUEST -> PS2 BOUNDARY -> Pi COMMIT -> PS2 COMPLETE

This is excellent qualification machinery because it proves the upstream reader
has stopped and all already-read RFB DATA precedes COMMIT before the PS2 proves
its channel queue empty.

It is **not yet evidence that production data transport should reserve empty RFB
DATA for lifecycle control forever**. The finite H1 run ends on a timer and needs
sealed evidence; the normal product is long-lived. Therefore the quiesce state
machine remains classified as H1 engineering/qualification behavior unless a
future product lifecycle requirement independently earns a general channel
quiesce/control mechanism.

This distinction prevents a successful test protocol from becoming accidental
product protocol law.

## Provisional vocabulary map

These are responsibility names for discussion/integration readiness, not final
filenames or a new source topology:

| Current name | Responsibility that should survive | Provisional durable vocabulary |
|---|---|---|
| `h1_transport_runtime` | one physical PSTV connection + frame sequencing/dispatch core | `data transport connection/runtime` |
| `transport_protocol` | PSTV frame/channel wire vocabulary | `transport protocol` |
| `transport_queue` | bounded byte ring | `transport byte queue` |
| `h1_rfb_channel` | generic ordered logical byte-channel mechanics | `logical byte channel` |
| `h1_rfb_credit_policy` | per-channel returned-credit decision | `channel credit policy` |
| `h1_rfb_runtime_resources` | PS2 logical-channel queue/synchronization resource bundle | `PS2 channel runtime/resources` |
| durable part of `h1_rfb_transport_live` | RFB exact I/O implemented on one logical stream | `RFB stream/channel adapter` |
| `h1_rfb_pi_bridge` | raw upstream VNC bytes <-> RFB logical stream | `RFB upstream bridge` |

The naming policy should be applied before production movement so durable symbols
are renamed once for their true responsibility rather than carrying `h1_` or
`rfb_` labels into a generic transport owner.

## Recommended integration order after the CP2N gate

This inventory does not authorize implementation yet, but it narrows the next
architectural sequence:

1. finish CP2N's real-PS2 gate so the existing clean input/UI composition has a
   hardware-qualified mux-backed reference;
2. freeze the exact qualified H1 behavior as regression evidence;
3. agree the shared data-transport domain name/topology and explicit logical
   stream endpoint shape;
4. move/adapt the already-generic protocol/queue mechanisms first;
5. extract one physical PSTV connection owner and generic logical-channel
   queue/credit/dispatch mechanics without media/RFB semantics;
6. adapt `rfb_io` to the explicit logical-stream endpoint while keeping
   `rfb_session` parsing behavior unchanged;
7. replace H1 app/input/UI scaffolding with `src/app.c` and the existing clean
   modules;
8. hardware-gate every PT_LOAD-changing integration ledge before adding another
   consumer combination;
9. only after the product-shaped RFB path is stable, integrate audio/MPEG through
   their own proper domains and the same shared transport interface.

Issue #40 remains outside this sequence unless separately authorized later.
