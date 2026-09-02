# MI-001 — GS/EE HIRES HSync interrupt handoff and `ExitHandler()`

Status: `RESOLVED`

Historical campaign: legacy PS2VNC Test14 Cf/Cg1-Cg6

Opened historically: 2026-08-20

Mechanism synthesis: 2026-08-21

## Full preserved investigation

The complete historical mechanism-synthesis report has been copied into this
repository so future reconstruction does not require access to the legacy
PS2VNC repository:

- [`source/MI-001-TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md`](source/MI-001-TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md)

The preserved copy is byte-for-byte identical to the legacy file
`docs/test14/TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md` on branch
`experiment/test14-720p-freeze`; both have Git blob SHA
`4994b8e076c2e70248be433b466f0ccced5d0f02`.

Use the preserved full report when the detailed Cf/Cg1-Cg6 chronology, evidence
categories, commercial comparator, rejected explanations, measured states, or
reconstruction limits matter. This MI-001 file is the successor service-bulletin
summary and clean-project design guardrail.

## Executive summary

The legacy PS2VNC 720p/HIRES path could eventually freeze at a HIRES
synchronization boundary even though the whole application had not died. The
investigation initially had to distinguish GS rendering, semaphores, EE
interrupt state, GS HSINT acknowledgement, `ExitHandler()`, and mode-specific
timing.

The final hardware campaign reconstructed a vulnerable interrupt-delivery
interval rather than finding a conventional framebuffer bug.

The validated callback discipline is:

```text
enter INTC_GS callback
  -> suppress HSync interrupt delivery while this callback owns the transaction
  -> acknowledge the current HSINT at the ordinary ACK point
  -> perform ordinary scanline/pass/semaphore work
  -> restore HSync interrupt delivery immediately before interrupt retirement
  -> ExitHandler() (`sync; ei` in the observed machine shape)
  -> return
```

The important correction is **delivery suppression across the callback-owned
interval**. It is not a blind second HSINT acknowledgement and it does not prevent
physical HSync events from occurring.

This case is resolved at the engineering-contract level. The exact internal ROM
kernel chronology after source delivery is restored is still not directly
observable and remains a bounded implementation detail, not a blocker to the
validated contract.

## Symptom and impact

In the historical 720p HIRES path, ordinary rendering could eventually stop at
`gsKit_hires_sync()`. A frozen screen initially looked compatible with several
higher-level failures, but controller activity could remain alive.

Test14-Cf reproduced the genuine failure at frame 478 after frame 477 had
completed the observed render path normally. The main/render path stopped at the
HIRES synchronization boundary.

A single controlled write-one HSINT acknowledgement/re-arm to GS CSR restored
ordinary GS/HSYNC activity, and a fresh synchronization transaction then
completed. That established that the subsystem was recoverable in place and that
a PS2 reboot was not intrinsically required.

## Historical investigation

### Cf — prove the terminal state is real and recoverable

Cf established:

- the failure is genuine;
- the HIRES machinery is not irreversibly corrupted;
- GS/EE interrupt service can be restored in place;
- a genuinely new synchronization cycle can succeed afterward;
- a display freeze does not prove every PS2VNC thread is dead.

This moved the investigation upstream toward interrupt-source/destination
handoff rather than framebuffer/RFB mutation.

### Cg1 — destination request state before ordinary source ACK

Immediately before the existing gsKit HSINT ACK/re-arm:

```text
I_STAT.GS = 0
```

This falsified the simple theory that an old EE `I_STAT.GS` request remained
occupied through the early source acknowledgement.

### Cg2 — destination mask state

At the same logical pre-ACK point in a separate minimal probe:

```text
I_MASK.GS = 1
```

The GS interrupt destination was enabled. A disabled EE destination mask did not
explain the observed failure.

### Cg3 — destination request near callback retirement

Immediately before `ExitHandler()` on a genuine failure run:

```text
I_STAT.GS = 0
```

No fresh EE GS request was visibly pending at that measured point.

### Cg4 — source state near callback retirement

At the same logical pre-exit location in a separate minimal probe:

```text
normal initial execution:       CSR.HSINT = 0
failure-associated final state: CSR.HSINT = 1
```

The eventual stranded GS source had therefore already reasserted **before**
`ExitHandler()` began.

Cg3 and Cg4 were intentionally separate one-register probes, not a simultaneous
snapshot, but together they reconstructed the relevant mismatch:

```text
current callback active
  -> ordinary early HSINT ACK/re-arm
  -> callback continues work
  -> another HSync reasserts CSR.HSINT
  -> no fresh I_STAT.GS is visible at matched pre-exit point
  -> ExitHandler / return
```

This falsified the narrower theory that `ExitHandler()` itself first created the
stranded source condition.

### Cg5 — corrective A/B

Cg5 kept the original ACK point and normal callback behavior but changed delivery
ownership:

```text
mask HSync delivery
  -> original HSINT ACK
  -> ordinary callback work
  -> ordinary semaphore behavior
  -> restore HSync delivery
  -> ExitHandler
```

Static machine-code review confirmed there was no new EE `I_STAT`/`I_MASK`
mutation, no callback telemetry I/O, no second HSINT ACK, and both callback exits
ended with delivery restoration followed by the expected interrupt-retirement
shape.

Cg5 remained healthy for 40 minutes of operator-observed use. The immutable
machine checkpoint and the later operator-duration observation are separate
evidence statements and must remain described separately.

### Cg6 — decisive mechanism discriminator

Cg6 preserved the Cg5 delivery discipline and added only a sticky RAM observation
of whether CSR.HSINT reasserted before delivery was restored.

The sealed result observed:

- CSR.HSINT reasserted while HSync delivery was still masked;
- healthy synchronization continued for hundreds of frames afterward;
- no Test14 recovery reason fired;
- no callback network/logging perturbation was introduced.

This directly disproved the explanation that masking succeeds by preventing a
new HSync event.

The source event still occurs. The meaningful change is whether that newly
pending source is exposed to the EE delivery path while the current callback is
still active.

## What is known with high confidence

### Hardware-observed

- The historical HIRES freeze can leave the broader application partially alive.
- The GS/EE synchronization path can be restored in place.
- `I_STAT.GS` was clear before the ordinary early HSINT ACK in Cg1.
- `I_MASK.GS` was enabled at the corresponding Cg2 point.
- On a failure run, no fresh `I_STAT.GS` was visible immediately before
  `ExitHandler()` in Cg3.
- On a failure-associated run, CSR.HSINT had reasserted by the same logical
  pre-exit location in Cg4.
- Callback-duration HSync delivery suppression was a successful corrective A/B
  in Cg5.
- Cg6 directly observed HSINT source reassertion while delivery remained masked
  and continued healthy progress afterward.

### Source / platform contract

- GS CSR source state and GS IMR delivery permission are separate concepts.
- EE `I_STAT` request state and EE `I_MASK` destination permission are also
  separate concepts.
- PS2SDK documents `ExitHandler()` as an EE interrupt-handler return workaround
  that belongs immediately before handler return.
- The observed low-level machine shape retained `sync; ei` at interrupt
  retirement.
- A commercial Sony-SDK-era comparator independently used the same broad
  discipline: suppress source delivery, ACK source, perform callback work,
  restore source delivery, then `ExitHandler()`.

## Rejected explanations

The complete Cf/Cg1-Cg6 campaign rejects these explanations for the investigated
failure:

- the handler simply forgot the ordinary HSINT ACK;
- an old `I_STAT.GS` remained occupied through the early ACK;
- EE `I_MASK.GS` was disabled at the measured pre-ACK point;
- CSR.HSINT first became stranded only during or after `ExitHandler()`;
- delivery masking succeeds because it prevents HSync events;
- a blind second HSINT ACK near callback return is required;
- reboot is inherently required to restore GS/EE synchronization.

## Reconstructed root mechanism

The smallest mechanism consistent with the hardware evidence is:

```text
INTC_GS callback owns current interrupt transaction
  -> gsKit ACK/re-arms HSINT early
  -> callback remains active
  -> later physical HSync reasserts the GS source
  -> exposing that reasserted source during the still-active transaction can
     leave the HIRES delivery path in the stranded failure condition
```

The professional correction is not to make physical HSync impossible. It is to
make source-delivery ownership deterministic across the callback lifetime.

The exact cycle-by-cycle ROM-kernel handoff after delivery restoration is not
directly proved. That lower-level detail remains a reconstruction limit.

## Corrective contract / service bulletin

For the investigated PS2VNC/gsKit HIRES HSync callback:

1. Enter the `INTC_GS` callback.
2. Suppress HSync interrupt delivery while the callback owns the transaction.
3. Acknowledge the current HSINT at the normal source-ACK location.
4. Perform normal scanline/pass accounting.
5. Preserve normal semaphore signaling.
6. Do **not** add a blind second HSINT ACK near return.
7. Restore HSync delivery after ordinary callback work is complete.
8. Keep restoration immediately adjacent to interrupt retirement.
9. Execute the required `ExitHandler()` sequence.
10. Return.

The exact historical tested IMR constants were specific to the observed state.
Any generalized library implementation must preserve unrelated GS interrupt
permissions rather than overwrite the whole mask blindly.

## Design guardrails

Future reconstruction must preserve the **interrupt-ownership contract**, not
copy an accidental Test14 patch literally.

In particular:

- source condition, source-delivery permission, EE request state, and EE
  destination permission remain separate concepts;
- do not replace the validated discipline with periodic register clearing,
  arbitrary sleeps, or a second ACK without evidence;
- do not conflate PS2SDK `ExitHandler()` with application shutdown/exit;
- if multiple GS interrupt users are later introduced, preserve unrelated IMR
  ownership explicitly;
- a static screen alone never proves total application death.

## Residual open details

Resolution of this case does **not** claim:

- exact internal ROM-kernel instruction chronology after GS IMR restoration;
- the exact effective edge that becomes the next dispatchable EE transaction;
- that every other GS interrupt cause requires identical masking discipline;
- a final generalized multi-owner GS interrupt library design.

Those are future platform/library questions if a real requirement makes them
relevant. They do not invalidate the tested HIRES contract.

## Evidence and authority

The clean repository now contains the complete mechanism-synthesis report needed
to reconstruct the engineering conclusion without consulting the legacy
repository:

- [`docs/investigations/source/MI-001-TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md`](source/MI-001-TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md)
- Git blob SHA: `4994b8e076c2e70248be433b466f0ccced5d0f02`

Legacy PS2VNC remains read-only **provenance/evidence authority**, but is no
longer required merely to read the full report. Original provenance:

- repository: `Olsens11/PS2VNC`
- branch: `experiment/test14-720p-freeze`
- original path: `docs/test14/TEST14-GS-INTERRUPT-CONTRACT-RECONSTRUCTION.md`
- historical plan: `docs/test14/TEST14-HSINT-INTC-EXITHANDLER-RECONSTRUCTION-PLAN-20260820.md`
- related Cg1-Cg4 result documents under `docs/test14/`
- Cg5/Cg6 hardware evidence referenced by the preserved synthesis.

Successor durable lesson:

- `docs/audit/HISTORICAL_LESSONS.md`, especially L01, L10, and L11.

This case file is the successor index/closeout report. The preserved source copy
retains the full original mechanism synthesis; neither changes the immutable
legacy evidence.