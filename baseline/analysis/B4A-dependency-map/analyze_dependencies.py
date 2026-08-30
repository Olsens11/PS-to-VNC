#!/usr/bin/env python3

import collections
import csv
import pathlib
import re
import sys


source_path = pathlib.Path(sys.argv[1])
nm_path = pathlib.Path(sys.argv[2])
out_dir = pathlib.Path(sys.argv[3])

raw = source_path.read_text(
    encoding="utf-8",
    errors="replace",
)

raw_lines = raw.splitlines()


# ------------------------------------------------------------
# Strip comments and quoted strings while preserving line count
# and approximately preserving character positions.
# ------------------------------------------------------------

def strip_c_text(text):
    result = []
    i = 0
    in_block = False
    in_string = None
    escape = False

    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if in_block:
            if ch == "*" and nxt == "/":
                result.extend("  ")
                in_block = False
                i += 2
            else:
                result.append("\n" if ch == "\n" else " ")
                i += 1
            continue

        if in_string is not None:
            if escape:
                escape = False
                result.append(" ")
                i += 1
                continue

            if ch == "\\":
                escape = True
                result.append(" ")
                i += 1
                continue

            if ch == in_string:
                in_string = None
                result.append(" ")
                i += 1
                continue

            result.append("\n" if ch == "\n" else " ")
            i += 1
            continue

        if ch == "/" and nxt == "*":
            result.extend("  ")
            in_block = True
            i += 2
            continue

        if ch == "/" and nxt == "/":
            while i < len(text) and text[i] != "\n":
                result.append(" ")
                i += 1
            continue

        if ch in ('"', "'"):
            in_string = ch
            result.append(" ")
            i += 1
            continue

        result.append(ch)
        i += 1

    return "".join(result)


clean = strip_c_text(raw)
clean_lines = clean.splitlines()


# ------------------------------------------------------------
# Source function inventory.
# ------------------------------------------------------------

control_words = {
    "if",
    "for",
    "while",
    "switch",
    "else",
    "do",
    "sizeof",
}

functions = []

depth = 0
header_start = None
header_parts = []
current_function = None

for lineno, line in enumerate(clean_lines, start=1):
    stripped = line.strip()

    if depth == 0:
        if header_start is None:
            if stripped and not stripped.startswith("#"):
                header_start = lineno
                header_parts = [stripped]
        else:
            header_parts.append(stripped)

        if "{" in line and header_start is not None:
            before = " ".join(header_parts)
            before = before.split("{", 1)[0].strip()

            m = re.search(
                r"([A-Za-z_][A-Za-z0-9_]*)\s*"
                r"\([^;{}]*\)\s*$",
                before,
            )

            if m:
                name = m.group(1)

                if name not in control_words:
                    current_function = {
                        "name": name,
                        "start": header_start,
                    }

            header_start = None
            header_parts = []

        elif ";" in line:
            header_start = None
            header_parts = []

    previous_depth = depth
    depth += line.count("{")
    depth -= line.count("}")

    if (
        current_function is not None
        and previous_depth > 0
        and depth == 0
    ):
        current_function["end"] = lineno
        current_function["lines"] = (
            lineno - current_function["start"] + 1
        )

        functions.append(current_function)
        current_function = None


function_names = {
    f["name"]
    for f in functions
}

function_by_name = {
    f["name"]: f
    for f in functions
}


# ------------------------------------------------------------
# Compiler symbol inventory.
#
# nm types:
#   T/t = text
#   B/b = bss
#   D/d = initialized data
#   R/r = readonly data
#   S/s = small data
#   C/c = common
# ------------------------------------------------------------

nm_symbols = []

nm_re = re.compile(
    r"^\s*"
    r"([0-9A-Fa-f]+)\s+"
    r"([0-9A-Fa-f]+)\s+"
    r"([A-Za-z])\s+"
    r"(\S+)"
)

for line in nm_path.read_text(
    encoding="utf-8",
    errors="replace",
).splitlines():
    m = nm_re.match(line)

    if not m:
        continue

    address = int(m.group(1), 16)
    size = int(m.group(2), 16)
    kind = m.group(3)
    name = m.group(4)

    nm_symbols.append(
        {
            "address": address,
            "size": size,
            "kind": kind,
            "name": name,
        }
    )


compiled_functions = {
    x["name"]
    for x in nm_symbols
    if x["kind"] in "Tt"
}

compiled_globals = {
    x["name"]
    for x in nm_symbols
    if x["kind"] in "BbDdRrSsCc"
}


#
# Compiler-generated/local labels are not useful ownership state.
#
compiled_globals = {
    name
    for name in compiled_globals
    if not name.startswith((
        ".",
        "$",
        "__func__",
    ))
}


# ------------------------------------------------------------
# Proposed module classifier.
#
# This is deliberately conservative. "orchestration" means we
# should not move it early simply to force a prettier tree.
# ------------------------------------------------------------

def classify(name):
    lname = name.lower()

    if name == "main":
        return "core"

    if name in {
        "controller_thread",
        "display_live_desktop",
    }:
        return "orchestration"

    if (
        lname.startswith("debug_")
        or lname.startswith("profile_")
        or "telemetry" in lname
    ):
        return "diag"

    if (
        lname.startswith("ps2vnc_config_")
        or "config_load" in lname
        or "startup_mode_save" in lname
    ):
        return "config"

    if (
        lname.startswith("ps2vnc_management_")
        or "display_tx_" in lname
        or "transaction_fetch" in lname
        or "transaction_begin_to_pi" in lname
        or "transaction_commit_to_pi" in lname
        or "transaction_restore_to_pi" in lname
        or "transaction_restored_to_pi" in lname
        or "transaction_ack_to_pi" in lname
    ):
        return "management"

    if (
        lname.startswith("display_menu_")
        or lname.startswith("system_menu_")
        or lname.startswith("osk_")
        or "render_bitmap" in lname
        or "curtain" in lname
    ):
        return "ui"

    if (
        "calibration" in lname
        or lname.startswith("screen_fit_")
    ):
        return "display_calibration"

    if (
        lname.startswith("controller_")
        or lname.startswith("ps2vnc_controller_")
        or "pointer_event" in lname
        or "key_event" in lname
        or "key_tap" in lname
        or "hotkey" in lname
        or "dpad_" in lname
    ):
        return "input"

    if (
        lname.startswith("hextile_")
        or lname.startswith("rfb_dirty_")
        or "receive_framebuffer_update" in lname
        or "set_raw_encoding" in lname
        or "set_live_encoding" in lname
    ):
        return "rfb_decode"

    if (
        lname.startswith("rfb_")
        or "rfb_connect" in lname
        or "recv_exact" in lname
        or "send_rfb_message" in lname
        or "flush_rfb" in lname
        or "request_framebuffer" in lname
        or "recover_rfb" in lname
    ):
        return "rfb_transport"

    if (
        "remote_control" in lname
        or "rollback_display_transaction" in lname
        or "switch_display_mode" in lname
        or "display_confirm_" in lname
        or "display_transition_" in lname
    ):
        return "display_control"

    if (
        "video_mode" in lname
        or "display_geometry" in lname
        or "logical_desktop" in lname
        or "presented_geometry" in lname
    ):
        return "display_geometry"

    if (
        "hires" in lname
        or "framebuffer" in lname
        or "display_publish" in lname
        or "display_commit" in lname
        or "display_direct" in lname
        or "finish_draw" in lname
        or "draw_desktop" in lname
        or "present" in lname
        or lname.startswith("gs")
    ):
        return "display_backend"

    if (
        "network" in lname
        or "netman" in lname
    ):
        return "net"

    return "other"


for f in functions:
    f["module"] = classify(f["name"])


# ------------------------------------------------------------
# Build clean function body text.
# ------------------------------------------------------------

function_text = {}

for f in functions:
    body = "\n".join(
        clean_lines[
            f["start"] - 1:
            f["end"]
        ]
    )

    function_text[f["name"]] = body


# ------------------------------------------------------------
# Function call edges.
# ------------------------------------------------------------

call_edges = set()

for caller, body in function_text.items():
    for callee in function_names:
        if callee == caller:
            pattern = (
                r"\b" +
                re.escape(callee) +
                r"\s*\("
            )
        else:
            pattern = (
                r"\b" +
                re.escape(callee) +
                r"\s*\("
            )

        if re.search(pattern, body):
            call_edges.add((caller, callee))


# ------------------------------------------------------------
# Compiler-global references from each source function.
# ------------------------------------------------------------

global_refs = set()

#
# Build chunks so we do not create a gigantic regex.
#
global_names_sorted = sorted(
    compiled_globals,
    key=len,
    reverse=True,
)

chunk_size = 100

for caller, body in function_text.items():
    for pos in range(
        0,
        len(global_names_sorted),
        chunk_size,
    ):
        chunk = global_names_sorted[
            pos:
            pos + chunk_size
        ]

        if not chunk:
            continue

        rx = re.compile(
            r"\b(" +
            "|".join(
                re.escape(x)
                for x in chunk
            ) +
            r")\b"
        )

        for match in rx.finditer(body):
            global_refs.add(
                (
                    caller,
                    match.group(1),
                )
            )


# ------------------------------------------------------------
# Incoming/outgoing maps.
# ------------------------------------------------------------

out_calls = collections.defaultdict(set)
in_calls = collections.defaultdict(set)

for caller, callee in call_edges:
    out_calls[caller].add(callee)
    in_calls[callee].add(caller)

globals_by_function = collections.defaultdict(set)
functions_by_global = collections.defaultdict(set)

for func, glob in global_refs:
    globals_by_function[func].add(glob)
    functions_by_global[glob].add(func)


# ------------------------------------------------------------
# Module dependency edges.
# ------------------------------------------------------------

module_call_edges = collections.Counter()

for caller, callee in call_edges:
    a = function_by_name[caller]["module"]
    b = function_by_name[callee]["module"]

    if a != b:
        module_call_edges[(a, b)] += 1


module_global_edges = collections.defaultdict(set)

for func, glob in global_refs:
    module = function_by_name[func]["module"]
    module_global_edges[glob].add(module)


# ------------------------------------------------------------
# Strongly connected components of source call graph.
# ------------------------------------------------------------

index = 0
stack = []
indices = {}
lowlink = {}
onstack = set()
sccs = []


def strongconnect(v):
    global index

    indices[v] = index
    lowlink[v] = index
    index += 1

    stack.append(v)
    onstack.add(v)

    for w in out_calls[v]:
        if w not in indices:
            strongconnect(w)
            lowlink[v] = min(
                lowlink[v],
                lowlink[w],
            )
        elif w in onstack:
            lowlink[v] = min(
                lowlink[v],
                indices[w],
            )

    if lowlink[v] == indices[v]:
        component = []

        while True:
            w = stack.pop()
            onstack.remove(w)
            component.append(w)

            if w == v:
                break

        sccs.append(component)


for name in function_names:
    if name not in indices:
        strongconnect(name)


# ------------------------------------------------------------
# Output TSVs.
# ------------------------------------------------------------

with (
    out_dir / "functions.tsv"
).open(
    "w",
    encoding="utf-8",
    newline="",
) as f:
    w = csv.writer(
        f,
        delimiter="\t",
        lineterminator="\n",
    )

    w.writerow([
        "function",
        "module",
        "start_line",
        "end_line",
        "source_lines",
        "compiled_symbol",
        "incoming_calls",
        "outgoing_calls",
        "global_refs",
        "cross_module_in",
        "cross_module_out",
    ])

    for item in sorted(
        functions,
        key=lambda x: x["start"],
    ):
        name = item["name"]
        module = item["module"]

        cross_in = sum(
            function_by_name[x]["module"] != module
            for x in in_calls[name]
        )

        cross_out = sum(
            function_by_name[x]["module"] != module
            for x in out_calls[name]
        )

        w.writerow([
            name,
            module,
            item["start"],
            item["end"],
            item["lines"],
            int(name in compiled_functions),
            len(in_calls[name]),
            len(out_calls[name]),
            len(globals_by_function[name]),
            cross_in,
            cross_out,
        ])


with (
    out_dir / "call-edges.tsv"
).open(
    "w",
    encoding="utf-8",
    newline="",
) as f:
    w = csv.writer(
        f,
        delimiter="\t",
        lineterminator="\n",
    )

    w.writerow([
        "caller",
        "caller_module",
        "callee",
        "callee_module",
        "cross_module",
    ])

    for caller, callee in sorted(call_edges):
        a = function_by_name[caller]["module"]
        b = function_by_name[callee]["module"]

        w.writerow([
            caller,
            a,
            callee,
            b,
            int(a != b),
        ])


with (
    out_dir / "global-refs.tsv"
).open(
    "w",
    encoding="utf-8",
    newline="",
) as f:
    w = csv.writer(
        f,
        delimiter="\t",
        lineterminator="\n",
    )

    w.writerow([
        "function",
        "module",
        "global",
    ])

    for func, glob in sorted(global_refs):
        w.writerow([
            func,
            function_by_name[func]["module"],
            glob,
        ])


with (
    out_dir / "globals.tsv"
).open(
    "w",
    encoding="utf-8",
    newline="",
) as f:
    w = csv.writer(
        f,
        delimiter="\t",
        lineterminator="\n",
    )

    w.writerow([
        "global",
        "compiled_size_bytes",
        "nm_kind",
        "referencing_functions",
        "referencing_modules",
        "modules",
    ])

    symbol_by_name = {
        x["name"]: x
        for x in nm_symbols
    }

    for glob in sorted(
        compiled_globals,
        key=lambda g: (
            -len(functions_by_global[g]),
            g,
        ),
    ):
        modules = sorted(
            module_global_edges[glob]
        )

        sym = symbol_by_name[glob]

        w.writerow([
            glob,
            sym["size"],
            sym["kind"],
            len(functions_by_global[glob]),
            len(modules),
            ",".join(modules),
        ])


with (
    out_dir / "module-call-edges.tsv"
).open(
    "w",
    encoding="utf-8",
    newline="",
) as f:
    w = csv.writer(
        f,
        delimiter="\t",
        lineterminator="\n",
    )

    w.writerow([
        "from_module",
        "to_module",
        "call_edge_count",
    ])

    for (a, b), count in sorted(
        module_call_edges.items(),
        key=lambda x: (
            -x[1],
            x[0][0],
            x[0][1],
        ),
    ):
        w.writerow([
            a,
            b,
            count,
        ])


with (
    out_dir / "strongly-connected-components.tsv"
).open(
    "w",
    encoding="utf-8",
    newline="",
) as f:
    w = csv.writer(
        f,
        delimiter="\t",
        lineterminator="\n",
    )

    w.writerow([
        "component_size",
        "modules",
        "functions",
    ])

    for component in sorted(
        sccs,
        key=lambda x: (
            -len(x),
            sorted(x)[0],
        ),
    ):
        if len(component) <= 1:
            continue

        modules = sorted({
            function_by_name[x]["module"]
            for x in component
        })

        w.writerow([
            len(component),
            ",".join(modules),
            ",".join(sorted(component)),
        ])


# ------------------------------------------------------------
# Summary / extraction-risk view.
# ------------------------------------------------------------

module_functions = collections.defaultdict(list)

for f in functions:
    module_functions[f["module"]].append(f)


summary = []

summary.append(
    f"SOURCE_LINES={len(raw_lines)}"
)
summary.append(
    f"SOURCE_FUNCTIONS={len(functions)}"
)
summary.append(
    f"COMPILED_TEXT_SYMBOLS={len(compiled_functions)}"
)
summary.append(
    f"COMPILED_DATA_SYMBOLS={len(compiled_globals)}"
)
summary.append(
    f"FUNCTION_CALL_EDGES={len(call_edges)}"
)
summary.append(
    f"FUNCTION_GLOBAL_REF_EDGES={len(global_refs)}"
)
summary.append(
    f"CROSS_MODULE_CALL_EDGE_TYPES={len(module_call_edges)}"
)

multi_module_globals = [
    g
    for g in compiled_globals
    if len(module_global_edges[g]) > 1
]

summary.append(
    f"MULTI_MODULE_GLOBALS={len(multi_module_globals)}"
)

nontrivial_sccs = [
    x
    for x in sccs
    if len(x) > 1
]

summary.append(
    f"NONTRIVIAL_CALL_SCCS={len(nontrivial_sccs)}"
)


with (
    out_dir / "dependency-summary.txt"
).open(
    "w",
    encoding="utf-8",
) as f:
    for line in summary:
        f.write(line + "\n")

    f.write("\n===== PROPOSED MODULE FUNCTION COUNTS =====\n")

    for module, items in sorted(
        module_functions.items()
    ):
        f.write(
            f"{module:<24} "
            f"functions={len(items):>3} "
            f"source_lines="
            f"{sum(x['lines'] for x in items):>6}\n"
        )

    f.write("\n===== TOP SHARED GLOBALS =====\n")

    for glob in sorted(
        compiled_globals,
        key=lambda g: (
            -len(module_global_edges[g]),
            -len(functions_by_global[g]),
            g,
        ),
    )[:40]:
        modules = sorted(module_global_edges[glob])

        f.write(
            f"{glob:<42} "
            f"functions={len(functions_by_global[glob]):>3} "
            f"modules={len(modules):>2} "
            f"[{','.join(modules)}]\n"
        )

    f.write("\n===== HIGHEST CROSS-MODULE FUNCTIONS =====\n")

    ranked = []

    for item in functions:
        name = item["name"]
        module = item["module"]

        cross_in = sum(
            function_by_name[x]["module"] != module
            for x in in_calls[name]
        )

        cross_out = sum(
            function_by_name[x]["module"] != module
            for x in out_calls[name]
        )

        ranked.append(
            (
                cross_in + cross_out,
                cross_in,
                cross_out,
                len(globals_by_function[name]),
                item["lines"],
                module,
                name,
            )
        )

    for (
        total,
        cin,
        cout,
        gcount,
        lines,
        module,
        name,
    ) in sorted(
        ranked,
        reverse=True,
    )[:40]:
        f.write(
            f"{name:<46} "
            f"module={module:<20} "
            f"cross={total:>3} "
            f"in={cin:>3} "
            f"out={cout:>3} "
            f"globals={gcount:>3} "
            f"lines={lines:>4}\n"
        )

    f.write("\n===== LOW-COUPLING LEAF CANDIDATES =====\n")

    leaves = []

    for item in functions:
        name = item["name"]
        module = item["module"]

        if module in {
            "orchestration",
            "core",
            "other",
        }:
            continue

        cross_out = sum(
            function_by_name[x]["module"] != module
            for x in out_calls[name]
        )

        leaves.append(
            (
                cross_out,
                len(globals_by_function[name]),
                -len(in_calls[name]),
                item["lines"],
                module,
                name,
            )
        )

    for (
        cross_out,
        gcount,
        neg_in,
        lines,
        module,
        name,
    ) in sorted(leaves)[:50]:
        f.write(
            f"{name:<46} "
            f"module={module:<20} "
            f"cross_out={cross_out:>2} "
            f"globals={gcount:>3} "
            f"incoming={-neg_in:>3} "
            f"lines={lines:>4}\n"
        )

    f.write("\n===== NONTRIVIAL CALL SCCS =====\n")

    for component in sorted(
        nontrivial_sccs,
        key=lambda x: -len(x),
    )[:30]:
        modules = sorted({
            function_by_name[x]["module"]
            for x in component
        })

        f.write(
            f"size={len(component):>2} "
            f"modules=[{','.join(modules)}]\n"
        )

        for name in sorted(component):
            f.write(f"  {name}\n")


print("\n".join(summary))
