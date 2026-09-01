# B12-B14 Diagnostics, Pi Companion, and Development Infrastructure Behavioral Audit

## Status

    AUDIT_WORKSTREAM=GITHUB_ISSUE_2
    TRANCHE=B12_B13_B14
    MATURITY=EVIDENCE_SUPPORTED
    ROLE=DETAILED_BEHAVIOR_AUDIT

This document records the source-plus-historical-evidence audit for the final
three seeded behavior families: diagnostics/identity, the Raspberry Pi
companion, and product-significant development infrastructure.

It is subordinate to `docs/audit/BEHAVIORAL_INVENTORY.md`.

The reconstruction rule remains:

> preserve behavior and proven mechanisms, not accidental historical structure.

These three domains are grouped because they close the evidence and
reproducibility surface around the runtime product. They remain distinct product
responsibilities.

---

## B12 — Diagnostics, runtime identity, telemetry, profiling, and reporting

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Make internal runtime state observable enough to classify failures, measure
performance, identify the exact running executable, and create trustworthy
qualification evidence without making diagnostics a prerequisite for ordinary
product startup.

B12 does not own the product state it reports. Networking, RFB, presentation,
input, UI, and display transactions retain their own state; diagnostics receives
snapshots or stage changes and serializes them for evidence.

### Optional diagnostics transport

The permanent diagnostic transport uses UDP to the Pi at:

    192.168.50.1:5999

`pstvnc_debug_init()` owns only the diagnostic UDP socket/address. Failure to
initialize diagnostics is non-fatal to ordinary startup. Runtime code can query
whether diagnostics are ready and report nothing when they are unavailable.

This is an important product boundary: observability may be mandatory for a
particular qualification test while remaining optional to normal VNC use.

### Named runtime stages

The normalized debug layer represents important execution locations as semantic
stages rather than scattering free-form strings through product code. Existing
stage names cover startup, network readiness, GS initialization, controller
startup, draw/flip boundaries, RFB request/receive boundaries, texture work, and
major controller/request/receive failures.

The stage value is diagnostic state, not product control state. Updating it must
not decide whether networking, RFB, presentation, or input succeeds.

### Structured evidence reports

The permanent report layer serializes typed snapshots into several evidence
families:

- `DBG` — execution stage plus current RFB rectangle/header state, controller,
  OSK/hotkey/menu/action state, display-transition phase, libpad pause state,
  and outbound queue depth;
- `PRF` — update counts, changed bytes, Raw/Hextile counts, and timing buckets
  for presentation, RFB, receive wait, Hextile, and dirty-region work;
- `GEOM` — safe-area/screen-fit state, logical desktop geometry, output
  placement, config-loaded state, and force-calibration state.

The durable architectural rule is that subsystem-owned typed state is collected
into explicit report values before serialization. Diagnostics should not reach
through unrelated modules and become a second mutable owner of product state.

### Exact runtime identity

Runtime identity is deliberately stronger than a build-time filename or operator
claim.

A fixed-size identity blob is linked into the ELF and can be stamped after link
with:

- a test ID;
- a 64-hex-character identity digest.

The linker wraps `sendto()`. Immediately before the first diagnostic UDP
datagram to port 5999, the running ELF emits exactly one identity packet through
the same destination/socket path:

    PS2VNC_ID version=1 test=<test-id> digest=<64-hex>

This ties later telemetry to the executable that actually reached runtime.

### The 106-byte identity failure

M4I produced an important infrastructure failure that became a durable runtime
lesson.

The first hardware identity attempt had:

- expected payload length 107 bytes;
- observed payload length 106 bytes;
- a historical Test15 identity with expected 108 bytes also observed at 106.

The stamped ELF storage was correct and the TestKit parser correctly failed
closed. The fault was the target runtime identity serialization path. A visible
480p desktop and DBG traffic were present, but hardware qualification was
**not** granted because exact DUT identity had not been proved.

The repair removed printf-family formatting from the identity path and replaced
it with deterministic bounded byte serialization. The format helper explicitly
validates test-ID length, exact digest length, hex characters, output capacity,
and NUL termination.

Do not overgeneralize this result. The evidence proves the need for deterministic
identity serialization on this target/path. Other DBG/PRF/GEOM report
serializers currently use bounded `snprintf()` successfully; they should be
changed only for a demonstrated reason.

### Qualification role of identity

M4I-FINAL-HW1 directly proved the completed diagnostics generation:

- runtime identity result: PASS;
- identity packets: 1 received / 1 parsed / 1 unique;
- observed test ID and identity digest exactly matched expected values;
- DBG records: 2460;
- PRF records: 3;
- GEOM records: 11;
- diagnostic error gate: PASS;
- TestKit state: SEALED;
- five-mode machine result: PASS 5/5;
- physical result: FULL PASS;
- operator result: FULL PASS.

The sealed hardware authority records the exact tested ELF, PT_LOAD, runtime
identity digest, test definition, and evidence seal.

### Diagnostics versus qualification policy

B12 provides identity and telemetry mechanisms. B14 decides what a specific
experiment requires before a result can become authority.

For example, the M4I final manifest required runtime identity plus DBG, PRF, and
GEOM traffic and a physical operator pass. A different experiment may require a
different evidence set without changing the ordinary diagnostics API.

### Invariants

- diagnostics failure does not become an ordinary product-start prerequisite;
- a qualification that requires diagnostics may fail closed if required
  identity/telemetry is absent or malformed;
- runtime identity must prove the executing DUT, not merely the intended
  deployment artifact;
- identity serialization remains deterministic and bounds-checked;
- identity is emitted before ordinary diagnostic telemetry from that socket/path;
- product subsystems retain ownership of the state diagnostics reports;
- telemetry must not alter product control flow merely to make a test pass;
- evidence claims distinguish observed telemetry from inferred user-visible
  behavior.

### Primary authority

- `src/diagnostics/identity.c` / `identity.h`;
- `src/diagnostics/debug.c` / `debug.h`;
- `src/diagnostics/report.c` / `report.h`;
- M4I identity-failure classification and deterministic-serialization repair;
- `tests/m4/M4I-FINAL-HW1.env`;
- `evidence/m4/m4i-final-hw1-hardware-qualification/`;
- final M4I hardware authority and identity proof.

### Clean-rebuild implication

Keep diagnostics as a narrow optional service: semantic stage tracking,
structured reports, and exact runtime identity. Give product modules small
snapshot/report interfaces rather than exposing their internals to a global
logger. Preserve the deterministic identity mechanism and qualification-facing
identity contract from the start of reconstruction.

---

## B13 — Raspberry Pi companion

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Provide the ordinary Linux environment and the small amount of custom
PS-to-VNC companion behavior needed to make the PS2-side client useful,
recoverable, and reproducible.

The Pi is part of the product, but it should not become a bespoke operating
system. Commodity responsibilities remain standard OS responsibilities wherever
possible.

### Product philosophy: commodity first

`docs/PROJECT_INTENT.md` establishes the clean rule:

- begin from a normal supported Raspberry Pi OS baseline;
- use ordinary package management;
- use standard networking/service management/SSH/Samba where they fit;
- add custom Pi-side code only for behavior PS-to-VNC actually needs;
- make a fresh supported OS reproducibly transformable into the companion;
- treat the installer/package definition, not a hand-tuned SD image, as the
  canonical environment;
- record adopted/rejected dependencies and configuration changes in a dependency
  ledger.

This means the historical development Pi is evidence, not the installation
model for the clean product.

### Historical proven companion runtime

Test12 promoted the Pi side from manually launched development processes into an
installed supervised runtime. The later final installer retains that model.

Its ordinary runtime package dependencies include standard components such as:

- TigerVNC standalone server;
- Openbox;
- lxpanel;
- xterm/X11 utilities;
- ordinary Python;
- standard Linux networking tools.

The installer copies custom PS2VNC runtime code/configuration into installed
locations under `/usr/local` and `/var/lib/ps2vnc`, rather than requiring the
product to execute from a mutable development checkout.

### Dedicated PS2 network

The historical companion owns the private PS2-facing address:

    Pi eth0 = 192.168.50.1/24

It uses both persistent NetworkManager configuration and an immediate systemd
network unit. The explicit systemd bring-up exists so the interface/address is
available even when physical Ethernet carrier is absent, avoiding a startup race
where the PS2 reaches the Pi before NetworkManager reacts to carrier.

The private link is marked never-default; general Internet/Wi-Fi routing remains
a separate platform/product concern.

### Dedicated VNC desktop

The proven runtime launches a dedicated TigerVNC X display:

    DISPLAY=:1
    geometry=1280x720
    depth=24
    RFB TCP 5900
    bind/interface=192.168.50.1
    SecurityTypes=None
    AlwaysShared

Openbox and lxpanel are ordinary desktop children of this dedicated X/VNC
session. They are not started until `xdpyinfo` proves the X display is actually
usable.

The exact future desktop components can be reconsidered on the clean supported
OS; the durable behavior is a predictable dedicated PS2 desktop/session with a
known VNC endpoint and usable local tools.

### Service graph and health

The historical runtime uses systemd instead of manual shell sessions. Its core
graph includes:

    ps2vnc-network.service
        -> ps2vnc-vnc-pacing.service
            -> ps2vnc-vnc.service
                -> ps2vnc-openbox.service
                -> ps2vnc-panel.service

A separate `ps2vnc-health.service` supervises runtime usability.

The health supervisor does not trust process state alone. It verifies/recoveries
include:

- eth0 administratively UP;
- 192.168.50.1/24 present;
- expected VNC pacing state;
- VNC service active;
- 192.168.50.1:5900 actually listening;
- `DISPLAY=:1` actually responding;
- Openbox/lxpanel running only after X is usable.

A process being `active` is therefore not sufficient evidence that the service
is useful to the PS2.

### Management service is separate from VNC

The management/control service listens on TCP 5959, independently from RFB
5900. The historical systemd unit explicitly states that management failure must
not prevent the VNC desktop from running.

The master target requires the graphical/network stack but only *wants* the
management service. This mirrors B11's semantic distinction between core desktop
availability and optional/best-effort management clients.

The installed management service owns persistent configuration and durable
transaction/control files in its state directory. The PS2 interacts through the
management protocol rather than knowing the Pi filesystem layout.

### Runtime operator controls

The installed runtime provides ordinary service operations:

    ps2vnc-status
    ps2vnc-start
    ps2vnc-stop
    ps2vnc-restart

Status checks services, Ethernet/carrier, VNC pacing, TCP 5900, TCP 5959, X
usability, and relevant processes. These operations are part of the companion
runtime contract and should be preserved in a clean successor form rather than
requiring SSH users to reconstruct service commands manually.

### VNC-specific traffic pacing

The final historical runtime includes a narrowly scoped Linux traffic-control
rule for RFB traffic from TCP 5900 to the PS2:

    rate=60mbit
    burst=16kb
    latency=50ms

It does not intentionally pace every PS2/Pi packet, and it refuses to replace an
unknown foreign root qdisc.

The health supervisor treats this as expected historical runtime state and the
final hardware campaigns ran with the installed runtime. However this audit does
not claim that each pacing constant has been independently isolated as
universally necessary.

The clean Pi baseline should record this mechanism in the dependency/runtime
ledger and deliberately preserve, replace, or requalify it rather than silently
assuming either that it is mandatory forever or that it can be removed without
testing.

### Samba/file storage scope

The broader project intentionally uses standard Samba/file services where
useful, but the proven VNC runtime installer does not require Samba to establish
PS2VNC networking, VNC, management, desktop, or recovery.

Therefore Samba is a commodity companion capability, not an implicit dependency
of the core VNC path. Future game/file utilities may make particular share
contracts product-significant; those should be specified explicitly rather than
folded into the VNC service graph by accident.

Likewise, PS2 development toolchains, unrelated containers, caches, and other
features of the exploratory Pi are development-environment concerns unless a
clean product requirement earns them a place in the companion installer.

### Reproducibility contract

A fresh supported Raspberry Pi OS must be transformable into the intended
companion through versioned tracked installation definitions. Every deliberate
manual deviation should become declared installer state or be removed.

At meaningful milestones, the installer/package definition should be tested on
a clean supported OS. A ready-to-flash image can be a convenience artifact but
must not become the only definition of the system.

### Invariants

- the Pi companion remains based on a supported ordinary OS;
- standard platform mechanisms are preferred to bespoke replacements;
- the PS2-facing private Ethernet address is available independently of carrier
  timing;
- VNC desktop availability does not depend on management TCP 5959 succeeding;
- desktop children do not race an unusable X/VNC root;
- health checks actual service endpoints/usability, not process existence alone;
- custom persistent state lives outside a mutable source checkout;
- historical development-machine baggage is not silently promoted to runtime
  dependency;
- every adopted runtime dependency/configuration change becomes reproducible
  installer/ledger state;
- VNC-specific pacing remains an explicit qualified/evaluated mechanism, not an
  invisible machine tweak;
- Samba/file services are included only where a product behavior actually
  requires them.

### Primary authority

- legacy `docs/tests/TEST12_RECOVERY.md`;
- legacy `pi/runtime/install-runtime.sh`;
- legacy `pi/runtime/management-server.py`;
- legacy `pi/runtime/vnc-pacing.sh`;
- successor hardware evidence snapshots showing the installed service graph;
- `docs/PROJECT_INTENT.md` for the clean Pi philosophy/reproducibility contract.

### Clean-rebuild implication

Define the companion as a reproducible package/service layer on top of a normal
supported Raspberry Pi OS. Keep private Ethernet, dedicated VNC desktop,
management service, runtime state, health/recovery, and operator controls
explicit. Re-evaluate historical package choices and VNC pacing through the
dependency ledger; do not clone the exploratory Pi wholesale.

---

## B14 — Product-significant build, deployment, test, evidence, and qualification infrastructure

**Maturity:** `EVIDENCE_SUPPORTED`

### Purpose

Make changes to PS-to-VNC reproducibly buildable, deployable, identifiable, and
qualifiable on real PS2 hardware while preserving the meaning of prior evidence.

B14 is not a runtime user feature. It is product-significant because hardware
claims, dependency identity, exact DUT identity, and reproducibility constrain
which code can safely become product authority.

### Canonical build authority

Routine builds use `scripts/build.sh`. The current exploratory-generation build
wrapper demonstrates two durable requirements:

- pin the build environment/toolchain by immutable container image digest;
- pin behaviorally significant external dependencies by exact hash.

In particular the qualified MTU1458/window-scale PS2IP archive is checked against
its expected SHA256 before it is copied into the build tree. A nominally similar
SDK archive is not silently substituted.

The clean reconstruction may replace this particular B4A-oriented build layout,
but it must retain reproducible toolchain/dependency identity.

### Exact DUT identity

Hardware evidence must identify the executable actually under test. The project
uses multiple related identities because they answer different questions:

- source authority / source commit;
- whole ELF SHA256;
- PT_LOAD SHA256 and byte count;
- stamped runtime identity digest;
- deployment/readback identity where applicable.

Whole-ELF equality and runtime-loaded PT_LOAD equality are related but not
interchangeable claims.

### PT_LOAD hardware gate

The development policy is explicit:

> A changed PT_LOAD requires hardware qualification unless an explicit recorded
> project rule permits inherited qualification.

Qualification transfer is therefore a recorded conclusion with a stated basis,
not a casual assumption that a source refactor is harmless. M4F is an example
where byte-exact PT_LOAD identity legitimately allowed hardware authority to be
transferred from an already qualified image.

### Hardware identity preparation

`prepare-hardware-elf.sh` turns a pristine build into a named hardware DUT using
established TestKit identity tools. It:

- refuses missing/ambiguous inputs or overwrite of an existing output;
- stamps the test ID/digest;
- verifies the stamped identity;
- repeats the stamp independently and requires byte-identical output;
- fingerprints whole ELF and PT_LOAD before/after stamping;
- records exactly whether identity stamping changed the runtime-loaded image.

The infrastructure therefore does not assume that identity metadata lives
outside PT_LOAD. The **stamped artifact actually deployed** is the DUT whose
runtime identity and PT_LOAD must be qualified.

### Declarative experiment definition

Hardware tests use versioned manifests that pin the experiment before execution.
The final M4I manifest records, among other things:

- test ID;
- source/build authority;
- source and Makefile hashes;
- ELF and identity hashes;
- PT_LOAD hash;
- PS2 FTP destination names;
- required execution-proof regex;
- observer/capture tool hashes;
- UDP/PCAP endpoints;
- capture-space/time budgets;
- exact qualification workload/mode sequence;
- required runtime telemetry classes;
- required physical operator pass.

Changing a named identity or apparatus component changes experimental meaning
and must be recorded rather than silently treated as the same run.

### Inherited TestKit without legacy mutation

The successor initially retains the mature legacy TestKit through an explicit
bridge rather than copying commands informally from memory.

The bridge:

- requires the exact frozen legacy Git HEAD;
- refuses tracked working-tree or index mutation in legacy;
- preserves pre-existing untracked forensic/runtime artifacts;
- copies the legacy TestKit tree byte/mode-exactly into an isolated successor
  hardware workspace;
- records successor HEAD, legacy HEAD, manifest hash, TestKit tree hash, and
  workspace provenance;
- revalidates those identities before use;
- exposes only an allowlisted inherited operation set such as verify-build,
  deploy-elf, start-hardware, status, watch, and result.

This preserves proven apparatus while keeping `/home/ps2/ps2vnc` immutable.
Successor-owned self-tests verify the bridge and identity/authority helpers.

### Toolkit-first routine operations

Routine build, deployment, identity, hardware-test, evidence, and fingerprint
mechanics should use saved tooling when it exists. Repeated ad-hoc procedures
are development friction and should graduate into tested, documented tools
rather than being regenerated each session.

This is not bureaucracy: changing apparatus can change the meaning of a hardware
experiment.

### Machine evidence and physical evidence

Machine evidence and operator observation are independent authorities.

A telemetry log can prove protocol/state events; it cannot by itself prove that
an image was visibly correct on a physical display. Likewise an operator report
cannot prove the exact ELF or packet stream without machine identity/evidence.

Tests that require both must record both. M4I-FINAL-HW1 required and obtained a
machine PASS, physical FULL PASS, and operator FULL PASS.

### Fail closed on invalid evidence

The M4I 106-byte identity incident is a key infrastructure example. The DUT
appeared to run and emitted diagnostics, but exact identity failed. TestKit did
not reinterpret the truncated packet or grant qualification; the run was
classified invalid, source was repaired, and a new hardware qualification was
required.

This is the desired evidence posture: uncertainty about DUT/apparatus/evidence
identity invalidates the claim rather than being patched with intuition.

### Evidence preservation and sealing

Captured empirical files under `evidence/` are byte records, not ordinary source
text. Their recorded hashes/manifests define integrity; generic whitespace or
line-ending normalization must not rewrite them.

A completed test can be sealed with a recorded evidence manifest/result. M4I
records a SEALED TestKit state and seal-record SHA256 in the final hardware
authority.

### Development continuity is part of correctness

The repository is intended to reconstruct an effective development session
without relying on conversation memory. Current state, architectural intent,
decisions, empirical evidence, history, and development policy have separate
authorities.

Dirty/uncommitted state is presumed meaningful until inspected. Development
procedures must not reset, clean, or overwrite unknown work merely to simplify a
session handoff.

### What does not automatically survive reconstruction

Migration-specific extraction machinery, temporary forensic scripts, and
one-off apparatus are historical unless they express a recurring invariant.

B14 preserves the **contracts** that remain product-significant:

- reproducible build/dependency identity;
- exact DUT identity;
- routine canonical deployment;
- declarative experiment definition;
- apparatus provenance;
- machine/operator evidence separation;
- PT_LOAD hardware gate/explicit qualification transfer;
- raw evidence integrity/sealing;
- safe session continuity and dirty-state preservation.

The clean project should not carry every M0-M4 migration mechanism merely
because it once helped reorganize B4A.

### Invariants

- routine operations use canonical saved tooling when available;
- build environments and behaviorally important dependencies are explicitly
  identified/pinned;
- a hardware claim names the exact DUT actually deployed/executed;
- PT_LOAD changes cross the hardware gate unless an explicit authority records a
  valid transfer;
- apparatus changes are experimental variables;
- machine and physical/operator evidence remain separate;
- invalid/ambiguous identity fails closed;
- raw evidence is preserved byte-for-byte and can be sealed/hashed;
- the legacy repository remains immutable while inherited apparatus is used;
- development continuity must preserve unknown dirty work, not erase it;
- migration history is not mistaken for permanent product infrastructure.

### Primary authority

- `scripts/build.sh`;
- `scripts/testkit/prepare-hardware-elf.sh`;
- `scripts/testkit/pt-load-fingerprint.sh`;
- `scripts/testkit/legacy-hardware-bridge.py` and successor TestKit self-tests;
- `tests/m4/M4I-FINAL-HW1.env` as an example declarative hardware manifest;
- `evidence/m4/m4i-final-hw1-hardware-qualification/`;
- `AGENTS.md`;
- `docs/development/README.md`;
- `docs/development/testing.md`;
- `runtime/DEVELOPMENT_SYSTEM.env`.

### Clean-rebuild implication

Keep a small successor-owned development toolchain around explicit contracts:
build, dependency identity, ELF/PT_LOAD/runtime identity, deployment, experiment
manifest, capture, result, and evidence seal. Continue using inherited legacy
TestKit only through the proven immutable bridge until equivalent successor-owned
mechanisms are deliberately adopted and qualified.

---

## Cross-domain conclusions from B12/B13/B14

1. **Observability is optional runtime support but can be mandatory experiment
   evidence.** Product startup and qualification requirements are different
   contracts.
2. **Runtime identity closes the DUT-to-telemetry loop.** A filename, intended
   deployment, or visible desktop is not enough to prove which executable
   produced evidence.
3. **The Pi is a companion, not a custom OS.** Standard Linux mechanisms should
   carry commodity responsibilities; custom code exists only where PS-to-VNC
   needs product-specific behavior.
4. **Service health is semantic.** `active` is not sufficient when the required
   listener/X/display is unusable.
5. **The development machine is not the product.** A fresh supported Pi must be
   reproducibly transformable into the companion without inherited machine
   folklore.
6. **Build/test infrastructure contains hardware safety contracts.** Exact
   dependencies, PT_LOAD identity, apparatus provenance, and physical evidence
   directly constrain what may be called qualified product behavior.
7. **Evidence must fail closed.** Unclear identity or apparatus invalidates a
   claim rather than being repaired narratively afterward.
8. **Preserve recurring invariants, not migration ceremony.** The clean project
   should retain build/qualification contracts while allowing M0-M4-specific
   extraction machinery to remain historical.

## Rebuild-readiness note

B12, B13, and B14 are now evidence-supported enough to inform the clean
architecture.

All B01-B14 behavior families have therefore reached at least
`EVIDENCE_SUPPORTED`. This does **not** mean the semantic audit is complete or
that every historical module should be promoted to `REBUILD_READY`.

The next audit phase is cross-domain synthesis: reconcile mutable-state ownership,
interfaces, dependency direction, Pi/PS2 responsibility boundaries, and the
minimum clean architecture needed to reproduce the audited behavior. Only then
should behavior families be promoted to `REBUILD_READY` and clean PS2
reconstruction begin.
