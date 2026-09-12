#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parents[2]
h1 = root / 'experiments/media-harness-h1'
cal = h1 / 'mpeg_presentation_calibration'


def replace_once(path, old, new, label):
    text = path.read_text()
    if old not in text:
        raise SystemExit(f'missing transport-fix anchor: {label}')
    path.write_text(text.replace(old, new, 1))

# Keep transport opaque at the session boundary. Do not introduce another typedef
# for a type already forward-declared by existing H1 RFB headers.
header = h1 / 'h1_cp2p_session_coordinator.h'
replace_once(
    header,
    '#include "h1_transport_runtime.h"\n',
    'struct pstvnc_h1_transport_runtime;\n',
    'session header transport include',
)
replace_once(
    header,
    '    pstvnc_h1_transport_runtime_t *transport;\n',
    '    struct pstvnc_h1_transport_runtime *transport;\n',
    'session transport field',
)
replace_once(
    header,
    '    pstvnc_h1_transport_runtime_t *transport,\n',
    '    struct pstvnc_h1_transport_runtime *transport,\n',
    'session init transport parameter',
)

source = h1 / 'h1_cp2p_session_coordinator.c'
replace_once(
    source,
    '#include "mpeg_presentation_calibration/h1_mpeg_start_transport.h"\n',
    '#include "h1_config.h"\n#include "mpeg_presentation_calibration/h1_mpeg_start_transport.h"\n\nconst pstvnc_h1_config_t *pstvnc_h1_transport_config(\n    const struct pstvnc_h1_transport_runtime *runtime);\n',
    'session source transport/config seam',
)
replace_once(
    source,
    '    pstvnc_h1_transport_runtime_t *transport,\n',
    '    struct pstvnc_h1_transport_runtime *transport,\n',
    'session source init transport parameter',
)

# Clean the START transport public seam itself: a struct declaration is enough.
start_header = cal / 'h1_mpeg_start_transport.h'
replace_once(
    start_header,
    'struct pstvnc_h1_transport_runtime;\ntypedef struct pstvnc_h1_transport_runtime pstvnc_h1_transport_runtime_t;\n\nint pstvnc_h1_mpeg_start_transport_send(\n    pstvnc_h1_transport_runtime_t *runtime,\n',
    'struct pstvnc_h1_transport_runtime;\n\nint pstvnc_h1_mpeg_start_transport_send(\n    struct pstvnc_h1_transport_runtime *runtime,\n',
    'START transport public typedef',
)

start_source = cal / 'h1_mpeg_start_transport.c'
text = start_source.read_text()
if text.count('pstvnc_h1_transport_runtime_t *runtime') != 2:
    raise SystemExit('unexpected START transport implementation typedef count')
start_source.write_text(
    text.replace(
        'pstvnc_h1_transport_runtime_t *runtime',
        'struct pstvnc_h1_transport_runtime *runtime',
    )
)

start_test = cal / 'h1_mpeg_start_transport_test.c'
text = start_test.read_text()
text = text.replace(
    'static pstvnc_h1_transport_runtime_t *last_runtime;',
    'static struct pstvnc_h1_transport_runtime *last_runtime;',
)
text = text.replace(
    '    pstvnc_h1_transport_runtime_t *runtime,',
    '    struct pstvnc_h1_transport_runtime *runtime,',
)
text = text.replace(
    '    pstvnc_h1_transport_runtime_t *runtime =\n        (pstvnc_h1_transport_runtime_t *)&runtime_token;',
    '    struct pstvnc_h1_transport_runtime *runtime =\n        (struct pstvnc_h1_transport_runtime *)&runtime_token;',
)
if 'pstvnc_h1_transport_runtime_t' in text:
    raise SystemExit('START transport test still contains transport typedef')
start_test.write_text(text)

# The session host contract needs only an opaque transport identity and the
# authoritative CONFIG returned by the stub accessor; do not instantiate the
# transport runtime merely to provide a session id.
session_test = cal / 'h1_cp2p_session_coordinator_test.c'
text = session_test.read_text()
text = text.replace(
    '#include <stdio.h>\n#include <string.h>\n',
    '#include <stdint.h>\n#include <stdio.h>\n#include <string.h>\n',
)
text = text.replace(
    'static uint32_t cleared_generation;\n',
    'static uint32_t cleared_generation;\nstatic pstvnc_h1_config_t fake_config;\n',
)
text = text.replace(
    'const pstvnc_h1_config_t *pstvnc_h1_transport_config(\n    const pstvnc_h1_transport_runtime_t *runtime)\n{\n    if (runtime == NULL || !runtime->config_accepted)\n        return NULL;\n    return &runtime->config;\n}',
    'const pstvnc_h1_config_t *pstvnc_h1_transport_config(\n    const struct pstvnc_h1_transport_runtime *runtime)\n{\n    return runtime != NULL ? &fake_config : NULL;\n}',
)
text = text.replace(
    'int pstvnc_h1_mpeg_start_transport_send(\n    pstvnc_h1_transport_runtime_t *runtime,',
    'int pstvnc_h1_mpeg_start_transport_send(\n    struct pstvnc_h1_transport_runtime *runtime,',
)
text = text.replace(
    '    pstvnc_h1_transport_runtime_t transport;\n    pstvnc_rfb_session_t rfb_session;',
    '    struct pstvnc_h1_transport_runtime *transport =\n        (struct pstvnc_h1_transport_runtime *)(uintptr_t)1;\n    pstvnc_rfb_session_t rfb_session;',
)
text = text.replace(
    '    memset(&transport, 0, sizeof(transport));\n    memset(&rfb_session, 0, sizeof(rfb_session));\n    transport.config_accepted = 1;\n    transport.config.session_id = UINT32_C(0x1234abcd);',
    '    memset(&rfb_session, 0, sizeof(rfb_session));\n    memset(&fake_config, 0, sizeof(fake_config));\n    fake_config.session_id = UINT32_C(0x1234abcd);',
)
text = text.replace(
    '        &coordinator, &transport, clear_mpeg, NULL));',
    '        &coordinator, transport, clear_mpeg, NULL));',
)
if 'pstvnc_h1_transport_runtime_t' in text:
    raise SystemExit('session coordinator host test still contains transport typedef')
session_test.write_text(text)

# Compile-only makefile no longer needs the full transport header as a direct
# dependency for the opaque session coordinator interface.
check_make = root / 'mk/media-harness-h1-cp2p-compositor-check.mk'
text = check_make.read_text()
text = text.replace(
    '\texperiments/media-harness-h1/h1_transport_runtime.h \\\n',
    '',
)
check_make.write_text(text)

Path(__file__).unlink()
