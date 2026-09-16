# Clean symbols — `src/media`

DIRECTORY=src/media
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the session-scoped common media epoch, signed/saturating presentation-deadline math, synchronization contract, and deterministic wait boundary shared by later audio and neutral-video consumers. It does not own PCM/AUDSRV runtime, MPEG/video presentation policy, or platform timer/lock bindings.

The inventory below covers maintained clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| PSTVNC_MEDIA_CLOCK_US_PER_SECOND | macro | src/media/clock.c | Media clock | file | Microseconds per second used for tick conversion. | A002 common media clock |
| pstvnc_media_clock_add_saturating | function | src/media/clock.c | Media clock | file | Adds two tick values with UINT64_MAX saturation. | A002 common media clock |
| left | parameter | src/media/clock.c | pstvnc_media_clock_add_saturating | local | Supplies the left tick value. | A002 common media clock |
| right | parameter | src/media/clock.c | pstvnc_media_clock_add_saturating | local | Supplies the right tick value. | A002 common media clock |
| pstvnc_media_clock_unsigned_us_to_ticks | function | src/media/clock.c | Media clock | file | Converts an unsigned microsecond value into the configured tick domain. | A002 common media clock |
| microseconds | parameter | src/media/clock.c | pstvnc_media_clock_unsigned_us_to_ticks | local | Supplies unsigned microseconds. | A002 common media clock |
| ticks_per_second | parameter | src/media/clock.c | pstvnc_media_clock_unsigned_us_to_ticks | local | Supplies the clock tick rate. | A002 common media clock |
| pstvnc_media_clock_signed_us_to_ticks | function | src/media/clock.c | Media clock | file | Converts an exact signed microsecond offset into signed ticks. | A002 common media clock |
| microseconds | parameter | src/media/clock.c | pstvnc_media_clock_signed_us_to_ticks | local | Supplies signed microseconds. | A002 common media clock |
| ticks_per_second | parameter | src/media/clock.c | pstvnc_media_clock_signed_us_to_ticks | local | Supplies the clock tick rate. | A002 common media clock |
| pstvnc_media_clock_apply_offset | function | src/media/clock.c | Media clock | file | Applies one signed offset to an epoch with underflow/overflow saturation. | A002 common media clock |
| epoch_tick | parameter | src/media/clock.c | pstvnc_media_clock_apply_offset | local | Supplies the common epoch tick. | A002 common media clock |
| offset_us | parameter | src/media/clock.c | pstvnc_media_clock_apply_offset | local | Supplies the exact signed offset. | A002 common media clock |
| ticks_per_second | parameter | src/media/clock.c | pstvnc_media_clock_apply_offset | local | Supplies the clock tick rate. | A002 common media clock |
| signed_offset | variable | src/media/clock.c | pstvnc_media_clock_apply_offset | local | Stores the converted signed tick offset. | A002 common media clock |
| magnitude | variable | src/media/clock.c | pstvnc_media_clock_apply_offset | local | Stores a negative offset magnitude. | A002 common media clock |
| pstvnc_media_clock_lock | function | src/media/clock.c | Media clock | file | Acquires the required session-clock synchronization boundary. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_lock | local | Supplies the session clock. | A002 common media clock |
| pstvnc_media_clock_unlock | function | src/media/clock.c | Media clock | file | Releases the required session-clock synchronization boundary. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_unlock | local | Supplies the session clock. | A002 common media clock |
| pstvnc_media_clock_deadline_with_offset | function | src/media/clock.c | Media clock | file | Computes an armed deadline for one signed stream offset plus extra ticks. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Supplies the session clock. | A002 common media clock |
| offset_us | parameter | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Supplies the signed stream offset. | A002 common media clock |
| additional_ticks | parameter | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Supplies an additional tick delay. | A002 common media clock |
| deadline_tick | parameter | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Receives the saturated deadline. | A002 common media clock |
| epoch_tick | variable | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Stores the observed common epoch. | A002 common media clock |
| deadline | variable | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Stores the saturated candidate deadline. | A002 common media clock |
| result | variable | src/media/clock.c | pstvnc_media_clock_deadline_with_offset | local | Stores operation status. | A002 common media clock |
| pstvnc_media_clock_stop_requested | function | src/media/clock.c | Media clock | file | Samples an optional cancellation observer. | A002 common media clock |
| stop_observer | parameter | src/media/clock.c | pstvnc_media_clock_stop_requested | local | Supplies the optional stop observer. | A002 common media clock |
| pstvnc_media_clock_wait_offset | function | src/media/clock.c | Media clock | file | Waits for arm and deadline while honoring stop and observer failures. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Supplies the session clock. | A002 common media clock |
| offset_us | parameter | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Supplies the signed stream offset. | A002 common media clock |
| additional_ticks | parameter | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Supplies an additional tick delay. | A002 common media clock |
| poll_us | parameter | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Supplies the nonzero delay cadence. | A002 common media clock |
| time_ops | parameter | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Supplies deterministic timer/delay observers. | A002 common media clock |
| stop_observer | parameter | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Supplies optional cancellation observation. | A002 common media clock |
| deadline_tick | variable | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Stores the armed presentation deadline. | A002 common media clock |
| result | variable | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Stores operation status. | A002 common media clock |
| now_tick | variable | src/media/clock.c | pstvnc_media_clock_wait_offset | local | Stores the current observed tick. | A002 common media clock |
| pstvnc_media_clock_init | function | src/media/clock.c | Media clock | public | Initializes one unarmed session media clock. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_init | local | Receives session clock state. | A002 common media clock |
| profile | parameter | src/media/clock.c | pstvnc_media_clock_init | local | Supplies immutable A002 media-clock values. | A002 common media clock |
| ticks_per_second | parameter | src/media/clock.c | pstvnc_media_clock_init | local | Supplies the timer tick rate. | A002 common media clock |
| sync | parameter | src/media/clock.c | pstvnc_media_clock_init | local | Supplies required acquire/release callbacks. | A002 common media clock |
| pstvnc_media_clock_is_armed | function | src/media/clock.c | Media clock | public | Reads armed state under synchronization. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_is_armed | local | Supplies the session clock. | A002 common media clock |
| armed | parameter | src/media/clock.c | pstvnc_media_clock_is_armed | local | Receives synchronized armed state. | A002 common media clock |
| observed_armed | variable | src/media/clock.c | pstvnc_media_clock_is_armed | local | Stores synchronized armed state. | A002 common media clock |
| result | variable | src/media/clock.c | pstvnc_media_clock_is_armed | local | Stores operation status. | A002 common media clock |
| pstvnc_media_clock_epoch | function | src/media/clock.c | Media clock | public | Reads the one-shot epoch or reports unarmed. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_epoch | local | Supplies the session clock. | A002 common media clock |
| epoch_tick | parameter | src/media/clock.c | pstvnc_media_clock_epoch | local | Receives the published epoch tick. | A002 common media clock |
| observed_epoch | variable | src/media/clock.c | pstvnc_media_clock_epoch | local | Stores synchronized epoch state. | A002 common media clock |
| observed_armed | variable | src/media/clock.c | pstvnc_media_clock_epoch | local | Stores synchronized armed state. | A002 common media clock |
| result | variable | src/media/clock.c | pstvnc_media_clock_epoch | local | Stores operation status. | A002 common media clock |
| pstvnc_media_clock_arm | function | src/media/clock.c | Media clock | public | Publishes the common epoch once using configured lead. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_arm | local | Supplies the session clock. | A002 common media clock |
| observed_now_tick | parameter | src/media/clock.c | pstvnc_media_clock_arm | local | Supplies the caller-observed current tick. | A002 common media clock |
| lead_ticks | variable | src/media/clock.c | pstvnc_media_clock_arm | local | Stores configured epoch lead in ticks. | A002 common media clock |
| result | variable | src/media/clock.c | pstvnc_media_clock_arm | local | Stores operation status. | A002 common media clock |
| pstvnc_media_clock_audio_deadline | function | src/media/clock.c | Media clock | public | Derives audio deadline from the shared epoch. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_audio_deadline | local | Supplies the session clock. | A002 common media clock |
| additional_ticks | parameter | src/media/clock.c | pstvnc_media_clock_audio_deadline | local | Supplies extra audio delay ticks. | A002 common media clock |
| deadline_tick | parameter | src/media/clock.c | pstvnc_media_clock_audio_deadline | local | Receives the saturated audio deadline. | A002 common media clock |
| pstvnc_media_clock_video_deadline | function | src/media/clock.c | Media clock | public | Derives neutral-video deadline from the shared epoch. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_video_deadline | local | Supplies the session clock. | A002 common media clock |
| additional_ticks | parameter | src/media/clock.c | pstvnc_media_clock_video_deadline | local | Supplies extra neutral-video delay ticks. | A002 common media clock |
| deadline_tick | parameter | src/media/clock.c | pstvnc_media_clock_video_deadline | local | Receives the saturated neutral-video deadline. | A002 common media clock |
| pstvnc_media_clock_wait_audio | function | src/media/clock.c | Media clock | public | Waits for the audio presentation deadline. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_wait_audio | local | Supplies the session clock. | A002 common media clock |
| additional_ticks | parameter | src/media/clock.c | pstvnc_media_clock_wait_audio | local | Supplies extra audio delay ticks. | A002 common media clock |
| poll_us | parameter | src/media/clock.c | pstvnc_media_clock_wait_audio | local | Supplies the nonzero polling delay. | A002 common media clock |
| time_ops | parameter | src/media/clock.c | pstvnc_media_clock_wait_audio | local | Supplies timer/delay observers. | A002 common media clock |
| stop_observer | parameter | src/media/clock.c | pstvnc_media_clock_wait_audio | local | Supplies optional cancellation observation. | A002 common media clock |
| pstvnc_media_clock_wait_video | function | src/media/clock.c | Media clock | public | Waits for the neutral-video presentation deadline. | A002 common media clock |
| clock | parameter | src/media/clock.c | pstvnc_media_clock_wait_video | local | Supplies the session clock. | A002 common media clock |
| additional_ticks | parameter | src/media/clock.c | pstvnc_media_clock_wait_video | local | Supplies extra neutral-video delay ticks. | A002 common media clock |
| poll_us | parameter | src/media/clock.c | pstvnc_media_clock_wait_video | local | Supplies the nonzero polling delay. | A002 common media clock |
| time_ops | parameter | src/media/clock.c | pstvnc_media_clock_wait_video | local | Supplies timer/delay observers. | A002 common media clock |
| stop_observer | parameter | src/media/clock.c | pstvnc_media_clock_wait_video | local | Supplies optional cancellation observation. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_H | include guard | src/media/clock.h | Media clock | file | Guards common-media-clock declarations. | A002 common media clock |
| pstvnc_media_clock_result | enum | src/media/clock.h | Media clock | public | Defines common-media-clock success/state/error results. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_OK | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Operation completed; wait deadline is ready when returned by wait. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_UNARMED | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Deadline/epoch is unavailable until common epoch publication. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_STOPPED | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Wait ended because session cancellation was requested. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_INVALID | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Arguments or zero polling cadence violate the clock contract. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_SYNC_FAILED | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Required synchronization callback failed. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_TIMER_FAILED | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Timer observation failed. | A002 common media clock |
| PSTVNC_MEDIA_CLOCK_DELAY_FAILED | enum value | src/media/clock.h | pstvnc_media_clock_result | public | Delay operation failed. | A002 common media clock |
| pstvnc_media_clock_result_t | type | src/media/clock.h | clock interface | public | Defines the media-clock result type. | A002 common media clock |
| pstvnc_media_clock_sync | structure | src/media/clock.h | Media clock | public | Defines required acquire/release synchronization. | A002 common media clock |
| lock | field | src/media/clock.h | pstvnc_media_clock_sync | public | Acquire callback for session clock state. | A002 common media clock |
| unlock | field | src/media/clock.h | pstvnc_media_clock_sync | public | Release callback for session clock state. | A002 common media clock |
| context | field | src/media/clock.h | pstvnc_media_clock_sync | public | Synchronization callback context. | A002 common media clock |
| pstvnc_media_clock_sync_t | type | src/media/clock.h | clock interface | public | Defines pstvnc_media_clock_sync_t as a clean media-clock interface type. | A002 common media clock |
| pstvnc_media_clock_time_ops | structure | src/media/clock.h | Media clock | public | Defines injected timer/delay mechanisms for waiting. | A002 common media clock |
| read_ticks | field | src/media/clock.h | pstvnc_media_clock_time_ops | public | Reads current time in the configured tick domain. | A002 common media clock |
| delay_us | field | src/media/clock.h | pstvnc_media_clock_time_ops | public | Performs a bounded microsecond delay. | A002 common media clock |
| context | field | src/media/clock.h | pstvnc_media_clock_time_ops | public | Time callback context. | A002 common media clock |
| pstvnc_media_clock_time_ops_t | type | src/media/clock.h | clock interface | public | Defines pstvnc_media_clock_time_ops_t as a clean media-clock interface type. | A002 common media clock |
| pstvnc_media_clock_stop_observer | structure | src/media/clock.h | Media clock | public | Defines optional wait cancellation observation. | A002 common media clock |
| is_stop_requested | field | src/media/clock.h | pstvnc_media_clock_stop_observer | public | Observes session cancellation intent. | A002 common media clock |
| context | field | src/media/clock.h | pstvnc_media_clock_stop_observer | public | Stop-observer callback context. | A002 common media clock |
| pstvnc_media_clock_stop_observer_t | type | src/media/clock.h | clock interface | public | Defines pstvnc_media_clock_stop_observer_t as a clean media-clock interface type. | A002 common media clock |
| pstvnc_media_clock | structure | src/media/clock.h | Media clock | public | Owns one session-scoped common media epoch and immutable timing values. | A002 common media clock |
| profile | field | src/media/clock.h | pstvnc_media_clock | public | Immutable A002 media-clock profile copy. | A002 common media clock |
| sync | field | src/media/clock.h | pstvnc_media_clock | public | Required synchronization contract copy. | A002 common media clock |
| ticks_per_second | field | src/media/clock.h | pstvnc_media_clock | public | Timer conversion rate. | A002 common media clock |
| epoch_tick | field | src/media/clock.h | pstvnc_media_clock | public | One-shot published common epoch. | A002 common media clock |
| armed | field | src/media/clock.h | pstvnc_media_clock | public | Epoch publication state protected by sync. | A002 common media clock |
| pstvnc_media_clock_t | type | src/media/clock.h | clock interface | public | Defines pstvnc_media_clock_t as a clean media-clock interface type. | A002 common media clock |
| pstvnc_media_clock_init | function declaration | src/media/clock.h | Media clock | public | Declares initializes one unarmed session media clock. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_init | prototype | Receives session clock state. | A002 common media clock |
| profile | prototype parameter | src/media/clock.h | pstvnc_media_clock_init | prototype | Supplies immutable A002 media-clock values. | A002 common media clock |
| ticks_per_second | prototype parameter | src/media/clock.h | pstvnc_media_clock_init | prototype | Supplies the timer tick rate. | A002 common media clock |
| sync | prototype parameter | src/media/clock.h | pstvnc_media_clock_init | prototype | Supplies required acquire/release callbacks. | A002 common media clock |
| pstvnc_media_clock_is_armed | function declaration | src/media/clock.h | Media clock | public | Declares reads armed state under synchronization. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_is_armed | prototype | Supplies the session clock. | A002 common media clock |
| armed | prototype parameter | src/media/clock.h | pstvnc_media_clock_is_armed | prototype | Receives synchronized armed state. | A002 common media clock |
| pstvnc_media_clock_epoch | function declaration | src/media/clock.h | Media clock | public | Declares reads the one-shot epoch or reports unarmed. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_epoch | prototype | Supplies the session clock. | A002 common media clock |
| epoch_tick | prototype parameter | src/media/clock.h | pstvnc_media_clock_epoch | prototype | Receives the published epoch tick. | A002 common media clock |
| pstvnc_media_clock_arm | function declaration | src/media/clock.h | Media clock | public | Declares publishes the common epoch once using configured lead. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_arm | prototype | Supplies the session clock. | A002 common media clock |
| observed_now_tick | prototype parameter | src/media/clock.h | pstvnc_media_clock_arm | prototype | Supplies the caller-observed current tick. | A002 common media clock |
| pstvnc_media_clock_audio_deadline | function declaration | src/media/clock.h | Media clock | public | Declares derives audio deadline from the shared epoch. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_audio_deadline | prototype | Supplies the session clock. | A002 common media clock |
| additional_ticks | prototype parameter | src/media/clock.h | pstvnc_media_clock_audio_deadline | prototype | Supplies extra audio delay ticks. | A002 common media clock |
| deadline_tick | prototype parameter | src/media/clock.h | pstvnc_media_clock_audio_deadline | prototype | Receives the saturated audio deadline. | A002 common media clock |
| pstvnc_media_clock_video_deadline | function declaration | src/media/clock.h | Media clock | public | Declares derives neutral-video deadline from the shared epoch. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_video_deadline | prototype | Supplies the session clock. | A002 common media clock |
| additional_ticks | prototype parameter | src/media/clock.h | pstvnc_media_clock_video_deadline | prototype | Supplies extra neutral-video delay ticks. | A002 common media clock |
| deadline_tick | prototype parameter | src/media/clock.h | pstvnc_media_clock_video_deadline | prototype | Receives the saturated neutral-video deadline. | A002 common media clock |
| pstvnc_media_clock_wait_audio | function declaration | src/media/clock.h | Media clock | public | Declares waits for the audio presentation deadline. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_audio | prototype | Supplies the session clock. | A002 common media clock |
| additional_ticks | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_audio | prototype | Supplies extra audio delay ticks. | A002 common media clock |
| poll_us | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_audio | prototype | Supplies the nonzero polling delay. | A002 common media clock |
| time_ops | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_audio | prototype | Supplies timer/delay observers. | A002 common media clock |
| stop_observer | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_audio | prototype | Supplies optional cancellation observation. | A002 common media clock |
| pstvnc_media_clock_wait_video | function declaration | src/media/clock.h | Media clock | public | Declares waits for the neutral-video presentation deadline. | A002 common media clock |
| clock | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_video | prototype | Supplies the session clock. | A002 common media clock |
| additional_ticks | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_video | prototype | Supplies extra neutral-video delay ticks. | A002 common media clock |
| poll_us | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_video | prototype | Supplies the nonzero polling delay. | A002 common media clock |
| time_ops | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_video | prototype | Supplies timer/delay observers. | A002 common media clock |
| stop_observer | prototype parameter | src/media/clock.h | pstvnc_media_clock_wait_video | prototype | Supplies optional cancellation observation. | A002 common media clock |
