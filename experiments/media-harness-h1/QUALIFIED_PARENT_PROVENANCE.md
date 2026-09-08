# H1 Qualified Parent Provenance

H1 is the resident, configurable audio/video media-harness experiment.

## Video parent

The tracked file:

`qualified_p11_video_seed.c`

is a mechanically generated copy of the exact hardware-qualified EXP3 P11
source before any H1 integration changes.

- EXP3 parent commit: `5ff7fabd04ed4f00c8302331c0cc0dde55d19c57`
- canonical live reference blob: `4e34795823cbb386f7ac958c43a2ddbce4e255e5`
- P11 generator blob: `b7126410ef990d76fe47048221c020c2b49d12bd`
- qualified generated P11 blob: `007a439f6070e1b37e75c742cd551c290fe69b47`
- generated source SHA-256: `35fb3b6c2423637aaea0c1fe0790e7998cc0f8e0d0ee3d7240e8ec05383346bc`

The seed is preserved so H1 changes can be reviewed directly against the exact
qualified playback mechanism rather than reconstructed from chat history.

## Audio parent

H1 also inherits the configurable Audio Transport EXP2 tree already present in
the EXP3 parent. Its Profile 0 remains the known audio compatibility baseline.

H1 may expose experimental settings beyond previously qualified values.
Configuration acceptance does not imply that a setting is known-good; crashes,
freezes, allocation failures, underruns, starvation, missed deadlines, and
decoder failures are valid experimental outcomes when limits are pushed.
