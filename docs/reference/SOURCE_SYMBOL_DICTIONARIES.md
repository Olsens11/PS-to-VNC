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
| `src` | coordinator, framebuffer, display conversion, RFB core | [`src/SYMBOLS.md`](../../src/SYMBOLS.md) | IN_PROGRESS |
| `src/platform` | PS2 system, Ethernet, and GS mechanisms | [`src/platform/SYMBOLS.md`](../../src/platform/SYMBOLS.md) | COMPLETE |
| `src/diagnostics` | adopted deterministic runtime identity | [`src/diagnostics/SYMBOLS.md`](../../src/diagnostics/SYMBOLS.md) | COMPLETE |
| `scripts` | clean-generation development tooling | [`scripts/SYMBOLS.md`](../../scripts/SYMBOLS.md) | IN_PROGRESS |
| `tests/unit` | clean-generation host unit and failure-path tests | [`tests/unit/SYMBOLS.md`](../../tests/unit/SYMBOLS.md) | IN_PROGRESS |

Generate an optional comprehensive view with:

```sh
python3 scripts/source-dictionary.py aggregate
```

The final portal will be regenerated from the directory dictionaries when the
retrofit reaches exhaustive coverage; strict completion is intentionally not a
project gate while any directory still declares `IN_PROGRESS`.

Historical, migration-era, evidence, and retained pre-refresh symbols are
deliberately outside this clean-generation portal.
