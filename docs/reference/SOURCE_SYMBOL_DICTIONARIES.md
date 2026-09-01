# Clean Source Symbol Dictionaries

    GENERATED_PORTAL=BOOTSTRAP
    COMPLETE_SYMBOL_COVERAGE=NO
    ENFORCEMENT_IN_PROJECT_CHECK=NO

Canonical descriptions live in directory-owned `SYMBOLS.md` files. This portal
will be regenerated automatically after exhaustive clean-generation population.

| Directory | Responsibility | Dictionary | Current status |
|---|---|---|---|
| `src` | coordinator, framebuffer, display conversion, RFB core | [`src/SYMBOLS.md`](../../src/SYMBOLS.md) | population in progress |
| `src/platform` | PS2 system, Ethernet, and GS mechanisms | [`src/platform/SYMBOLS.md`](../../src/platform/SYMBOLS.md) | population in progress |
| `src/diagnostics` | adopted deterministic runtime identity | [`src/diagnostics/SYMBOLS.md`](../../src/diagnostics/SYMBOLS.md) | population in progress |
| `scripts` | clean-generation development tooling | [`scripts/SYMBOLS.md`](../../scripts/SYMBOLS.md) | population in progress |

Generate an optional comprehensive view with:

```sh
python3 scripts/source-dictionary.py aggregate
```

Historical, migration-era, evidence, and retained pre-refresh symbols are
deliberately outside this clean-generation portal.
