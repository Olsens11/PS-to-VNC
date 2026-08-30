# Testing and Experiment Policy

Empirical testing is part of the project authority model.

## Discover before constructing

Before proposing or repeating a routine empirical test:

1. inspect `docs/development/tooling.md`;
2. inspect existing TestKit procedures and manifests;
3. find the previous test record;
4. identify the established workload and apparatus;
5. reuse the canonical saved procedure when one exists.

Do not reconstruct an equivalent bespoke test simply because its commands are
known.

## Continuing an experiment

Identify:

- last proven result;
- next intended test;
- exact DUT;
- baseline configuration;
- workload;
- observers/capture tools;
- operator procedure;
- failure-safety mechanism.

Preserve those variables unless the experiment deliberately changes them.

## Apparatus is part of the experiment

A tooling change can alter experimental meaning.

Apparatus changes must therefore be explicit and recorded.

## Exact DUT identity

Hardware claims must identify the tested executable sufficiently to distinguish
it from neighboring builds.

For current PS-to-VNC development that can include:

- source authority;
- source commit;
- ELF SHA256;
- PT_LOAD SHA256;
- reproducible-build result;
- deployment readback identity.

## Machine and physical evidence

Machine validation and physical/operator observation are independent
authorities.

Do not infer visible hardware behavior from logs alone.

A qualification may require both.

## Failure classification

When a run fails, classify the failure before replaying anything:

- source/product;
- build/DUT;
- deployment;
- apparatus;
- evidence collection;
- operator procedure.

Continue from the exact state left by the failure rather than automatically
replaying the whole experiment.

## Hardware gate

A PT_LOAD change requires hardware qualification unless an explicit recorded
project rule permits inherited qualification.


## Raw evidence byte preservation

Files captured under `evidence/` are empirical records and must be preserved
byte-for-byte.

Git must not normalize their line endings or other textual representation merely
because a captured file has a text-like extension such as `.env` or `.tsv`.

Evidence integrity is established by its recorded hashes/manifests. Source-style
whitespace checks apply to authored source, configuration, scripts, and
documentation as appropriate; they must not require rewriting raw evidence.

If a generic repository check conflicts with valid sealed/captured evidence,
repair or scope the check/policy rather than cosmetically changing the evidence.
