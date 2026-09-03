# Issue #7 First Clean Hardware Qualification Plan

## Status

    WORKSTREAM=GITHUB_ISSUE_7
    ROLE=PRE_HARDWARE_TEST_DEFINITION
    CURRENT_BRANCH=reconstruct/issue7-minimal-core
    HARDWARE_RESULT=NOT_RUN
    OPERATOR_RESULT=NOT_RUN

This document defines the first real-hardware qualification target for the clean
PS2 reconstruction. It is deliberately written before deployment so the test is
not redesigned around whatever the first run happens to do.

The first hardware test exists to answer one narrow question:

> Can the clean reconstruction establish the private PS2-to-Pi network, complete
> an RFB 3.8 / Raw-only session, obtain one complete authoritative 704x462
> desktop, present it through the conventional Standard 480p GS path, and then
> service at least one ordinary incremental Raw update without losing framing or
> framebuffer authority?

It does not qualify later input, Hextile, menus, Refresh, configuration,
calibration, management, resize, or multimode behavior.

## Preconditions

Do not prepare or deploy a DUT until all of these are true:

1. PR #15 branch CI is green for the exact intended source head.
2. Host unit tests pass.
3. Strict PS2 translation-unit compile gate passes.
4. Separate clean linked-ELF build passes with the pinned toolchain image.
5. Qualified PS2IP archive identity is exactly:

       b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

6. The linked pristine ELF contains exactly one valid `PS2VNCIDv1!BLOB!`
   identity record and is still `UNSTAMPED` before DUT preparation.
7. Whole-ELF and PT_LOAD identities for the pristine build are recorded.
8. Initial-frame authority is proven by the clean implementation and host tests:
   the first non-incremental update must account for exactly the expected Raw
   byte total **and** cover every 704x462 pixel exactly once. Duplicate/overlap
   coverage cannot substitute for a gap, and `framebuffer.valid` remains false
   until both proofs succeed. The corresponding short/truncated, duplicate,
   overlap/gap, and malformed-frame negative tests must be green on the exact
   intended source head.
9. The clean Pi foundation is applied and verified.
10. The PS2-facing TigerVNC endpoint is validated independently before the PS2
    DUT is blamed for endpoint setup failures.

## Pi endpoint contract for this test

The first hardware test expects one deterministic RFB endpoint:

    Pi private IP = 192.168.50.1
    PS2 private IP = 192.168.50.2
    RFB TCP = 192.168.50.1:5900
    RFB desktop = 704x462
    server = adopted Xtigervnc runtime
    security offered = None
    fixed desktop geometry = yes

The endpoint should be restricted to the private PS2-facing interface. A listener
on Wi-Fi/all interfaces is an endpoint configuration failure, not a successful
qualification setup.

Before DUT deployment, record:

- exact TigerVNC package/version;
- exact service/invocation;
- exact listener ownership/address/port;
- RFB banner/security/ServerInit geometry from an independent endpoint probe;
- service start/stop/restart result;
- the deterministic desktop-change stimulus selected for the incremental-update
  portion of the test.

## Corrected qualification test identity

`ISSUE7-RAW480P-HW1` is preserved as the first formal hardware attempt. Its
product path passed, but the formal qualification failed H7 because the required
early runtime identity / startup-stage evidence did not survive unresolved ARP.

Do not reuse that test identity. The corrected startup-order requalification is:

    ISSUE7-RAW480P-HW2

The final manifest must not be committed with guessed ELF values. After the exact
pristine ELF has been selected, prepare one stamped DUT through the established
identity contract and record at minimum:

- source branch and exact commit;
- build script/Makefile identities;
- pinned PS2DEV image identity;
- qualified PS2IP SHA256;
- pristine whole-ELF SHA256;
- pristine PT_LOAD SHA256 and byte count;
- stamped test ID;
- stamped runtime identity digest;
- stamped whole-ELF SHA256;
- stamped PT_LOAD SHA256 and byte count;
- exact unique PS2 deployment path;
- exact rolling launch path, if one is used;
- deployment/readback identity when available.

Identity stamping is an experimental transformation. Never assume it leaves
PT_LOAD unchanged; measure and record the relationship.

## Machine evidence required

The clean milestone intentionally has small diagnostics rather than the full M4I
DBG/PRF/GEOM reporting surface. Qualification should require only evidence the
clean milestone actually owns.

### Runtime identity

The UDP observer must receive exactly the stamped runtime identity expected for
the DUT:

    PS2VNC_ID version=1 test=<test-id> digest=<64-hex>

An absent, malformed, duplicate/ambiguous, or mismatched identity is a failed
hardware qualification even if a picture appears on screen.

### Stage evidence

The current coordinator emits these optional diagnostic stage literals through
UDP 5999 when diagnostics are available:

    PSTVNC_STAGE NET_READY
    PSTVNC_STAGE GS_READY
    PSTVNC_STAGE DESKTOP_READY
    PSTVNC_STAGE FATAL

For a passing normal startup, require evidence of the ordered progression:

    identity
    NET_READY
    GS_READY
    DESKTOP_READY

`FATAL` at any point is failure evidence, not a recovery trigger.

Do not make diagnostics transport a normal product startup prerequisite. It is
allowed to be a qualification prerequisite for this test.

### Network/RFB capture

Capture the PS2-facing Ethernet traffic for the bounded qualification window.
The capture should allow later classification of:

- PS2 private-link activity;
- TCP connection to 192.168.50.1:5900;
- RFB protocol startup;
- the initial non-incremental framebuffer request/response;
- subsequent incremental request/response traffic;
- socket close/reset behavior if the run fails.

The capture is machine evidence. Do not infer user-visible picture quality from
packets alone.

## Physical/operator observations required

A passing first run requires a separate operator record confirming:

1. PS2 application visibly starts rather than returning immediately to OSDSYS.
2. Video output is stable Standard 480p.
3. A desktop image becomes visible after startup.
4. The visible desktop is not grossly corrupted, scrambled, obviously
   byte-swapped, or positioned outside the usable raster.
5. The first image remains stable while idle.
6. After the predetermined Pi-side desktop stimulus, the corresponding visible
   region changes without losing the rest of the authoritative desktop.
7. The application remains alive after that incremental update for the planned
   dwell period.
8. No unexplained automatic recovery/reconnect hides a silent stall.

The first test is not calibration qualification. Minor safe-area aesthetics do
not fail this milestone unless they prevent ordinary visibility or indicate the
wrong display path.

## Canonical successor apparatus

The corrected qualification uses only successor-owned tools:

    scripts/testkit/issue7-arm-observers.sh
    scripts/testkit/issue7-deploy-elf.sh
    scripts/testkit/issue7-stop-observers.sh
    scripts/testkit/issue7-result.py

Arming starts the UDP 5999 observer and PS2-facing packet capture before
deployment. The capture has a hard safety timeout but no automatic DUT recovery
or silent-stall masking. Observer shutdown is ownership-checked and hashes raw
run evidence before interpretation.

## Suggested test sequence

1. Verify exact GitHub branch head and green CI.
2. Produce the pristine clean ELF through `scripts/build-issue7-clean.sh`.
3. Reconfirm pristine ELF/PT_LOAD/dependency identities.
4. Prepare one unique stamped `ISSUE7-RAW480P-HW2` DUT.
5. Verify stamp reproducibility and create the exact successor DUT manifest.
6. Confirm the clean Pi endpoint is already validated and listening only on the
   intended private address.
7. Arm UDP identity/stage and PS2-facing packet observers with
   `issue7-arm-observers.sh`.
8. Deploy the exact stamped ELF with `issue7-deploy-elf.sh` and require both FTP
   readback hashes to match.
9. Launch the exact DUT on the PS2.
10. Wait for identity and ordered startup-stage evidence.
11. Record the initial physical 480p desktop result.
12. Apply the predetermined Pi desktop-change stimulus once.
13. Confirm incremental network traffic and the visible screen change.
14. Hold the application for the planned short dwell period.
15. Stop the owned observers with `issue7-stop-observers.sh` so raw evidence is
   hash-sealed.
16. Run `issue7-result.py` and record its machine result separately from the
   physical/operator result and detailed RFB-frame analysis.

Do not add feature work during the run to rescue a failure. Classify the failure,
preserve evidence, then design the next experiment.

## Failure classification

Use the earliest supported boundary rather than calling every symptom a
"freeze".

### H1 — application/lifecycle

Examples:
- ELF does not start;
- immediate OSDSYS return before network work;
- crash before meaningful traffic.

### H2 — PS2 private network

Examples:
- no carrier progression;
- no PS2 private-link packets;
- incorrect PS2/Pi addressing.

Carrier or successful ping alone does not prove application progress.

### H3 — TCP/RFB startup

Examples:
- TCP refusal/reset;
- wrong RFB banner/security;
- wrong ServerInit geometry;
- framing failure before authoritative frame.

### H4 — authoritative initial framebuffer

Examples:
- malformed/unsupported rectangle;
- incomplete/truncated full frame;
- strict-full completeness failure;
- framebuffer never becomes authoritative.

### H5 — GS presentation

Examples:
- RFB/desktop-ready machine evidence exists but physical output is absent or
  grossly corrupt;
- presentation path fails before stable first image.

### H6 — incremental Raw service

Examples:
- first image passes but predetermined desktop change never reaches PS2;
- incremental update loses framing;
- malformed/truncated update invalidates framebuffer;
- application stalls while Pi endpoint remains responsive.

### H7 — DUT identity/evidence

Examples:
- runtime identity missing or mismatched;
- intended deployment artifact cannot be tied to the executing ELF;
- required packet/stage evidence was not captured.

A physically attractive desktop with failed DUT identity is not a qualified
result.

## Pass definition

The first clean milestone earns hardware authority only when all of these agree:

    exact stamped DUT identity = PASS
    machine startup/RFB evidence = PASS
    initial authoritative Raw frame = PASS
    Standard 480p physical presentation = PASS
    one deterministic incremental Raw update = PASS
    short dwell / continued application life = PASS
    operator observation = PASS

If any element is missing, preserve the result as evidence but do not promote it
as Issue #7 hardware qualification.

## Explicit non-goals

Do not add or qualify in HW1:

- controller or pointer input;
- keyboard/OSK;
- Hextile;
- nonblocking receive/input concurrency;
- manual Refresh;
- automatic silent-stall recovery;
- menus/overlays;
- config/persistence;
- display-mode switching;
- calibration transactions;
- 576i/720p/1080i;
- Pi management 5959;
- VNC traffic pacing;
- WayVNC or remote-desktop routing.

Those return only after the Raw 480p baseline has earned hardware authority.
