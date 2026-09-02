# MI-005 — Persistent 576p low-level display-transition stall

Status: `OPEN_NON_BLOCKING`

Successor working issue: #22

Historical authority: read-only legacy `Olsens11/PS2VNC`, especially the
D17 display campaign and its retained Test15 progression/state records.

## Executive summary

576p deserves its own major-investigation case because its history is both
persistent and easy to misclassify.

The earliest relevant comparison did **not** establish a software failure.
Exact historical Test13D 576p and a later then-current 576p path both produced
`No Signal` on the Insignia NS-19D220NA16. At that checkpoint it was reasonable
to treat sink/display compatibility as a plausible explanation.

Later D17AF-D17AJ testing superseded that provisional interpretation. Both
Standard and HIRES 576p paths produced a genuine low-level transition stall.
That substantially weakened a renderer-backend explanation and narrowed the
unresolved boundary into or around low-level GS mode setup / display transition.
The legacy project state subsequently made the classification explicit:

> keep 576p classified separately as an unresolved low-level mode-transition
> stall.

A later D17AL-F hardware attempt again produced No Signal with the PS2 apparently
hung. During that attempted switch the Windows VNC observer did not show the
expected desktop-size transition. That observation is consistent with the known
transition problem, but it must not be used to collapse two different facts:

- whether the television can display a 576p timing;
- whether PS2VNC remains alive and completes the software transition.

`No Signal` is a physical sink observation. It is not, by itself, proof of a PS2
software stall.

The durable engineering lesson is therefore:

> **Qualify display-sink compatibility and PS2 transition liveness separately.
> 576p remains a low-level software-transition investigation until evidence
> demonstrates a healthy PS2 transition independently of what a particular sink
> displays.**

This case is intentionally separate from MI-004. MI-004 tracks the broader
intermittent display-reconstruction/green-corruption family across mode changes
and transaction history. MI-005 tracks a repeated, mode-specific 576p transition
failure whose evidence already crossed Standard/HIRES renderer boundaries.
Future evidence may connect the mechanisms, but the project must not assume that
connection merely because both can end with a frozen or unusable display.

## Why this crosses the major-case threshold

This is not merely an unsupported video mode or ordinary compatibility bug.

The case has several properties that make its history architecturally important:

- the failure survived multiple display-architecture generations;
- the first plausible diagnosis (display compatibility) was later superseded by
  stronger low-level evidence;
- both Standard and HIRES implementations reproduced the stall;
- the current historical authority explicitly keeps 576p separate from resolved
  HIRES pitch/staircase work;
- the unresolved boundary lies close to GS mode setup / display-transition
  machinery rather than ordinary RFB rendering;
- a future implementation could easily repeat the incorrect inference that
  `No Signal == unsupported television == no software defect`;
- resolving 576p cleanly may teach something reusable about progressive DTV mode
  setup, transition sequencing, or low-level GS ownership.

## Historical progression

### 1. Initial 576p bring-up — No Signal, software status ambiguous

A historical 576p attempt selected the mode and the television reported No
Signal. At the same time, PS2VNC/test apparatus evidence did not yet provide a
sufficiently strong low-level discriminator to prove where execution had stopped.

The project therefore correctly avoided claiming that the renderer itself was
broken solely from the physical output.

### 2. Test15E3KH2 compatibility comparison

Test15E3KH2 compared exact historical Test13D 576p with the then-current 576p
path on the same Insignia NS-19D220NA16. Both produced the same No Signal
observation.

At that checkpoint the provisional disposition was that 576p might simply be
incompatible with the television/sink rather than a demonstrated regression in
the current renderer.

That conclusion was reasonable **for the evidence available at the time**, but
it is no longer current authority.

### 3. D17AF-D17AJ supersedes compatibility-only interpretation

Later D17AF-D17AJ testing changed the classification materially.

Both Standard and HIRES 576p paths subsequently produced a genuine low-level
transition stall. Because the failure crossed the Standard/HIRES backend
boundary, the evidence no longer supported treating it as a simple renderer
implementation defect or as television compatibility alone.

The retained legacy synthesis places the unresolved boundary into/around
low-level GS mode setup.

This is the pivotal discriminator in the case.

### 4. D17AL-F repetition

On the later D17AL-F ELF, selecting 576p again produced No Signal with the PS2
apparently hung. During the attempted switch, the Windows VNC observer did not
change desktop size.

The historical record classifies this as additional evidence for the already
known 576p transition failure. It specifically instructs future work to preserve
the D17AF-D17AJ low-level findings rather than reverting to the earlier
compatibility-only explanation.

### 5. 576p remains separate after other display cleanup closes

Later generalized HIRES width/pitch work successfully closed staircase defects
in other modes, including 576i and VGA 800x600-60. 576p remained No Signal and
was explicitly deferred separately.

That separation matters. The project already demonstrated that the generalized
HIRES pitch problem and the 576p failure are not interchangeable labels.

## What is known with high confidence

- Historical/current 576p physical attempts repeatedly produced No Signal on the
  tested Insignia sink.
- No Signal alone is not sufficient to classify software execution state.
- Test15E3KH2 initially supported a compatibility-only possibility because both
  an exact historical and then-current path looked the same on that sink.
- Later D17AF-D17AJ evidence superseded that provisional interpretation.
- Both Standard and HIRES 576p paths subsequently produced a genuine low-level
  transition stall.
- The unresolved boundary was narrowed into/around low-level GS mode setup rather
  than ordinary Standard-versus-HIRES rendering.
- The later D17AL-F attempt again failed before an observable Windows-side RFB
  desktop-geometry transition.
- Other generalized HIRES pitch/staircase defects were later fixed without
  closing 576p.
- Legacy project authority explicitly requires 576p to remain a separate
  unresolved low-level mode-transition problem.

## What is not proven

The retained synthesis does not justify claiming, without further evidence:

- the exact GS register write or kernel operation that stalls;
- whether the first failing operation is timing setup, synchronization,
  interrupt state, GS reset/reinitialization, or a tightly coupled adjacent step;
- that the television itself supports or does not support every PS2-produced
  576p timing variant;
- that Standard and HIRES fail for exactly the same micro-mechanism merely
  because both enter the same low-level boundary;
- that MI-001's HSync/`ExitHandler()` mechanism is the cause;
- that MI-004's intermittent reconstruction mechanism is the cause;
- that lack of an RFB desktop-size change proves the precise stall site;
- that a different television or capture device would display a healthy 576p
  signal if the software transition were corrected.

## Relationship to MI-001 and MI-004

### MI-001 — resolved HSync / `ExitHandler()` ownership

MI-001 establishes a durable interrupt-delivery contract for HIRES HSync
handling. Any future 576p implementation must preserve that contract where
applicable.

However, similarity of visible symptoms is not evidence that the resolved
MI-001 mechanism causes the 576p stall. Do not close MI-005 by analogy.

### MI-004 — intermittent display reconstruction liveness

MI-004 covers failures whose occurrence depends on transition history,
controller workload, RFB/request epoch, ordering, or other reconstruction
conditions across multiple modes.

MI-005 is more mode-specific: repeated 576p attempts fail across renderer
backends and are already localized toward low-level mode setup. Keep the cases
separate until a controlled experiment demonstrates a shared mechanism.

## Design guardrails while open

- Do not classify 576p support from television output alone.
- Keep **physical sink compatibility** separate from **PS2 software liveness**.
- Do not reclassify 576p as another HIRES pitch/staircase defect; that class was
  separately fixed while 576p remained unresolved.
- Do not work around the failure by hiding it behind safe-area calibration or
  RFB geometry.
- Preserve known-good display modes and their transaction behavior while testing
  576p; a 576p experiment must not destabilize 480p/720p/1080i authority.
- Keep the low-level transition instrumentation independent of RFB desktop resize
  so the first missing progress boundary can be identified.
- Do not silently promote a mode because a different sink happens to accept its
  signal. Healthy software progression and accepted physical timing are separate
  qualification gates.
- Conversely, do not reject a healthy software implementation merely because one
  known sink cannot display the timing; use an independent signal/capture method
  where practical.
- Preserve MI-001 interrupt-ownership rules during any GS transition rewrite.
- Treat legacy PS2VNC as read-only evidence authority.

## Future investigation plan

When 576p becomes active work again:

1. Start from the current clean known-good display implementation rather than
   reviving a historical D17 source tree as architecture.
2. Reconstruct the exact D17AF-D17AJ discriminator sequence from the immutable
   legacy evidence before designing a new test.
3. Establish two independent observation channels:
   - PS2 execution/progress telemetry through the low-level transition;
   - physical video/sink observation.
4. Instrument the transition in stages around the lowest practical GS mode-setup
   operations so the last proven progress point is known even if the screen is
   blank.
5. Keep RFB resize/presentation downstream of the low-level mode transition and
   record whether it is ever reached.
6. Compare a known-good progressive DTV transition (for example 720p where
   appropriate) against 576p with matched surrounding transaction machinery.
7. Where safe and meaningful, compare Standard and HIRES 576p using the same
   low-level instrumentation to identify the common failing boundary.
8. Preserve the MI-001 HSync-delivery contract and explicitly record whether the
   tested path invokes it.
9. Use an independent capture/display device if available so sink compatibility
   can be separated from software progression.
10. After fixing the transition, require both:
    - proven PS2 liveness through mode activation and return;
    - separately recorded physical 576p signal/display qualification.

## Closure criteria

MI-005 can close when the project can state, with controlled evidence:

- the first meaningful low-level failure boundary or equivalent causal contract;
- why both historical Standard and HIRES paths were vulnerable, or why their
  superficially similar failures were actually different;
- the corrective implementation/ownership rule;
- successful transition into and back out of 576p without losing PS2 execution;
- the relationship, if any, to MI-001 or MI-004;
- physical sink compatibility as a separately classified result;
- remaining limitations that do not invalidate the software conclusion.

It is not necessary to reverse-engineer every undocumented GS/kernel cycle if a
bounded causal contract and repeatable hardware qualification are sufficient to
make the mode reliable.

## Evidence and authority

Primary read-only historical authority:

- repository: `Olsens11/PS2VNC`
- branch: `experiment/test15E3K-D17AL-F-display-cleanup`
- `docs/PROJECT_STATE.md`
- `docs/architecture/DISPLAY_AND_GEOMETRY.md`
- `docs/tests/TEST15_DISPLAY_PROGRESSION.md`
- `docs/tests/Test15E3KH2.md`
- D17AF-D17AJ retained investigation/evidence referenced by those authorities.

Successor working thread:

- `Olsens11/PS-to-VNC` Issue #22.

This case preserves the historical knowledge boundary. It does not make 576p a
current release requirement or block unrelated clean reconstruction work.