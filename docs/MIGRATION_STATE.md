# PS-to-VNC Migration State

This document is the human-readable companion to:

    runtime/MIGRATION_STATE.env

## Last completed stage

M0

M0 is the last completed migration stage.

It established the first writable PS-to-VNC source/build authority and
independently reproduced the exact historical B4A executable bytes.

M0 completion authority is recorded in:

    runtime/M0_BUILD_AUTHORITY.env

## Current stage

M1 — IN PROGRESS

M1A, the publication gate, M1B, and M1C are complete.

The first modularized executable has been built, independently reproduced,
and characterized.

It is a new DUT identity and has not yet been hardware validated.

## Current working source authority

working/b4a/ps2ip.c

This remains the transitional monolith path, but its contents are now M1B
source rather than byte-identical B4A source.

Current M1B source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

Current monolith SHA256:

    7bafa7b58daa086cf10ce2e7ae467889873f9b282adc8101da966dc8d65ff653

First extracted module:

    working/b4a/ps2vnc_config_text.c
    working/b4a/ps2vnc_config_text.h

Frozen B4A remains unchanged under:

    baseline/frozen-b4a/

## Frozen behavioral authority

Baseline DUT:

    D17AL-F8J2-B4A

Frozen source:

    baseline/frozen-b4a/ps2ip.c

Frozen source SHA256:

    67d7ae945553a391689447f07c2fcf8057301bf82a998a598c866739da449aac

Known tested ELF SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Frozen dependency analysis:

    baseline/analysis/B4A-dependency-map/

Dependency-analysis SHA256SUMS manifest SHA256:

    075d68d9992332a32bdca54a32964363fb893071fdcc5c9e2dfc7fb26a2e1749

## Legacy repository boundary

Historical PS2VNC repository:

    /home/ps2/ps2vnc

That repository is read-only historical authority for successor development.

All new work belongs in:

    /home/ps2/PS-to-VNC

## Bootstrap preservation

Before the recovery-state machinery was added, the exact staged successor tree
was preserved as Git tree:

    2f7f3dcae878251bda1ad8169912df035ca34020

It was also preserved as:

    /home/ps2/ps2vnc-legacy-backups/PS-to-VNC-pre-recovery-staged-20260830-003258.tar

Archive SHA256:

    a8426ff95e82b06fe1dc9056d7dabeeca91b3bccb5d2711ac3d2d8c59ecd5251

## Current build state

M1C build result:

    PASS — NEW MODULAR DUT

Current M1 ELF SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Current M1 ELF size:

    2927748 bytes

Second clean build reproduction:

    BYTE EXACT

Host config-text parity:

    PASS

Hardware validation:

    NOT RUN

Durable characterization:

    docs/M1C_DUT_CHARACTERIZATION.md

## Current documentation state

The successor documentation and recovery system are committed and active.

M0 source, build, dependency, reproduction, evidence, and hardware-resolution
authority are recorded inside PS-to-VNC.

Migration recoverability remains a definition-of-done requirement for every
subsequent modularization stage.

## Next action

Perform the required M1D hardware regression on the characterized modular
DUT.

The exact executable under test must be:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Machine observations and physical observations must remain separately
recorded.

## Conversation-limit recovery

A fresh conversation should begin with:

    START_HERE.md

Then run:

    scripts/resume-state.sh
    scripts/migration-check.sh
    scripts/docs-check.sh

The repository, not conversational memory, determines where development
resumes.

## Machine-state mirror

The following exact indented tokens mirror the stable migration fields in the
canonical machine-readable state.

    LAST_COMPLETE_STAGE=M3
    CURRENT_STAGE=M4
    CURRENT_STAGE_STATUS=ARCHITECTURAL_NORMALIZATION
    CURRENT_WORKING_SOURCE=working/b4a/ps2ip.c
    CURRENT_SOURCE_HEAD=661f14576892e533475818704d6a1391ad950414
    LAST_HARDWARE_RESULT=M4H_PASS_MACHINE_AND_PHYSICAL
    NEXT_ACTION=M4H-B_video_geometry_residual_boundary_census
    BLOCKED_BY=NONE

The canonical machine-readable source remains:

    runtime/MIGRATION_STATE.env

## M0B checkpoint

The M0 reproduction contract is now defined at:

    docs/M0_BASELINE_CONTRACT.md

M0 has formally entered IN_PROGRESS state.

No writable source has been created yet.

Next operation:

    M0C_reconstruct_and_fingerprint_historical_build_environment

## M0C checkpoint

M0C historical build-environment reconstruction is COMPLETE.

Canonical human build provenance:

    docs/M0_BUILD_PROVENANCE.md

Canonical machine-readable build authority:

    runtime/M0_BUILD_AUTHORITY.env

The B4A libps2ip authority is the frozen archive with SHA256:

    b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74

The different legacy-root archive with SHA256:

    f16bd80321657a854f09d96622a11308232c9053486d6158b78825e452b35860

is older historical residue and is not the B4A link input.

No writable working source exists yet.

Next operation:

    M0D_create_writable_B4A_authority_and_controlled_rebuild

## M0D1 checkpoint

The pristine writable B4A implementation has been established at:

    working/b4a/

The primary writable source is:

    working/b4a/ps2ip.c

At creation it is byte-identical to the frozen B4A source.

A controlled reproduction builder has been established at:

    scripts/build-m0-b4a.sh

M0D1 does not execute the compiler.

The next operation after this checkpoint is committed is the first controlled
M0 reproduction build.

## M0D1 committed authority

Writable-source commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

Machine state now recognizes:

    CURRENT_WORKING_SOURCE=working/b4a/ps2ip.c
    CURRENT_SOURCE_HEAD=d1c0d6a4829c03f3a062095afd00859188e13dfe
    NEXT_ACTION=M0D2_run_controlled_B4A_reproduction_build

## M0E checkpoint

M0E reproduction classification is COMPLETE.

Classification:

    OUTCOME_A_BYTE_EXACT_ELF

The independently generated working ELF is byte-identical to the historical
and frozen B4A ELF:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

The earlier M0D2 outer-harness exit 60 was caused only by an over-strict
byte-hash requirement on the GCC slim-LTO `ps2vnc_gsHires.o` intermediate.

M0D2A established that its eighteen LTO payloads are byte-identical and only
the generated LTO section-name suffix, `.strtab`, and `.shstrtab` vary.

Canonical result:

    docs/M0_BUILD_RESULT.md

Committed evidence:

    evidence/m0/m0d2/

Next operation:

    M0F_resolve_hardware_validation_requirement_for_byte_exact_DUT

## M0F checkpoint

M0F hardware-validation requirement resolution is COMPLETE.

New hardware regression required:

    NO

New M0 hardware run performed:

    NO

Reason:

    the M0 reproduced ELF is byte-identical to the historically validated B4A
    ELF, so no new DUT identity exists.

Canonical resolution:

    docs/M0_HARDWARE_RESOLUTION.md

Next operation:

    M0G_close_M0_and_define_M1
## M0G checkpoint

M0 is COMPLETE.

Completed M0 source authority:

    working/b4a/ps2ip.c

Completed M0 source commit:

    d1c0d6a4829c03f3a062095afd00859188e13dfe

Completed M0 build result:

    OUTCOME_A_BYTE_EXACT_ELF

Completed M0 executable SHA256:

    2deb9f4c3dab34eebdccc31a53fe7158ebce625aec4e9cf03cda902abf2611e8

Completed M0 hardware resolution:

    no new M0 hardware run was required because the reproduced executable is
    byte-identical to the historically validated B4A DUT.

This is inherited validation for identical DUT bytes, not a new physical
hardware PASS.

Current stage:

    M1 — NOT STARTED

M1 governing contract:

    docs/M1_EXTRACTION_CONTRACT.md

Next operation:

    M1A_select_first_low_coupling_leaf_extraction_boundary
## M1A checkpoint

M1A is COMPLETE.

Selected functions:

    ps2vnc_config_trim_left
    ps2vnc_config_trim_right

Selected responsibility:

    configuration text whitespace trimming

Selection authority:

    docs/M1A_EXTRACTION_BOUNDARY.md

Implementation source mutation:

    NO

Next operation:

    PUBLISH_create_verify_and_push_new_PS-to-VNC_GitHub_repository

M1B remains blocked until publication passes.
## GitHub publication checkpoint

Publication is COMPLETE.

Repository:

    Olsens11/PS-to-VNC

Visibility:

    PRIVATE

Default branch:

    main

Publication occurred after M1A and before any M1 implementation source
mutation.

Authority:

    docs/GITHUB_PUBLICATION.md

Next operation:

    M1B_mechanically_extract_config_text_module
## M1B checkpoint

M1B is COMPLETE.

Mechanical source commit:

    0f1b88ddf7821c935b68aabe6d65180bf02b074f

Extracted module:

    working/b4a/ps2vnc_config_text.c
    working/b4a/ps2vnc_config_text.h

M1 implementation source mutation:

    YES — CONFIG_TEXT_EXTRACTION_ONLY

M1 build performed:

    NO

M1 DUT binary:

    NONE

Next operation:

    M1C_build_and_characterize_first_modularized_DUT
## M1C checkpoint

M1C is COMPLETE.

M1 executable SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Second clean reproduction:

    PASS — BYTE EXACT

Extracted-function host parity:

    PASS

Hardware validation:

    NOT RUN

Authority:

    runtime/M1_DUT_AUTHORITY.env
    docs/M1C_DUT_CHARACTERIZATION.md

Next operation:

    M1D_run_required_hardware_regression

## M1D deployment checkpoint

The first modular M1 DUT is now present on the PS2.

DUT SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Deployment status:

    DEPLOYED_VERIFIED

Hardware status:

    NOT_RUN

Unique forensic PS2 target:

    /mass/0/PS-to-VNC-M1D1-20260830-020419-26ae06ff.ELF

Rolling launch target:

    /mass/0/PS2VNC.ELF

Deployment evidence:

    evidence/m1/m1d1-deployment/

The rolling ELF must not be treated as hardware validated until M1D records
both machine and physical results.

## M1D hardware-regression completion

M1D completed with independent machine and physical PASS authorities.

Exact DUT SHA256:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

Startup gate:

    PASS

Machine result:

    PASS

Physical result:

    PASS

Evidence:

    evidence/m1/m1d2-hardware-20260830-022035/

The next lifecycle operation is M1E closure.

## M1E closure and M2A activation

M1 is complete.

Validated M1 DUT:

    26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b

M1 hardware result:

    M1D_PASS_MACHINE_AND_PHYSICAL

M2 is now the active migration stage.

M2A selected:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

Target module:

    working/b4a/ps2vnc_config_text.c

Next:

    M2B_mechanically_extract_config_scalar_parsers

## M2B mechanical extraction checkpoint

M2B mechanically moved:

- ps2vnc_config_parse_int
- ps2vnc_config_parse_bool

from the monolith into:

    working/b4a/ps2vnc_config_text.c

Source commit:

    58d22cba30174f92ebc50418da30080cef68d7c1

Caller mutation:

    NO

Behavior mutation:

    NO

Makefile mutation:

    NO

Build performed:

    NO

Hardware run performed:

    NO

Next:

    M2C_build_and_characterize_scalar_parser_DUT

## M2 closure / accelerated migration transition

M2 is complete.

M2 DUT:

    af245ae9f1145b5750377f83375fa9ec640f19ecd320461dd5ef80809186b84e

M2 validation:

    PASS

Physical PS2 hardware requirement:

    NOT_REQUIRED

The active stage is now M3.

M3 begins the accelerated migration strategy by planning the remaining
monolith as coherent extraction waves rather than repeating one-helper
migration cycles.

Next:

    M3A_plan_accelerated_extraction_waves

## M3 coarse implementation move

The complete implementation envelope has been moved into:

    working/b4a/ps2vnc_macro_body.inc

Extracted lines:

    18313

Remaining ps2ip.c lines:

    467

PS2 load image:

    BYTE_EXACT_TO_M2

Next:

    M3B_recursive_macro_split

## M3B three-region hierarchy

The M3 implementation body is now represented by three large child regions:

    6510
    5625
    6178

Parent reconstruction:

    BYTE_EXACT

PS2 load image:

    BYTE_EXACT_TO_M2

Next:

    M3C_recursive_split_macro_regions

## M3C six-leaf hierarchy

The three large M3B branches have now been recursively divided into six
roughly subsystem-sized leaves.

All parent reconstructions are byte-exact.

PS2 load image:

    BYTE_EXACT_TO_M2

Next:

    M3D_classify_six_macro_leaves_and_promote_subsystems

## M3E first real translation-unit checkpoint

Machine validation:

    PASS

Independent service translation unit:

    PASS

Migrated source region:

    6379 lines

ELF:

    ef0124605ebb2729ad0cf26eb9ba0e3d2ac0656eb72f4d8d82a2203a37088d4b

Two-build reproduction:

    BYTE_EXACT

Load-image relation:

    NEW_NONIDENTICAL_REAL_TU_DUT

Hardware validation:

    REQUIRED

Next:

    M3G_continue_real_translation_unit_decomposition

## M3F first-real-TU hardware checkpoint

M3E's `management_config + calibration_state` real translation unit is now
hardware-qualified.

- machine matrix: 5/5 PASS
- physical result: PASS
- startup mode invariant: 480p
- live VNC session after sweep: PASS
- source repair required for M3E: NO
- next migration action: continue real translation-unit decomposition

## M3G second real translation-unit checkpoint

M3G machine validation: PASS.

- state TU: `ps2ip.c`
- runtime TU: `ps2vnc_runtime.c`
- services TU: `ps2vnc_services.c`
- runtime semantic lines: 11933
- services semantic lines: 6379
- clean-build reproduction: byte-exact
- load relation: `NEW_NONIDENTICAL_THREE_TU_LOAD_IMAGE`
- hardware requirement: `REQUIRED_NEW_LOAD_IMAGE`
- next action: `M3H_hardware_validate_three_object_architecture`

## M3H hardware checkpoint

The M3G three-object architecture has completed its required hardware
checkpoint.

Machine result: PASS.
Physical result: PASS.
Display matrix: 5/5 PASS.
Startup/final baseline: 480p PASS.
Evidence: `evidence/m3/m3h-hardware-20260830-055234`.

Next action: `M3I_continue_recursive_runtime_translation_unit_decomposition`.

## M3Z management recursive split

M3Z completes the final blocking coarse structural split identified by the
M3W semantic-leaf census.

Qualified source authority:

    CURRENT_SOURCE_HEAD=c224b75e24d79fa7cbebd84236123e97c31ac9f1
    CURRENT_DOC_HEAD=bcc310b87d5c13e8b0630e2b8901599ff5965e1e
    LAST_BUILD_RESULT=M3Z_TWELVE_OBJECT_PASS_NEW_REPRODUCIBLE_DUT
    LAST_HARDWARE_RESULT=M3Y_PASS_WITH_CLASSIFIED_INTERMITTENT_1080I_TO_480P_RESTORE_FAILURE
    NEXT_ACTION=M4A_validate_twelve_object_management_split_on_hardware

Management/config partition:

    original logical lines: 3120
    GCC-authorized cut line: 1535
    management core: 1534 logical lines
    management tail: 1586 logical lines
    real translation units: 12

The boundary is bidirectional but deliberately narrow:

    core -> management tail functions: 4
    core -> management tail exact macros: 2
    management tail -> services/calibration-core functions: 4
    linkage-only static promotions: 8
    strong global duplicate symbols: 0

Machine qualification:

    ELF reproducibility: BYTE_EXACT_PASS
    ELF SHA256: d9ddbcb14cf1007236bdf77d9a8bb48ce99390e2144cfab81c75cee6513f18f1
    load SHA256: 62d62c543186802e657d4ba4044e879568a6dd51b707b056846df4c563de4f8b
    machine-evidence manifest SHA256: e1f80673363b774023ebf27f15e8d8b960f06af0d5f7bf4cab162edf42a23a79

The M3Z load image is nonidentical to the M3X hardware-qualified load image,
so M4A hardware validation is required before M3Z becomes the hardware
authority. Until M4A completes, the last hardware result remains the M3Y
qualification with its classified intermittent 1080i-to-480p restore
failure. That prior failure did not establish a deterministic M3X
regression.

After M4A passes, rerun the semantic-leaf census. Under the established M3W
policy, optional 1500-2000-line leaves do not block completion of the coarse
migration; the expected blocking-coarse-split count is zero.

## M4A twelve-object hardware qualification

The M3Z twelve-real-translation-unit management split is hardware-qualified.

    SOURCE_COMMIT=c224b75e24d79fa7cbebd84236123e97c31ac9f1
    HARDWARE_AUTHORITY_COMMIT=60413bc86ad008f5449a47a28f06b2b3429d9565
    DUT_ELF_SHA256=d9ddbcb14cf1007236bdf77d9a8bb48ce99390e2144cfab81c75cee6513f18f1
    MACHINE_RESULT=PASS
    MACHINE_PASS_COUNT=5
    MACHINE_FAIL_COUNT=0
    PHYSICAL_RESULT=FULL_PASS
    STARTUP_MODE_INVARIANT=480p
    FINAL_DISPLAY_STATE=IDLE
    NEXT_ACTION=M4B_post_split_semantic_leaf_census

The exact five-mode sequence was:

    480i
    480p-hires
    576i
    720p
    1080i

Each temporary mode restored successfully to the 480p baseline, including
the 1080i-to-480p transition that had intermittently failed during M3Y.
The RFB connection remained established after the harness.

Two setup/apparatus errors occurred before the real harness run: an
evidence-directory cleanliness check and a pre-created harness run
directory. Neither started a display transition. A malformed shell test in
the post-run wrapper also printed a false failure classification after the
authoritative harness had already returned PASS 5/5. None is classified as
a DUT failure.

M4A therefore closes the hardware requirement created by M3Z's new real
compiler boundary. The next action is the post-split semantic-leaf census.

## M4B coarse-migration closure and versioned architecture baseline

The coarse structural migration is complete.

M4B established:

    REAL_TRANSLATION_UNITS=12
    ACTIVE_SEMANTIC_LEAVES=12
    LARGEST_ACTIVE_LEAF_LOGICAL_LINES=1717
    BLOCKING_COARSE_SPLITS=0
    OPTIONAL_SIZE_CANDIDATES=9
    OPTIONAL_SIZE_LEAVES_GATE_COMPLETION=NO

The project therefore stops using source-file size as the primary reason
for further decomposition.

The first standard product version is:

    PRODUCT_VERSION=0.1.0-alpha.1
    PRODUCT_TAG=v0.1.0-alpha.1

The exact qualified PS2 executable remains:

    ELF_SHA256=d9ddbcb14cf1007236bdf77d9a8bb48ce99390e2144cfab81c75cee6513f18f1
    HARDWARE_RESULT=M4A_PASS_MACHINE_AND_PHYSICAL

No binary change was made merely to introduce the version number.

The permanent development rules are now defined by:

    docs/ARCHITECTURE.md
    docs/VERSIONING.md
    docs/ARCHITECTURAL_NORMALIZATION.md

The repository state transitions from:

    LAST_COMPLETE_STAGE=M3
    CURRENT_STAGE=M4
    CURRENT_STAGE_STATUS=ARCHITECTURAL_NORMALIZATION

The next action is:

    NEXT_ACTION=M4C_architectural_responsibility_census

M4 begins with a responsibility and ownership census before any further
implementation movement. Remaining migration-era core/tail and .inc
partitions are treated as qualified transitional scaffolding and will be
retired according to semantic module boundaries rather than geometric
line-count cuts.

## M4C/M4D architectural census and M4E target design

M4C measured the compiled migration architecture without moving source.
M4D then completed the link/type/interface universe.

The full current link contains 18 project-root C objects. The twelve M3
objects are the migration decomposition, while additional support/build
objects include configuration text helpers, identity, high-resolution GS
support, and three generated embedded IRX translation units.

The four project-named symbols initially outside the M4C twelve-object
scope were all resolved to the existing configuration text module.

M4E adopts the permanent responsibility-oriented architecture documented
in:

    docs/TARGET_ARCHITECTURE.md
    docs/STATE_OWNERSHIP.md
    docs/DEPENDENCY_RULES.md
    docs/NORMALIZATION_SEQUENCE.md

The central architectural corrections are:

    - mutable state receives subsystem owners;
    - application-wide writable extern coupling is retired incrementally;
    - RFB protocol and video presentation become sibling domains;
    - cross-domain display transitions are orchestrated by app;
    - UI/input no longer own transport effects;
    - PS2-specific mechanisms acquire an explicit platform home;
    - generated IRX C inputs are treated as build-generated inputs;
    - core/tail and directional migration interfaces are transitional.

No implementation was moved by M4E.

The first implementation normalization tranche is:

    src/config/text.c
    src/config/text.h

with host tests and permanent pstvnc_ naming.

    M4C_EVIDENCE_COMMIT=d40421a65c3d78146c490f7be903a97edf391f12
    M4D_EVIDENCE_COMMIT=bf3dc76257a692cb10b13ec8fc9e7f035e7fcbf4
    M4E_DESIGN_COMMIT=8889c6eeae0a466c3f87d392fd7bac5bda568a1f
    M4E_DESIGN_MANIFEST_SHA256=681fce2b475c29bfab835a88798dd1239806a13cd80b0d631dd4cb032466db31
    NEXT_ACTION=M4F_normalize_config_text_module

## M4F first permanent-module normalization

M4F completed the first implementation tranche of the permanent
responsibility-oriented architecture.

The historical configuration text helper module:

    working/b4a/ps2vnc_config_text.c
    working/b4a/ps2vnc_config_text.h

was retired and replaced by:

    src/config/text.c
    src/config/text.h

The permanent API now uses:

    pstvnc_config_trim_left
    pstvnc_config_trim_right
    pstvnc_config_parse_int
    pstvnc_config_parse_bool

Only those four module-owned symbols were renamed. Historical management
functions sharing the old ps2vnc_config_ prefix retained their names until
their own owning domain is normalized.

M4F also established host-side C unit tests, repository LF/editor policy,
and the current repository-level reproducible PS2 build authority:

    scripts/build.sh

Two independent PS2 builds were byte-identical:

    ELF_SHA256=9416044effd726cc9233ca8ae3f18bf1234d9d11c3a2cbd78a6674df5ea075cd
    ELF_BYTES=2970896

The full ELF differs from M4A because source/debug/symbol identity changed.

The PS2-loaded PT_LOAD image remains byte-exact:

    M4F_PT_LOAD_SHA256=62d62c543186802e657d4ba4044e879568a6dd51b707b056846df4c563de4f8b
    M4A_PT_LOAD_SHA256=62d62c543186802e657d4ba4044e879568a6dd51b707b056846df4c563de4f8b
    RELATION=BYTE_EXACT_IDENTICAL

No new hardware run was required. M4F inherits the M4A physical
qualification for these exact loaded bytes while M4A remains the most
recent direct hardware checkpoint.

Raw machine/pre-normalization evidence retains its captured bytes even
where historical whitespace differs from current authored-source policy.

    M4F_SOURCE_COMMIT=ab9ae2ee34631c657b6e75a80d79a3a0595e0128
    M4F_SOURCE_AUTHORITY_COMMIT=ebab70b19f0ae3863424d2299127712eddcdf667
    M4F_RAW_EVIDENCE_MANIFEST_SHA256=543ab7d9c0cc9ef43a9ebb37ed8f935f27623cc1488712678003e93d734decef
    NEXT_ACTION=M4G_video_mode_geometry_boundary_census

## M4G-A video-mode source normalization — hardware pending

Permanent ownership is established for:

    src/video/mode.c
    src/video/mode.h

The permanent module owns the immutable 22-entry catalog, video mode and
backend model types, backend support/default logic, and lookup-by-name.

The exact catalog order is host tested.

Live runtime state remains transitional and was not moved:

    active_video_mode
    active_display_backend
    desktop geometry state

Two independent PS2 builds are byte-identical to each other:

    M4G2_ELF_SHA256=6ef11851ae3965fa24504eb805a6b42cf95eaebaa7948ba99f067bed60c991f2
    M4G2_PT_LOAD_SHA256=5e6a0fe12d7562e0bd88d47b0ca1929f8565fccfd9bbb0b00b498f178ff354b3
    M4G2_PT_LOAD_BYTES=449416

The PT_LOAD differs from the previously qualified M4F/M4A image:

    PREVIOUS_PT_LOAD_SHA256=62d62c543186802e657d4ba4044e879568a6dd51b707b056846df4c563de4f8b
    RELATION=NONIDENTICAL

Therefore M4G2 is an unqualified hardware candidate until M4G4 passes.

M4G_SOURCE_COMMIT=30a1ca80340e3e61a8256aabd22cca552ff8fdce
M4G_SOURCE_AUTHORITY_COMMIT=1dba6c3d68f5275795e20be3ffd7c7d2263729a5
M4G_SOURCE_AUTHORITY_MANIFEST_SHA256=844a43e10a320486c109bd6e16df24f8dedc97d9781b25e73ce38e307a392ad4

NEXT_ACTION=M4G4_video_mode_hardware_checkpoint

## M4G-A video-mode normalization — hardware qualified

The permanent video-mode model is now directly hardware qualified.

Permanent ownership:

    src/video/mode.c
    src/video/mode.h

Qualified machine image:

    ELF_SHA256=6ef11851ae3965fa24504eb805a6b42cf95eaebaa7948ba99f067bed60c991f2
    PT_LOAD_SHA256=5e6a0fe12d7562e0bd88d47b0ca1929f8565fccfd9bbb0b00b498f178ff354b3
    PT_LOAD_BYTES=449416

The established five-mode machine contract passed 5/5:

    480i       -> 480p
    480p-hires -> 480p
    576i       -> 480p
    720p       -> 480p
    1080i      -> 480p

Machine result:

    PASS_COUNT=5
    FAIL_COUNT=0
    STARTUP_MODE_INVARIANT=480p
    FINAL_STATE=IDLE
    FINAL_MODE=480p
    FINAL_RFB_ESTABLISHED_COUNT=1

Independent physical observation:

    FULL PASS

An earlier launch stalled while the last visible startup message was
"Ethernet link is UP". Network reachability remained alive and stale
Pi-side management connections were observed. Restarting the management
service terminated that attempt; a fresh launch of the same rolling ELF
then reached the 480p desktop and passed the complete hardware contract.

That incident is preserved as a nonblocking management-service robustness
issue. Its exact root cause remains unproven and is not attributed to the
video-mode model.

M4G_HARDWARE_AUTHORITY_COMMIT=f19a2fd317dc440e403acef3d55de7ac0bace69f
M4G_HARDWARE_MANIFEST_SHA256=afbfd1636dbed4d88307431fe1cfa4c588a2960bfaec209e6117c9f20f2b04d7

NEXT_ACTION=M4H_video_geometry_boundary_census

## M4H-A safe-area / presentation geometry — hardware pending

The first permanent video/geometry subtranche is normalized.

Permanent ownership:

    src/video/geometry.c
    src/video/geometry.h

Public pure API:

    pstvnc_video_geometry_safe_area_valid
    pstvnc_video_geometry_safe_area_rect
    pstvnc_video_geometry_presented_rect

Historical runtime/config surfaces remain transitional adapters. Mutable
video state, calibration state, RFB mechanics, and PS2 GS programming did
not move.

Host tests pass.

Two independent canonical PS2 builds are byte reproducible.

Candidate identity:

    ELF_SHA256=42163c30b68ebf84b51b1c8c81d541017e64ffaa52e27bb0408f91edb25e2992
    ELF_BYTES=2978188

    PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9
    PT_LOAD_BYTES=449800

The loaded image is not identical to hardware-qualified M4G:

    M4G_PT_LOAD_SHA256=5e6a0fe12d7562e0bd88d47b0ca1929f8565fccfd9bbb0b00b498f178ff354b3
    M4H_PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9

Therefore M4H-A does not inherit M4G hardware qualification.

The exact reproducible M4H candidate must pass the PS2 hardware checkpoint
before it can replace M4G as LAST_VALIDATED_WORKING_ELF.

M4H_SOURCE_COMMIT=661f14576892e533475818704d6a1391ad950414
M4H_SOURCE_AUTHORITY_COMMIT=f8abc11c187b9505f88a443d4c4878a092f8a999
M4H_SOURCE_AUTHORITY_MANIFEST_SHA256=4ec8f03c217b71cfd9ee08621dcfc233cde0514ab8e94b964f573a5cf71c07aa

NEXT_ACTION=M4H4_safe_area_presentation_hardware_checkpoint
BLOCKED_BY=M4H3B_NONIDENTICAL_PT_LOAD_REQUIRES_HARDWARE

## M4H-A hardware qualification — COMPLETE

Validated candidate:

    ELF_SHA256=42163c30b68ebf84b51b1c8c81d541017e64ffaa52e27bb0408f91edb25e2992
    PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9

Qualification:

    controlled nontrivial safe-area geometry = PASS
    signed center-relative offsets = PASS
    normal configuration restoration = PASS
    five-mode machine regression = PASS 5/5
    five-mode physical result = FULL PASS
    final state = IDLE / 480p
    final RFB established sessions = 1

Hardware authority:

    /home/ps2/PS-to-VNC/evidence/m4/m4h-hardware-20260830-161605

M4H_HARDWARE_HEAD=b5c5f6c9a9262152553d4af477c03bae59c63f82
M4H_HARDWARE_MANIFEST_SHA256=3fc92b5f4405183838ea06c2b0772fbeeac2828e7b862f67adf4cfd049f035bb

M4H_A_HARDWARE_QUALIFICATION=PASS_MACHINE_AND_PHYSICAL

The first sealing attempt stopped because a source-style whitespace gate was
incorrectly applied to raw captured evidence. Raw machine evidence was preserved
byte-for-byte; its integrity is governed by the evidence SHA256 manifest.

NEXT_ACTION=DOCS1_development_continuity_baseline
BLOCKED_BY=NONE

## DOCS1 development-continuity baseline

The successor repository now has a conventional repository-centered
development-continuity baseline.

Canonical session bootstrap:

    AGENTS.md

Canonical contributor workflow:

    CONTRIBUTING.md

Canonical documentation router:

    docs/README.md

Canonical concise current state:

    docs/status.md

Development-system policy:

    docs/development/

Architecture Decision Records:

    docs/adr/

Machine-readable development-system contract:

    runtime/DEVELOPMENT_SYSTEM.env

Combined consistency command:

    scripts/check.sh

Migration-era bootstrap/router/current-state documents remain compatibility
surfaces while existing tooling depends on them.

The successor explicitly preserves the real-project agitation model:
PS2VNC was the first proving ground and PS-to-VNC continues to pressure-test
the reusable development-continuity rules.

Routine saved build/deploy/test tooling must be discovered and reused rather
than reconstructed in chat.

The M4H-A hardware authority was already sealed before this documentation
normalization:

    M4H_HARDWARE_HEAD=b5c5f6c9a9262152553d4af477c03bae59c63f82
    M4H_HARDWARE_MANIFEST_SHA256=3fc92b5f4405183838ea06c2b0772fbeeac2828e7b862f67adf4cfd049f035bb
    VALIDATED_ELF_SHA256=42163c30b68ebf84b51b1c8c81d541017e64ffaa52e27bb0408f91edb25e2992
    VALIDATED_PT_LOAD_SHA256=a33efdb0bee3f383d4828db8fb6a06f62ddb90fdc42a97308dc12777ed8197b9

NEXT_ACTION=M4H-B_video_geometry_residual_boundary_census
BLOCKED_BY=NONE

DEVELOPMENT_CONTINUITY_BASELINE_COMMIT=e88c7bfd84ee3cf056b11ff9107aca8873cb1ce5
