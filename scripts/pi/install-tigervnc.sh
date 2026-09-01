#!/usr/bin/env bash

set -euo pipefail

EXPECTED_VERSION='1.15.0+dfsg-2.1~deb13u1'
PACKAGE='tigervnc-standalone-server'

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run this script through sudo/root.' >&2
    exit 2
fi

candidate="$(
    apt-cache policy "$PACKAGE" |
        awk '/Candidate:/ { print $2; exit }'
)"

if [ "$candidate" != "$EXPECTED_VERSION" ]; then
    echo "ERROR: TigerVNC candidate drifted." >&2
    echo "EXPECTED=$EXPECTED_VERSION" >&2
    echo "CANDIDATE=${candidate:-NONE}" >&2
    echo 'Re-evaluate the dependency ledger before installing a different version.' >&2
    exit 10
fi

status="$(dpkg-query -W -f='${db:Status-Status}' "$PACKAGE" 2>/dev/null || true)"
installed="$(dpkg-query -W -f='${Version}' "$PACKAGE" 2>/dev/null || true)"

if [ "$status" = 'installed' ]; then
    if [ "$installed" != "$EXPECTED_VERSION" ]; then
        echo "ERROR: installed TigerVNC version differs from the adopted version." >&2
        echo "EXPECTED=$EXPECTED_VERSION" >&2
        echo "INSTALLED=$installed" >&2
        exit 11
    fi

    echo 'TIGERVNC_ALREADY_INSTALLED=YES'
else
    echo "INSTALLING_TIGERVNC_VERSION=$EXPECTED_VERSION"
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-upgrade \
        "$PACKAGE=$EXPECTED_VERSION"
fi

installed="$(dpkg-query -W -f='${Version}' "$PACKAGE")"
if [ "$installed" != "$EXPECTED_VERSION" ]; then
    echo 'ERROR: post-install TigerVNC identity mismatch.' >&2
    exit 12
fi

if ! command -v Xtigervnc >/dev/null 2>&1; then
    echo 'ERROR: Xtigervnc executable not found after installation.' >&2
    exit 13
fi

echo
echo '===== TIGERVNC PACKAGE SET ====='
for pkg in \
    tigervnc-standalone-server \
    tigervnc-common \
    tigervnc-tools \
    libfile-readbackwards-perl \
    xfonts-base
do
    state="$(dpkg-query -W -f='${db:Status-Status}' "$pkg" 2>/dev/null || true)"
    version="$(dpkg-query -W -f='${Version}' "$pkg" 2>/dev/null || true)"
    printf '%-32s status=%-14s version=%s\n' \
        "$pkg" "${state:-NOT_KNOWN}" "${version:-NONE}"
done

echo
echo "XTIGERVNC=$(command -v Xtigervnc)"
echo "TIGERVNC_VERSION=$installed"
echo 'TIGERVNC_FOUNDATION_INSTALL=PASS'
