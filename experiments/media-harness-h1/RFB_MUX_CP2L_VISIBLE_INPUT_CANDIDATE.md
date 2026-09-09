# CP2L visible RFB + PS2 mouse-input hardware candidate

## Status

**READY FOR HARDWARE QUALIFICATION; NOT YET HARDWARE QUALIFIED.**

CP2L extends the CP2K visible-RFB-only checkpoint by activating only the existing
through-Issue-39 PS2 controller/mouse semantic path. The controller worker remains
a semantic-event producer; the RFB-owning application/main thread drains those
events only at complete RFB server-message boundaries and performs the native RFB
pointer writes.

This candidate does not activate keyboard serialization, OSK, local UI, AUDIO,
MPEG, hybrid composition, or Issue #40 work.

## Scope guard

- Issue frontier: through Issue #39.
- Issue #40 started: **NO**.
- Physical Pi-to-PS2 PSTV TCP streams: **1**.
- RFB logical channel: **1**.
- RFB mode: **2 (visible)**.
- Presentation: CP2K through-Issue-39 display/PS2 graphics path.
- PS2 input: **mouse/pointer semantics only**.
- Keyboard: **OFF**.
- OSK/local UI: **OFF**.
- Audio: **OFF**.
- MPEG: **OFF**.

Controller semantics available to this checkpoint are the already-earned
through-Issue-39 mouse mappings: D-pad and left stick pointer movement, Cross
left click, Circle right click, Triangle+D-pad wheel, and L3 analog-wheel-mode
toggle. The first hardware qualification need only prove ordinary pointer motion
and clicking; wheel behavior may be exercised as additional evidence.

## Exact source/build authority

Branch:

    experiment/h1-rfb-mux-prep

Exact CP2L source/build head:

    75ed9aae1764c256a8019b5a4a4ff3b5945721cc

The preceding implementation commit was:

    c5875b9d70c15fb2ddbccbc79bd92722f1f8772d

Its first CI attempt exposed only a make dependency-scheduling error: the CP2L
input-service object was present in the final link expansion but was not yet an
explicit prerequisite, so make reached the link before building it. Commit
`75ed9aae...` repaired that build-graph wiring and added a source check for the
explicit prerequisite. No hardware claim is transferred from that failed build.

Pinned CI authority:

    workflow=H1 RFB mux preparation checks
    workflow_run=34324553964
    workflow_result=PASS
    host_preflight=PASS
    ps2_build=PASS
    pinned_toolchain=ps2dev/ps2dev@sha256:8fba50ecc2229acd7f8da63d34302f12939b7d4fa6848dda1e6a0ce083321a11

Exact CI-built CP2L ELF:

    ELF=PS2VNC-H1-CP2L-VisibleRFBInput.ELF
    ELF_SHA256=651da23aff616784cd38715a46fe639fa8adc985d290c2c80275b4c5b396b73f
    ELF_BYTES=3028556
    PT_LOAD_SEGMENTS=1
    PT_LOAD_SHA256=7579a7d2a21fa8f80ba8290c69bcb7c094e2c814c313f32327cfc8188db797db
    PT_LOAD_BYTES=488980

CI artifact:

    artifact_name=h1-cp2l-visible-rfb-input-unqualified-elf
    artifact_id=10093217940
    artifact_zip_sha256=d518bdd72633da039a5d5eb5d77984153ab55ef28f27fb15cbe143d67583ce8a
    artifact_zip_bytes=1062577

The downloaded CI artifact was independently unpacked after the run and contained
exactly one 3,028,556-byte ELF whose SHA-256 matched the CI console identity above.

## Architectural gates passed

The green workflow proved the CP2L source and linked-object contract, including:

- exactly one physical PSTV transport remains the RFB path;
- CP2J/CP2K logical-channel queue, credit, mux, and quiesce mechanics remain in use;
- CP2K's existing presenter-only coordinator remains input-inert;
- CP2L alone uses the new application-service seam;
- the controller/input runtime is the existing through-Issue-39 owner;
- the controller worker never owns RFB socket writes or GS presentation;
- mouse semantic events are serialized by the main/RFB-owning thread;
- keyboard events fail closed in this checkpoint and no key serializer is linked
  from the CP2L input-service object;
- AUDIO, MPEG, OSK, and local UI are inactive by the CP2L coordinator;
- input shutdown remains cooperative and CP2L parks rather than reusing worker
  storage if dormancy cannot be proven.

The workflow also rebuilt and rechecked the inherited RFB mux seam and prior
source-level contracts. Those checks do not transfer real-hardware qualification
to this changed PT_LOAD.

## Hardware gate

The exact ELF above has a changed PT_LOAD and is therefore **unqualified until a
real PS2 run observes it**.

The first qualification should keep the same Pi RFB-only profile used for CP2K:
AUDIO OFF, MPEG OFF, CONFIG mode 2, upstream VNC `127.0.0.1:5903`, and the single
PSTV listener on port 5902. During the finite session the operator should verify
on the PS2/TV that the desktop remains live while PS2 controller input moves the
remote cursor and performs at least one left click. A right click is a useful
second proof. The run must still complete the existing RFB quiesce handshake and
return a clean H1 result.

Until that observation and sealed run evidence exist:

    CP2L_HARDWARE_QUALIFIED=NO
    CURRENT_HARDWARE_AUTHORITY=CP2K_VISIBLE_RFB_ONLY

