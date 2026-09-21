#!/usr/bin/env bash
# Install the exact reviewed TigerVNC provider package set needed by both the
# historical dedicated Xtigervnc :1 control and the selected native-desktop
# X0tigervnc :0 provider.
set -euo pipefail

EXPECTED_VERSION='1.15.0+dfsg-2.1~deb13u1'
STANDALONE_PACKAGE='tigervnc-standalone-server'
SCRAPING_PACKAGE='tigervnc-scraping-server'

if [ "${EUID}" -ne 0 ]; then
    echo 'ERROR: run this script through sudo/root.' >&2
    exit 2
fi

candidate_for()
{
    apt-cache policy "$1" |
        awk '/Candidate:/ { print $2; exit }'
}

for package in "$STANDALONE_PACKAGE" "$SCRAPING_PACKAGE"
do
    candidate="$(candidate_for "$package")"

    if [ "$candidate" != "$EXPECTED_VERSION" ]; then
        echo "ERROR: TigerVNC candidate drifted for $package." >&2
        echo "EXPECTED=$EXPECTED_VERSION" >&2
        echo "CANDIDATE=${candidate:-NONE}" >&2
        echo 'Re-evaluate the dependency ledger before installing a different version.' >&2
        exit 10
    fi
done

all_exact=1
for package in "$STANDALONE_PACKAGE" "$SCRAPING_PACKAGE"
do
    status="$(dpkg-query -W -f='${db:Status-Status}' "$package" 2>/dev/null || true)"
    installed="$(dpkg-query -W -f='${Version}' "$package" 2>/dev/null || true)"

    if [ "$status" = installed ] && [ "$installed" != "$EXPECTED_VERSION" ]
    then
        echo "ERROR: installed TigerVNC version differs for $package." >&2
        echo "EXPECTED=$EXPECTED_VERSION" >&2
        echo "INSTALLED=$installed" >&2
        exit 11
    fi

    if [ "$status" != installed ]
    then
        all_exact=0
    fi
done

if [ "$all_exact" -eq 1 ]
then
    echo 'TIGERVNC_PROVIDER_PACKAGES_ALREADY_INSTALLED=YES'
else
    echo "INSTALLING_TIGERVNC_PROVIDER_VERSION=$EXPECTED_VERSION"
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-upgrade \
        "$STANDALONE_PACKAGE=$EXPECTED_VERSION" \
        "$SCRAPING_PACKAGE=$EXPECTED_VERSION"
fi

for package in "$STANDALONE_PACKAGE" "$SCRAPING_PACKAGE"
do
    installed="$(dpkg-query -W -f='${Version}' "$package" 2>/dev/null || true)"
    if [ "$installed" != "$EXPECTED_VERSION" ]
    then
        echo "ERROR: post-install TigerVNC identity mismatch for $package." >&2
        echo "EXPECTED=$EXPECTED_VERSION" >&2
        echo "INSTALLED=${installed:-NONE}" >&2
        exit 12
    fi
done

command -v Xtigervnc >/dev/null 2>&1 || {
    echo 'ERROR: Xtigervnc executable not found after installation.' >&2
    exit 13
}

[ -x /usr/bin/X0tigervnc ] || {
    echo 'ERROR: selected native provider /usr/bin/X0tigervnc not found after installation.' >&2
    exit 14
}

echo
echo '===== TIGERVNC PACKAGE SET ====='
for pkg in \
    tigervnc-standalone-server \
    tigervnc-scraping-server \
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
echo 'X0TIGERVNC=/usr/bin/X0tigervnc'
echo "TIGERVNC_PROVIDER_VERSION=$EXPECTED_VERSION"
echo 'TIGERVNC_FOUNDATION_INSTALL=PASS'
