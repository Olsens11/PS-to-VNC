# Clean symbols — `src/diagnostics`

DIRECTORY=src/diagnostics
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

Only the runtime-identity module in this directory is adopted by the new Issue
#7 linked core. Retained pre-refresh debug/report modules are intentionally not
indexed.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| pstvnc_diagnostics_identity_format_message | function | src/diagnostics/identity.c | runtime identity | public | Serializes the exact bounded TestKit-compatible runtime identity datagram. | ISSUE7_MINIMAL_CORE: deterministic runtime identity |
| PSTVNC_DIAGNOSTICS_IDENTITY_MAGIC | macro | src/diagnostics/identity.h | runtime identity | public | Names the fixed stampable ELF identity-blob magic string. | ISSUE7_DUT_MANIFEST |
