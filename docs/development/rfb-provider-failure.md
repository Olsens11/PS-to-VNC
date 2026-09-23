# RFB Provider-Failure Development Seam

This note is the developer-facing continuity companion to
`../ledge/LEDGE_R16A_RFB_PROVIDER_FAILURE_CONTRACT.md`.

R16A adds a mechanism for one Pi-local RFB provider failure to become a typed
PS2 RFB-local terminal fact **without** making the physical Wire Session itself
failed. It does not add product retry/reconnect policy.

## Compatibility identity

Keep the two version dimensions separate:

- fixed PSTV frame/header version remains `1`;
- Q4 product-establishment compatibility is `2` for the R16A channel-1
  provider-terminal semantic.

An old product-v1 peer and a product-v2 peer must reject during Q4 before ACTIVE.
Do not make the post-Q4 meaning backward-compatible by silently accepting a
peer that cannot interpret it.

## Wire representation

The only R16A provider-terminal frame is:

```text
kind=ERROR(7)
channel=RFB(1)
flags=0
payload_length=4
payload=uint32 reason in network byte order
```

Reasons are CONNECT=1, READ=2, WRITE=3. READ includes provider EOF because EOF
and recv/read failure are terminal at the same provider-read mechanism boundary.

Do not reuse DATA, CREDIT, or a zero-length RFB DATA marker for provider failure.
The R13 zero-length DATA lifecycle remains exactly REQUEST -> BOUNDARY -> COMMIT
-> COMPLETE.

## Ownership

On Pi:

1. the session-scoped RFB attachment detects and retains the first specific
   provider cause;
2. the attachment remains FAILED after the fact is reported;
3. the attachment never sends on the PS2-facing socket;
4. `WireConnectionOwner` remains the sole physical sender and sequence owner and
   serializes the ERROR frame.

On PS2:

1. Transport decodes and latches the first valid RFB provider reason under the
   existing RFB synchronization boundary;
2. RFB readers/activity waiters and writers waiting for provider credit are
   woken so they cannot hang on a dead provider;
3. the provider terminal fact is exposed through Transport's RFB-specific seam;
4. the RFB bridge/session maps the reason into RFB-local terminal errors;
5. correct provider failure does not by itself set generic Wire failure or stop
   the physical-I/O owner.

## Ordered old-session bytes

DATA already sequenced before ERROR remains old-session data. The PS2 runtime may
consume those queued bytes before provider terminality wins once the queue is
empty, but it must not return replacement provider credit after the terminal
fact. Late state is never transferred into another attachment or Wire Session.

A failed R13 attachment is never rebound. A later session gets fresh Transport,
attachment, credit, wake, and failure state.

## Application recovery after accepted R16A

R16B consumes the accepted typed provider result at one explicit Application
policy branch. CONNECT, READ/EOF, and WRITE remain distinct RFB-local causes,
but the initial product policy for all three is the same:

1. leave the failed attempt's ordinary RFB/input admission path immediately;
2. prove the attempt's input worker/runtime completely stopped;
3. abort the containing Transport session through its owner seam so the sole
   receiver is interrupted, observed complete, and released;
4. admit a replacement only after both stop proofs succeed;
5. run ordinary startup again with a fresh network connection, Q4 Wire Session,
   Transport access, Pi attachment, RFB parser/session, framebuffer validity,
   and input publication/runtime state.

The failed attachment is never rebound and no component is restarted in place
inside its still-live old attempt. The next attempt must independently reach the
ordinary DESKTOP_READY/INPUT_READY boundary; successful teardown is not startup
success.

There is deliberately no success-by-delay in this policy. A sleep, retry count,
timeout, or backoff interval is never retirement evidence. If input dormancy or
Transport receiver/session retirement cannot be proven, Application fails
closed and does not admit a replacement.

R16B does not change the R16A frame/Q4 compatibility representation, R13
quiesce, the singular R14 flow profile, AUDIO/MPEG/CONFIG behavior, heartbeat
policy, systemd/provider restart behavior, or direct-RFB fallback.

## Evidence expectations

Changes to this seam should preserve tests that separately prove:

- CONNECT, READ/EOF, and WRITE reason identity;
- report serialization by the sole Pi Wire owner;
- provider failure does not falsely kill Wire;
- R13 quiesce bytes remain distinct;
- product-v1/product-v2 mismatch rejects before ACTIVE;
- a PS2 RFB read waiting for provider bytes wakes terminally;
- a PS2 RFB write waiting for provider credit wakes terminally;
- stale session/attachment state is not rebound;
- linked PS2 bytes are reproducible but remain hardware-unqualified until a
  physical qualification packet says otherwise.
