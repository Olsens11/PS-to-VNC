# Clean symbols — `scripts`

DIRECTORY=scripts
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This dictionary begins with the hierarchical dictionary tooling itself. Other
clean-generation build and qualification tools will be added during the
retrofit; migration-era scripts remain excluded.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| validate | function | scripts/source-dictionary.py | dictionary validator | private | Validates directory ownership, live symbol references, descriptions, duplicates, and clean-file coverage. | source-naming-and-symbols policy |
| render_portal | function | scripts/source-dictionary.py | dictionary generator | private | Produces the lightweight directory and symbol-count navigation document. | source-naming-and-symbols policy |
| render_aggregate | function | scripts/source-dictionary.py | dictionary generator | private | Produces the optional comprehensive Markdown view in deterministic order. | source-naming-and-symbols policy |
| run | function | scripts/source-dictionary-self-test.py | dictionary self-test | private | Executes the validator against a disposable fixture and checks its exit status. | source-naming-and-symbols policy |
