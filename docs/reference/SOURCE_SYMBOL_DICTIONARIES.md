# Clean Source Symbol Dictionaries

    GENERATED_PORTAL=BOOTSTRAP
    COMPLETE_SYMBOL_COVERAGE=NO
    ENFORCEMENT_IN_PROJECT_CHECK=NO

Canonical descriptions live in directory-owned `SYMBOLS.md` files. This portal
remains a lightweight bootstrap view while the clean-generation retrofit is
being populated; directory `COVERAGE=` metadata is the machine-checked authority
for whether a local dictionary is complete.

| Directory | Responsibility | Dictionary | Current status |
|---|---|---|---|
| `src` | coordinator, framebuffer, display conversion, RFB core | [`src/SYMBOLS.md`](../../src/SYMBOLS.md) | COMPLETE |
| `src/platform` | PS2 system, Ethernet, and GS mechanisms | [`src/platform/SYMBOLS.md`](../../src/platform/SYMBOLS.md) | COMPLETE |
| `src/diagnostics` | adopted deterministic runtime identity | [`src/diagnostics/SYMBOLS.md`](../../src/diagnostics/SYMBOLS.md) | COMPLETE |
| `scripts` | clean Issue #7 build/compile verification and dictionary tooling | [`scripts/SYMBOLS.md`](../../scripts/SYMBOLS.md) | COMPLETE |
| `tests/unit` | clean-generation host unit and failure-path tests | [`tests/unit/SYMBOLS.md`](../../tests/unit/SYMBOLS.md) | IN_PROGRESS |
| `scripts/testkit` | successor identity, DUT preparation, and manifest tooling | [`scripts/testkit/SYMBOLS.md`](../../scripts/testkit/SYMBOLS.md) | IN_PROGRESS |

Generate an optional comprehensive view with:

```sh
python3 scripts/source-dictionary.py aggregate
```

The remaining retrofit work is concentrated in the pre-policy clean unit tests
and the successor-owned Issue #7 TestKit files. The successor identity
implementation, historical digest forensics, and byte-exact compatibility proof
are now explicitly adopted and indexed there; the rest of that directory remains
under audit. Historical TestKit/M4/legacy apparatus remains excluded rather than
being relabeled as clean-generation code.

The final portal will be regenerated from the directory dictionaries when the
retrofit reaches exhaustive coverage; strict completion is intentionally not a
project gate while any directory still declares or is recorded as `IN_PROGRESS`.

Historical, migration-era, evidence, and retained pre-refresh symbols are
deliberately outside this clean-generation portal.
