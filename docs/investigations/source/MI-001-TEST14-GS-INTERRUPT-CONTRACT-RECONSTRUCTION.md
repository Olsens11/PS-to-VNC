# Test14 GS / EE Interrupt Contract Reconstruction

Status: CURRENT TEST14 MECHANISM SYNTHESIS / HARDWARE-GROUNDED

Date: 2026-08-21

This is the authoritative synthesis of the Test14-Cf through Test14-Cg6
investigation of the gsKit HIRES HSync / EE INTC_GS handoff.

The evidence categories in this document are deliberate.

**DOCUMENTED HARDWARE FACT** means behavior supported by the public EE/GS
programming model reviewed during Test14.

**OBSERVED HARDWARE BEHAVIOR** means behavior directly measured on the PS2 by
the Test14 hardware campaign.

**PS2SDK / HOMEBREW HISTORICAL EVIDENCE** means behavior encoded in ps2sdk,
gsKit, or established homebrew practice.

**COMMERCIAL IMPLEMENTATION EVIDENCE** means behavior independently observed
in the commercial comparator reviewed during Test14.

**RECONSTRUCTED CONTRACT** means the smallest programming discipline that
explains the available evidence and survived the corrective hardware
experiment.

**HYPOTHESIS / OPEN DETAIL** means a lower-level mechanism that was not
directly observed.

Do not collapse these categories.

## Executive result

Test14 is no longer merely a 720p freeze-localization campaign.

The hardware experiments reconstructed a specific vulnerable interval in the
gsKit HIRES HSync callback:

    current INTC_GS callback is active
        ->
    current HSINT is acknowledged/re-armed early
        ->
    callback continues ordinary work
        ->
    another HSync can reassert CSR.HSINT
        ->
    the new source condition exists before callback retirement
        ->
    under the original delivery discipline the HIRES path can enter
    the terminal Test14 synchronization failure

The corrective discriminator tested in Cg5 was:

    mask HSync interrupt delivery
        ->
    acknowledge current HSINT at its original location
        ->
    perform ordinary callback work
        ->
    restore HSync delivery immediately before ExitHandler
        ->
    sync
    ei
    return

Cg5 remained healthy for 40 minutes of operator-observed use.

Cg6 then proved that the mask did not prevent a later HSync from occurring or
prevent CSR.HSINT from becoming asserted.

Instead:

    HSync delivery remained masked
        AND
    CSR.HSINT reasserted
        AND
    healthy frame/synchronization progress continued

Therefore the experimentally validated discriminator is suppression of
**interrupt delivery during the active callback**, not prevention of the
source event.

---

## Test14-Cf — failure is recoverable in place

### OBSERVED HARDWARE BEHAVIOR

Cf reproduced the genuine HIRES synchronization failure.

The main/render thread reached the HIRES synchronization boundary and stopped
making normal progress.

One controlled GS CSR HSINT acknowledgement then restored ordinary GS/HSYNC
interrupt activity.

A fresh stock-equivalent synchronization transaction subsequently crossed the
normal post-HIRES boundary.

Cf therefore established that:

- the terminal park was genuine;
- the HIRES machinery was not irreversibly corrupted;
- GS/EE interrupt service could be restored in place;
- a genuinely new synchronization transaction could succeed;
- reboot was not intrinsically required to recover this subsystem.

The controller thread remained responsive during the deliberate terminal park,
so "freeze" did not mean every PS2VNC thread had stopped.

---

## Test14-Cg1 — destination request already clear before ACK

### OBSERVED HARDWARE BEHAVIOR

Cg1 sampled EE `I_STAT` immediately before the ordinary gsKit HSINT
acknowledgement/re-arm.

Result:

    pre-ACK I_STAT.GS = 0

Therefore the old model:

    old I_STAT.GS remains occupied through the early source ACK

was falsified at the measured point.

The EE destination latch for GS was already clear before the ordinary source
acknowledgement.

---

## Test14-Cg2 — destination mask enabled

### OBSERVED HARDWARE BEHAVIOR

Cg2 replaced Cg1's one pre-ACK `I_STAT` observation with one pre-ACK
`I_MASK` observation.

Result:

    pre-ACK I_MASK.GS = 1

Therefore transient EE destination masking at that measured point does not
explain the failure.

Combined pre-ACK reconstruction:

    I_STAT.GS = clear
    I_MASK.GS = enabled

Cg1 and Cg2 were separate deliberately minimal probes.

---

## Test14-Cg3 — no fresh EE GS request visible pre-exit

### OBSERVED HARDWARE BEHAVIOR

Cg3 moved the observation point to the end of the ordinary HSync callback,
after the early HSINT acknowledgement and ordinary callback work, immediately
before `ExitHandler()`.

On a genuine failure run:

    pre-ExitHandler I_STAT.GS = 0

Thus no fresh EE GS request was visible at that logical point.

---

## Test14-Cg4 — GS source already reasserted pre-exit

### OBSERVED HARDWARE BEHAVIOR

Cg4 replaced Cg3's one pre-exit EE register read with one GS CSR observation
at the same logical callback location.

Normal initial execution:

    pre-exit CSR.HSINT = 0

Failure-associated final callback publication:

    pre-exit CSR.HSINT = 1

The terminal state again contained:

    CSR.HSINT asserted
    I_STAT.GS clear
    I_MASK.GS enabled

Cg3 and Cg4 are split probes from separate repeatable failure runs. They are
not one simultaneous register snapshot.

Together, however, they place the source-side/destination-side mismatch before
`ExitHandler()` begins:

    current callback active
        ->
    ordinary early HSINT ACK/re-arm
        ->
    ordinary callback work
        ->
    CSR.HSINT has reasserted
    I_STAT.GS is not visibly pending
        ->
    ExitHandler
        ->
    return

Cg4 therefore falsified the narrow model in which CSR.HSINT first enters the
eventual stranded asserted state only during or after `ExitHandler()` or ROM
kernel return.

Cg4 did not prove that return semantics are irrelevant.

---

## Test14-Cg5 — corrective source-delivery suppression

### OBSERVED HARDWARE BEHAVIOR

Cg5 changed the delivery discipline around the existing callback body while
preserving the original acknowledgement location and ordinary callback
semantics.

The tested order was:

    GS_IMR = 0x7f00
        HSync delivery masked
        ->
    original HSINT ACK
        ->
    ordinary callback scanline/pass work
        ->
    ordinary iSignalSema behavior when required
        ->
    pre-exit CSR observation
        ->
    GS_IMR = 0x7b00
        HSync delivery restored
        ->
    ExitHandler

Static machine-code validation established:

- the original HSINT ACK remained in place;
- ordinary callback work remained;
- the ordinary semaphore call remained;
- there were no new EE I_STAT/I_MASK writes;
- there was no callback telemetry I/O;
- both callback exits ended with HSync restore followed by `sync; ei`.

Cg5 remained healthy for 40 minutes of operator-observed use.

Immutable machine checkpoint:

    logs/test14C/hardware-cg5-20260820-235406/
        healthy-checkpoint-20260821-000923

Checkpoint seal SHA256:

    35b2dfeacfbb0513f11a19ffb62e0926fcb2e47e5d10e04d7fe74b04fab75d76

The immutable checkpoint and the later 40-minute operator observation are
different evidence statements.

Do not describe the entire 40 minutes as machine-sealed evidence.

---

## Test14-Cg6 — source accumulation while delivery is masked

### OBSERVED HARDWARE BEHAVIOR / SEALED

Cg6 preserved the Cg5 delivery discipline exactly and added one sticky RAM
latch at the existing pre-restore CSR sample:

    if (pre_restore_csr & HSINT)
        sticky = 1

The sticky value was reported through the already-existing periodic
main-thread DBG path.

Cg6 added:

- no callback network I/O;
- no formatted callback logging;
- no EE I_STAT/I_MASK manipulation;
- no second HSINT acknowledgement.

Sealed result:

    FIRST_G6HS1_FRAME=2
    LATEST_CONFIRMED_FRAME_AT_MACHINE_EXTRACTION=868
    FRAME_DELTA_AFTER_FIRST_G6HS1=866
    SECONDS_OBSERVED_AFTER_FIRST_G6HS1=136.534079
    DBG_PACKETS_AFTER_FIRST_G6HS1=7975
    T14C_REASON1_COUNT=0
    RECOVERY_TELEMETRY_COUNT=0

Therefore Cg6 directly observed:

    CSR.HSINT reasserted
    while HSync interrupt delivery remained masked

and then directly observed continued healthy synchronization afterward.

Evidence:

    logs/test14C/hardware-cg6-20260821-003801/
        mechanism-confirmed-20260821-004207

Seal SHA256:

    d4b586bbd9794b7d2a05221ff7f64384af192818ce70b34572e65cc4d51b4dfb

This is the decisive mechanism discriminator.

---

## What Cg6 proves about masking

Cg6 directly disproves the description:

    "masking prevents another HSync"

It does not.

The HSync event can still occur.

CSR.HSINT can still become asserted.

What changes is whether that newly pending source is exposed to the EE
interrupt-delivery path while the current callback is still active.

Therefore the validated corrective discipline is about **delivery timing**.

---

## DOCUMENTED HARDWARE FACT — EE INTC

The EE interrupt controller records interrupt requests in `I_STAT`.

The GS reaches the EE through the `INTC_GS` source.

The public EE programming model treats this as an edge-sensitive interrupt
path.

A following effective edge for the same source cannot simply be assumed to be
rediscovered while the corresponding destination state is unable to accept it.

For Test14, source acknowledgement/re-arm and destination retirement therefore
must be treated as one handshake rather than as unrelated status-bit
operations.

---

## DOCUMENTED HARDWARE FACT — GS source state and mask state differ

GS CSR interrupt fields participate in source acknowledgement.

GS IMR separately controls interrupt-source delivery permission.

Therefore these are distinct:

    GS CSR source condition
    GS IMR source-delivery permission
    EE I_STAT destination request
    EE I_MASK destination permission

Cg6 is a direct demonstration of why that distinction matters:

    CSR.HSINT = asserted

can coexist with:

    HSync delivery masked

---

## ExitHandler

### PS2SDK / HOMEBREW HISTORICAL EVIDENCE

ps2sdk requires `ExitHandler()` immediately before returning from an EE
interrupt handler.

Its low-level workaround sequence is represented by:

    sync
    ei

Test14 does not claim to possess the complete original Sony explanation of the
kernel defect that motivated this workaround.

### OBSERVED TEST14 MACHINE SHAPE

Cg5/Cg6 retained:

    restore HSync delivery
    sync
    ei

on both generated callback exits.

### RECONSTRUCTION LIMIT

Test14 does not directly observe the internal ROM-kernel transition after
delivery is restored and the current handler retires.

Therefore the project does not claim a cycle-by-cycle hardware fact for how an
already asserted GS source becomes the next dispatchable EE interrupt
transaction.

That remains a reconstructed lower-level detail.

---

## Commercial comparator

### COMMERCIAL IMPLEMENTATION EVIDENCE

The commercial Sony-SDK-era comparator examined during Test14 used the same
broad discipline for an active GS interrupt source:

    suppress source delivery
        ->
    acknowledge source
        ->
    perform callback work
        ->
    restore source delivery
        ->
    ExitHandler

It used direct volatile GS IMR access.

This is independent evidence that callback-duration GS source-delivery
suppression is a real PS2 interrupt-handling discipline rather than a
PS2VNC-specific invention.

It is not proof that every GS interrupt cause must use identical constants or
identical callback structure.

---

## RECONSTRUCTED CONTRACT

For the PS2VNC / gsKit HIRES HSync callback investigated by Test14:

1. Enter the `INTC_GS` callback.
2. Suppress HSync interrupt delivery while the current callback owns the
   transaction.
3. Acknowledge the current HSINT at the ordinary source-ACK location.
4. Perform normal scanline/pass accounting.
5. Preserve the ordinary semaphore signal behavior.
6. Do not add a blind second HSINT acknowledgement near return.
7. Restore HSync interrupt delivery only after ordinary callback work is
   complete.
8. Keep the restore immediately adjacent to interrupt retirement.
9. Execute the required `ExitHandler()` sequence.
10. Return.

For the exact PS2VNC/gsKit state tested:

    GS_IMR 0x7f00 = HSync delivery masked
    GS_IMR 0x7b00 = HSync delivery enabled

Only the HSync permission differs in the corrective A/B.

Those constants describe the tested state.

Generalized library code must preserve unrelated GS interrupt permissions
rather than blindly assuming all other GS IMR bits may be overwritten.

---

## REJECTED explanations

The complete Cg1-Cg6 campaign rejects these explanations for the observed
Test14 failure:

- the code simply forgot the normal HSINT acknowledgement;
- an old `I_STAT.GS` remains occupied through the early ACK;
- EE `I_MASK.GS` is disabled at the measured pre-ACK point;
- CSR.HSINT first becomes stranded only after `ExitHandler()`;
- the successful mask works by preventing HSync events;
- the evidence requires a blind second ACK near callback return;
- reboot is inherently required to restore the GS/EE synchronization path.

---

## HYPOTHESIS / OPEN DETAIL

Test14 does not directly establish:

- the exact ROM-kernel internal chronology after GS IMR restore;
- the exact effective-edge transition that produces the next EE `INTC_GS`
  transaction;
- whether identical delivery suppression is required for every other GS
  interrupt cause;
- the final generalized library design when multiple independent GS interrupt
  users coexist.

Those questions must remain labeled as reconstruction or future work.

---

## Production consequence

The next source change should not introduce another broad experimental theory.

It should remove Test14-only Cg6 telemetry while preserving the machine-
relevant Cg5/Cg6 discipline:

    callback entry
        ->
    HSync delivery suppressed
        ->
    original ACK
        ->
    ordinary callback work
        ->
    HSync delivery restored
        ->
    ExitHandler

The resulting clean implementation must then receive its own exact build and
hardware validation.

---

## Related evidence documents

- `docs/test14/TEST14-CG1-PREACK-ISTAT-RESULT-20260820.md`
- `docs/test14/TEST14-CG2-PREACK-IMASK-RESULT-20260820.md`
- `docs/test14/TEST14-CG3-PREEXIT-ISTAT-RESULT-20260820.md`
- `docs/test14/TEST14-CG4-PREEXIT-CSR-HSINT-RESULT-20260820.md`
- `docs/test14/TEST14-HSINT-INTC-EXITHANDLER-RECONSTRUCTION-PLAN-20260820.md`

Teaching and implementation references to be added by the same documentation
transaction:

- `docs/reference/PS2_INTERRUPTS_FROM_FIRST_PRINCIPLES.md`
- `docs/reference/EE_GS_INTERRUPT_HANDLER_GUIDE.md`
