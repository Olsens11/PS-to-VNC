#!/usr/bin/env bash
# File synopsis:
# Perform offline structural checks on the preserved Issue #7 provider-handoff
# scripts without contacting the network or changing any runtime state.
#
# The test verifies shell syntax and the important fixed contracts that define
# this exact experiment: 704x462 setup geometry, loopback w0 port 5901, the
# temporary proxy handoff, canonical-target restoration, stock 1920x1080
# teardown geometry, and retention of the known-good w0vncserver binary.
#
# Plain-language context: README.md
# Symbol and variable lookup: SYMBOLS.md
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
for SCRIPT in \
    setup-wayland-w0-apparatus.sh \
    switch-rfb-to-wayland.sh \
    switch-rfb-to-xtiger.sh \
    teardown-wayland-w0-apparatus.sh
do
    bash -n "$HERE/$SCRIPT"
done

grep -Fq -- '--custom-mode 704x462' "$HERE/setup-wayland-w0-apparatus.sh"
grep -Fq -- '-interface 127.0.0.1' "$HERE/setup-wayland-w0-apparatus.sh"
grep -Fq -- '-rfbport 5901' "$HERE/setup-wayland-w0-apparatus.sh"
grep -Fq 'systemd-socket-proxyd 127.0.0.1:5901' "$HERE/setup-wayland-w0-apparatus.sh"
grep -Fq 'Service=ps-to-vnc-rfb-w0-proxy-test.service' "$HERE/switch-rfb-to-wayland.sh"
grep -Fq 'rm "$DROPIN"' "$HERE/switch-rfb-to-xtiger.sh"
grep -Fq -- '--custom-mode 1920x1080' "$HERE/teardown-wayland-w0-apparatus.sh"
! grep -Fq -- '--off' "$HERE/teardown-wayland-w0-apparatus.sh"
grep -Fq 'WAYLAND_STOCK_GEOMETRY=1920x1080' "$HERE/teardown-wayland-w0-apparatus.sh"
grep -Fq 'W0_BINARY_RETAINED=' "$HERE/teardown-wayland-w0-apparatus.sh"

printf 'WAYLAND_W0_APPARATUS_SELF_TEST=PASS\n'
printf 'NETWORK_CONTACT=NONE\n'
printf 'RUNTIME_MUTATION=NONE\n'
