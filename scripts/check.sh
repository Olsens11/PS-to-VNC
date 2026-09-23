#!/usr/bin/env bash

set -euo pipefail

DIR="$(
    cd "$(dirname "${BASH_SOURCE[0]}")"
    pwd
)"

"$DIR/docs-check.sh"
"$DIR/continuity-check.sh"

echo '===== RFB RUNTIME PROFILE GENERATION CHECK ====='
python3 "$DIR/generate-rfb-runtime-profile.py" --check

echo '===== MPEG RUNTIME PROFILE GENERATION CHECK ====='
python3 "$DIR/generate-mpeg-runtime-profile.py" --check

echo '===== WORK LOG CHECK ====='
python3 "$DIR/work-log-check.py"

echo '===== SOURCE DICTIONARY CHECK ====='
python3 "$DIR/source-dictionary.py" check

echo 'PS_TO_VNC_PROJECT_CHECK=PASS'
