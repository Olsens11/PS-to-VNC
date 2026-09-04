#!/usr/bin/env bash

set -euo pipefail

DIR="$(
    cd "$(dirname "${BASH_SOURCE[0]}")"
    pwd
)"

"$DIR/docs-check.sh"
"$DIR/continuity-check.sh"

echo 'PS_TO_VNC_PROJECT_CHECK=PASS'
