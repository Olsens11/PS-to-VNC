#!/usr/bin/env python3
"""File synopsis:
Owns one exact Raspberry Pi MPEG generation from validated START through proven
RETIRE, including exact capture/suppression preparation, bounded producer
buffering, channel-4 credit bookkeeping, and emission retirement fencing.

This module never reads or writes the PS2-facing socket and never allocates PSTV
sequence numbers. WireConnectionOwner remains the sole physical Wire owner.
SuppressionPreparation owns only the exact generation-specific outer footprint;
it deliberately does not modify the maintained RFB implementation. A later
Application composition may bind that prepared footprint into presentation/RFB
policy without changing this producer lifecycle.

Context: docs/ledge/LEDGE_AUDIT_A003_MPEG_GENERATION.md;
docs/ledge/LEDGE_FOREMAN_STATE.md,
A003-PI-MPEG-CONTROL-PRODUCER-R17.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
import socket
import subprocess
import threading
import time
from typing import Callable

import mpeg_runtime_profile
import wire_protocol as protocol


class MpegGenerationError(RuntimeError):
    """Reject invalid generation state or an unproven producer lifecycle."""


class MpegGenerationState(Enum):
    IDLE = "IDLE"
    PREPARING = "PREPARING"
    LIVE = "LIVE"
    RETIRING = "RETIRING"
    RETIRED_PENDING_COMPLETION = "RETIRED_PENDING_COMPLETION"
    FAILED = "FAILED"


@dataclass(frozen=True)
class MpegRectangle:
    x: int
    y: int
    width: int
    height: int

    @property
    def right(self) -> int:
        return self.x + self.width

    @property
    def bottom(self) -> int:
        return self.y + self.height


@dataclass(frozen=True)
class MpegCapturePlan:
    session_id: int
    generation: int
    desktop_width: int
    desktop_height: int
    capture: MpegRectangle
    suppression: MpegRectangle
    display: str
    command: tuple[str, ...]


@dataclass(frozen=True)
class MpegEmissionLease:
    generation: int
    payload: bytes


class SuppressionPreparation:
    """Own one exact outer footprint without taking RFB transport ownership."""

    def __init__(self) -> None:
        self.generation = 0
        self.rectangle: MpegRectangle | None = None

    def prepare_exact(
        self,
        generation: int,
        rectangle: MpegRectangle,
    ) -> None:
        if generation <= 0 or self.generation != 0 or self.rectangle is not None:
            raise MpegGenerationError("suppression preparation is not idle")
        self.generation = generation
        self.rectangle = rectangle

    def retire_exact(self, generation: int) -> None:
        if (
            generation <= 0
            or self.generation != generation
            or self.rectangle is None
        ):
            raise MpegGenerationError(
                "suppression retirement generation mismatch"
            )
        self.generation = 0
        self.rectangle = None

    def clear_session(self) -> None:
        self.generation = 0
        self.rectangle = None


ProducerFactory = Callable[
    [MpegCapturePlan, int, Callable[[], None]],
    object,
]


def prepare_capture_plan(
    control: protocol.MpegStartControl,
    *,
    active_session_id: int,
    desktop_width: int,
    desktop_height: int,
    display: str,
    profile: mpeg_runtime_profile.MpegProducerProfile,
) -> MpegCapturePlan:
    """Validate exact START geometry and build one START-derived FFmpeg plan."""

    if active_session_id <= 0 or control.session_id != active_session_id:
        raise MpegGenerationError("MPEG START session does not match active Wire")
    if control.generation <= 0:
        raise MpegGenerationError("MPEG START generation must be nonzero")
    if desktop_width <= 0 or desktop_height <= 0:
        raise MpegGenerationError("MPEG START requires active desktop geometry")
    if desktop_width > 0xFFFF or desktop_height > 0xFFFF:
        raise MpegGenerationError("active desktop exceeds RFB coordinate range")
    if not display:
        raise MpegGenerationError("MPEG capture display must be non-empty")

    capture = MpegRectangle(
        control.base_x,
        control.base_y,
        control.base_width,
        control.base_height,
    )
    suppression = MpegRectangle(
        control.suppression_x,
        control.suppression_y,
        control.suppression_width,
        control.suppression_height,
    )

    if (
        capture.width < 16
        or capture.height < 16
        or capture.width % 16 != 0
        or capture.height % 16 != 0
    ):
        raise MpegGenerationError(
            "MPEG base rectangle must be >=16 and 16-pixel aligned"
        )
    if suppression.width <= 0 or suppression.height <= 0:
        raise MpegGenerationError("MPEG suppression rectangle must be non-empty")
    if (
        capture.right > desktop_width
        or capture.bottom > desktop_height
        or suppression.right > desktop_width
        or suppression.bottom > desktop_height
    ):
        raise MpegGenerationError("MPEG geometry exceeds active desktop")
    if (
        suppression.x > capture.x
        or suppression.y > capture.y
        or suppression.right < capture.right
        or suppression.bottom < capture.bottom
    ):
        raise MpegGenerationError(
            "MPEG suppression rectangle must contain the base rectangle"
        )
    if (
        capture.width > profile.max_width
        or capture.height > profile.max_height
    ):
        raise MpegGenerationError("MPEG base rectangle exceeds selected profile")

    frame_rate = f"{profile.fps_numerator}/{profile.fps_denominator}"
    command = (
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "warning",
        "-f",
        "x11grab",
        "-framerate",
        frame_rate,
        "-video_size",
        f"{capture.width}x{capture.height}",
        "-i",
        f"{display}+{capture.x},{capture.y}",
        "-an",
        "-c:v",
        "mpeg2video",
        "-pix_fmt",
        "yuv420p",
        "-r",
        frame_rate,
        "-f",
        "mpeg2video",
        "pipe:1",
    )
    return MpegCapturePlan(
        session_id=control.session_id,
        generation=control.generation,
        desktop_width=desktop_width,
        desktop_height=desktop_height,
        capture=capture,
        suppression=suppression,
        display=display,
        command=command,
    )


class BufferedMpegProducer:
    """Own one FFmpeg process and a condition-bounded stdout buffer."""

    READ_BYTES = 8192

    def __init__(
        self,
        plan: MpegCapturePlan,
        buffer_capacity: int,
        notify_activity: Callable[[], None],
        *,
        popen: Callable[..., subprocess.Popen[bytes]] = subprocess.Popen,
    ) -> None:
        if buffer_capacity <= 0:
            raise ValueError("MPEG producer buffer capacity must be positive")
        self.plan = plan
        self.buffer_capacity = buffer_capacity
        self._notify_activity = notify_activity
        self._condition = threading.Condition()
        self._buffer = bytearray()
        self._retiring = False
        self._eof = False
        self._error: BaseException | None = None

        try:
            self.process = popen(
                list(plan.command),
                stdout=subprocess.PIPE,
                stderr=None,
                bufsize=0,
                start_new_session=True,
            )
        except BaseException as exc:
            raise MpegGenerationError(
                f"failed to launch MPEG generation {plan.generation}"
            ) from exc
        if self.process.stdout is None:
            try:
                self.process.terminate()
            except BaseException:
                pass
            raise MpegGenerationError("MPEG producer has no stdout pipe")

        self._thread = threading.Thread(
            target=self._reader,
            name=f"pstvnc-mpeg-{plan.generation}",
            daemon=True,
        )
        self._thread.start()

    def _reader(self) -> None:
        try:
            while True:
                chunk = self.process.stdout.read(self.READ_BYTES)
                if not chunk:
                    break
                offset = 0
                while offset < len(chunk):
                    with self._condition:
                        while (
                            len(self._buffer) >= self.buffer_capacity
                            and not self._retiring
                        ):
                            self._condition.wait()
                        if self._retiring:
                            return
                        room = self.buffer_capacity - len(self._buffer)
                        part = chunk[offset : offset + room]
                        self._buffer.extend(part)
                        offset += len(part)
                        self._condition.notify_all()
                    self._notify_activity()
        except BaseException as exc:
            with self._condition:
                self._error = exc
                self._condition.notify_all()
            self._notify_activity()
        finally:
            with self._condition:
                self._eof = True
                self._condition.notify_all()
            self._notify_activity()

    @property
    def available(self) -> int:
        with self._condition:
            return len(self._buffer)

    @property
    def unexpectedly_terminal(self) -> bool:
        with self._condition:
            return self._eof and not self._retiring

    @property
    def error(self) -> BaseException | None:
        with self._condition:
            return self._error

    def take(self, maximum: int) -> bytes:
        if maximum <= 0:
            return b""
        with self._condition:
            if self._error is not None:
                raise MpegGenerationError("MPEG producer reader failed") from self._error
            count = min(maximum, len(self._buffer))
            if count == 0:
                if self._eof and not self._retiring:
                    raise MpegGenerationError(
                        "MPEG producer ended before exact retirement"
                    )
                return b""
            payload = bytes(self._buffer[:count])
            del self._buffer[:count]
            self._condition.notify_all()
            return payload

    def retire(self, timeout: float) -> None:
        """Stop and prove process/reader retirement; timeout alone never succeeds."""

        if timeout <= 0:
            raise ValueError("MPEG retirement timeout must be positive")
        with self._condition:
            unexpected_terminal = self._eof and not self._retiring
            prior_error = self._error
            self._retiring = True
            self._buffer.clear()
            self._condition.notify_all()

        deadline = time.monotonic() + timeout
        if self.process.poll() is None:
            self.process.terminate()
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise MpegGenerationError("MPEG producer retirement deadline expired")
        try:
            self.process.wait(timeout=remaining)
        except subprocess.TimeoutExpired:
            self.process.kill()
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise MpegGenerationError(
                    "MPEG producer remained live at retirement deadline"
                )
            try:
                self.process.wait(timeout=remaining)
            except subprocess.TimeoutExpired as exc:
                raise MpegGenerationError(
                    "MPEG producer process did not retire"
                ) from exc

        remaining = max(0.0, deadline - time.monotonic())
        self._thread.join(timeout=remaining)
        if self._thread.is_alive() or self.process.poll() is None:
            raise MpegGenerationError("MPEG producer ownership did not retire")
        if prior_error is not None:
            raise MpegGenerationError(
                "MPEG producer failed before retirement"
            ) from prior_error
        if unexpected_terminal:
            raise MpegGenerationError(
                "MPEG producer terminated before retirement request"
            )

    def force_close(self, timeout: float) -> bool:
        try:
            self.retire(timeout)
            return True
        except (MpegGenerationError, ValueError):
            return False


def create_buffered_producer(
    plan: MpegCapturePlan,
    buffer_capacity: int,
    notify_activity: Callable[[], None],
) -> BufferedMpegProducer:
    return BufferedMpegProducer(plan, buffer_capacity, notify_activity)


class MpegGenerationController:
    """Own exact-generation local state while Wire owns all physical I/O."""

    def __init__(
        self,
        *,
        session_id: int,
        desktop_width: int,
        desktop_height: int,
        display: str,
        retirement_timeout_seconds: float,
        profile: mpeg_runtime_profile.MpegProducerProfile | None = None,
        producer_factory: ProducerFactory = create_buffered_producer,
        suppression: SuppressionPreparation | None = None,
    ) -> None:
        if session_id <= 0 or session_id > protocol.UINT32_MAX:
            raise ValueError("MPEG controller session_id must be nonzero uint32")
        if retirement_timeout_seconds <= 0:
            raise ValueError("MPEG retirement timeout must be positive")
        self.session_id = session_id
        self.desktop_width = desktop_width
        self.desktop_height = desktop_height
        self.display = display
        self.retirement_timeout_seconds = retirement_timeout_seconds
        self.profile = profile or mpeg_runtime_profile.selected_mpeg_producer_profile()
        self.producer_factory = producer_factory
        self.suppression = suppression or SuppressionPreparation()

        self.state = MpegGenerationState.IDLE
        self.generation = 0
        self.highest_generation = 0
        self.plan: MpegCapturePlan | None = None
        self.producer: object | None = None
        self.credit_bytes = 0
        self._in_flight = 0
        self._condition = threading.Condition()
        self._wake_reader, self._wake_writer = socket.socketpair()
        self._wake_reader.setblocking(False)
        self._wake_writer.setblocking(False)
        self._closed = False

    @property
    def activity_reader(self) -> socket.socket:
        return self._wake_reader

    def _notify_activity(self) -> None:
        try:
            self._wake_writer.send(b"\x01")
        except (BlockingIOError, OSError):
            pass

    def acknowledge_activity(self) -> None:
        while True:
            try:
                if not self._wake_reader.recv(4096):
                    return
            except BlockingIOError:
                return
            except OSError:
                return

    def add_credit(self, amount: int) -> None:
        if amount <= 0 or amount > protocol.UINT32_MAX:
            raise MpegGenerationError("MPEG credit must be nonzero uint32")
        with self._condition:
            if self._closed or self.state is MpegGenerationState.FAILED:
                raise MpegGenerationError("MPEG credit arrived after terminal state")
            next_credit = self.credit_bytes + amount
            if next_credit > self.profile.channel_window_bytes:
                raise MpegGenerationError("MPEG credit exceeds selected channel window")
            self.credit_bytes = next_credit
            self._condition.notify_all()
        self._notify_activity()

    def start_exact(self, control: protocol.MpegStartControl) -> MpegCapturePlan:
        with self._condition:
            if self._closed or self.state is not MpegGenerationState.IDLE:
                raise MpegGenerationError("MPEG generation owner is not idle")
            if control.session_id != self.session_id:
                raise MpegGenerationError("MPEG START session mismatch")
            if control.generation <= self.highest_generation:
                raise MpegGenerationError("MPEG START generation is stale")
            self.state = MpegGenerationState.PREPARING

        try:
            plan = prepare_capture_plan(
                control,
                active_session_id=self.session_id,
                desktop_width=self.desktop_width,
                desktop_height=self.desktop_height,
                display=self.display,
                profile=self.profile,
            )
            self.suppression.prepare_exact(
                control.generation,
                plan.suppression,
            )
            producer = self.producer_factory(
                plan,
                self.profile.buffer_capacity,
                self._notify_activity,
            )
            if producer is None:
                raise MpegGenerationError("MPEG producer factory returned no producer")
        except BaseException:
            try:
                if self.suppression.generation == control.generation:
                    self.suppression.retire_exact(control.generation)
            except BaseException:
                pass
            with self._condition:
                self.state = MpegGenerationState.FAILED
                self.highest_generation = max(
                    self.highest_generation,
                    control.generation,
                )
                self._condition.notify_all()
            raise

        with self._condition:
            self.generation = control.generation
            self.highest_generation = control.generation
            self.plan = plan
            self.producer = producer
            self.state = MpegGenerationState.LIVE
            self._condition.notify_all()
        self._notify_activity()
        return plan

    def begin_emission(self, maximum_payload: int) -> MpegEmissionLease | None:
        if maximum_payload <= 0:
            raise ValueError("maximum_payload must be positive")
        with self._condition:
            if self.state is MpegGenerationState.FAILED:
                raise MpegGenerationError("MPEG generation is failed")
            if self.state is not MpegGenerationState.LIVE:
                return None
            producer = self.producer
            generation = self.generation
            if producer is None or generation <= 0:
                raise MpegGenerationError("live MPEG generation lacks producer")
            if self.credit_bytes == 0:
                return None
            budget = min(
                self.credit_bytes,
                maximum_payload,
                protocol.MAX_PAYLOAD_BYTES,
            )
            self._in_flight += 1

        try:
            payload = producer.take(budget)
        except BaseException:
            with self._condition:
                self._in_flight -= 1
                self.state = MpegGenerationState.FAILED
                self._condition.notify_all()
            raise

        if not payload:
            with self._condition:
                self._in_flight -= 1
                self._condition.notify_all()
            return None

        with self._condition:
            if len(payload) > self.credit_bytes:
                self._in_flight -= 1
                self.state = MpegGenerationState.FAILED
                self._condition.notify_all()
                raise MpegGenerationError("MPEG producer exceeded admitted credit")
            self.credit_bytes -= len(payload)
        return MpegEmissionLease(generation=generation, payload=payload)

    def finish_emission(self, lease: MpegEmissionLease) -> None:
        notify_more = False
        with self._condition:
            if (
                lease.generation <= 0
                or lease.generation != self.generation
                or self._in_flight <= 0
            ):
                raise MpegGenerationError("MPEG emission lease mismatch")
            self._in_flight -= 1
            producer = self.producer
            notify_more = (
                self.state is MpegGenerationState.LIVE
                and self.credit_bytes > 0
                and producer is not None
                and int(getattr(producer, "available", 0)) > 0
            )
            self._condition.notify_all()
        if notify_more:
            self._notify_activity()

    def retire_exact(
        self,
        control: protocol.MpegRetireControl,
    ) -> protocol.MpegRetireControl:
        if control.session_id != self.session_id or control.generation <= 0:
            raise MpegGenerationError("MPEG RETIRE session/generation mismatch")

        deadline = time.monotonic() + self.retirement_timeout_seconds
        with self._condition:
            if (
                self.state is not MpegGenerationState.LIVE
                or self.generation != control.generation
                or self.producer is None
                or self.plan is None
            ):
                raise MpegGenerationError("MPEG RETIRE does not match live generation")
            self.state = MpegGenerationState.RETIRING
            while self._in_flight != 0:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    self.state = MpegGenerationState.FAILED
                    raise MpegGenerationError(
                        "MPEG in-flight emission did not retire"
                    )
                self._condition.wait(timeout=remaining)
            producer = self.producer

        remaining = deadline - time.monotonic()
        if remaining <= 0:
            with self._condition:
                self.state = MpegGenerationState.FAILED
            raise MpegGenerationError("MPEG retirement deadline expired")
        try:
            producer.retire(remaining)
            self.suppression.retire_exact(control.generation)
        except BaseException:
            with self._condition:
                self.state = MpegGenerationState.FAILED
                self._condition.notify_all()
            raise

        with self._condition:
            self.producer = None
            self.plan = None
            self.state = MpegGenerationState.RETIRED_PENDING_COMPLETION
            self._condition.notify_all()
        return protocol.MpegRetireControl(
            session_id=control.session_id,
            generation=control.generation,
        )

    def confirm_retire_completion(
        self,
        control: protocol.MpegRetireControl,
    ) -> None:
        with self._condition:
            if (
                self.state
                is not MpegGenerationState.RETIRED_PENDING_COMPLETION
                or control.session_id != self.session_id
                or control.generation != self.generation
                or self._in_flight != 0
                or self.producer is not None
                or self.plan is not None
                or self.suppression.generation != 0
            ):
                raise MpegGenerationError(
                    "MPEG RETIRE completion confirmation is not exact"
                )
            self.generation = 0
            self.state = MpegGenerationState.IDLE
            self._condition.notify_all()

    def close(self) -> bool:
        if self._closed:
            return True
        with self._condition:
            self._closed = True
            producer = self.producer
            generation = self.generation
            self.state = MpegGenerationState.FAILED
            self._condition.notify_all()

        retired = True
        if producer is not None:
            force_close = getattr(producer, "force_close", None)
            if force_close is None:
                retired = False
            else:
                retired = bool(force_close(self.retirement_timeout_seconds))
        if retired and generation > 0 and self.suppression.generation == generation:
            try:
                self.suppression.retire_exact(generation)
            except MpegGenerationError:
                retired = False
        if retired:
            self.producer = None
            self.plan = None
            self.generation = 0
        try:
            self._wake_reader.close()
        except OSError:
            pass
        try:
            self._wake_writer.close()
        except OSError:
            pass
        return retired
