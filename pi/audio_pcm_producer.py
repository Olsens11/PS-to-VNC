#!/usr/bin/env python3
"""File synopsis:
Own one Wire-Session-scoped Raspberry Pi PCM capture producer for clean A002.

The owner discovers the PipeWire default sink monitor, launches one pw-record
process using the selected generated PCM projection, buffers at most the selected
channel window, accepts exact PS2 AUDIO credit, and publishes local readiness to
WireServer through a private socketpair. It never touches the PS2-facing socket
or allocates Wire sequence numbers. Unexpected capture termination is failure;
ordinary cleanup never fabricates the zero-length AUDIO producer-done marker.

Context: docs/ledge/LEDGE_AUDIT_A002_CONFIG_AUDIO_CLOCK.md and R39 in
docs/ledge/LEDGE_FOREMAN_STATE.md.
"""

from __future__ import annotations

from dataclasses import dataclass
import re
import socket
import subprocess
import threading
import time
from typing import Callable

import audio_runtime_profile_generated as generated
import wire_protocol as protocol


class AudioPcmProducerError(RuntimeError):
    """Fail one exact Pi AUDIO producer owner without inventing recovery."""


@dataclass(frozen=True)
class AudioPcmProfile:
    """Narrow generated Pi projection consumed by the capture owner."""

    channel_window_bytes: int
    rate_hz: int
    channels: int
    bits_per_sample: int
    frame_bytes: int

    def __post_init__(self) -> None:
        if self.channel_window_bytes <= 0:
            raise ValueError("AUDIO channel window must be positive")
        if self.rate_hz <= 0 or self.channels <= 0 or self.bits_per_sample <= 0:
            raise ValueError("AUDIO PCM fields must be positive")
        if self.bits_per_sample % 8 != 0:
            raise ValueError("AUDIO sample width must be byte aligned")
        expected = self.channels * (self.bits_per_sample // 8)
        if self.frame_bytes != expected or self.frame_bytes <= 0:
            raise ValueError("AUDIO PCM frame alignment is inconsistent")
        if self.channel_window_bytes % self.frame_bytes != 0:
            raise ValueError("AUDIO channel window must contain whole PCM frames")


def selected_audio_pcm_profile() -> AudioPcmProfile:
    """Return an immutable projection of the generated R36-selected values."""

    return AudioPcmProfile(
        channel_window_bytes=generated.CHANNEL_WINDOW_BYTES,
        rate_hz=generated.PCM_RATE_HZ,
        channels=generated.PCM_CHANNELS,
        bits_per_sample=generated.PCM_BITS_PER_SAMPLE,
        frame_bytes=generated.PCM_FRAME_BYTES,
    )


def discover_default_sink_monitor(
    *,
    run: Callable[..., subprocess.CompletedProcess[str]] = subprocess.run,
) -> str:
    """Resolve one qualified PipeWire default-sink node.name monitor target."""

    try:
        result = run(
            ["wpctl", "inspect", "@DEFAULT_AUDIO_SINK@"],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
    except Exception as exc:
        raise AudioPcmProducerError(
            "failed to inspect PipeWire default sink"
        ) from exc

    for raw_line in result.stdout.splitlines():
        match = re.search(r'node\.name\s*=\s*"([^"\r\n]+)"', raw_line)
        if match is None:
            continue
        node_name = match.group(1)
        if (
            not node_name
            or node_name.strip() != node_name
            or any(character.isspace() for character in node_name)
        ):
            raise AudioPcmProducerError(
                "default sink node.name is malformed"
            )
        return node_name + ".monitor"

    raise AudioPcmProducerError("default sink node.name not found")


def build_capture_command(
    monitor: str,
    profile: AudioPcmProfile,
) -> tuple[str, ...]:
    """Build the exact selected pw-record stdout capture command."""

    if not monitor or any(character.isspace() for character in monitor):
        raise ValueError("AUDIO monitor target must be one qualified node name")
    if not isinstance(profile, AudioPcmProfile):
        raise ValueError("AUDIO profile must be AudioPcmProfile")
    return (
        "pw-record",
        "--target",
        monitor,
        "--rate",
        str(profile.rate_hz),
        "--format",
        "s16",
        "--channels",
        str(profile.channels),
        "-",
    )


class AudioPcmProducer:
    """Own one exact accepted-Wire-session PCM process, spool and credit."""

    def __init__(
        self,
        *,
        session_id: int,
        retirement_timeout_seconds: float,
        profile: AudioPcmProfile | None = None,
        run: Callable[..., subprocess.CompletedProcess[str]] = subprocess.run,
        popen: Callable[..., subprocess.Popen[bytes]] = subprocess.Popen,
    ) -> None:
        if session_id <= 0 or session_id > protocol.UINT32_MAX:
            raise ValueError("AUDIO producer session_id must be nonzero uint32")
        if retirement_timeout_seconds <= 0:
            raise ValueError("AUDIO retirement timeout must be positive")

        self.session_id = session_id
        self.retirement_timeout_seconds = retirement_timeout_seconds
        self.profile = profile or selected_audio_pcm_profile()
        self.credit_bytes = 0

        self._condition = threading.Condition()
        self._buffer = bytearray()
        self._retiring = False
        self._closed = False
        self._eof = False
        self._error: BaseException | None = None

        self._wake_reader, self._wake_writer = socket.socketpair()
        self._wake_reader.setblocking(False)
        self._wake_writer.setblocking(False)

        try:
            monitor = discover_default_sink_monitor(run=run)
            self.monitor = monitor
            self.command = build_capture_command(monitor, self.profile)
            self.process = popen(
                list(self.command),
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
                bufsize=0,
            )
            if self.process.stdout is None:
                raise AudioPcmProducerError(
                    "AUDIO capture process has no stdout pipe"
                )
        except Exception as exc:
            try:
                self._wake_reader.close()
            except OSError:
                pass
            try:
                self._wake_writer.close()
            except OSError:
                pass
            if isinstance(exc, AudioPcmProducerError):
                raise
            raise AudioPcmProducerError("failed to launch AUDIO capture") from exc

        self._thread = threading.Thread(
            target=self._reader,
            name=f"pstvnc-audio-{session_id}",
            daemon=True,
        )
        self._thread.start()

    @property
    def activity_reader(self) -> socket.socket:
        """Expose only the local readiness descriptor to the Wire owner."""

        return self._wake_reader

    @property
    def available_complete_bytes(self) -> int:
        with self._condition:
            return len(self._buffer) - (
                len(self._buffer) % self.profile.frame_bytes
            )

    def _notify_activity(self) -> None:
        try:
            self._wake_writer.send(b"\x01")
        except (BlockingIOError, OSError):
            pass

    def acknowledge_activity(self) -> None:
        """Drain the one-bit local wake edge without touching Wire state."""

        while True:
            try:
                if not self._wake_reader.recv(4096):
                    return
            except BlockingIOError:
                return
            except OSError:
                return

    def _reader(self) -> None:
        try:
            stdout = self.process.stdout
            assert stdout is not None
            while True:
                chunk = stdout.read(protocol.MAX_PAYLOAD_BYTES)
                if not chunk:
                    break
                offset = 0
                while offset < len(chunk):
                    with self._condition:
                        while (
                            len(self._buffer) >= self.profile.channel_window_bytes
                            and not self._retiring
                        ):
                            self._condition.wait()
                        if self._retiring:
                            return
                        room = (
                            self.profile.channel_window_bytes
                            - len(self._buffer)
                        )
                        part = chunk[offset : offset + room]
                        self._buffer.extend(part)
                        offset += len(part)
                        self._condition.notify_all()
                    self._notify_activity()
        except Exception as exc:
            with self._condition:
                if not self._retiring:
                    self._error = exc
                self._condition.notify_all()
            self._notify_activity()
        finally:
            with self._condition:
                self._eof = True
                if not self._retiring and self._error is None:
                    if len(self._buffer) % self.profile.frame_bytes != 0:
                        self._error = AudioPcmProducerError(
                            "AUDIO capture ended with incomplete PCM frame"
                        )
                    else:
                        self._error = AudioPcmProducerError(
                            "AUDIO capture ended unexpectedly"
                        )
                self._condition.notify_all()
            self._notify_activity()

    def check_health(self) -> None:
        """Raise the exact retained producer failure, if any."""

        with self._condition:
            if self._error is not None:
                raise AudioPcmProducerError("AUDIO capture owner failed") from self._error
            if self._closed:
                raise AudioPcmProducerError("AUDIO capture owner is closed")

    def add_credit(self, amount: int) -> None:
        """Admit exact PS2 capacity without exceeding the selected channel window."""

        if amount <= 0 or amount > protocol.UINT32_MAX:
            raise AudioPcmProducerError("AUDIO credit must be nonzero uint32")
        with self._condition:
            if self._closed or self._retiring or self._error is not None:
                raise AudioPcmProducerError("AUDIO credit arrived after terminal state")
            next_credit = self.credit_bytes + amount
            if next_credit > self.profile.channel_window_bytes:
                raise AudioPcmProducerError(
                    "AUDIO credit exceeds selected channel window"
                )
            self.credit_bytes = next_credit
            self._condition.notify_all()
        self._notify_activity()

    def begin_emission(self, maximum_payload: int) -> bytes | None:
        """Take one credit-authorized complete-PCM-frame payload for WireServer."""

        if maximum_payload <= 0:
            raise ValueError("AUDIO emission maximum must be positive")

        notify_more = False
        with self._condition:
            if self._closed or self._retiring:
                raise AudioPcmProducerError("AUDIO owner is not live")
            if self._error is not None:
                raise AudioPcmProducerError("AUDIO capture owner failed") from self._error

            frame_bytes = self.profile.frame_bytes
            available = len(self._buffer) - (len(self._buffer) % frame_bytes)
            if self.credit_bytes < frame_bytes or available < frame_bytes:
                return None

            count = min(
                maximum_payload,
                protocol.MAX_PAYLOAD_BYTES,
                self.credit_bytes,
                available,
            )
            count -= count % frame_bytes
            if count == 0:
                return None

            payload = bytes(self._buffer[:count])
            del self._buffer[:count]
            self.credit_bytes -= count
            self._condition.notify_all()

            remaining = len(self._buffer) - (
                len(self._buffer) % frame_bytes
            )
            notify_more = (
                self.credit_bytes >= frame_bytes
                and remaining >= frame_bytes
            )

        if notify_more:
            self._notify_activity()
        return payload

    def close(self) -> bool:
        """Stop and prove process/thread dormancy; deadline alone never succeeds."""

        with self._condition:
            if self._closed:
                return True
            self._closed = True
            prior_error = self._error
            self._retiring = True
            self._buffer.clear()
            self._condition.notify_all()

        deadline = time.monotonic() + self.retirement_timeout_seconds
        retired = True

        try:
            if self.process.poll() is None:
                self.process.terminate()
        except Exception:
            retired = False

        remaining = deadline - time.monotonic()
        if remaining <= 0:
            retired = False
        elif self.process.poll() is None:
            try:
                self.process.wait(timeout=remaining)
            except subprocess.TimeoutExpired:
                try:
                    self.process.kill()
                except Exception:
                    retired = False
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    retired = False
                else:
                    try:
                        self.process.wait(timeout=remaining)
                    except subprocess.TimeoutExpired:
                        retired = False
            except Exception:
                retired = False

        remaining = max(0.0, deadline - time.monotonic())
        self._thread.join(timeout=remaining)
        if self._thread.is_alive() or self.process.poll() is None:
            retired = False

        try:
            self._wake_reader.close()
        except OSError:
            pass
        try:
            self._wake_writer.close()
        except OSError:
            pass

        return retired and prior_error is None
