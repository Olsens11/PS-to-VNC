# Clean symbols — `scripts`

DIRECTORY=scripts
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This dictionary begins with the hierarchical dictionary tooling itself. Other
clean-generation build and qualification tools will be added during the
retrofit; migration-era scripts remain excluded.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| COVERAGE_STATES | constant | scripts/source-dictionary.py | dictionary validator | private | Defines the only coverage claims accepted from a directory-owned dictionary. | source-naming-and-symbols policy |
| Entry | type | scripts/source-dictionary.py | dictionary validator | private | Holds one parsed symbol description together with its source dictionary location. | source-naming-and-symbols policy |
| clean_files | function | scripts/source-dictionary.py | dictionary validator | private | Discovers clean-generation source files by their required file-synopsis marker. | source-naming-and-symbols policy |
| parse_dictionary | function | scripts/source-dictionary.py | dictionary validator | private | Parses directory identity, coverage state, and symbol rows from one dictionary. | source-naming-and-symbols policy |
| validate | function | scripts/source-dictionary.py | dictionary validator | private | Validates metadata, ownership, live symbol references, descriptions, duplicates, and clean-file coverage. | source-naming-and-symbols policy |
| render_portal | function | scripts/source-dictionary.py | dictionary generator | private | Produces lightweight directory navigation with honest coverage state and symbol counts. | source-naming-and-symbols policy |
| render_aggregate | function | scripts/source-dictionary.py | dictionary generator | private | Produces the optional comprehensive Markdown view in deterministic order. | source-naming-and-symbols policy |
| main | function | scripts/source-dictionary.py | dictionary command line | private | Parses the requested dictionary operation and reports a fail-closed result. | source-naming-and-symbols policy |
| write | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Creates a disposable fixture file and any required parent directories. | source-naming-and-symbols policy |
| run | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Executes the validator against a disposable fixture and checks its exit status. | source-naming-and-symbols policy |
| dictionary | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Builds fixture dictionary text with an explicit test coverage state. | source-naming-and-symbols policy |
