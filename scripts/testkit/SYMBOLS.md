# Clean symbols — `scripts/testkit`

DIRECTORY=scripts/testkit
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=IN_PROGRESS

This directory contains retained historical TestKit/M4 tooling plus
successor-owned clean development tooling. Generic PS2 ELF deployment is
separate from Issue-specific qualification policy; Issue #7 owns its
manifest/observer/evaluator apparatus and consumes the generic deployment
evidence. The former frozen-legacy compatibility bridge is retired. Only
deliberately adopted clean-generation files receive the clean `File synopsis:`
marker and enter this dictionary.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| MAGIC | constant | scripts/testkit/elf-identity.py | successor ELF identity | module | Identifies the unique fixed-format runtime identity blob embedded in a candidate ELF. | Issue #7 DUT identity |
| TEST_SIZE | constant | scripts/testkit/elf-identity.py | successor ELF identity | module | Defines the fixed byte capacity of the stampable hardware test identifier field. | Issue #7 DUT identity |
| DIGEST_SIZE | constant | scripts/testkit/elf-identity.py | successor ELF identity | module | Defines the fixed byte capacity of the lowercase hexadecimal identity digest plus terminator. | Issue #7 DUT identity |
| PRISTINE_TEST | constant | scripts/testkit/elf-identity.py | successor ELF identity | module | Defines the exact unstamped test-identifier bytes required before stamping may proceed. | Issue #7 DUT identity |
| ZERO_DIGEST | constant | scripts/testkit/elf-identity.py | successor ELF identity | module | Defines the normalized all-zero hexadecimal digest field used while computing identity. | Issue #7 DUT identity |
| LOWER_HEX | constant | scripts/testkit/elf-identity.py | successor ELF identity | module | Holds the lowercase hexadecimal character set used by identity validation. | Issue #7 DUT identity |
| IdentityError | exception type | scripts/testkit/elf-identity.py | successor ELF identity | module | Represents fail-closed validation or mutation errors in successor identity handling. | Issue #7 DUT identity |
| sha256 | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Returns the lowercase SHA-256 digest of caller-supplied bytes. | Issue #7 DUT identity |
| encode_test_id | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Validates an ASCII hardware test ID and encodes it into the fixed 64-byte field. | Issue #7 DUT identity |
| locate_blob | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Requires exactly one complete identity blob and returns its field offsets. | Issue #7 DUT identity |
| decode_test_field | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Validates and decodes a fixed-size NUL-terminated test identifier field. | Issue #7 DUT identity |
| decode_digest_field | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Validates and decodes the fixed lowercase hexadecimal identity digest field. | Issue #7 DUT identity |
| stamp | function | scripts/testkit/elf-identity.py | successor ELF identity | public tool core | Stamps a pristine identity blob without changing ELF size and returns exact resulting digests. | Issue #7 DUT preparation |
| verify | function | scripts/testkit/elf-identity.py | successor ELF identity | public tool core | Recomputes normalized whole-ELF identity and fails on any test-ID or digest mismatch. | Issue #7 DUT preparation |
| print_common | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Emits the deterministic shared identity fields used by stamp and verify commands. | Issue #7 DUT preparation |
| usage | function | scripts/testkit/elf-identity.py | successor ELF identity | private | Emits the command contract and exits with a usage failure. | Issue #7 DUT preparation |
| main | function | scripts/testkit/elf-identity.py | successor ELF identity | command line | Dispatches fail-closed stamp and verify operations for one ELF. | Issue #7 DUT preparation |
| MAGIC | constant | scripts/testkit/identity-digest-forensics.py | identity forensics | module | Identifies the historical embedded identity blob examined in sealed ELF fixtures. | historical identity contract recovery |
| TEST_SIZE | constant | scripts/testkit/identity-digest-forensics.py | identity forensics | module | Defines the historical fixed test-identifier field size used during forensic normalization. | historical identity contract recovery |
| DIGEST_SIZE | constant | scripts/testkit/identity-digest-forensics.py | identity forensics | module | Defines the historical fixed digest-field size examined in sealed fixtures. | historical identity contract recovery |
| ZERO_DIGEST | constant | scripts/testkit/identity-digest-forensics.py | identity forensics | module | Defines the zeroed digest representation tested against historical fixtures. | historical identity contract recovery |
| PRISTINE_TEST | constant | scripts/testkit/identity-digest-forensics.py | identity forensics | module | Defines the pristine test-ID bytes used to evaluate candidate historical hash formulas. | historical identity contract recovery |
| FIXTURES | variable | scripts/testkit/identity-digest-forensics.py | identity forensics | module | Lists the sealed historical stamped ELF fixtures used as durable forensic evidence. | historical identity contract recovery |
| sha | function | scripts/testkit/identity-digest-forensics.py | identity forensics | private | Computes a lowercase SHA-256 digest for one candidate normalized byte sequence. | historical identity contract recovery |
| load_segments | function | scripts/testkit/identity-digest-forensics.py | identity forensics | private | Reads ELF program headers and returns file offsets and sizes for PT_LOAD segments. | historical identity contract recovery |
| joined_load | function | scripts/testkit/identity-digest-forensics.py | identity forensics | private | Concatenates PT_LOAD file bytes to evaluate load-segment-only digest hypotheses. | historical identity contract recovery |
| analyze | function | scripts/testkit/identity-digest-forensics.py | identity forensics | private | Evaluates mechanically plausible normalization/hash formulas for one sealed ELF fixture. | historical identity contract recovery |
| main | function | scripts/testkit/identity-digest-forensics.py | identity forensics | command line | Compares candidate digest formulas across all sealed fixtures and reports consistent matches. | historical identity contract recovery |
| MAGIC | constant | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Identifies the fixed identity blob reconstructed by the successor implementation. | successor identity compatibility |
| TEST_SIZE | constant | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Defines the fixture test-identifier field size used for normalization. | successor identity compatibility |
| DIGEST_SIZE | constant | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Defines the fixture digest-field size used for normalization. | successor identity compatibility |
| PRISTINE_TEST | constant | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Defines the exact pristine test-ID bytes reconstructed before successor stamping. | successor identity compatibility |
| ZERO_DIGEST | constant | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Defines the exact pristine digest bytes reconstructed before successor stamping. | successor identity compatibility |
| TOOL | constant | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Names the successor identity implementation exercised by compatibility fixtures. | successor identity compatibility |
| FIXTURES | variable | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | module | Lists sealed historical ELFs that successor stamping must reproduce byte-for-byte. | successor identity compatibility |
| sha | function | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | private | Computes fixture and candidate SHA-256 digests for exact comparison. | successor identity compatibility |
| fields | function | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | private | Locates and decodes the identity fields from one sealed stamped ELF fixture. | successor identity compatibility |
| run | function | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | private | Runs the successor identity command and fails if the subprocess does not succeed. | successor identity compatibility |
| main | function | scripts/testkit/successor-identity-compat-self-test.py | successor identity compatibility | command line | Proves byte-exact fixture reproduction plus restamp and overlong-ID fail-closed behavior. | successor identity compatibility |
| DEFAULT_FTP_HOST | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Defines the established private-link PS2 FTP host used when the caller does not override it. | deployment mechanics |
| DEFAULT_FTP_PORT | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Defines the established PS2 FTP control port used when the caller does not override it. | deployment mechanics |
| DEFAULT_ROLLING_PATH | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Defines the stable FreeMcBoot/wLaunchELF launch target `/mass/0/PS2VNC.ELF`. | deployment mechanics |
| UNIQUE_PREFIX | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Defines the deterministic archival filename prefix for unique deployed ELFs. | deployment mechanics |
| TEST_ID_RE | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Restricts test identifiers to filename-safe deterministic values. | deployment safety |
| SHA256_RE | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Validates optional caller-supplied SHA-256 identity expectations. | deployment safety |
| REMOTE_NAME_RE | constant | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Confines writable FTP targets to simple filenames under PS2 mass:/0. | deployment safety |
| DeploymentError | exception type | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | module | Represents a fail-closed repository, identity, transfer, collision, readback, or evidence error. | deployment mechanics |
| sha256_file | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Streams one ELF/readback/evidence file and returns its lowercase SHA-256 identity. | deployment identity |
| run_git | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Performs read-only Git queries used to validate the caller-selected DUT repository. | repository context |
| resolve_repository | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Resolves explicit `--repo` or current-working-directory Git context and labels its source. | repository context |
| resolve_local_path | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Resolves relative ELF/evidence operands against the selected DUT repository. | repository context |
| repository_metadata | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Records informative repository head/branch identity without imposing qualification policy. | repository context |
| validate_test_id | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Validates the caller's deterministic archival test identifier. | deployment safety |
| validate_remote_path | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Prevents deployment outside simple `/mass/0` filenames. | deployment safety |
| validate_expected_identity | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Computes local ELF SHA-256/bytes and enforces optional expected identity values before FTP. | deployment identity |
| curl_executable | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Selects normal curl while permitting a fake executable only under explicit self-test mode. | transfer mechanics |
| ftp_url | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Constructs one FTP URL from validated deployment inputs. | transfer mechanics |
| run_curl | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Performs one bounded noninteractive FTP operation and reports useful failure detail. | transfer mechanics |
| ensure_unique_remote_absent | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Probes the deterministic archival target and refuses silent overwrite of an existing unique ELF. | archival safety |
| default_evidence_path | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Chooses the generated DUT-repository evidence path when the caller does not supply one. | deployment evidence |
| validate_evidence_path | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Refuses to overwrite an existing deployment-evidence record. | deployment evidence |
| write_evidence | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Atomically records repository context, exact dual readbacks, and the fact that deployment is not hardware execution. | deployment evidence |
| build_parser | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | private | Defines the generic repository/ELF/test/evidence/FTP/dry-run/authorization command interface. | command line |
| main | function | scripts/testkit/deploy-elf.py | generic PS2 ELF deployment | command line | Plans or executes one exact unique-plus-rolling deployment without experiment-specific qualification policy. | deployment mechanics |
| cleanup | function | scripts/testkit/deploy-elf-self-test.sh | generic deployment regression | local helper | Removes only the disposable fake-FTP deployment fixture after the regression test. | development tooling |
| issue7_validate_manifest_current | function | scripts/testkit/issue7-apparatus-common.sh | Issue #7 apparatus contract | shared shell helper | Validates that one DUT manifest names the exact current committed Issue #7 source and qualified PS2IP authority. | hardware qualification |
| issue7_pid_owned | function | scripts/testkit/issue7-apparatus-common.sh | observer ownership | shared shell helper | Proves a live process is the run-specific successor UDP observer before any signal may be sent to it. | fail-closed cleanup |
| issue7_pcap_owned | function | scripts/testkit/issue7-apparatus-common.sh | observer ownership | shared shell helper | Proves a systemd unit owns the expected run-specific tcpdump capture before it may be stopped. | fail-closed cleanup |
| cleanup_error | function | scripts/testkit/issue7-arm-observers.sh | observer arming | local trap handler | Cleans only observers started by a failed arming transaction and preserves an apparatus-failure marker. | hardware apparatus |
| stop_udp | function | scripts/testkit/issue7-stop-observers.sh | observer shutdown | local helper | Stops the UDP observer only after proving run ownership and fails closed if ownership changes. | evidence preservation |
| now_iso | function | scripts/testkit/issue7-udp-observer.py | UDP evidence receiver | module helper | Returns an offset-aware timestamp for one received runtime diagnostic datagram. | machine evidence |
| main | function | scripts/testkit/issue7-udp-observer.py | UDP evidence receiver | command line | Receives UDP diagnostics and writes ordered timestamped JSONL without interpreting or controlling the DUT. | machine evidence |
| parse_key_values | function | scripts/testkit/issue7-result.py | Issue #7 evidence evaluator | private | Parses the generated DUT manifest without executing it as shell code. | evidence analysis |
| evaluate_payloads | function | scripts/testkit/issue7-result.py | Issue #7 evidence evaluator | reusable core | Requires one exact runtime identity followed by NET_READY, GS_READY, and DESKTOP_READY with no FATAL stage. | qualification evidence |
| count_packets | function | scripts/testkit/issue7-result.py | Issue #7 evidence evaluator | private | Counts captured packets matching one explicit tcpdump expression without claiming RFB frame semantics. | packet evidence |
| main | function | scripts/testkit/issue7-result.py | Issue #7 evidence evaluator | command line | Verifies sealed raw evidence, deployment readback, ordered diagnostics, and basic PS2-facing traffic. | qualification evidence |
| cleanup | function | scripts/testkit/issue7-apparatus-self-test.sh | apparatus regression test | local trap handler | Removes only disposable self-test state and terminates the loopback UDP receiver when necessary. | development tooling |
