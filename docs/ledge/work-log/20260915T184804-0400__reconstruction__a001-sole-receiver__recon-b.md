# Reconstruction Shift B — A001 logical RFB storage

DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-15T18:48:04-04:00
COMPLETED_AT=2026-09-15T18:49:20-04:00
ROLE_KEY=reconstruction
WORK_ITEM_KEY=a001-sole-receiver
WORKER_KEY=recon-b
STATUS=PARTIAL
STARTING_BRANCH_COMMIT=02ebed563a78ebb2a773da6f53673337ca938d05
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

## Objective and authority consumed

Continue audit-ready A001 from reconstruction state revision 0005 after the validation review of the sole physical receiver. Consumed reconstruction contract revision 0002, work-log contract revision 0001, A001 audit disposition, reconstruction state revision 0005, and the latest validation immutable record. Branch authority was re-read before each GitHub mutation. Unknown Pi-local work remains outside this GitHub-native mutation surface and was neither overwritten nor declared absent.

## Work performed

Added the smallest transport-owned logical RFB byte-storage mechanism needed above the physical frame receiver. `rfb_channel.*` owns caller-provided session storage, circular committed-byte state, exact logical reads, and a monotonically advancing producer-activity generation. It contains no physical socket access, RFB parser semantics, threading/semaphore policy, credit policy, or application lifecycle policy.

A complete inbound payload is rejected without mutation when capacity is insufficient; successful non-empty commits become visible atomically at the channel abstraction and advance activity generation once. Exact reads reject insufficient committed bytes without mutation. This prepares the higher receiver runtime to dispatch complete channel-1 DATA frames without teaching `physical_stream.*` RFB semantics.

## Exact commits/files

- `ac2cb88964c8af9ea25aced8cbee1a36447b5c7d` — add `src/transport/rfb_channel.h` logical-channel contract/state.
- `1b150cd73981334ad50faab5bd3b8740098965c1` — add `src/transport/rfb_channel.c` circular commit/read implementation.

## Checks and evidence

Static review against A001 required behavior confirms separation of physical framing from logical RFB storage and preserves parser ownership outside Transport. The mechanism does not use diagnostic counters as synchronization authority. No executable GitHub CI/status evidence was available for these new files during this shift.

PENDING_LOCAL: compile/build of `rfb_channel.*`; canonical `scripts/check.sh`; host tests for wraparound/full/empty/atomic-rejection/activity behavior; strict source-dictionary/topology checks; build integration; exact ELF/PT_LOAD identity; PS2DEV-dependent checks.

HARDWARE_PENDING: no hardware PASS claimed; A001 is still source-incomplete and has no newly established DUT identity.

## Known-defect accounting / blockers

The historical receiver-dispatch shutdown race remains unresolved and visible. This shift did not add the receiver runtime or quiescence mechanism and therefore did not silently claim the race fixed. Parser-consumption credit and terminal residual discard remain distinct outstanding behaviors.

V003 remains OPEN because A001 is not a coherent validation-ready tranche. V004 remains OPEN: the new `rfb_channel.*` symbols still need `src/transport/SYMBOLS.md` indexing and generated source-dictionary/topology integration before validation readiness.

No persistent blocker and no overlapping reconstruction mutation was observed.

## State/contract revisions

Consumed: reconstruction contract 0002; work-log contract 0001; reconstruction state 0005; latest A001 audit and validation authority.

Produced: this immutable shift record only. Reconstruction current state was not advanced because this was a bounded partial increment and the existing state plus this newer immutable handoff unambiguously identifies the continuation.

## Exact next pickup

Continue `a001-sole-receiver`. First index `rfb_channel.*` completely in `src/transport/SYMBOLS.md`. Then implement the higher transport receiver runtime as the sole caller of `pstvnc_transport_physical_stream_receive_frame()`: accept only audit-authorized channel-1 DATA into synchronized `pstvnc_transport_rfb_channel_t` storage and turn activity generation into a real non-blind producer rendezvous. Follow with parser-consumption credit/residual distinction, outbound fragmentation through the serialized send path, and explicit receiver-dispatch quiescence before any receiver-touched resource reclamation. Do not mark A001 VALIDATION_READY until V004, build/tests, exact ELF/PT_LOAD evidence, and required machine validation are complete.
