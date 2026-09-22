# R16A RFB Provider-Failure Representation Contract

DOCUMENT=LEDGE_R16A_RFB_PROVIDER_FAILURE_CONTRACT
DOCUMENT_REVISION=0001
RECORDED_AT=2026-09-22T14:34:40-04:00
SOURCE_COMMIT=SELF
BASED_ON_FOREMAN_STATE_REVISION=0046
PACKET_ID=A003-RFB-PROVIDER-FAILURE-REPRESENTATION-R16A
TEMPORAL_CLASS=RECONSTRUCTION_SOURCE_CONTRACT
TEMPORAL_SEMANTICS=IMPLEMENTED_CANDIDATE_PENDING_FOREMAN_ACCEPTANCE
BRANCH_SCOPE=ledge/h1-all-guns

This record makes the R16A Reconstruction decision recoverable from repository
authority without conversational memory. It describes the bounded mechanism
implemented by Reconstruction under Foreman State revision 0046. It is **not** a
Foreman acceptance record and does not resume the downstream R16 Application
failure/stop/restart packet.

## Problem and boundary

Accepted R13 already made Pi-local provider connect, provider EOF/read, and
provider write failure terminal to one session-scoped RFB attachment. The
blocked R16 attempt proved that the then-accepted Wire vocabulary had no
unambiguous way to deliver that fact to the PS2 while preserving a healthy Wire
Session as a separate fact.

R16A therefore owns only the missing protocol/interface mechanism. It does not
own Application retry, reconnect, backoff, fresh-Q4 creation, provider restart,
or replacement attachment policy.

The governing failure-domain distinction is:

```text
provider failure
    -> RFB attachment terminal
    -> typed RFB-local Wire fact
    -> PS2 RFB-facing terminal fact
    != physical Wire/Transport failure
```

Correctly delivering the provider fact must not close, rebind, or mark the
physical Wire Session failed.

## Exact representation

R16A deliberately assigns the previously dormant frame-kind reservation
`ERROR = 7` to one exact bounded semantic:

- frame kind: `ERROR` / numeric `7`;
- channel: RFB / numeric `1`;
- flags: `0`;
- payload length: exactly `4` bytes;
- payload: one unsigned network-order 32-bit reason value.

The only valid provider-terminal reason values are:

| Value | Meaning |
|---:|---|
| `1` | provider CONNECT failure |
| `2` | provider READ terminality, including provider EOF or recv/read failure |
| `3` | provider WRITE failure |

Zero is not a transmitted terminal reason. Unknown values fail closed as
malformed protocol input.

This representation is distinct by frame identity from ordinary RFB `DATA`, RFB
`CREDIT`, and the R13 zero-length channel-1 `DATA` quiesce exchange. R16A does
not overload or reinterpret any R13 quiesce marker.

## Compatibility contract

The fixed PSTV frame header and physical Wire framing remain version `1`.
R16A does **not** change the 16-byte header shape or its framing version.

The Q4 product-establishment compatibility version advances from `1` to `2` on
both peers. Product version `2` means that an ACTIVE session understands the
post-Q4 channel-1 `ERROR=7` provider-terminal semantic above.

Therefore:

- fixed/header Wire version remains `1`;
- product-establishment version is `2`;
- a product-v1 peer and product-v2 peer reject one another during Q4 before
  ACTIVE authority exists;
- a fixed-Wire-version mismatch also rejects during Q4 as before;
- no peer may enter ACTIVE and then disagree about whether `ERROR=7/channel 1`
  has provider-terminal meaning.

This is the smallest compatibility boundary because framing itself did not
change; only post-establishment product semantics did.

## Pi producer and physical-send ownership

`pi/rfb_attachment.py` remains the owner of the provider attachment lifecycle.
It retains the first mechanically distinguishable provider-terminal cause:

- connect setup/readiness failure -> CONNECT;
- provider EOF or provider recv/read failure -> READ;
- provider send/write failure -> WRITE.

The first specific provider cause is immutable for that attachment. Reporting it
successfully does not clear the attachment's `FAILED` state or erase its cause.
The failed attachment remains dead and cannot reconnect or rebind in place.

The attachment does **not** acquire Wire send authority. It only publishes a
pending typed fact to the existing Pi `WireConnectionOwner`.

`pi/wire_server.py` remains the sole owner of:

- the PS2-facing physical socket;
- physical receive;
- physical send;
- direction-local Wire sequence allocation.

That owner serializes the channel-1 ERROR using the ordinary next Wire sequence.
After a successful send it marks only the report as delivered; attachment
terminality remains intact. Correct report delivery does not retire the Wire
Session.

## PS2 Transport consumer

PS2 Transport decodes only the exact `ERROR=7`, channel-1, flags-zero,
four-byte envelope as an RFB provider-terminal fact. The first valid reason is
latched in the current Transport runtime under the existing RFB synchronization
boundary.

Receiving that fact:

- does not set generic Transport/Wire failure merely because the provider died;
- does not end the sole physical-I/O thread;
- wakes an RFB reader/activity waiter that might otherwise wait indefinitely;
- wakes an RFB writer waiting for provider-granted outbound credit;
- prevents new provider credit from being treated as usable authority after
  provider terminality;
- remains queryable through the Transport RFB-facing status seam.

Genuine physical framing/socket/Transport failure remains separately observable
through the pre-existing Transport terminal state.

## Ordered data and stale-state containment

Wire sequence order remains authoritative. RFB DATA accepted before the typed
ERROR remains old-session data and may be consumed in that same runtime before
the terminal fact wins once the queue is empty. The ERROR does not relabel or
rebind those bytes.

After provider terminality:

- no replacement provider is attached to the dead R13 attachment;
- no old credit is transferred to a replacement attachment;
- no report state is transferred to another Wire Session;
- no new RFB provider capacity is inferred from late old-session traffic;
- releasing the runtime destroys the session-local terminal fact with the rest
  of that runtime's authority.

A later Wire Session creates fresh Transport/RFB authority and, where composed,
a fresh Pi attachment. Session-A bytes, credit, wake descriptors, failure state,
or tickets cannot become Session-B authority.

## RFB-facing result mapping

Transport exposes the typed reason through its RFB-specific bridge seam without
turning it into a generic physical-Transport failure. The RFB bridge forwards
that mechanism fact to the RFB session owner.

The RFB session maps the three reasons to distinct RFB-session terminal errors:
provider connect, provider read, and provider write. That mapping gives the
future Application recovery packet a domain-local failure result it can consume
without guessing from socket loss or timeouts.

The RFB session does not acquire product recovery policy by performing this
mapping.

## Preserved R13 quiesce

R16A leaves the exact finite quiesce lifecycle unchanged:

```text
REQUEST -> BOUNDARY -> COMMIT -> COMPLETE
```

Its zero-length channel-1 DATA representation retains only that meaning. A
provider failure during ordinary running or quiesce retirement uses the typed
ERROR representation when it is a mechanically identified provider failure;
ERROR does not double as REQUEST, BOUNDARY, COMMIT, or COMPLETE.

## Explicit non-goals

R16A does not implement or authorize:

- Application retry/reconnect/backoff;
- creation of a fresh Q4 session after failure;
- in-place provider retry or attachment rebinding;
- a generic timeout or heartbeat substitute for typed provider failure;
- systemd restart as an RFB failure protocol;
- AUDIO, MPEG, CONFIG, telemetry, or other rider activation;
- direct-RFB fallback as an ordinary product data path;
- any R14 RFB runtime-profile retuning or duplicated configuration authority.

Downstream complete-stop-before-restart policy remains blocked on independent
Foreman acceptance of this R16A representation.

## Evidence boundary

Host tests exercise exact codecs, product-version rejection, Pi first-cause
reporting, sole-owner serialization, R13 quiesce separation, PS2 bridge/result
mapping, stale-session fencing, and canonical build/test integration.

Pinned PS2 compile/link and reproducibility evidence proves current source can be
built reproducibly. It does **not** qualify the new linked bytes on physical PS2
hardware. Hardware qualification remains explicit debt.