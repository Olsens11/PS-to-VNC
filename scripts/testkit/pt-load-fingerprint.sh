#!/usr/bin/env bash
set -euo pipefail

usage()
{
    echo \
        'usage: pt-load-fingerprint.sh ELF [OUTPUT_BIN]' \
        >&2
    exit 2
}

[ "$#" -ge 1 ] && [ "$#" -le 2 ] || usage

ELF="$1"
OUTPUT="${2:-}"

[ -f "$ELF" ] || {
    echo "ELF not found: $ELF" >&2
    exit 1
}

command -v readelf >/dev/null 2>&1 || {
    echo 'readelf not found' >&2
    exit 1
}

command -v python3 >/dev/null 2>&1 || {
    echo 'python3 not found' >&2
    exit 1
}

if [ -n "$OUTPUT" ]; then
    parent="$(dirname "$OUTPUT")"
    [ -d "$parent" ] || {
        echo "output directory not found: $parent" >&2
        exit 1
    }
fi

python3 - "$ELF" "$OUTPUT" <<'PY'
import hashlib
from pathlib import Path
import subprocess
import sys

elf = Path(sys.argv[1])
output_name = sys.argv[2]

text = subprocess.check_output(
    ["readelf", "-W", "-l", str(elf)],
    text=True,
)

segments = []

for line in text.splitlines():
    fields = line.split()

    if fields and fields[0] == "LOAD":
        segments.append(
            (
                int(fields[1], 16),
                int(fields[4], 16),
            )
        )

if not segments:
    raise SystemExit("no PT_LOAD segments")

digest = hashlib.sha256()
total = 0

output = (
    open(output_name, "wb")
    if output_name
    else None
)

try:
    with elf.open("rb") as source:
        for offset, size in segments:
            source.seek(offset)
            data = source.read(size)

            if len(data) != size:
                raise SystemExit(
                    "short PT_LOAD read"
                )

            digest.update(data)
            total += len(data)

            if output is not None:
                output.write(data)
finally:
    if output is not None:
        output.close()

print("TESTKIT_PT_LOAD_FINGERPRINT_VERSION=1")
print(f"ELF={elf}")
print(f"PT_LOAD_SEGMENTS={len(segments)}")
print(f"PT_LOAD_SHA256={digest.hexdigest()}")
print(f"PT_LOAD_BYTES={total}")

if output_name:
    print(f"PT_LOAD_OUTPUT={output_name}")

print("TESTKIT_PT_LOAD_FINGERPRINT=PASS")
PY
