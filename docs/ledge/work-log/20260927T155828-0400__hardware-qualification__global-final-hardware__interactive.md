DOCUMENT=LEDGE_WORK_LOG_ENTRY
LOG_FORMAT_REVISION=0001
STARTED_AT=2026-09-27T15:58:28-04:00
COMPLETED_AT=2026-09-27T16:36:25-04:00
ROLE_KEY=hardware-qualification
WORK_ITEM_KEY=global-final-hardware
WORKER_KEY=interactive
STATUS=BLOCKED
STARTING_BRANCH_COMMIT=81d5915eb0d57d769a66f19e4f180b8eef0a2b79
ENDING_BRANCH_COMMIT=SELF
SELF_PAUSED=NO

# Final ledge hardware qualification — LEDGE-FINAL-HW1

## Objective

Execute final operator-backed PS2/Pi hardware qualification for the complete
current ledge A001-A006 source tranche against the exact linked identity named
by Validation state revision 0007, without modifying product source to rescue a
hardware failure.

This log's STARTED_AT is the first exact timestamp captured by the live
qualification staging attempt. Earlier connector pairing and read-only
apparatus inspection were setup/preflight and did not mutate repository product
source.

## Authority consumed

- branch `ledge/h1-all-guns` at
  `81d5915eb0d57d769a66f19e4f180b8eef0a2b79`;
- final reconstructed product-source authority
  `e2727cb31e214d11e094f1f45b0b9d8ab01d84e9`;
- `docs/ledge/LEDGE_VALIDATION_STATE.md` revision 0007;
- `docs/ledge/LEDGE_ARCHITECTURE_OVERLAY.md` revision 0007;
- `docs/ledge/LEDGE_WIRE_RUNTIME_DECISIONS.md` revision 0011;
- `docs/ledge/work-log/README.md` revision 0007;
- `docs/development/testing.md`;
- `docs/development/tooling.md`;
- `scripts/testkit/README.md`;
- current generic TestKit identity/deployment mechanisms;
- canonical Pi Wire/internal-provider stagers and selected native
  LightDM/Xorg `:0` provider architecture.

Repository authority was refreshed immediately before this record was created;
remote `ledge/h1-all-guns` remained exactly
`81d5915eb0d57d769a66f19e4f180b8eef0a2b79`.

## Exact DUT identity

Canonical clean build on the Pi reproduced Validation's exact pristine identity:

- pristine ELF SHA256:
  `993655ccffc430347281291c1fa917e3b22ce77f17407a99edbc920b71a65a2a`;
- pristine ELF bytes: `3441752`;
- one PT_LOAD segment;
- pristine PT_LOAD SHA256:
  `3bf4319f56d5678550fc0b05cc438d5a4281df133d544ce3ff269163185dba1b`;
- pristine PT_LOAD bytes: `556180`;
- qualified PS2IP SHA256:
  `b2959fe364b374d7d8984969b6444b92743ed671f4d41d27cb284d4ac7ab6a74`.

Generic TestKit preparation created `LEDGE-FINAL-HW1`:

- runtime identity digest:
  `7341c1d7a6643f040f96c60880f765ce06cc7b95bdaf1a2e8f0a85aafe2de00f`;
- stamped ELF SHA256:
  `917ccae5b4d8b1230770a16273f624cb5889a32ed79a94022b9572902c341ac8`;
- stamped ELF bytes: `3441752`;
- stamped PT_LOAD SHA256:
  `1469140fa5ad0671184988b1105d4b20807addc6732783b48d98b05f76f4daab`;
- stamped PT_LOAD bytes: `556180`;
- identity stamp reproducibility: PASS;
- stamp/PT_LOAD relation: NONIDENTICAL, explicitly recorded rather than
  treated as inherited byte identity.

Generic deployment wrote and read back both:

- `/mass/0/PS2VNC-LEDGE-FINAL-HW1-917ccae5.ELF`;
- `/mass/0/PS2VNC.ELF`.

Both readbacks exactly matched stamped SHA256
`917ccae5b4d8b1230770a16273f624cb5889a32ed79a94022b9572902c341ac8`
and byte count `3441752`.

## Pi product-path preparation

The old direct-RFB apparatus on `192.168.50.1:5900` was retired. The final
selected product path was staged from current ledge bytes and activated:

- product Wire: `192.168.50.1:5902`;
- internal provider activation socket: `127.0.0.1:5900`;
- provider: socket-activated
  `/usr/bin/X0tigervnc -display :0 -rfbport -1 ...`;
- operator/development observer `127.0.0.1:5903` was preserved separately and
  was never counted as the product path.

Installed authority hashes are preserved in
`evidence/ledge/ledge-final-hw1-hardware-qualification/PI-RUNTIME-AUTHORITY.env`.

The internal-provider stager exposed one separate APPARATUS_DEFECT: its
inactive-only verification treats systemd `static` as enabled because
`systemctl is-enabled` returns status 0 for a static service. Exact installed
unit bytes nevertheless matched repository authority and the defect did not
block live runtime. The stager was not modified during this role.

## Initial hardware success

The operator launched exact rolling target `mass:/0/PS2VNC.ELF`.

Machine evidence recorded exactly one runtime identity:

`PS2VNC_ID version=1 test=LEDGE-FINAL-HW1 digest=7341c1d7a6643f040f96c60880f765ce06cc7b95bdaf1a2e8f0a85aafe2de00f`

Startup diagnostics arrived in order:

1. `PSTVNC_STAGE NET_READY`;
2. `PSTVNC_STAGE GS_READY`;
3. `PSTVNC_STAGE DESKTOP_READY`;
4. `PSTVNC_STAGE INPUT_READY`.

No `PSTVNC_STAGE FATAL` was observed.

The product established one PS2 -> Pi TCP session to `192.168.50.1:5902`.
That session lazily activated the internal X0tigervnc provider on
`127.0.0.1:5900`. X0tigervnc recorded:

- display `:0`;
- geometry `704x462`;
- RFB protocol 3.8;
- security type None;
- initial 16-bit server pixel format and negotiated 15-bit/16bpp client pixel
  format.

Operator physical observation:

- PS2 booted directly to the Pi desktop;
- PS2 desktop visibly matched the separately observed Windows VNC view through
  operator-only port 5903.

This establishes an initial-desktop PASS only. It is not blanket hardware
qualification.

## Product defect

The run later stopped making PS2 application-level RFB progress.

The operator reported:

- a newly opened `PS2VNC INPUT TEST` terminal was visible through the
  independent Windows 5903 VNC client but never appeared on the PS2;
- the PS2 display appeared frozen / stopped receiving RFB updates;
- controller movement produced no visible cursor movement or input response.

Forensic machine state at the failure boundary:

- `ps-to-vnc-wire.service`: active/running, zero restarts;
- `ps-to-vnc-rfb-internal-x0tigervnc.service`: active/running, zero restarts;
- product Wire TCP session: still ESTABLISHED;
- internal provider TCP session: still ESTABLISHED;
- operator 5903 path: live and showing current desktop changes;
- no new fatal diagnostic.

The sealed pcap reconstructs both PSTV directions with no framing gap.

Last PS2 application frame, at
`2026-09-27T16:20:01.848-04:00`:

- PSTV `DATA/RFB`, PS2 send sequence `132`, payload `10` bytes;
- exact RFB `FramebufferUpdateRequest`;
- `incremental=1`;
- rectangle request `x=0 y=0 width=704 height=462`.

Last Pi application frame, at
`2026-09-27T16:21:00.861-04:00`:

- PSTV `DATA/RFB`, Pi send sequence `130`, payload `328` bytes;
- exact RFB `FramebufferUpdate`;
- one Raw rectangle;
- `x=681 y=11 width=12 height=13`;
- Raw pixel bytes `312`;
- rectangle fully inside `704x462`;
- RFB payload consumes exactly all `328` bytes.

The PS2 TCP stack acknowledged the Pi's final TCP data, but no subsequent PS2
PSTV CREDIT or next framebuffer request occurred.

Classification:

`PRODUCT_DEFECT`

Boundary:

PS2 runtime ceased application-level Wire/RFB consumption/progress after
receipt/ACK of an ordinary valid in-bounds incremental Raw update while the
Pi Wire process, internal provider, native desktop, and physical TCP session
remained alive.

No product-source mutation, automatic recovery, reconnect watchdog, or
source-side rescue was attempted after classification.

## Preserved evidence

Tracked sealed directory:

`evidence/ledge/ledge-final-hw1-hardware-qualification/`

Key records include:

- exact stamped `LEDGE-FINAL-HW1.ELF`;
- `HARDWARE-AUTHORITY.env`;
- `PI-RUNTIME-AUTHORITY.env`;
- `FRAME-BOUNDARY.txt`;
- raw preparation and deployment records;
- raw UDP 5999 identity/stage log;
- raw packet capture;
- raw tcpdump final log;
- raw provider journal;
- raw operator observation;
- raw failure classification;
- raw X input observer output;
- `SHA256SUMS.txt`.

Sealed pcap:

- SHA256
  `ff9c7a8428ced011dfda606bef424d723055a282b8ccb928084d9d0dc34c25e5`;
- packets captured: `829`;
- packets received by filter: `829`;
- packets dropped by kernel: `0`.

## Repository checks

Before commit:

- `python3 scripts/work-log-check.py`: `WORK_LOG_CHECK=PASS records=258 grandfathered=9 format_compat=2 stamp_compat=1`;
- `git diff --check`: PASS;
- `./scripts/check.sh`: `PS_TO_VNC_PROJECT_CHECK=PASS`;
- canonical long dictionary scan reported the existing advisory `SOURCE_DICTIONARIES=ATTENTION` with `ATTENTION_COUNT=62`; the canonical checker explicitly continued and passed. No dictionary/source changes were made by this hardware role.

## Qualification disposition

`HARDWARE_QUALIFICATION=FAIL_PRODUCT_DEFECT`.

The following were reached:

- exact final ELF identity binding;
- exact deploy/readback;
- final product Q4/Wire establishment;
- selected native `:0` provider activation;
- physical initial desktop presentation.

The following are not qualified because the campaign stopped at the first
product defect:

- ordinary sustained incremental RFB continuity;
- controller/input delivery;
- on-screen keyboard;
- provider failure/replacement;
- MPEG calibration and presentation;
- AUDIO/common-clock physical observation;
- repeated MPEG generations;
- stale-generation fencing;
- Wire loss during MPEG;
- Q7 retirement/restoration overlap;
- orderly/abnormal retirement;
- repeated sessions;
- all-guns endurance.

`QUALIFY_NATIVE_PI_DESKTOP_RFB_PATH_REPRODUCIBILITY=PARTIAL_NOT_SATISFIED`.

The native path was proven to activate and present once through the final
Wire/internal-provider topology, but reproducibility cannot be declared from a
single run that then encountered this product defect. A clean reboot/repeat
qualification was not performed after the failure.

## Pending / blocker accounting

PENDING_LOCAL=NONE_FOR_THIS_FAILED_RUN
HARDWARE_PENDING=YES_AFTER_PRODUCT_CORRECTION
PRODUCT_DEFECT_OPEN=YES
APPARATUS_DEFECT_OPEN=YES_STAGER_STATIC_ENABLEMENT_GUARD
PRODUCT_SOURCE_MUTATED_BY_HARDWARE_ROLE=NO

## Exact next pickup

Foreman and independent Validation should consume the tracked
`LEDGE-FINAL-HW1` evidence and classify/route the PS2-side incremental-RFB
runtime stall to the appropriate reconstructed product owner. Hardware
qualification must remain failed/pending; do not continue to MPEG/audio or
endurance qualification against these bytes and do not mutate source inside the
hardware role.

After a product correction is independently machine/source validated and a new
exact linked ELF/PT_LOAD identity is established, begin a fresh hardware test
identity rather than overwriting or reusing `LEDGE-FINAL-HW1`.
