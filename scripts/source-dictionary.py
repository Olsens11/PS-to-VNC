#!/usr/bin/env python3
"""File synopsis:
Validate and aggregate directory-owned PS-to-VNC symbol dictionaries.

This tool owns clean-generation dictionary validation, trusted comprehensive
audit scope, incremental Git-delta scope, and deterministic dictionary views.
It does not define product symbols or include historical/pre-refresh source.
"""

from __future__ import annotations

import argparse
import hashlib
import ast
import json
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

CLEAN_MARKER = "File synopsis:"
DICT_NAME = "SYMBOLS.md"

# Dictionary completeness is a product-source concern. Development tools,
# TestKit, tests, build machinery, and retained evidence may keep useful
# dictionaries of their own, but they do not enter the product completeness
# gate merely because they live in this repository.
#
# Add another root only when that tree deliberately becomes maintained
# PS-to-VNC product source.
PRODUCT_SOURCE_ROOTS = (Path("src"),)

EXCLUDED_PARTS = {"baseline", "evidence", "working", "build", ".git"}
PLACEHOLDERS = {"todo", "tbd", "unknown", "placeholder", "describe me"}
HEADER = ("Name", "Kind", "File", "Owner", "Scope", "Description", "Context")
COVERAGE_STATES = {"IN_PROGRESS", "COMPLETE"}
STATE_PATH = Path("runtime/SOURCE_DICTIONARY_STATE.env")
STATE_VERSION = "1"
BASELINE_UNSET = "UNSET"
DEFINITION_DISCOVERY_STATUS = "PENDING"
DICTIONARY_KIND_COMPATIBILITY = {
    "function": {"function"},
    "function declaration": {
        "function declaration",
        "function alias declaration",
    },
    "parameter": {"parameter"},
    # Older clean headers often used the generic parameter kind for
    # prototype-local names. Both labels describe the same project-defined
    # identifier for coverage purposes when file/name/owner also match.
    "prototype parameter": {"prototype parameter", "parameter"},
    "variable": {"variable", "constant", "array"},
    "field": {"field"},
    "structure": {"structure", "type"},
    "type": {"type"},
    "enum": {"enum", "type"},
    "enum value": {"enum value", "constant"},
    "macro": {
        "macro",
        "constant",
        "include guard",
        "include-guard macro",
    },
    "external symbol declaration": {
        "external symbol declaration",
        "variable",
    },
    # Python source uses the same maintained dictionary vocabulary. These
    # compatibility sets preserve older deliberate labels without weakening
    # file/name/lexical-owner matching.
    "constant": {"constant", "variable", "array"},
    "type": {"type", "exception type", "structure"},
    "variable": {
        "variable",
        "constant",
        "array",
        "function alias declaration",
    },
}


class DictionaryError(RuntimeError):
    pass


@dataclass(frozen=True)
class Entry:
    name: str
    kind: str
    file: str
    owner: str
    scope: str
    description: str
    context: str
    dictionary: Path
    line: int


@dataclass(frozen=True, order=True)
class Definition:
    """One project-owned source definition discovered mechanically."""

    file: str
    owner: str
    kind: str
    name: str
    line: int


def is_excluded(path: Path, root: Path) -> bool:
    return any(part in EXCLUDED_PARTS for part in path.relative_to(root).parts)


def is_product_path(path: Path) -> bool:
    """Return whether one repository-relative path belongs to product source."""
    return any(
        path == product_root
        or product_root in path.parents
        for product_root in PRODUCT_SOURCE_ROOTS
    )


def is_make_source_path(path: Path) -> bool:
    """Identify maintained Makefile and .mk source paths."""
    return path.name == "Makefile" or path.suffix == ".mk"


def clean_files(root: Path) -> set[Path]:
    """Return maintained product files subject to dictionary completeness."""
    result: set[Path] = set()

    for product_root in PRODUCT_SOURCE_ROOTS:
        scan_root = root / product_root

        if not scan_root.is_dir():
            continue

        for pattern in ("*.c", "*.h", "*.py", "*.sh", "*.mk", "Makefile"):
            for path in scan_root.rglob(pattern):
                if is_excluded(path, root) or not path.is_file():
                    continue

                source_text = path.read_text(
                    encoding="utf-8",
                    errors="strict",
                )

                # Product C/Python/shell membership remains explicitly marked.
                # The Make adapter is retained and self-tested, but repository
                # build/test Makefiles outside a product root do not enter
                # dictionary completeness.
                if (
                    CLEAN_MARKER in source_text[:2048]
                    or is_make_source_path(path)
                ):
                    result.add(path.relative_to(root))

    return result


def unescape(cell: str) -> str:
    return cell.strip().replace("\\|", "|").replace("`", "")


def parse_dictionary(path: Path, root: Path) -> tuple[str, str, list[Entry]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    directory = None
    coverage = None
    entries: list[Entry] = []
    in_table = False
    for number, line in enumerate(lines, 1):
        if line.startswith("DIRECTORY="):
            directory = line.split("=", 1)[1].strip()
        if line.startswith("COVERAGE="):
            coverage = line.split("=", 1)[1].strip()
        if line.strip().startswith("| Name | Kind | File |"):
            cells = tuple(unescape(x) for x in line.strip().strip("|").split("|"))
            if cells != HEADER:
                raise DictionaryError(f"{path}:{number}: unexpected table columns")
            in_table = True
            continue
        if in_table and re.fullmatch(r"\s*\|(?:\s*:?-+:?\s*\|){7}\s*", line):
            continue
        if in_table and line.lstrip().startswith("|"):
            cells = [unescape(x) for x in line.strip().strip("|").split("|")]
            if len(cells) != 7:
                raise DictionaryError(f"{path}:{number}: expected seven cells")
            entries.append(Entry(*cells, dictionary=path, line=number))
        elif in_table and line.strip():
            in_table = False
    if not directory:
        raise DictionaryError(f"{path}: missing DIRECTORY metadata")
    if coverage not in COVERAGE_STATES:
        allowed = ", ".join(sorted(COVERAGE_STATES))
        raise DictionaryError(f"{path}: COVERAGE must be one of {allowed}")
    expected = path.parent.relative_to(root).as_posix() or "."
    if directory != expected:
        raise DictionaryError(f"{path}: DIRECTORY={directory}, expected {expected}")
    return directory, coverage, entries


def run_git(
    root: Path,
    *arguments: str,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    """Run one Git query without changing repository state."""
    completed = subprocess.run(
        ["git", "-C", str(root), *arguments],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if check and completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise DictionaryError(
            f"git {' '.join(arguments)} failed"
            + (f": {detail}" if detail else "")
        )
    return completed


def read_long_pass_baseline(root: Path) -> str:
    """Read and validate the trusted comprehensive-audit baseline."""
    state_path = root / STATE_PATH
    if not state_path.is_file():
        return BASELINE_UNSET

    fields: dict[str, str] = {}
    for line in state_path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise DictionaryError(
                f"{STATE_PATH}: malformed state line {line!r}"
            )
        key, value = line.split("=", 1)
        if not key or key in fields:
            raise DictionaryError(
                f"{STATE_PATH}: invalid or duplicate field {key!r}"
            )
        fields[key] = value

    version = fields.get("SOURCE_DICTIONARY_STATE_VERSION")
    if version != STATE_VERSION:
        raise DictionaryError(
            f"{STATE_PATH}: SOURCE_DICTIONARY_STATE_VERSION="
            f"{version!r}, expected {STATE_VERSION}"
        )

    baseline = fields.get("LAST_LONG_PASS_COMMIT")
    if not baseline:
        raise DictionaryError(
            f"{STATE_PATH}: missing LAST_LONG_PASS_COMMIT"
        )

    return baseline


def changed_paths_since_baseline(
    root: Path,
    baseline: str,
) -> set[Path]:
    """Return committed plus index, worktree, and untracked path changes."""
    paths: set[Path] = set()

    commands = (
        ("diff", "--name-only", "-z", f"{baseline}..HEAD", "--"),
        ("diff", "--cached", "--name-only", "-z", "--"),
        ("diff", "--name-only", "-z", "--"),
        ("ls-files", "--others", "--exclude-standard", "-z"),
    )

    for command in commands:
        completed = run_git(root, *command)
        for value in completed.stdout.split("\0"):
            if value:
                paths.add(Path(value))

    return paths


def is_definition_source_path(path: Path) -> bool:
    """Identify source languages covered by project-definition discovery."""
    return (
        path.suffix in {".c", ".h", ".py", ".sh"}
        or is_make_source_path(path)
    )


def resolve_definition_scope(
    root: Path,
    force_long: bool,
) -> tuple[str, str, str, set[Path]]:
    """Choose comprehensive or trusted-baseline incremental source scope."""
    baseline = read_long_pass_baseline(root)

    if force_long:
        return "LONG", baseline, "EXPLICIT_LONG", clean_files(root)

    if baseline == BASELINE_UNSET:
        return (
            "LONG_FALLBACK",
            baseline,
            "NO_TRUSTED_LONG_PASS",
            clean_files(root),
        )

    commit_probe = run_git(
        root,
        "cat-file",
        "-e",
        f"{baseline}^{{commit}}",
        check=False,
    )
    if commit_probe.returncode != 0:
        return (
            "LONG_FALLBACK",
            baseline,
            "BASELINE_COMMIT_UNRESOLVABLE",
            clean_files(root),
        )

    ancestor_probe = run_git(
        root,
        "merge-base",
        "--is-ancestor",
        baseline,
        "HEAD",
        check=False,
    )
    if ancestor_probe.returncode != 0:
        return (
            "LONG_FALLBACK",
            baseline,
            "BASELINE_NOT_ANCESTOR",
            clean_files(root),
        )

    changed = changed_paths_since_baseline(root, baseline)
    candidates = {
        path
        for path in changed
        if (
            is_product_path(path)
            and is_definition_source_path(path)
        )
    }

    return "INCREMENTAL", baseline, "TRUSTED_BASELINE", candidates


def write_long_pass_baseline(root: Path, commit: str) -> None:
    """Record one explicitly proven comprehensive dictionary-audit commit."""
    state_path = root / STATE_PATH
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        "\n".join(
            (
                f"SOURCE_DICTIONARY_STATE_VERSION={STATE_VERSION}",
                f"LAST_LONG_PASS_COMMIT={commit}",
                "",
            )
        ),
        encoding="utf-8",
    )


def resolve_universal_ctags() -> str:
    """Find Universal Ctags and reject incompatible ctags implementations."""
    candidate = (
        shutil.which("ctags-universal")
        or shutil.which("ctags")
    )
    if not candidate:
        raise DictionaryError(
            "Universal Ctags is required for C definition discovery"
        )

    completed = subprocess.run(
        [candidate, "--version"],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if (
        completed.returncode != 0
        or "Universal Ctags" not in completed.stdout
    ):
        raise DictionaryError(
            f"{candidate}: incompatible ctags implementation"
        )

    return candidate


def canonical_c_definition(record: dict[str, object]) -> Definition | None:
    """Normalize one Universal Ctags C record into dictionary vocabulary."""
    if record.get("_type") != "tag":
        return None

    name = record.get("name")
    source = record.get("path")
    parser_kind = record.get("kind")
    scope = record.get("scope")
    scope_kind = record.get("scopeKind")
    line = record.get("line")

    if (
        not isinstance(name, str)
        or not isinstance(source, str)
        or not isinstance(parser_kind, str)
        or not isinstance(line, int)
    ):
        raise DictionaryError(
            "Universal Ctags returned an incomplete C definition record"
        )

    owner = ""

    if parser_kind == "function":
        kind = "function"
    elif parser_kind == "prototype":
        kind = "function declaration"
    elif parser_kind == "parameter":
        if scope_kind == "prototype":
            kind = "prototype parameter"
        else:
            kind = "parameter"
        if isinstance(scope, str):
            owner = scope
    elif parser_kind == "local":
        kind = "variable"
        if isinstance(scope, str):
            owner = scope
    elif parser_kind == "variable":
        kind = "variable"
    elif parser_kind == "member":
        kind = "field"
        if isinstance(scope, str):
            owner = scope
    elif parser_kind in {"struct", "union"}:
        kind = "structure"
    elif parser_kind == "typedef":
        kind = "type"
    elif parser_kind == "enum":
        kind = "enum"
    elif parser_kind == "enumerator":
        kind = "enum value"
        if isinstance(scope, str):
            owner = scope
    elif parser_kind == "macro":
        kind = "macro"
    elif parser_kind == "externvar":
        kind = "external symbol declaration"
    else:
        return None

    return Definition(
        file=source,
        owner=owner,
        kind=kind,
        name=name,
        line=line,
    )


def discover_c_definitions(
    root: Path,
    source: Path,
) -> set[Definition]:
    """Discover C/C-header definitions without treating identifier uses as symbols."""
    absolute = root / source
    if not absolute.is_file():
        return set()

    ctags = resolve_universal_ctags()
    completed = subprocess.run(
        [
            ctags,
            "--options=NONE",
            "--output-format=json",
            "--fields=+nKsS",
            "--kinds-C=+lzpx",
            "-o",
            "-",
            source.as_posix(),
        ],
        cwd=root,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if completed.returncode != 0:
        detail = completed.stderr.strip()
        raise DictionaryError(
            f"Universal Ctags failed for {source}"
            + (f": {detail}" if detail else "")
        )

    definitions: set[Definition] = set()

    for raw_line in completed.stdout.splitlines():
        if not raw_line.strip():
            continue
        try:
            record = json.loads(raw_line)
        except json.JSONDecodeError as exc:
            raise DictionaryError(
                f"Universal Ctags emitted invalid JSON for {source}"
            ) from exc

        definition = canonical_c_definition(record)
        if definition is not None:
            definitions.add(definition)

    return definitions


def definition_is_documented(
    definition: Definition,
    entries: list[Entry],
) -> bool:
    """Match parser vocabulary to the richer maintained dictionary vocabulary."""
    accepted_kinds = DICTIONARY_KIND_COMPATIBILITY.get(
        definition.kind,
        {definition.kind},
    )

    for entry in entries:
        if entry.file != definition.file:
            continue
        if entry.name != definition.name:
            continue
        if entry.kind not in accepted_kinds:
            continue

        # Lexically owned definitions must remain disambiguated by that owner.
        # Top-level definitions deliberately use semantic dictionary owners.
        if definition.owner and entry.owner != definition.owner:
            continue

        return True

    return False


def python_assignment_kind(name: str, owner: str) -> str:
    """Classify one Python binding using maintained dictionary vocabulary."""
    if owner:
        return "variable"

    has_letter = any(character.isalpha() for character in name)
    if has_letter and name.upper() == name:
        return "constant"

    return "variable"


class PythonDefinitionVisitor(ast.NodeVisitor):
    """Discover project-owned Python bindings without counting identifier uses."""

    def __init__(self, source: Path) -> None:
        self.source = source
        self.function_stack: list[str] = []
        self.nonlocal_names_stack: list[set[str]] = []
        self.class_stack: list[str] = []
        self.comprehension_depth = 0
        self.anonymous_owner_counts: dict[str, int] = {}
        self.definitions: dict[
            tuple[str, str, str, str],
            Definition,
        ] = {}

    def add(
        self,
        name: str,
        kind: str,
        line: int,
        owner: str = "",
    ) -> None:
        """Store the first source location for one logical Python definition."""
        if not name or name == "_":
            return

        definition = Definition(
            file=self.source.as_posix(),
            owner=owner,
            kind=kind,
            name=name,
            line=line,
        )
        key = (
            definition.file,
            definition.owner,
            definition.kind,
            definition.name,
        )
        self.definitions.setdefault(key, definition)

    def current_function(self) -> str:
        """Return the nearest lexical function owner for local bindings."""
        if self.function_stack:
            return self.function_stack[-1]
        return ""

    def current_class(self) -> str:
        """Return the nearest fully qualified lexical class identity."""
        if self.class_stack:
            return self.class_stack[-1]
        return ""

    def lexical_parent(self) -> str:
        """Return the nearest function or class containing a new definition."""
        if self.function_stack:
            return self.function_stack[-1]
        if self.class_stack:
            return self.class_stack[-1]
        return ""

    def qualify(self, name: str, parent: str) -> str:
        """Build one stable lexical identity without changing source names."""
        if parent:
            return f"{parent}.{name}"
        return name

    def add_argument(self, argument: ast.arg, owner: str) -> None:
        """Record one explicit Python function parameter."""
        self.add(
            argument.arg,
            "parameter",
            argument.lineno,
            owner,
        )

    def add_binding_name(
        self,
        name: str,
        line: int,
    ) -> None:
        """Record one Python name introduced by assignment-like syntax."""
        function_owner = self.current_function()
        class_owner = self.current_class()

        if (
            self.nonlocal_names_stack
            and name in self.nonlocal_names_stack[-1]
        ):
            # Support the unambiguous form used by current clean source:
            # one nested function assigning a name from its sole enclosing
            # function scope. Deeper function nesting requires lexical
            # binding resolution and therefore remains fail-closed.
            if len(self.function_stack) != 2:
                raise DictionaryError(
                    f"{self.source}:{line}: "
                    f"nonlocal binding {name!r} requires unsupported "
                    "multi-level lexical ownership resolution"
                )

            self.add(
                name,
                "variable",
                line,
                self.function_stack[-2],
            )
            return

        if class_owner and not function_owner:
            self.add(
                name,
                "field",
                line,
                class_owner,
            )
            return

        self.add(
            name,
            python_assignment_kind(
                name,
                function_owner,
            ),
            line,
            function_owner,
        )

    def add_target(
        self,
        target: ast.expr,
        line: int | None = None,
    ) -> None:
        """Record names introduced by one assignment-like Python target."""
        target_line = line or getattr(target, "lineno", 0)
        class_owner = self.current_class()

        if isinstance(target, ast.Name):
            self.add_binding_name(
                target.id,
                target_line,
            )
            return

        if isinstance(target, ast.Starred):
            self.add_target(
                target.value,
                target_line,
            )
            return

        if isinstance(target, (ast.Tuple, ast.List)):
            for element in target.elts:
                self.add_target(element, target_line)
            return

        # Instance/class attribute assignment defines a project-owned field
        # only when it is explicitly rooted at self/cls inside a class.
        if (
            isinstance(target, ast.Attribute)
            and class_owner
            and isinstance(target.value, ast.Name)
            and target.value.id in {"self", "cls"}
        ):
            self.add(
                target.attr,
                "field",
                target_line,
                class_owner,
            )

    def visit_FunctionDef(self, node: ast.FunctionDef) -> None:
        self._visit_function(node)

    def visit_AsyncFunctionDef(
        self,
        node: ast.AsyncFunctionDef,
    ) -> None:
        self._visit_function(node)

    def _visit_function(
        self,
        node: ast.FunctionDef | ast.AsyncFunctionDef,
    ) -> None:
        lexical_parent = self.lexical_parent()
        self.add(
            node.name,
            "function",
            node.lineno,
            lexical_parent,
        )

        owner = self.qualify(
            node.name,
            lexical_parent,
        )
        arguments = node.args

        if getattr(node, "type_params", ()):
            raise DictionaryError(
                f"{self.source}:{node.lineno}: "
                "Python type parameters are not yet supported "
                "by definition discovery"
            )

        # Decorators, defaults, and annotations execute/evaluate in the
        # enclosing lexical scope rather than the new function body scope.
        for decorator in node.decorator_list:
            self.visit(decorator)

        for default in arguments.defaults:
            self.visit(default)

        for default in arguments.kw_defaults:
            if default is not None:
                self.visit(default)

        for argument in (
            *arguments.posonlyargs,
            *arguments.args,
            *arguments.kwonlyargs,
        ):
            if argument.annotation is not None:
                self.visit(argument.annotation)

        if (
            arguments.vararg is not None
            and arguments.vararg.annotation is not None
        ):
            self.visit(arguments.vararg.annotation)

        if (
            arguments.kwarg is not None
            and arguments.kwarg.annotation is not None
        ):
            self.visit(arguments.kwarg.annotation)

        if node.returns is not None:
            self.visit(node.returns)

        for argument in arguments.posonlyargs:
            self.add_argument(argument, owner)
        for argument in arguments.args:
            self.add_argument(argument, owner)
        if arguments.vararg is not None:
            self.add_argument(arguments.vararg, owner)
        for argument in arguments.kwonlyargs:
            self.add_argument(argument, owner)
        if arguments.kwarg is not None:
            self.add_argument(arguments.kwarg, owner)

        self.function_stack.append(owner)
        self.nonlocal_names_stack.append(set())

        try:
            for statement in node.body:
                self.visit(statement)
        finally:
            self.nonlocal_names_stack.pop()
            self.function_stack.pop()

    def visit_ClassDef(self, node: ast.ClassDef) -> None:
        lexical_parent = self.lexical_parent()
        self.add(
            node.name,
            "type",
            node.lineno,
            lexical_parent,
        )

        class_identity = self.qualify(
            node.name,
            lexical_parent,
        )

        if getattr(node, "type_params", ()):
            raise DictionaryError(
                f"{self.source}:{node.lineno}: "
                "Python type parameters are not yet supported "
                "by definition discovery"
            )

        # Class decorators, bases, and keywords are evaluated in the
        # enclosing lexical scope, before the class body executes.
        for decorator in node.decorator_list:
            self.visit(decorator)

        for base in node.bases:
            self.visit(base)

        for keyword in node.keywords:
            self.visit(keyword.value)

        self.class_stack.append(class_identity)
        try:
            for statement in node.body:
                self.visit(statement)
        finally:
            self.class_stack.pop()

    def visit_Assign(self, node: ast.Assign) -> None:
        for target in node.targets:
            self.add_target(target, node.lineno)
        self.visit(node.value)

    def visit_AnnAssign(self, node: ast.AnnAssign) -> None:
        self.add_target(node.target, node.lineno)
        if node.value is not None:
            self.visit(node.value)

    def visit_AugAssign(self, node: ast.AugAssign) -> None:
        """Visit augmented-assignment expressions without inventing a binding."""
        self.visit(node.value)

    def visit_NamedExpr(self, node: ast.NamedExpr) -> None:
        if self.comprehension_depth:
            raise DictionaryError(
                f"{self.source}:{node.lineno}: "
                "assignment expressions inside Python comprehensions "
                "require explicit containing-scope ownership support"
            )

        self.add_target(node.target, node.lineno)
        self.visit(node.value)

    def visit_For(self, node: ast.For) -> None:
        self.add_target(node.target, node.lineno)
        self.visit(node.iter)
        for statement in node.body:
            self.visit(statement)
        for statement in node.orelse:
            self.visit(statement)

    def visit_AsyncFor(self, node: ast.AsyncFor) -> None:
        self.visit_For(node)

    def visit_With(self, node: ast.With) -> None:
        for item in node.items:
            self.visit(item.context_expr)
            if item.optional_vars is not None:
                self.add_target(
                    item.optional_vars,
                    item.optional_vars.lineno,
                )
        for statement in node.body:
            self.visit(statement)

    def visit_AsyncWith(self, node: ast.AsyncWith) -> None:
        self.visit_With(node)

    def visit_ExceptHandler(self, node: ast.ExceptHandler) -> None:
        if isinstance(node.name, str):
            self.add(
                node.name,
                "variable",
                node.lineno,
                self.current_function(),
            )
        if node.type is not None:
            self.visit(node.type)
        for statement in node.body:
            self.visit(statement)

    def target_binding_names(
        self,
        target: ast.AST,
    ) -> tuple[str, ...]:
        """Return source names introduced by one assignment-style target."""
        if isinstance(target, ast.Name):
            return (target.id,)

        if isinstance(target, ast.Starred):
            return self.target_binding_names(target.value)

        if isinstance(target, (ast.Tuple, ast.List)):
            names: list[str] = []

            for element in target.elts:
                names.extend(
                    self.target_binding_names(element)
                )

            return tuple(names)

        return ()

    def anonymous_scope_owner(
        self,
        node: ast.AST,
        kind: str,
        binding_names: tuple[str, ...],
    ) -> str:
        """Build a semantic anonymous owner independent of source position."""
        lexical_parent = self.lexical_parent()

        syntax = ast.dump(
            node,
            annotate_fields=True,
            include_attributes=False,
        ).encode("utf-8")

        digest = hashlib.sha256(syntax).hexdigest()[:12]
        names = ",".join(binding_names) or "-"

        # Owner text participates in colon-delimited diagnostics, so the
        # anonymous identity itself must not contain ':'.
        anonymous = (
            f"<{kind}[{names}]#{digest}>"
        )

        semantic_base = self.qualify(
            anonymous,
            lexical_parent,
        )

        occurrence = (
            self.anonymous_owner_counts.get(
                semantic_base,
                0,
            )
            + 1
        )

        self.anonymous_owner_counts[
            semantic_base
        ] = occurrence

        # The ordinal distinguishes otherwise-identical anonymous scopes
        # without depending on source coordinates or broad ancestor bodies.
        return f"{semantic_base}~{occurrence}"

    def visit_Lambda(self, node: ast.Lambda) -> None:
        """Record a location-independent anonymous lambda lexical scope."""
        arguments = node.args

        argument_names = tuple(
            argument.arg
            for argument in (
                *arguments.posonlyargs,
                *arguments.args,
                *arguments.kwonlyargs,
            )
        )

        if arguments.vararg is not None:
            argument_names += (
                arguments.vararg.arg,
            )

        if arguments.kwarg is not None:
            argument_names += (
                arguments.kwarg.arg,
            )

        owner = self.anonymous_scope_owner(
            node,
            "lambda",
            argument_names,
        )

        # Lambda defaults are evaluated outside the lambda's local scope.
        for default in arguments.defaults:
            self.visit(default)

        for default in arguments.kw_defaults:
            if default is not None:
                self.visit(default)

        for argument in arguments.posonlyargs:
            self.add_argument(argument, owner)

        for argument in arguments.args:
            self.add_argument(argument, owner)

        if arguments.vararg is not None:
            self.add_argument(arguments.vararg, owner)

        for argument in arguments.kwonlyargs:
            self.add_argument(argument, owner)

        if arguments.kwarg is not None:
            self.add_argument(arguments.kwarg, owner)

        self.function_stack.append(owner)
        self.nonlocal_names_stack.append(set())

        try:
            self.visit(node.body)
        finally:
            self.nonlocal_names_stack.pop()
            self.function_stack.pop()

    def visit_MatchAs(self, node: ast.MatchAs) -> None:
        """Record names captured by `case ... as name` and bare captures."""
        if node.name is not None:
            self.add_binding_name(
                node.name,
                node.lineno,
            )

        if node.pattern is not None:
            self.visit(node.pattern)

    def visit_MatchStar(self, node: ast.MatchStar) -> None:
        """Record names captured by starred sequence patterns."""
        if node.name is not None:
            self.add_binding_name(
                node.name,
                node.lineno,
            )

    def visit_MatchMapping(self, node: ast.MatchMapping) -> None:
        """Record mapping-rest captures and visit nested mapping patterns."""
        if node.rest is not None:
            self.add_binding_name(
                node.rest,
                node.lineno,
            )

        for key in node.keys:
            self.visit(key)

        for pattern in node.patterns:
            self.visit(pattern)

    def visit_Global(self, node: ast.Global) -> None:
        """Fail closed until explicit global-assignment ownership is modeled."""
        raise DictionaryError(
            f"{self.source}:{node.lineno}: "
            "Python global binding ownership is not yet supported "
            "by definition discovery"
        )

    def visit_Nonlocal(self, node: ast.Nonlocal) -> None:
        """Declare nonlocal names for the supported one-parent function case."""
        if (
            not self.nonlocal_names_stack
            or len(self.function_stack) != 2
        ):
            raise DictionaryError(
                f"{self.source}:{node.lineno}: "
                "Python nonlocal binding requires unsupported "
                "multi-level lexical ownership resolution"
            )

        self.nonlocal_names_stack[-1].update(node.names)

    def visit_TypeAlias(self, node: ast.AST) -> None:
        """Fail closed on PEP 695 aliases until their ownership is mapped."""
        raise DictionaryError(
            f"{self.source}:{getattr(node, 'lineno', 0)}: "
            "Python type aliases are not yet supported "
            "by definition discovery"
        )

    def comprehension_owner(
        self,
        node: ast.AST,
        kind: str,
        generators: list[ast.comprehension],
    ) -> str:
        """Build a location-independent semantic comprehension owner."""
        names: list[str] = []

        for generator in generators:
            names.extend(
                self.target_binding_names(
                    generator.target
                )
            )

        return self.anonymous_scope_owner(
            node,
            kind,
            tuple(names),
        )

    def visit_comprehension_expression(
        self,
        node: ast.AST,
        kind: str,
        result_expressions: tuple[ast.expr, ...],
        generators: list[ast.comprehension],
    ) -> None:
        """Visit one Python 3 comprehension using its implicit lexical scope."""
        if not generators:
            raise DictionaryError(
                f"{self.source}:{getattr(node, 'lineno', 0)}: "
                "comprehension has no generator clauses"
            )

        first = generators[0]

        # Python evaluates the leftmost iterable in the containing scope
        # before entering the implicit comprehension function.
        self.visit(first.iter)

        owner = self.comprehension_owner(
            node,
            kind,
            generators,
        )

        self.function_stack.append(owner)
        self.nonlocal_names_stack.append(set())
        self.comprehension_depth += 1

        try:
            self.add_target(
                first.target,
                getattr(first.target, "lineno", 0),
            )

            for condition in first.ifs:
                self.visit(condition)

            for generator in generators[1:]:
                self.visit(generator.iter)

                self.add_target(
                    generator.target,
                    getattr(generator.target, "lineno", 0),
                )

                for condition in generator.ifs:
                    self.visit(condition)

            for expression in result_expressions:
                self.visit(expression)
        finally:
            self.comprehension_depth -= 1
            self.nonlocal_names_stack.pop()
            self.function_stack.pop()

    def visit_ListComp(self, node: ast.ListComp) -> None:
        self.visit_comprehension_expression(
            node,
            "listcomp",
            (node.elt,),
            node.generators,
        )

    def visit_SetComp(self, node: ast.SetComp) -> None:
        self.visit_comprehension_expression(
            node,
            "setcomp",
            (node.elt,),
            node.generators,
        )

    def visit_DictComp(self, node: ast.DictComp) -> None:
        self.visit_comprehension_expression(
            node,
            "dictcomp",
            (
                node.key,
                node.value,
            ),
            node.generators,
        )

    def visit_GeneratorExp(self, node: ast.GeneratorExp) -> None:
        self.visit_comprehension_expression(
            node,
            "genexpr",
            (node.elt,),
            node.generators,
        )


def discover_python_definitions(
    root: Path,
    source: Path,
) -> set[Definition]:
    """Discover Python definitions using the standard-library syntax tree."""
    absolute = root / source
    if not absolute.is_file():
        return set()

    try:
        source_text = absolute.read_text(encoding="utf-8")
    except OSError as exc:
        raise DictionaryError(
            f"{source}: unable to read Python source: {exc}"
        ) from exc

    try:
        tree = ast.parse(
            source_text,
            filename=source.as_posix(),
        )
    except SyntaxError as exc:
        raise DictionaryError(
            f"{source}: Python syntax parse failed: {exc}"
        ) from exc

    visitor = PythonDefinitionVisitor(source)
    visitor.visit(tree)
    return set(visitor.definitions.values())



SHFMT_EXPECTED_VERSION = "3.8.0"

# These names are controlled by the shell/runtime or by external tooling rather
# than invented by PS-to-VNC. Assigning/exporting them does not make them
# project dictionary symbols.
SHELL_EXTERNAL_NAMES = frozenset(
    {
        "HOME",
        "IFS",
        "LINENO",
        "OLDPWD",
        "OPTIND",
        "PATH",
        "PWD",
        "PYTHONDONTWRITEBYTECODE",
        "RANDOM",
        "SECONDS",
    }
)

# These commands can create shell variables through argument semantics rather
# than through ordinary assignment AST nodes. The current clean source does not
# use them in a defining form. Fail closed if one appears until its exact
# binding grammar is deliberately supported.
SHELL_UNSUPPORTED_DEFINITION_COMMANDS = frozenset(
    {
        "eval",
        "getopts",
        "let",
        "mapfile",
        "readarray",
    }
)

# Arithmetic mutation is not present in the current clean shell surface.
# Detection here protects completeness: a future arithmetic definition must be
# implemented explicitly rather than being silently omitted.
SHELL_ARITHMETIC_MUTATION_PATTERN = re.compile(
    r"(?:"
    r"\+\+|--|<<=|>>=|\+=|-=|\*=|/=|%=|&=|\|=|\^="
    r"|(?<![<>=!])=(?!=)"
    r")"
)


def resolve_shfmt() -> str:
    """Resolve the exact shfmt AST provider supported by this validator."""
    candidate = shutil.which("shfmt")

    if candidate is None:
        raise DictionaryError(
            "shfmt is required for shell definition discovery"
        )

    completed = subprocess.run(
        [candidate, "-version"],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    version = completed.stdout.strip()

    if (
        completed.returncode != 0
        or version != SHFMT_EXPECTED_VERSION
    ):
        detail = (
            completed.stderr.strip()
            or version
            or f"exit {completed.returncode}"
        )
        raise DictionaryError(
            "unsupported shfmt for shell definition discovery: "
            f"{candidate}: {detail}; "
            f"expected {SHFMT_EXPECTED_VERSION}"
        )

    return candidate


def shfmt_node_line(node: dict[str, object]) -> int:
    """Return the source line carried by one shfmt JSON AST node."""
    position = node.get("Pos")

    if not isinstance(position, dict):
        raise DictionaryError(
            "shfmt definition node is missing Pos"
        )

    line = position.get("Line")

    if not isinstance(line, int) or line <= 0:
        raise DictionaryError(
            "shfmt definition node has invalid source line"
        )

    return line


def shfmt_literal_value(node: object) -> str:
    """Return a literal AST value when a shfmt node is statically named."""
    if not isinstance(node, dict):
        return ""

    value = node.get("Value")

    if isinstance(value, str):
        return value

    parts = node.get("Parts")

    if (
        isinstance(parts, list)
        and len(parts) == 1
        and isinstance(parts[0], dict)
        and parts[0].get("Type") == "Lit"
    ):
        literal = parts[0].get("Value")

        if isinstance(literal, str):
            return literal

    return ""



def shell_read_binding_nodes(
    arguments: list[object],
    source: Path,
    line: int,
    source_syntax: str,
) -> list[tuple[str, dict[str, object]]]:
    """Return bindings from the deliberately supported Bash read grammar."""
    words: list[tuple[str, dict[str, object]]] = []

    # Args[0] is the literal `read` command itself.
    for argument in arguments[1:]:
        if not isinstance(argument, dict):
            raise DictionaryError(
                "unsupported shell read grammar "
                f"in {source}:{line}: {source_syntax}"
            )

        word = shfmt_literal_value(argument)

        # Dynamic argument construction could hide either an option or a
        # destination name. Do not guess at its binding semantics.
        if not word:
            raise DictionaryError(
                "unsupported shell read grammar "
                f"in {source}:{line}: {source_syntax}"
            )

        words.append((word, argument))

    # The current clean-generation source requires only Bash's -r switch.
    if words and words[0][0] == "-r":
        words = words[1:]

    # read with no explicit variable writes the shell-owned REPLY variable.
    # That form is outside the current project-symbol contract.
    if not words:
        raise DictionaryError(
            "unsupported shell read grammar "
            f"in {source}:{line}: {source_syntax}"
        )

    bindings: list[tuple[str, dict[str, object]]] = []

    for word, argument in words:
        # Any remaining option represents read semantics that have not yet
        # been deliberately mapped by this validator.
        if word.startswith("-"):
            raise DictionaryError(
                "unsupported shell read grammar "
                f"in {source}:{line}: {source_syntax}"
            )

        if re.fullmatch(
            r"[A-Za-z_][A-Za-z0-9_]*",
            word,
        ) is None:
            raise DictionaryError(
                "unsupported shell read binding "
                f"in {source}:{line}: {source_syntax}"
            )

        bindings.append((word, argument))

    return bindings


def shfmt_node_text(
    source_text: str,
    node: dict[str, object],
) -> str:
    """Recover exact UTF-8 source text for one positioned shfmt AST node."""
    position = node.get("Pos")
    end = node.get("End")

    if not isinstance(position, dict) or not isinstance(end, dict):
        raise DictionaryError(
            "shfmt positioned node is missing Pos/End"
        )

    start_offset = position.get("Offset")
    end_offset = end.get("Offset")

    if (
        not isinstance(start_offset, int)
        or not isinstance(end_offset, int)
        or start_offset < 0
        or end_offset < start_offset
    ):
        raise DictionaryError(
            "shfmt positioned node has invalid offsets"
        )

    encoded = source_text.encode("utf-8")

    if end_offset > len(encoded):
        raise DictionaryError(
            "shfmt positioned node exceeds source length"
        )

    try:
        return encoded[start_offset:end_offset].decode("utf-8")
    except UnicodeDecodeError as exc:
        raise DictionaryError(
            "shfmt node offsets do not align to UTF-8 source"
        ) from exc


def shell_parameter_assignment_name(
    source_text: str,
    node: dict[str, object],
) -> str:
    """Return NAME for ${NAME:=...}/${NAME=...}; otherwise return empty."""
    parameter = node.get("Param")

    if not isinstance(parameter, dict):
        return ""

    name = parameter.get("Value")

    if (
        not isinstance(name, str)
        or not name
        or name in SHELL_EXTERNAL_NAMES
    ):
        return ""

    raw = shfmt_node_text(source_text, node)

    # shfmt has already established that this is one parameter expansion.
    # We inspect its exact source spelling only to distinguish the two Bash
    # parameter operators that assign from non-mutating :-, :+, %, etc.
    pattern = (
        r"^\$\{"
        + re.escape(name)
        + r"(?::?=)"
    )

    if re.search(pattern, raw):
        return name

    return ""


def discover_shell_definitions(
    root: Path,
    source: Path,
) -> set[Definition]:
    """Discover PS-to-VNC definitions from one clean Bash source file."""
    absolute = root / source

    if not absolute.exists():
        return set()

    try:
        source_text = absolute.read_text(encoding="utf-8")
    except OSError as exc:
        raise DictionaryError(
            f"cannot read shell source {source}: {exc}"
        ) from exc

    shfmt = resolve_shfmt()

    completed = subprocess.run(
        [
            shfmt,
            "-ln",
            "bash",
            "--to-json",
        ],
        input=source_text,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    if completed.returncode != 0:
        detail = (
            completed.stderr.strip()
            or f"exit {completed.returncode}"
        )
        raise DictionaryError(
            f"shfmt failed for {source}: {detail}"
        )

    try:
        tree = json.loads(completed.stdout)
    except json.JSONDecodeError as exc:
        raise DictionaryError(
            f"shfmt returned invalid JSON for {source}: {exc}"
        ) from exc

    if not isinstance(tree, dict) or tree.get("Type") != "File":
        raise DictionaryError(
            f"shfmt returned unexpected AST root for {source}"
        )

    # Key without line deliberately deduplicates reassignments. The dictionary
    # represents one maintained symbol, not every write to that symbol.
    definitions: dict[
        tuple[str, str, str, str],
        Definition,
    ] = {}

    def add(
        name: str,
        kind: str,
        line: int,
        owner: str,
    ) -> None:
        if not name or name == "_":
            return

        definition = Definition(
            file=source.as_posix(),
            owner=owner,
            kind=kind,
            name=name,
            line=line,
        )
        key = (
            definition.file,
            definition.owner,
            definition.kind,
            definition.name,
        )
        definitions.setdefault(key, definition)

    def qualified_owner(parent: str, name: str) -> str:
        if parent:
            return f"{parent}.{name}"
        return name

    def visit(value: object, owner: str = "") -> None:
        if isinstance(value, list):
            for child in value:
                visit(child, owner)
            return

        if not isinstance(value, dict):
            return

        node_type = value.get("Type")

        if node_type == "FuncDecl":
            name_node = value.get("Name")
            name = shfmt_literal_value(name_node)

            if not name:
                raise DictionaryError(
                    f"shfmt function without static name in {source}"
                )

            add(
                name,
                "function",
                shfmt_node_line(value),
                owner,
            )

            nested_owner = qualified_owner(owner, name)

            for key, child in value.items():
                if key == "Name":
                    continue
                visit(child, nested_owner)

            return

        if node_type == "CallExpr":
            assigns = value.get("Assigns")
            args = value.get("Args")

            if not isinstance(assigns, list):
                assigns = []

            if not isinstance(args, list):
                args = []

            # A CallExpr with assignments and no Args is a maintained shell
            # assignment. Assignments preceding a command are environment
            # prefixes for that command only and are intentionally not project
            # dictionary symbols.
            if assigns and not args:
                for assignment in assigns:
                    if not isinstance(assignment, dict):
                        raise DictionaryError(
                            f"invalid shfmt assignment in {source}"
                        )

                    name_node = assignment.get("Name")
                    name = shfmt_literal_value(name_node)

                    if (
                        not name
                        or name in SHELL_EXTERNAL_NAMES
                    ):
                        continue

                    kind = (
                        "array"
                        if assignment.get("Array") is not None
                        else "variable"
                    )

                    add(
                        name,
                        kind,
                        shfmt_node_line(assignment),
                        owner,
                    )

            command = (
                shfmt_literal_value(args[0])
                if args
                else ""
            )

            if command == "read":
                source_syntax = " ".join(
                    shfmt_node_text(
                        source_text,
                        value,
                    ).split()
                )

                for name, argument in shell_read_binding_nodes(
                    args,
                    source,
                    shfmt_node_line(value),
                    source_syntax,
                ):
                    add(
                        name,
                        "variable",
                        shfmt_node_line(argument),
                        owner,
                    )

            if command in SHELL_UNSUPPORTED_DEFINITION_COMMANDS:
                source_syntax = " ".join(
                    shfmt_node_text(
                        source_text,
                        value,
                    ).split()
                )

                raise DictionaryError(
                    "unsupported shell definition-capable command "
                    f"in {source}:{shfmt_node_line(value)}: "
                    f"{command}: {source_syntax}"
                )

            if command == "printf":
                literal_args = [
                    shfmt_literal_value(argument)
                    for argument in args
                ]

                if "-v" in literal_args:
                    raise DictionaryError(
                        "unsupported shell definition-capable command "
                        f"in {source}:{shfmt_node_line(value)}: "
                        "printf -v"
                    )

        elif node_type == "DeclClause":
            variant_node = value.get("Variant")
            variant = shfmt_literal_value(variant_node)

            if variant not in {
                "declare",
                "export",
                "local",
                "readonly",
                "typeset",
            }:
                raise DictionaryError(
                    "unsupported shell declaration variant "
                    f"in {source}:{shfmt_node_line(value)}: "
                    f"{variant or '<unknown>'}"
                )

            arguments = value.get("Args")

            if not isinstance(arguments, list):
                raise DictionaryError(
                    f"invalid shfmt declaration arguments in {source}"
                )

            for argument in arguments:
                if not isinstance(argument, dict):
                    continue

                name_node = argument.get("Name")
                name = shfmt_literal_value(name_node)

                # Option words such as -a have no Name and are not symbols.
                if (
                    not name
                    or name in SHELL_EXTERNAL_NAMES
                ):
                    continue

                kind = (
                    "array"
                    if argument.get("Array") is not None
                    else "variable"
                )

                add(
                    name,
                    kind,
                    shfmt_node_line(argument),
                    owner,
                )

        elif node_type == "WordIter":
            name_node = value.get("Name")
            name = shfmt_literal_value(name_node)

            if name and name not in SHELL_EXTERNAL_NAMES:
                add(
                    name,
                    "variable",
                    shfmt_node_line(name_node)
                    if isinstance(name_node, dict)
                    else shfmt_node_line(value),
                    owner,
                )

        elif node_type == "ParamExp":
            name = shell_parameter_assignment_name(
                source_text,
                value,
            )

            if name:
                parameter = value.get("Param")
                add(
                    name,
                    "variable",
                    shfmt_node_line(parameter)
                    if isinstance(parameter, dict)
                    else shfmt_node_line(value),
                    owner,
                )

        elif node_type in {"ArithmCmd", "ArithmExp"}:
            raw = shfmt_node_text(source_text, value)

            if SHELL_ARITHMETIC_MUTATION_PATTERN.search(raw):
                raise DictionaryError(
                    "unsupported arithmetic shell definition "
                    f"in {source}:{shfmt_node_line(value)}: {raw}"
                )

        for child in value.values():
            visit(child, owner)

    visit(tree)

    return set(definitions.values())


MAKE_EXTERNAL_VARIABLE_NAMES = frozenset(
    {
        # GNU Make / conventional host-build interface vocabulary.
        "AR",
        "AS",
        "CC",
        "CFLAGS",
        "CPPFLAGS",
        "LD",
        "LDFLAGS",
        "LDLIBS",
        "MAKE",
        "RM",
        # PS2SDK Makefile.pref / Makefile.eeglobal interface vocabulary
        # used by the current clean linked build.
        "EE_BIN",
        "EE_CFLAGS",
        "EE_INCS",
        "EE_LIB",
        "EE_LIBS",
        "EE_OBJS",
    }
)

MAKE_SPECIAL_TARGET_NAMES = frozenset(
    {
        ".DEFAULT",
        ".DELETE_ON_ERROR",
        ".EXPORT_ALL_VARIABLES",
        ".IGNORE",
        ".INTERMEDIATE",
        ".LOW_RESOLUTION_TIME",
        ".NOTPARALLEL",
        ".ONESHELL",
        ".PHONY",
        ".POSIX",
        ".PRECIOUS",
        ".SECONDARY",
        ".SECONDEXPANSION",
        ".SILENT",
        ".SUFFIXES",
        ".WAIT",
    }
)

MAKE_DIRECT_ASSIGNMENT_PATTERN = re.compile(
    r"""
    ^\s*
    (?:(?:override|export|private)\s+)*
    (?P<name>[A-Za-z_][A-Za-z0-9_]*)
    \s*
    (?P<operator>\?=|:=|\+=|!=|=)
    """,
    re.VERBOSE,
)

MAKE_TARGET_ASSIGNMENT_PATTERN = re.compile(
    r"""
    ^[^\t#][^:]*:
    \s*
    (?:(?:override|export|private)\s+)*
    (?P<name>[A-Za-z_][A-Za-z0-9_]*)
    \s*
    (?P<operator>\?=|:=|\+=|!=|=)
    """,
    re.VERBOSE,
)

MAKE_DEFINE_PATTERN = re.compile(
    r"""
    ^\s*
    (?:(?:override|export|private)\s+)*
    define
    \s+
    (?P<name>[A-Za-z_][A-Za-z0-9_]*)
    \s*$
    """,
    re.VERBOSE,
)

MAKE_ASSIGNMENT_OPERATOR_PATTERN = re.compile(
    r"\?=|:=|\+=|!="
    r"|(?<![<>=!])=(?!=)"
)

MAKE_EXTERNAL_TARGET_PATTERN = re.compile(
    r"^\$\((?P<name>[A-Za-z_][A-Za-z0-9_]*)\)$"
)


def make_variable_is_project_owned(name: str) -> bool:
    """Exclude established build-system interface vocabulary."""
    return name not in MAKE_EXTERNAL_VARIABLE_NAMES


def make_target_is_project_owned(name: str) -> bool:
    """Exclude Make special targets and pure external-variable targets."""
    if name in MAKE_SPECIAL_TARGET_NAMES:
        return False

    external = MAKE_EXTERNAL_TARGET_PATTERN.fullmatch(name)

    if (
        external is not None
        and external.group("name") in MAKE_EXTERNAL_VARIABLE_NAMES
    ):
        return False

    return True


def discover_make_definitions(
    root: Path,
    source: Path,
) -> set[Definition]:
    """Discover project-owned Make variables and targets."""
    absolute = root / source

    if not absolute.is_file():
        return set()

    source_text = absolute.read_text(
        encoding="utf-8",
        errors="strict",
    )

    definitions: dict[
        tuple[str, str, str, str],
        Definition,
    ] = {}

    def add(
        name: str,
        kind: str,
        line: int,
    ) -> None:
        key = (
            source.as_posix(),
            "",
            kind,
            name,
        )

        definitions.setdefault(
            key,
            Definition(
                file=source.as_posix(),
                owner="",
                kind=kind,
                name=name,
                line=line,
            ),
        )

    # Variable definitions are parsed directly because Universal Ctags 5.9
    # omits some legitimate GNU Make assignment forms such as += and
    # override NAME := value.
    in_define = False

    for line_number, raw_line in enumerate(
        source_text.splitlines(),
        1,
    ):
        stripped = raw_line.strip()

        if in_define:
            if stripped == "endef":
                in_define = False
            continue

        if not stripped or stripped.startswith("#"):
            continue

        # Recipe lines execute shell; they do not define Make variables here.
        if raw_line.startswith("\t"):
            continue

        define_match = MAKE_DEFINE_PATTERN.match(raw_line)

        if define_match is not None:
            name = define_match.group("name")

            if make_variable_is_project_owned(name):
                add(name, "variable", line_number)

            in_define = True
            continue

        if stripped.startswith("define"):
            raise DictionaryError(
                "unsupported Make define grammar "
                f"in {source}:{line_number}: {raw_line}"
            )

        assignment_match = MAKE_DIRECT_ASSIGNMENT_PATTERN.match(
            raw_line
        )

        if assignment_match is None:
            assignment_match = (
                MAKE_TARGET_ASSIGNMENT_PATTERN.match(raw_line)
            )

        if assignment_match is not None:
            name = assignment_match.group("name")

            if make_variable_is_project_owned(name):
                add(name, "variable", line_number)

            continue

        # A non-recipe line with an assignment operator is definition-capable.
        # Fail closed rather than silently miss computed or otherwise unmapped
        # Make variable syntax.
        if MAKE_ASSIGNMENT_OPERATOR_PATTERN.search(raw_line):
            raise DictionaryError(
                "unsupported Make assignment grammar "
                f"in {source}:{line_number}: {raw_line}"
            )

    if in_define:
        raise DictionaryError(
            f"unterminated Make define block in {source}"
        )

    ctags = resolve_universal_ctags()

    completed = subprocess.run(
        [
            ctags,
            "--options=NONE",
            "--languages=Make",
            "--output-format=json",
            "--fields=+nK",
            "-f",
            "-",
            str(absolute),
        ],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )

    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise DictionaryError(
            f"Universal Ctags Make discovery failed for {source}"
            + (f": {detail}" if detail else "")
        )

    for raw_record in completed.stdout.splitlines():
        if not raw_record.strip():
            continue

        try:
            record = json.loads(raw_record)
        except json.JSONDecodeError as error:
            raise DictionaryError(
                f"invalid Universal Ctags JSON for {source}: {error}"
            ) from error

        if record.get("_type") != "tag":
            continue

        if record.get("kind") != "target":
            continue

        name = record.get("name")
        line = record.get("line")

        if not isinstance(name, str) or not name:
            raise DictionaryError(
                f"Universal Ctags returned nameless Make target for {source}"
            )

        if not isinstance(line, int):
            raise DictionaryError(
                "Universal Ctags returned Make target without line "
                f"for {source}:{name}"
            )

        if make_target_is_project_owned(name):
            add(name, "target", line)

    return set(definitions.values())


def discover_missing_make_symbols(
    root: Path,
    definition_scope: set[Path],
    dictionaries: list[
        tuple[str, str, Path, list[Entry]]
    ],
) -> list[str]:
    """Report undocumented project-owned Make definitions."""
    entries: list[Entry] = []

    for (
        _directory,
        _coverage,
        _path,
        dictionary_entries,
    ) in dictionaries:
        entries.extend(dictionary_entries)

    findings: list[str] = []

    for source in sorted(definition_scope):
        if not is_make_source_path(source):
            continue

        for definition in sorted(
            discover_make_definitions(root, source)
        ):
            if definition_is_documented(definition, entries):
                continue

            findings.append(
                "MISSING_DICTIONARY_SYMBOL="
                f"{definition.file}:"
                f"{definition.name}:"
                f"{definition.kind}:"
                f"{definition.owner or '-'}:"
                f"line={definition.line}"
            )

    return findings


def discover_missing_shell_symbols(
    root: Path,
    definition_scope: set[Path],
    dictionaries: list[
        tuple[Path, str, str, list[Entry]]
    ],
) -> list[str]:
    """Report clean Bash definitions absent from their owning dictionary."""
    entries: list[Entry] = []

    for (
        _path,
        _directory,
        _coverage,
        dictionary_entries,
    ) in dictionaries:
        entries.extend(dictionary_entries)

    findings: list[str] = []

    for source in sorted(
        path
        for path in definition_scope
        if path.suffix == ".sh"
    ):
        definitions = discover_shell_definitions(
            root,
            source,
        )

        for definition in sorted(definitions):
            if definition_is_documented(
                definition,
                entries,
            ):
                continue

            owner = definition.owner or "-"

            findings.append(
                "MISSING_DICTIONARY_SYMBOL="
                f"{definition.file}:"
                f"{definition.name}:"
                f"{definition.kind}:"
                f"{owner}:"
                f"line={definition.line}"
            )

    return findings

def discover_missing_python_symbols(
    root: Path,
    definition_scope: set[Path],
    dictionaries: list[tuple[str, str, Path, list[Entry]]],
) -> list[str]:
    """Report Python definitions absent from maintained owning dictionaries."""
    entries: list[Entry] = []
    for _directory, _coverage, _path, dictionary_entries in dictionaries:
        entries.extend(dictionary_entries)

    definitions: set[Definition] = set()
    for source in sorted(definition_scope):
        if source.suffix != ".py":
            continue
        definitions.update(
            discover_python_definitions(root, source)
        )

    findings: list[str] = []
    for definition in sorted(definitions):
        if definition_is_documented(definition, entries):
            continue

        owner = definition.owner or "-"
        findings.append(
            "MISSING_DICTIONARY_SYMBOL="
            f"{definition.file}:{definition.name}:"
            f"{definition.kind}:{owner}:line={definition.line}"
        )

    return findings


def discover_missing_c_symbols(
    root: Path,
    definition_scope: set[Path],
    dictionaries: list[tuple[str, str, Path, list[Entry]]],
) -> list[str]:
    """Report C definitions absent from their owning maintained dictionaries."""
    entries: list[Entry] = []
    for _directory, _coverage, _path, dictionary_entries in dictionaries:
        entries.extend(dictionary_entries)

    definitions: set[Definition] = set()
    for source in sorted(definition_scope):
        if source.suffix not in {".c", ".h"}:
            continue
        definitions.update(discover_c_definitions(root, source))

    findings: list[str] = []
    for definition in sorted(definitions):
        if definition_is_documented(definition, entries):
            continue

        owner = definition.owner or "-"
        findings.append(
            "MISSING_DICTIONARY_SYMBOL="
            f"{definition.file}:{definition.name}:"
            f"{definition.kind}:{owner}:line={definition.line}"
        )

    return findings


def validate(root: Path, require_complete: bool = False) -> tuple[list[tuple[str, str, Path, list[Entry]]], set[Path], list[str]]:
    dictionaries = []
    covered: set[Path] = set()
    keys: set[tuple[str, str, str, str]] = set()
    incomplete: list[str] = []
    attention: list[str] = []
    for path in sorted(root.rglob(DICT_NAME)):
        if is_excluded(path, root):
            continue

        relative_dictionary = path.relative_to(root)

        # Tool/test/build dictionaries may remain as optional orientation
        # material, but only product-root dictionaries participate in
        # structural/maintenance completeness and generated product views.
        if not is_product_path(relative_dictionary):
            continue

        directory, coverage, entries = parse_dictionary(path, root)
        if coverage != "COMPLETE":
            incomplete.append(directory)
        if not entries:
            raise DictionaryError(f"{path}: dictionary has no entries")
        for entry in entries:
            source = root / entry.file
            try:
                source.relative_to(path.parent)
            except ValueError as exc:
                raise DictionaryError(
                    f"{path}:{entry.line}: {entry.file} is outside dictionary directory"
                ) from exc

            # Dictionary identity is structural metadata. Validate it before
            # source-existence drift so duplicate rows remain a hard failure
            # even when their referenced source is temporarily absent.
            key = (
                entry.file,
                entry.owner,
                entry.kind,
                entry.name,
            )

            if key in keys:
                raise DictionaryError(
                    f"{path}:{entry.line}: duplicate entry {key}"
                )

            keys.add(key)

            if not source.is_file():
                attention.append(
                    f"MISSING_DICTIONARY_SOURCE={path}:{entry.line}:{entry.file}"
                )
                continue
            source_text = source.read_text(encoding="utf-8")

            # C/Python/shell source carries the explicit clean-generation
            # synopsis marker. Maintained Makefile/.mk infrastructure enters
            # clean scope through is_make_source_path(), because the adopted
            # clean build/test Make sources predate that marker convention.
            if (
                CLEAN_MARKER not in source_text[:2048]
                and not is_make_source_path(Path(entry.file))
            ):
                raise DictionaryError(
                    f"{path}:{entry.line}: "
                    f"{entry.file} is not clean-generation source"
                )
            if not re.search(rf"(?<![A-Za-z0-9_]){re.escape(entry.name)}(?![A-Za-z0-9_])", source_text):
                attention.append(
                    f"STALE_DICTIONARY_SYMBOL={path}:{entry.line}:{entry.file}:{entry.name}"
                )
            if len(entry.description) < 12 or entry.description.lower() in PLACEHOLDERS:
                attention.append(
                    f"INADEQUATE_DICTIONARY_DESCRIPTION={path}:{entry.line}:{entry.file}:{entry.name}"
                )
            covered.add(Path(entry.file))
        dictionaries.append((directory, coverage, path, entries))
    missing = clean_files(root) - covered
    for source in sorted(missing):
        attention.append(
            f"MISSING_DICTIONARY_FILE_COVERAGE={source.as_posix()}"
        )
    if require_complete:
        for directory in sorted(incomplete):
            attention.append(
                f"INCOMPLETE_DICTIONARY_COVERAGE={directory}"
            )
    return dictionaries, covered, attention


def render_portal(root: Path, dictionaries) -> str:
    lines = [
        "# Clean Source Symbol Dictionaries", "",
        "This portal is generated from directory-owned `SYMBOLS.md` files.", "",
        "| Directory | Dictionary | Coverage | Symbols |", "|---|---|---|---:|",
    ]
    for directory, coverage, path, entries in dictionaries:
        rel = path.relative_to(root).as_posix()
        lines.append(f"| `{directory}` | [`{rel}`](../../{rel}) | {coverage} | {len(entries)} |")
    lines += ["", "Generate the comprehensive view with:", "", "```sh",
              "python3 scripts/source-dictionary.py aggregate", "```", ""]
    return "\n".join(lines)


def render_aggregate(dictionaries) -> str:
    lines = ["# Comprehensive Clean Source Symbol Index", "",
             "Generated from directory-owned dictionaries; do not edit here.", ""]
    for directory, coverage, _path, entries in dictionaries:
        lines += [f"## `{directory}`", "", f"Coverage: `{coverage}`", "",
                  "| Name | Kind | File | Owner | Scope | Description | Context |",
                  "|---|---|---|---|---|---|---|"]
        for entry in sorted(entries, key=lambda item: (item.file, item.owner, item.kind, item.name)):
            values = [entry.name, entry.kind, entry.file, entry.owner, entry.scope,
                      entry.description, entry.context]
            values = [value.replace("|", "\\|") for value in values]
            lines.append("| " + " | ".join(values) + " |")
        lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=("check", "portal", "aggregate"))
    parser.add_argument("--root", type=Path, default=Path.cwd())
    parser.add_argument("--output", type=Path)
    parser.add_argument(
        "--require-complete",
        action="store_true",
        help="report directories that still declare COVERAGE=IN_PROGRESS",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="exit nonzero when ordinary dictionary-maintenance findings exist",
    )
    parser.add_argument(
        "--long",
        action="store_true",
        help="audit the complete clean-generation source scope",
    )
    parser.add_argument(
        "--record-baseline",
        action="store_true",
        help="record clean HEAD after a passing comprehensive audit",
    )
    args = parser.parse_args()

    if args.command != "check" and (args.long or args.record_baseline):
        parser.error("--long and --record-baseline apply only to check")

    if args.record_baseline and not args.long:
        parser.error("--record-baseline requires --long")

    if args.record_baseline and not args.require_complete:
        parser.error("--record-baseline requires --require-complete")

    if (
        args.record_baseline
        and DEFINITION_DISCOVERY_STATUS != "READY"
    ):
        parser.error(
            "--record-baseline is unavailable until "
            "project-definition discovery is implemented"
        )

    try:
        root = args.root.resolve()
        dictionaries, _, attention = validate(
            root,
            args.require_complete,
        )

        check_mode = None
        baseline = None
        baseline_reason = None
        definition_scope: set[Path] = set()

        if args.command == "check":
            (
                check_mode,
                baseline,
                baseline_reason,
                definition_scope,
            ) = resolve_definition_scope(root, args.long)

            print(f"SOURCE_DICTIONARY_CHECK_MODE={check_mode}")
            print(f"SOURCE_DICTIONARY_BASELINE={baseline}")
            print(f"SOURCE_DICTIONARY_BASELINE_REASON={baseline_reason}")
            print(f"DEFINITION_SCOPE_COUNT={len(definition_scope)}")
            print(
                "DEFINITION_DISCOVERY_STATUS="
                f"{DEFINITION_DISCOVERY_STATUS}"
            )

            attention.extend(
                discover_missing_c_symbols(
                    root,
                    definition_scope,
                    dictionaries,
                )
            )
            attention.extend(
                discover_missing_python_symbols(
                    root,
                    definition_scope,
                    dictionaries,
                )
            )
            attention.extend(
                discover_missing_shell_symbols(
                    root,
                    definition_scope,
                    dictionaries,
                )
            )
            attention.extend(
                discover_missing_make_symbols(
                    root,
                    definition_scope,
                    dictionaries,
                )
            )

            if check_mode == "INCREMENTAL":
                for path in sorted(definition_scope):
                    print(f"DEFINITION_SCOPE_PATH={path.as_posix()}")

        if attention:
            if args.command == "check":
                for finding in attention:
                    print(finding)
                print(f"ATTENTION_COUNT={len(attention)}")
                if args.record_baseline:
                    print(
                        "SOURCE_DICTIONARY_BASELINE_RECORD="
                        "REFUSED_ATTENTION"
                    )
                    return 1
                if args.strict:
                    print("SOURCE_DICTIONARIES=FAIL_STRICT")
                    return 1
                print("SOURCE_DICTIONARIES=ATTENTION")
                print("CHECK_CONTINUES=YES")
                return 0

            for finding in attention:
                print(finding, file=sys.stderr)
            print(
                f"SOURCE_DICTIONARIES_ATTENTION_COUNT={len(attention)}",
                file=sys.stderr,
            )
            if args.strict:
                print("SOURCE_DICTIONARIES=FAIL_STRICT", file=sys.stderr)
                return 1

        if args.command == "check":
            if args.record_baseline:
                status = run_git(
                    root,
                    "status",
                    "--porcelain=v1",
                    "--untracked-files=all",
                )
                if status.stdout:
                    raise DictionaryError(
                        "--record-baseline requires a clean working tree"
                    )

                head = run_git(
                    root,
                    "rev-parse",
                    "HEAD",
                ).stdout.strip()

                write_long_pass_baseline(root, head)
                print(
                    f"SOURCE_DICTIONARY_BASELINE_RECORDED={head}"
                )

            print("SOURCE_DICTIONARIES=PASS")
            return 0
        rendered = (
            render_portal(args.root.resolve(), dictionaries)
            if args.command == "portal"
            else render_aggregate(dictionaries)
        )
        if args.output:
            args.output.write_text(rendered, encoding="utf-8")
        else:
            sys.stdout.write(rendered)
        return 0
    except DictionaryError as exc:
        print(f"SOURCE_DICTIONARIES=FAIL: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
