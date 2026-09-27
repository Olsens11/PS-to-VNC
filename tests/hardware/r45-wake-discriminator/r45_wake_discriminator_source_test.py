#!/usr/bin/env python3
"""Static ownership/scheduling contract check for the R45 PS2 harness."""

from pathlib import Path

SOURCE = Path(__file__).with_name("r45_wake_discriminator.c").read_text()


def between(start: str, end: str) -> str:
    start_index = SOURCE.index(start)
    end_index = SOURCE.index(end, start_index)
    return SOURCE[start_index:end_index]


send_exact = between("static int send_exact(", "static int wait_readable(")
wait_readable = between("static int wait_readable(", "static int take_outbound_ready(")
owner = between("static void owner_thread(", "static void submitter_thread(")
submitter = between("static void submitter_thread(", "static int create_semaphore(")
main = SOURCE[SOURCE.index("int main(") :]

assert '#define R45_CYCLE_COUNT 4096u' in SOURCE
assert '#define R45_PEER_PORT 5961' in SOURCE
assert '#define R45_VARIANT_NAME "BASELINE_1000US"' in SOURCE
assert '#define R45_READINESS_TIMEOUT_US 1000u' in SOURCE
assert '#define R45_VARIANT_NAME "CONTROL_ZERO_TIMEOUT"' in SOURCE
assert '#define R45_READINESS_TIMEOUT_US 0u' in SOURCE

assert send_exact.count("send(") == 1
assert wait_readable.count("select(") == 1
assert "recv(" not in SOURCE

assert "take_outbound_ready(context)" in owner
assert "SignalSema(context->owner_missed_semaphore)" in owner
assert "wait_readable(" in owner
assert "process_outbound(context, cycle)" in owner
assert "#if R45_ZERO_TIMEOUT_CONTROL" in owner

for forbidden in ("send(", "recv(", "select(", "socket(", "connect("):
    assert forbidden not in submitter, forbidden

assert "SignalSema(context->outbound_ready_semaphore)" in submitter
assert "WaitSema(context->outbound_done_semaphore)" in submitter
assert "context->serialized_count != cycle" in submitter

# Product receiver priority is 63. The diagnostic submitter is deliberately
# one priority step above it so publication is runnable inside the disputed gap.
assert "owner_stack,\n        (int)sizeof(owner_stack),\n        63," in main
assert "submitter_stack,\n        (int)sizeof(submitter_stack)," in main
assert "\n        62,\n        &context);" in main

print("R45_WAKE_DISCRIMINATOR_SOURCE_TEST=PASS")
