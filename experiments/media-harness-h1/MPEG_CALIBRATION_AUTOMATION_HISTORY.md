# MPEG calibration automation history

## Purpose

This file is the durable chronological index for the progressive MPEG-calibration continuity record produced by the automation on 2026-09-11.

The automation did not produce one timeless summary. It repeatedly updated `MPEG_CALIBRATION_CONTINUITY.md` as the work progressed, preserving what was known, what had just changed, failures still unresolved, evidence available at that moment, and the next intended action. Those intermediate states are valuable historical evidence and must not be flattened into a later current-state summary.

**Preservation rule:** this history is append-only. Do not replace, squash, rewrite, or reinterpret earlier entries merely because later work resolved or superseded them. Current-state summaries belong in a separate file such as `MPEG_CALIBRATION_CURRENT_STATE.md`.

The exact historical snapshots remain Git objects and are recoverable verbatim with:

```sh
git show <commit>:experiments/media-harness-h1/MPEG_CALIBRATION_CONTINUITY.md
```

The original current-path file on this branch has been restored byte-for-byte to the automation's final snapshot, commit `df11959c748b0abf019d74e6e11d06a789765f05`.

## Progressive automation snapshots

The sequence below is oldest to newest. The roughly hourly cadence is part of the evidence: each snapshot records the live understanding at that stage rather than a reconstruction written afterward.

1. **2026-09-11 12:48:36 UTC** — `210cfd0b0e79985a262d0d06e9dba726a3fbd94c` — `docs(h1): add MPEG calibration continuity handoff`
   - First continuity snapshot; portable calibration core established and host-tested.

2. **2026-09-11 13:47:39 UTC** — `d6adaae4e59ad68b1ca4bdb3dbfb0f5223b02e4a` — `docs(h1): hand off calibration adapter integration`
   - Adapter-integration stage.

3. **2026-09-11 14:47:50 UTC** — `22a29413e57b8bdf48d212d31b946e0f8b783a7f` — `docs(h1): hand off calibration foreground ownership bridge`
   - Foreground-ownership bridge stage.

4. **2026-09-11 15:49:01 UTC** — `efb24d51abc80583d9d3bd4fc7fb4da2d75e0816` — `docs(h1): hand off calibration RFB gate tranche`
   - RFB request/presentation gate stage.

5. **2026-09-11 16:48:38 UTC** — `69dfd7e416a2ea0355da521516b8d568344f870c` — `docs(h1): hand off calibration render-plan tranche`
   - Calibration render-plan stage.

6. **2026-09-11 17:48:25 UTC** — `a91f5b3fa0e4df25c6e967409445dde5e0058077` — `docs(h1): hand off calibration runtime tranche`
   - Runtime-composition stage.

7. **2026-09-11 18:49:49 UTC** — `961fcae90045357b76655d548b03589304640315` — `docs(h1): hand off calibration host-contract evidence`
   - Host-contract evidence stage, including the then-current evidence boundary.

8. **2026-09-11 19:46:34 UTC** — `c2f855e88e338e9e7096c695f72999511d8a9915` — `docs(h1): hand off calibration RFB scheduler tranche`
   - RFB scheduler stage.

9. **2026-09-11 20:50:08 UTC** — `dd6c5c8cc9aa9aae24976e8ef233aadbd487a9ce` — `docs(h1): hand off calibration RFB flow-policy tranche`
   - Generic RFB flow-policy integration stage.

10. **2026-09-11 21:47:39 UTC** — `d5db0e324fd229332645d87be1ed0bbc160801b6` — `docs(h1): hand off calibration interaction-binding tranche`
    - Interaction-binding stage. This snapshot intentionally preserves unresolved host-test failures that were still unresolved at that time.

11. **2026-09-11 22:52:54 UTC** — `df11959c748b0abf019d74e6e11d06a789765f05` — `docs(h1): hand off green calibration binding evidence`
    - Final automation snapshot. It records the diagnosed host `<libpad.h>` issue, the host-only shim repair, green strict-host evidence, green pinned-PS2 regression evidence, and the then-next action of live coordinator insertion.

## Why the intermediate snapshots matter

The later snapshots do not replace the evidentiary value of the earlier ones. In particular, the sequence preserves:

- which uncertainty existed before each implementation step;
- failures that were real at the time and were not yet explained;
- the reason a particular next step was selected;
- changes in the proven/unproven boundary;
- when host-only evidence became pinned-PS2 build evidence;
- the contemporaneous distinction between repository state and unknown Pi-local state;
- design constraints that were carried forward rather than reconstructed after success.

For forensic reconstruction, read the snapshots in chronological order using the exact `git show` command above. Do not rely only on the final snapshot or on a later current-state document.

## Post-automation continuation

Work after the automation's final snapshot is tracked separately. The current isolated CP2P continuation branch is `experiment/h1-cp2p-start-wire-accepted`.

The flattened state handoff created on 2026-09-12 has been preserved as:

`experiments/media-harness-h1/MPEG_CALIBRATION_CURRENT_STATE.md`

That file is a current-state aid only. It is not a substitute for this historical timeline or for the exact automation snapshots in Git history.

## Master experiment-history pointer — appended 2026-09-12

The automation is one phase of a larger continuous experimental drive. The canonical chronological history spanning the pre-automation lead-up, these automation snapshots, the subsequent live sessions, the CP2P source archaeology, and the current accepted-edge/START-wire work is:

`experiments/media-harness-h1/CP2P_MPEG_CALIBRATION_EXPERIMENT_HISTORY.md`

That master file follows the same temporal-truth rule as this index: earlier contemporaneous entries remain historical evidence even when later entries supersede their technical understanding.
