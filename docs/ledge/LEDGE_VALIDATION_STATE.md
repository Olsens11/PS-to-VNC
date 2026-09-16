# Ledge Validation — Lane State

DOCUMENT=LEDGE_VALIDATION_STATE
STATE_REVISION=0005
RECORDED_AT=2026-09-15T23:21:03-04:00
SOURCE_COMMIT=SELF
BASED_ON_STATE_REVISION=0004
BASED_ON_RECONSTRUCTION_STATE_REVISION=0007
BASED_ON_SEMANTIC_AUDIT_REVISION=0007
BASED_ON_GLOBAL_STATE_REVISION=0011
BASED_ON_VALIDATION_FINDINGS_REVISION=0004
TEMPORAL_CLASS=STATE_SNAPSHOT
TEMPORAL_SEMANTICS=SNAPSHOT_TRUE_AT_RECORDED_TIME

This state owns validation continuity only. It does not supersede global, audit, reconstruction, Foreman, or governance authority.

## Current validation phase

`A001_LIVE_PATH_REVIEW_BLOCKED_ON_FATAL_TEARDOWN_CONVERGENCE`

## Authority inspected

Validation began at committed branch HEAD `9790ae22f6db4d0aee20f822c9fd6c51d1c7d129`. That authority contains the completed RFB logical-bridge migration, application adoption of Transport lifecycle, and application Transport ownership test source. Foreman state revision 0002 assigns the same `a001-sole-receiver` seam and requires receiver completion before reclamation plus first-failure/fail-closed ownership. Reconstruction state revision 0007 is older than the newest live-path commits and remains useful only as historical lane state. Unknown Pi-local dirty work is outside this GitHub-native validation surface and was neither overwritten nor declared absent.

## Validation result

A001 remains not `VALIDATION_READY`.

The new live path provisionally satisfies the major ownership migration shape: application transfers the physical descriptor to Transport through `pstvnc_transport_session_open()`, RFB no longer receives that descriptor, and `rfb_session` consumes the logical bridge. The no-argument production entry also refuses to invent Transport configuration values.

Validation opened V005 HIGH because the post-adoption fatal cleanup path waits for sole-receiver completion without an application-requested convergence operation. A healthy receiver blocked in physical receive need not finish merely because input, presentation, or application-side RFB processing failed. The current bridge offers wait/close plus server-driven finite RFB quiescence, but no application-local fatal-abort operation. Therefore deterministic first-failure teardown and repeated-session progress are not proven and may stall indefinitely.

V003 and V004 remain OPEN. V004 dictionary/topology/portal work remains Foreman-owned integration/evidence work under the current role split. V005 is product behavior and must return to Reconstruction A/B.

## Evidence boundary

- live descriptor transfer / no direct RFB physical socket: PROVISIONAL PASS by source inspection;
- explicit caller-supplied Transport config boundary: PROVISIONAL PASS; concrete production config authority remains unresolved and `pstvnc_app_run()` intentionally fails closed;
- application ownership test source: PRESENT, but executable result not independently observed this shift;
- post-adoption fatal teardown convergence: FAIL / OPEN V005;
- receiver-completion-before-reclaim invariant: preserved structurally, but current wait can lack a cause for completion;
- V004 dictionary/topology/portal/canonical integration: OPEN / Foreman chore queue;
- host/static/canonical build and reproducibility/PT_LOAD evidence: PENDING_LOCAL for this shift;
- PS2 hardware qualification: HARDWARE_PENDING and not claimed.

## Findings

V001 RESOLVED; V002 PASS; V003 OPEN; V004 OPEN; V005 HIGH/OPEN and blocks `VALIDATION_READY`.

## Exact next pickup

First inspect Reconstruction's disposition of V005 and verify deterministic fatal-session convergence without timeout masking, unsafe physical close, a second receiver, or loss of RFB safe-boundary ownership. Then independently review the application ownership/lifecycle fixture execution and Foreman-prepared canonical integration evidence. Do not accept A001 as `VALIDATION_READY` while V005 or V004 remains open.
