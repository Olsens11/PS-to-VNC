# Clean Source Symbol Dictionaries

This portal is generated from directory-owned `SYMBOLS.md` files.

| Directory | Dictionary | Coverage | Symbols |
|---|---|---|---:|
| `mk` | [`mk/SYMBOLS.md`](../../mk/SYMBOLS.md) | COMPLETE | 32 |
| `scripts` | [`scripts/SYMBOLS.md`](../../scripts/SYMBOLS.md) | COMPLETE | 219 |
| `scripts/testkit` | [`scripts/testkit/SYMBOLS.md`](../../scripts/testkit/SYMBOLS.md) | IN_PROGRESS | 77 |
| `src` | [`src/SYMBOLS.md`](../../src/SYMBOLS.md) | COMPLETE | 414 |
| `src/diagnostics` | [`src/diagnostics/SYMBOLS.md`](../../src/diagnostics/SYMBOLS.md) | COMPLETE | 52 |
| `src/platform` | [`src/platform/SYMBOLS.md`](../../src/platform/SYMBOLS.md) | COMPLETE | 85 |
| `tests/unit` | [`tests/unit/SYMBOLS.md`](../../tests/unit/SYMBOLS.md) | IN_PROGRESS | 12 |

Generate the comprehensive view with:

```sh
python3 scripts/source-dictionary.py aggregate
```
