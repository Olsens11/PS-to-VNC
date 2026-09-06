#!/usr/bin/env python3
"""
File synopsis:
    Configurable Audio Transport EXP2 Pi policy/diagnostic wrapper.

The existing mux_daemon.py and mux_daemon_instrumented.py remain unchanged.

This wrapper adds a mandatory connection-scoped CONFIG exchange:

    PS2 -> HELLO
    Pi  -> complete CONFIG profile
    PS2 -> exact byte-for-byte CONFIG ACK
    PS2 -> initial receiver credits
    normal mux traffic begins

Queue capacities in CONFIG are actual requested PS2 malloc() sizes. There is no
Pi-side arbitrary capacity ceiling.
"""

from __future__ import annotations

import json
import socket
import struct
import subprocess
import sys
import threading
import time
from pathlib import Path

THIS_DIRECTORY = Path(__file__).resolve().parent

if str(THIS_DIRECTORY) not in sys.path:
    sys.path.insert(0, str(THIS_DIRECTORY))

import mux_daemon as base
import mux_daemon_instrumented as diag


BASE_VALIDATE_CONFIG = base.validate_config

CONFIG_VERSION = 1
CONFIG_ACK_FLAG = 0x01

CAP_CONFIG_DYNAMIC = 1 << 4

PS2_FIELD_IDS = {
    "rfb_queue_capacity": 1,
    "audio_queue_capacity": 2,

    "rfb_credit_batch_bytes": 3,
    "audio_credit_batch_bytes": 4,

    "rfb_credit_flush_on_empty": 5,
    "audio_credit_flush_on_empty": 6,

    "audio_start_mode": 7,
    "audio_start_target_bytes": 8,
    "audio_start_delay_us": 9,

    "audio_chunk_bytes": 10,
    "audio_idle_delay_us": 11,
    "rfb_empty_delay_us": 12,

    "audio_thread_priority": 13,
    "audio_thread_stack_size": 14,

    "receiver_thread_priority": 15,
    "receiver_thread_stack_size": 16,

    "audio_rate": 17,
    "audio_channels": 18,
    "audio_bits": 19,
    "audio_volume": 20,

    "max_data_payload": 21,

    "socket_receive_buffer_bytes": 22,
    "socket_send_buffer_bytes": 23,

    "queue_allocation_order": 24,
}


def build_ps2_config_payload(
    config: dict,
) -> bytes:
    profile = config["ps2_profile"]

    payload = bytearray(
        struct.pack(
            ">II",
            CONFIG_VERSION,
            int(profile["profile_id"]),
        )
    )

    for name, field_id in sorted(
        PS2_FIELD_IDS.items(),
        key=lambda pair: pair[1],
    ):
        payload.extend(
            struct.pack(
                ">II",
                field_id,
                int(profile[name]),
            )
        )

    return bytes(payload)


def validate_configurable(
    config: dict,
) -> None:
    profile = config["ps2_profile"]

    expected_keys = (
        {"profile_id"}
        | set(PS2_FIELD_IDS)
    )

    actual_keys = set(profile)

    if actual_keys != expected_keys:
        raise ValueError(
            "ps2_profile key mismatch "
            f"missing="
            f"{sorted(expected_keys - actual_keys)} "
            f"unknown="
            f"{sorted(actual_keys - expected_keys)}"
        )

    bits = int(
        profile["audio_bits"]
    )

    channels = int(
        profile["audio_channels"]
    )

    if bits not in (8, 16):
        raise ValueError(
            "audio_bits must be 8 or 16"
        )

    if channels not in (1, 2):
        raise ValueError(
            "audio_channels must be 1 or 2"
        )

    frame_bytes = (
        channels * (bits // 8)
    )

    old_frame_bytes = (
        base.AUDIO_FRAME_BYTES
    )

    try:
        base.AUDIO_FRAME_BYTES = (
            frame_bytes
        )

        BASE_VALIDATE_CONFIG(
            config
        )

    finally:
        base.AUDIO_FRAME_BYTES = (
            old_frame_bytes
        )

    if (
        int(config["audio_rate"])
        != int(profile["audio_rate"])
    ):
        raise ValueError(
            "Pi and PS2 audio_rate differ"
        )

    if (
        int(config["audio_channels"])
        != channels
    ):
        raise ValueError(
            "Pi and PS2 audio_channels differ"
        )

    if (
        int(config["audio_sample_bytes"])
        != bits // 8
    ):
        raise ValueError(
            "Pi audio_sample_bytes differs "
            "from PS2 audio_bits"
        )

    if (
        int(config["rfb_policy_window"])
        > int(profile["rfb_queue_capacity"])
    ):
        raise ValueError(
            "rfb_policy_window exceeds "
            "requested PS2 queue"
        )

    if (
        int(config["audio_policy_window"])
        > int(profile["audio_queue_capacity"])
    ):
        raise ValueError(
            "audio_policy_window exceeds "
            "requested PS2 queue"
        )

    if (
        int(config["rfb_quantum"])
        > int(profile["max_data_payload"])
    ):
        raise ValueError(
            "rfb_quantum exceeds "
            "PS2 max_data_payload"
        )

    if (
        int(config["audio_quantum"])
        > int(profile["max_data_payload"])
    ):
        raise ValueError(
            "audio_quantum exceeds "
            "PS2 max_data_payload"
        )

    if (
        int(config["audio_quantum"])
        % frame_bytes
    ):
        raise ValueError(
            "audio_quantum breaks "
            "configured frame alignment"
        )

    if (
        bits != 16
        and bool(
            config["mirror_sink_volume"]
        )
    ):
        raise ValueError(
            "volume mirroring currently "
            "requires 16-bit PCM"
        )


def start_audio_capture_configurable(
    rate: int,
    channels: int,
    bits: int,
) -> subprocess.Popen[bytes]:
    target = (
        base.discover_default_sink_monitor()
    )

    sample_format = {
        8: "s8",
        16: "s16",
    }[bits]

    command = [
        "pw-record",
        "--target",
        target,
        "--rate",
        str(rate),
        "--format",
        sample_format,
        "--channels",
        str(channels),
        "-",
    ]

    print(
        "AUDIO_CAPTURE_TARGET="
        + target,
        flush=True,
    )

    print(
        "AUDIO_CAPTURE_FORMAT="
        + json.dumps(
            {
                "rate": rate,
                "channels": channels,
                "bits": bits,
                "pipewire_format":
                    sample_format,
            },
            sort_keys=True,
        ),
        flush=True,
    )

    return subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        bufsize=0,
    )


class ConfigurableInstrumentedMuxSession(
    diag.InstrumentedMuxSession
):
    """
    Existing diagnostic mux plus mandatory CONFIG handshake.
    """

    def __init__(
        self,
        ps2_socket: socket.socket,
        config: dict,
    ) -> None:
        self.ps2_profile = (
            config["ps2_profile"]
        )

        bits = int(
            self.ps2_profile[
                "audio_bits"
            ]
        )

        channels = int(
            self.ps2_profile[
                "audio_channels"
            ]
        )

        self.audio_frame_bytes = (
            channels * (bits // 8)
        )

        # The base experiment handles one PS2 session at a time, so these
        # process-global format seams can safely describe the active profile.
        base.AUDIO_FRAME_BYTES = (
            self.audio_frame_bytes
        )

        def capture(
            rate: int,
            runtime_channels: int,
        ):
            return (
                start_audio_capture_configurable(
                    rate,
                    runtime_channels,
                    bits,
                )
            )

        base.start_audio_capture = (
            capture
        )

        super().__init__(
            ps2_socket,
            config,
        )

        self.ps2_config_payload = (
            build_ps2_config_payload(
                config
            )
        )

        self.ps2_config_sent = False

        self.ps2_config_ack_event = (
            threading.Event()
        )

    def ps2_reader(self) -> None:
        assert self.rfb is not None

        while not self.stop_event.is_set():
            frame = base.receive_frame(
                self.ps2
            )

            if (
                frame.sequence
                != self.expected_rx_sequence
            ):
                raise base.ProtocolError(
                    "PS2 sequence mismatch: "
                    f"expected="
                    f"{self.expected_rx_sequence} "
                    f"actual={frame.sequence}"
                )

            self.expected_rx_sequence += 1

            if frame.kind == base.FRAME_HELLO:
                if (
                    frame.channel
                    != base.CHANNEL_CONTROL
                    or len(frame.payload)
                    != base.HELLO_PAYLOAD_SIZE
                ):
                    raise base.ProtocolError(
                        "invalid PS2 HELLO"
                    )

                (
                    capabilities,
                    max_payload,
                    rfb_capacity,
                    audio_capacity,
                    audio_rate,
                    audio_frame_bytes,
                ) = struct.unpack(
                    ">IIIIII",
                    frame.payload,
                )

                self.ps2_hello = {
                    "capabilities":
                        capabilities,
                    "max_payload":
                        max_payload,
                    "rfb_capacity":
                        rfb_capacity,
                    "audio_capacity":
                        audio_capacity,
                    "audio_rate":
                        audio_rate,
                    "audio_frame_bytes":
                        audio_frame_bytes,
                }

                print(
                    "PS2_HELLO="
                    + json.dumps(
                        self.ps2_hello,
                        sort_keys=True,
                    ),
                    flush=True,
                )

                if (
                    capabilities
                    & CAP_CONFIG_DYNAMIC
                ) == 0:
                    raise base.ProtocolError(
                        "wrong PS2 ELF: dynamic "
                        "CONFIG capability absent"
                    )

                if (
                    rfb_capacity != 0
                    or audio_capacity != 0
                    or audio_rate != 0
                    or audio_frame_bytes != 0
                ):
                    raise base.ProtocolError(
                        "dynamic EXP2 HELLO "
                        "sent nonzero profile fields"
                    )

                if self.ps2_config_sent:
                    raise base.ProtocolError(
                        "duplicate PS2 HELLO"
                    )

                if (
                    int(
                        self.ps2_profile[
                            "max_data_payload"
                        ]
                    )
                    > max_payload
                ):
                    raise base.ProtocolError(
                        "profile DATA ceiling "
                        "exceeds wire maximum"
                    )

                self.send_frame(
                    base.FRAME_CONFIG,
                    base.CHANNEL_CONTROL,
                    self.ps2_config_payload,
                )

                self.ps2_config_sent = True

                print(
                    "PS2_CONFIG_SENT="
                    + json.dumps(
                        self.ps2_profile,
                        sort_keys=True,
                    ),
                    flush=True,
                )

            elif frame.kind == base.FRAME_CONFIG:
                if (
                    frame.channel
                    != base.CHANNEL_CONTROL
                    or frame.flags
                    != CONFIG_ACK_FLAG
                    or frame.payload
                    != self.ps2_config_payload
                ):
                    raise base.ProtocolError(
                        "invalid PS2 CONFIG ACK"
                    )

                self.ps2_config_ack_event.set()

                print(
                    "PS2_CONFIG_ACK="
                    + json.dumps(
                        self.ps2_profile,
                        sort_keys=True,
                    ),
                    flush=True,
                )

                print(
                    "PS2_RUNTIME_ALLOCATION_ACCEPTED="
                    + json.dumps(
                        {
                            "profile_id":
                                int(
                                    self.ps2_profile[
                                        "profile_id"
                                    ]
                                ),
                            "rfb_queue_bytes":
                                int(
                                    self.ps2_profile[
                                        "rfb_queue_capacity"
                                    ]
                                ),
                            "audio_queue_bytes":
                                int(
                                    self.ps2_profile[
                                        "audio_queue_capacity"
                                    ]
                                ),
                            "receiver_stack_bytes":
                                int(
                                    self.ps2_profile[
                                        "receiver_thread_stack_size"
                                    ]
                                ),
                        },
                        sort_keys=True,
                    ),
                    flush=True,
                )

            elif frame.kind == base.FRAME_CREDIT:
                if (
                    frame.channel
                    not in self.channels
                    or len(frame.payload)
                    != base.CREDIT_PAYLOAD_SIZE
                ):
                    raise base.ProtocolError(
                        "invalid CREDIT frame"
                    )

                amount = struct.unpack(
                    ">I",
                    frame.payload,
                )[0]

                with self.condition:
                    self.channels[
                        frame.channel
                    ].add_credit(amount)

                    self.condition.notify_all()

            elif frame.kind == base.FRAME_DATA:
                if (
                    frame.channel
                    != base.CHANNEL_RFB
                ):
                    raise base.ProtocolError(
                        "PS2 DATA on "
                        "unsupported channel"
                    )

                self.rfb.sendall(
                    frame.payload
                )

            elif (
                frame.kind
                == base.FRAME_TELEMETRY
            ):
                if (
                    frame.channel
                    != base.CHANNEL_TELEMETRY
                ):
                    raise base.ProtocolError(
                        "telemetry on wrong channel"
                    )

                telemetry = (
                    base.parse_telemetry(
                        frame.payload
                    )
                )

                self.last_telemetry = (
                    telemetry
                )

                print(
                    "PS2_TELEMETRY="
                    + json.dumps(
                        telemetry,
                        sort_keys=True,
                    ),
                    flush=True,
                )

            elif frame.kind == base.FRAME_ERROR:
                if len(frame.payload) == 4:
                    error_code = struct.unpack(
                        ">I",
                        frame.payload,
                    )[0]

                    raise base.ProtocolError(
                        "PS2 reported transport "
                        f"error code={error_code}"
                    )

                raise base.ProtocolError(
                    "PS2 reported malformed "
                    "transport error"
                )

            else:
                raise base.ProtocolError(
                    "unexpected PS2 frame kind "
                    f"{frame.kind}"
                )

    def scheduler(self) -> None:
        telemetry_interval = float(
            self.config[
                "telemetry_interval_seconds"
            ]
        )

        # Do not send normal mux traffic until the PS2 has successfully
        # allocated and ACKed the complete operating profile.
        if not self.ps2_config_ack_event.wait(
            timeout=10.0
        ):
            raise base.ProtocolError(
                "timed out waiting for "
                "PS2 CONFIG ACK"
            )

        next_telemetry = (
            time.monotonic()
            + telemetry_interval
        )

        self.send_frame(
            base.FRAME_HELLO,
            base.CHANNEL_CONTROL,
        )

        while not self.stop_event.is_set():
            now = time.monotonic()

            if (
                telemetry_interval > 0
                and now >= next_telemetry
            ):
                self.send_frame(
                    base.FRAME_HEARTBEAT,
                    base.CHANNEL_CONTROL,
                )

                next_telemetry = (
                    now + telemetry_interval
                )

            sent_something = False

            for _ in range(
                max(
                    1,
                    sum(
                        max(
                            1,
                            channel.weight,
                        )
                        for channel
                        in self.channels.values()
                    ),
                )
            ):
                channel = (
                    self.chooser.choose()
                )

                with self.condition:
                    budget = (
                        channel.send_budget()
                    )

                    if budget == 0:
                        continue

                    payload = (
                        channel.take_for_send(
                            budget
                        )
                    )

                    self.condition.notify_all()

                self.send_frame(
                    base.FRAME_DATA,
                    channel.channel_id,
                    payload,
                )

                sent_something = True
                break

            if sent_something:
                continue

            with self.condition:
                timeout = 0.05

                if telemetry_interval > 0:
                    timeout = min(
                        timeout,
                        max(
                            0.001,
                            next_telemetry
                            - time.monotonic(),
                        ),
                    )

                self.condition.wait(
                    timeout
                )


def main() -> int:
    base.validate_config = (
        validate_configurable
    )

    base.MuxSession = (
        ConfigurableInstrumentedMuxSession
    )

    return base.main()


if __name__ == "__main__":
    raise SystemExit(main())
