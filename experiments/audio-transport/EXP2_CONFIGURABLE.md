# Configurable Audio Transport EXP2

## Purpose

EXP2 is a laboratory transport ELF.

The PS2 supplies mechanisms and accepts one complete operating profile from the
Pi for each connection. Rebuilding the ELF should not be required merely to
change a queue size, startup reservoir, credit batching rule, consumer chunk,
thread priority/stack size, socket buffer request, or related experimental
parameter.

## Allocation policy

RFB and audio queue capacities are real runtime allocations.

There is deliberately no project-defined queue-capacity ceiling below the
platform's actual ability to satisfy malloc(). If an aggressive profile cannot
be allocated, that failed experiment is evidence about the real operating
envelope.

The audio consumer buffer and experimental EE thread stacks are also runtime
allocated from the requested profile sizes.

## Handshake

1. PS2 connects to TCP 5902.
2. PS2 sends HELLO advertising dynamic CONFIG support.
3. Pi sends one complete CONFIG profile.
4. PS2 validates structural consistency.
5. PS2 attempts the requested allocations and socket options.
6. PS2 returns the exact CONFIG payload as an ACK only after transport-side
   allocation succeeds.
7. PS2 grants initial credits equal to the actual configured queue capacities.
8. Normal multiplexed RFB/audio traffic begins.

## Profile 0

Profile 0 reproduces the September 6, 2026 W512 stability pass:

- RFB effective queue: 32768 bytes
- audio effective queue: 524288 bytes
- RFB/audio credit batch: 4096 bytes
- flush pending credit when queue empties: enabled
- startup mode: explicit delay
- startup delay: 3000000 us
- audio chunk: 4096 bytes
- RFB/audio idle delay: 1000 us
- receiver priority: 63
- audio priority: 65
- both experimental stacks: 16384 bytes
- 48000 Hz, 16-bit, stereo
- volume: 100
- DATA payload ceiling: 8192
- socket buffer overrides: disabled
- allocation order: RFB then audio

Pi-side scheduler/window/host-buffer values are also unchanged from the W512
pass.

Only after Profile 0 reproduces the known-good behavior should subsequent
profiles change experimental parameters.
