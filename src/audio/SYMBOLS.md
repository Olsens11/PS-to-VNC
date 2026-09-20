# Clean symbols — `src/audio`

DIRECTORY=src/audio
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns synchronous PCM consumption of Transport's logical AUDIO stream, the resident PS2SDK AUDSRV stream adapter, and the session-scoped audio worker/resource lifecycle. It owns explicit caller-supplied worker/timing authority, non-consuming startup-reservoir readiness, common-clock audio-deadline gating, finish/join/reclaim fencing, exact PCM application, wait-before-play ordering, truthful accounting, and per-session stream stop. It does not own the physical Transport receiver or abort/close orchestration, common-epoch arming, application orchestration, MPEG/video presentation, receive-poison repair, hardware qualification, or resident-service quit.

The inventory below covers maintained clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| pstvnc_audio_audsrv_initialize | function | src/audio/audsrv_service.c | AUDSRV adapter | file | Attaches to or initializes the resident AUDSRV service. | A002 PCM playback core |
| context | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_initialize | local | Supplies the unused generic service context. | A002 PCM playback core |
| pstvnc_audio_audsrv_set_format | function | src/audio/audsrv_service.c | AUDSRV adapter | file | Maps exact validated PCM format values to PS2SDK AUDSRV. | A002 PCM playback core |
| context | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_format | local | Supplies the unused generic service context. | A002 PCM playback core |
| rate_hz | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_format | local | Supplies exact PCM sample rate. | A002 PCM playback core |
| channels | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_format | local | Supplies exact PCM channel count. | A002 PCM playback core |
| bits_per_sample | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_format | local | Supplies exact PCM sample width. | A002 PCM playback core |
| format | variable | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_format | local | Holds the exact PS2SDK AUDSRV format request. | A002 PCM playback core |
| pstvnc_audio_audsrv_set_volume | function | src/audio/audsrv_service.c | AUDSRV adapter | file | Applies exact validated PCM volume percentage to AUDSRV. | A002 PCM playback core |
| context | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_volume | local | Supplies the unused generic service context. | A002 PCM playback core |
| volume_percent | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_set_volume | local | Supplies exact volume percentage. | A002 PCM playback core |
| pstvnc_audio_audsrv_wait_audio | function | src/audio/audsrv_service.c | AUDSRV adapter | file | Waits for AUDSRV capacity for one representable nonzero chunk. | A002 PCM playback core |
| context | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_wait_audio | local | Supplies the unused generic service context. | A002 PCM playback core |
| byte_count | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_wait_audio | local | Supplies the chunk size to reserve. | A002 PCM playback core |
| pstvnc_audio_audsrv_play_audio | function | src/audio/audsrv_service.c | AUDSRV adapter | file | Submits one chunk and accepts success only when every requested byte is reported sent. | A002 PCM playback core |
| context | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_play_audio | local | Supplies the unused generic service context. | A002 PCM playback core |
| bytes | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_play_audio | local | Supplies PCM bytes for submission. | A002 PCM playback core |
| byte_count | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_play_audio | local | Supplies the PCM chunk size. | A002 PCM playback core |
| submitted | variable | src/audio/audsrv_service.c | pstvnc_audio_audsrv_play_audio | local | Stores AUDSRV's reported submitted-byte count. | A002 PCM playback core |
| pstvnc_audio_audsrv_stop_audio | function | src/audio/audsrv_service.c | AUDSRV adapter | file | Stops the current session stream without quitting the resident service. | A002 PCM playback core |
| context | parameter | src/audio/audsrv_service.c | pstvnc_audio_audsrv_stop_audio | local | Supplies the unused generic service context. | A002 PCM playback core |
| pstvnc_audio_audsrv_service_ops | function | src/audio/audsrv_service.c | AUDSRV adapter | public | Returns the concrete resident AUDSRV operation table. | A002 PCM playback core |
| service | variable | src/audio/audsrv_service.c | pstvnc_audio_audsrv_service_ops | local | Holds the concrete operation table being returned. | A002 PCM playback core |
| PSTVNC_AUDIO_AUDSRV_SERVICE_H | include guard | src/audio/audsrv_service.h | AUDSRV adapter | file | Guards concrete AUDSRV adapter declarations. | A002 PCM playback core |
| pstvnc_audio_audsrv_service_ops | function declaration | src/audio/audsrv_service.h | AUDSRV adapter | public | Declares the concrete resident AUDSRV operation-table provider. | A002 PCM playback core |
| pstvnc_audio_playback_profile_valid | function | src/audio/playback.c | PCM playback | file | Revalidates exact PCM profile values at the hardware-facing playback boundary. | A002 PCM playback core |
| profile | parameter | src/audio/playback.c | pstvnc_audio_playback_profile_valid | local | Supplies immutable PCM authority. | A002 PCM playback core |
| pstvnc_audio_playback_service_valid | function | src/audio/playback.c | PCM playback | file | Verifies every required resident-service operation is present. | A002 PCM playback core |
| service | parameter | src/audio/playback.c | pstvnc_audio_playback_service_valid | local | Supplies the resident-service operation table. | A002 PCM playback core |
| pstvnc_audio_playback_transport_result | function | src/audio/playback.c | PCM playback | file | Maps terminal Transport results into distinct playback outcomes. | A002 PCM playback core |
| result | parameter | src/audio/playback.c | pstvnc_audio_playback_transport_result | local | Supplies a terminal Transport result. | A002 PCM playback core |
| pstvnc_audio_playback_finish | function | src/audio/playback.c | PCM playback | file | Stops the current stream once and preserves an earlier primary result over cleanup failure. | A002 PCM playback core |
| service | parameter | src/audio/playback.c | pstvnc_audio_playback_finish | local | Supplies the resident-service operation table. | A002 PCM playback core |
| result | parameter | src/audio/playback.c | pstvnc_audio_playback_finish | local | Supplies the primary playback outcome. | A002 PCM playback core |
| report | parameter | src/audio/playback.c | pstvnc_audio_playback_finish | local | Receives cleanup evidence. | A002 PCM playback core |
| pstvnc_audio_playback_run | function | src/audio/playback.c | PCM playback | public | Runs synchronous Transport-to-AUDSRV PCM consumption until normal exhaustion or a terminal outcome. | A002 PCM playback core |
| profile | parameter | src/audio/playback.c | pstvnc_audio_playback_run | local | Supplies immutable exact PCM format and volume values. | A002 PCM playback core |
| buffer | parameter | src/audio/playback.c | pstvnc_audio_playback_run | local | Supplies caller-owned chunk storage. | A002 PCM playback core |
| buffer_capacity | parameter | src/audio/playback.c | pstvnc_audio_playback_run | local | Supplies explicit maximum bytes consumed/submitted per iteration. | A002 PCM playback core |
| service | parameter | src/audio/playback.c | pstvnc_audio_playback_run | local | Supplies resident-service operations. | A002 PCM playback core |
| report | parameter | src/audio/playback.c | pstvnc_audio_playback_run | local | Receives truthful submission and cleanup facts. | A002 PCM playback core |
| result | variable | src/audio/playback.c | pstvnc_audio_playback_run | local | Stores the first meaningful playback outcome. | A002 PCM playback core |
| activity_sequence | variable | src/audio/playback.c | pstvnc_audio_playback_run | local | Holds Transport AUDIO activity generation for non-spinning waits. | A002 PCM playback core |
| read_count | variable | src/audio/playback.c | pstvnc_audio_playback_run | local | Holds bytes dequeued from Transport for the current chunk. | A002 PCM playback core |
| transport_result | variable | src/audio/playback.c | pstvnc_audio_playback_run | local | Holds the current public Transport AUDIO operation result. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_H | include guard | src/audio/playback.h | PCM playback | file | Guards synchronous PCM playback declarations. | A002 PCM playback core |
| pstvnc_audio_playback_result | enum | src/audio/playback.h | PCM playback | public | Defines normal, terminal, setup, playback, cleanup, and accounting outcomes. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_COMPLETE | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Producer completion plus empty Transport queue retired normally. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_STOPPED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Transport reported explicit stop. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_CLOSED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Transport receiver closed before normal producer exhaustion. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_INVALID | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Playback arguments/profile/service boundary are invalid. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_TRANSPORT_INVALID | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Public Transport AUDIO seam rejected live use as invalid. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_TRANSPORT_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Transport reported a terminal failure or impossible successful-read shape. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_SERVICE_INIT_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Resident AUDSRV service initialization/attachment failed. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_SERVICE_FORMAT_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Exact PCM format application failed. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_SERVICE_VOLUME_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Exact PCM volume application failed. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_SERVICE_WAIT_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | AUDSRV capacity wait failed before submission. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_SERVICE_PLAY_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | AUDSRV did not successfully submit the complete requested chunk. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_SERVICE_STOP_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Stream-stop cleanup failed after otherwise normal completion. | A002 PCM playback core |
| PSTVNC_AUDIO_PLAYBACK_ACCOUNTING_FAILED | enum value | src/audio/playback.h | pstvnc_audio_playback_result | public | Truthful submitted-byte/chunk accounting would overflow. | A002 PCM playback core |
| pstvnc_audio_playback_result_t | type | src/audio/playback.h | PCM playback interface | public | Defines the synchronous PCM playback result type. | A002 PCM playback core |
| pstvnc_audio_service_ops | structure | src/audio/playback.h | PCM playback | public | Defines the narrow injected resident-service operation boundary. | A002 PCM playback core |
| initialize | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Initializes or attaches to the resident service. | A002 PCM playback core |
| set_format | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Applies exact PCM rate/channel/sample-width values. | A002 PCM playback core |
| set_volume | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Applies exact PCM volume percentage. | A002 PCM playback core |
| wait_audio | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Waits until a nonzero chunk can be accepted. | A002 PCM playback core |
| play_audio | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Submits one nonzero PCM chunk. | A002 PCM playback core |
| stop_audio | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Stops the current stream without service quit. | A002 PCM playback core |
| context | field | src/audio/playback.h | pstvnc_audio_service_ops | public | Opaque service implementation context. | A002 PCM playback core |
| pstvnc_audio_service_ops_t | type | src/audio/playback.h | PCM playback interface | public | Defines the injected resident-service operation-table type. | A002 PCM playback core |
| pstvnc_audio_playback_report | structure | src/audio/playback.h | PCM playback | public | Records successful submission and cleanup facts for one synchronous run. | A002 PCM playback core |
| submitted_bytes | field | src/audio/playback.h | pstvnc_audio_playback_report | public | Counts bytes only after successful complete playback submission. | A002 PCM playback core |
| submitted_chunks | field | src/audio/playback.h | pstvnc_audio_playback_report | public | Counts chunks only after successful complete playback submission. | A002 PCM playback core |
| cleanup_attempted | field | src/audio/playback.h | pstvnc_audio_playback_report | public | Records whether current-stream stop was attempted. | A002 PCM playback core |
| cleanup_failed | field | src/audio/playback.h | pstvnc_audio_playback_report | public | Records cleanup failure without overwriting an earlier meaningful result. | A002 PCM playback core |
| pstvnc_audio_playback_report_t | type | src/audio/playback.h | PCM playback interface | public | Defines the synchronous playback report type. | A002 PCM playback core |
| pstvnc_audio_playback_run | function declaration | src/audio/playback.h | PCM playback | public | Declares synchronous Transport-to-AUDSRV PCM consumption. | A002 PCM playback core |
| profile | prototype parameter | src/audio/playback.h | pstvnc_audio_playback_run | prototype | Supplies immutable exact PCM authority. | A002 PCM playback core |
| buffer | prototype parameter | src/audio/playback.h | pstvnc_audio_playback_run | prototype | Supplies caller-owned playback chunk storage. | A002 PCM playback core |
| buffer_capacity | prototype parameter | src/audio/playback.h | pstvnc_audio_playback_run | prototype | Supplies explicit chunk capacity without inventing tuning defaults. | A002 PCM playback core |
| service | prototype parameter | src/audio/playback.h | pstvnc_audio_playback_run | prototype | Supplies injected resident-service operations. | A002 PCM playback core |
| report | prototype parameter | src/audio/playback.h | pstvnc_audio_playback_run | prototype | Receives truthful submitted-byte/chunk and cleanup facts. | A002 PCM playback core |
| pstvnc_audio_session_values_valid | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_values_valid as a current clean-source function. | mechanically reconciled current clean source |
| values | parameter | src/audio/session.c | pstvnc_audio_session_values_valid | local | Defines values as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_memory_ops_valid | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_memory_ops_valid as a current clean-source function. | mechanically reconciled current clean source |
| memory_ops | parameter | src/audio/session.c | pstvnc_audio_session_memory_ops_valid | local | Defines memory_ops as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_thread_ops_valid | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_thread_ops_valid as a current clean-source function. | mechanically reconciled current clean source |
| thread_ops | parameter | src/audio/session.c | pstvnc_audio_session_thread_ops_valid | local | Defines thread_ops as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_sync_valid | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_sync_valid as a current clean-source function. | mechanically reconciled current clean source |
| sync | parameter | src/audio/session.c | pstvnc_audio_session_sync_valid | local | Defines sync as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_time_ops_valid | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_time_ops_valid as a current clean-source function. | mechanically reconciled current clean source |
| time_ops | parameter | src/audio/session.c | pstvnc_audio_session_time_ops_valid | local | Defines time_ops as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_free_owned_memory | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_free_owned_memory as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_free_owned_memory | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_observe_stop | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_observe_stop as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_observe_stop | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| requested | parameter | src/audio/session.c | pstvnc_audio_session_observe_stop | local | Defines requested as a current clean-source parameter. | mechanically reconciled current clean source |
| observed | variable | src/audio/session.c | pstvnc_audio_session_observe_stop | local | Defines observed as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_audio_session_clock_stop_requested | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_clock_stop_requested as a current clean-source function. | mechanically reconciled current clean source |
| context | parameter | src/audio/session.c | pstvnc_audio_session_clock_stop_requested | local | Defines context as a current clean-source parameter. | mechanically reconciled current clean source |
| session | variable | src/audio/session.c | pstvnc_audio_session_clock_stop_requested | local | Defines session as a current clean-source variable. | mechanically reconciled current clean source |
| requested | variable | src/audio/session.c | pstvnc_audio_session_clock_stop_requested | local | Defines requested as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_audio_session_set_transport_outcome | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_set_transport_outcome as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_set_transport_outcome | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| result | parameter | src/audio/session.c | pstvnc_audio_session_set_transport_outcome | local | Defines result as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_wait_reservoir | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_wait_reservoir as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| activity_sequence | variable | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines activity_sequence as a current clean-source variable. | mechanically reconciled current clean source |
| transport_result | variable | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines transport_result as a current clean-source variable. | mechanically reconciled current clean source |
| available_count | variable | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines available_count as a current clean-source variable. | mechanically reconciled current clean source |
| producer_done | variable | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines producer_done as a current clean-source variable. | mechanically reconciled current clean source |
| stop_requested | variable | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines stop_requested as a current clean-source variable. | mechanically reconciled current clean source |
| observed_sequence | variable | src/audio/session.c | pstvnc_audio_session_wait_reservoir | local | Defines observed_sequence as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_audio_session_worker | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_worker as a current clean-source function. | mechanically reconciled current clean source |
| argument | parameter | src/audio/session.c | pstvnc_audio_session_worker | local | Defines argument as a current clean-source parameter. | mechanically reconciled current clean source |
| session | variable | src/audio/session.c | pstvnc_audio_session_worker | local | Defines session as a current clean-source variable. | mechanically reconciled current clean source |
| stop_observer | variable | src/audio/session.c | pstvnc_audio_session_worker | local | Defines stop_observer as a current clean-source variable. | mechanically reconciled current clean source |
| reservoir_ready | variable | src/audio/session.c | pstvnc_audio_session_worker | local | Defines reservoir_ready as a current clean-source variable. | mechanically reconciled current clean source |
| clock_result | variable | src/audio/session.c | pstvnc_audio_session_worker | local | Defines clock_result as a current clean-source variable. | mechanically reconciled current clean source |
| stop_requested | variable | src/audio/session.c | pstvnc_audio_session_worker | local | Defines stop_requested as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_audio_session_start | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_start as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| values | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines values as a current clean-source parameter. | mechanically reconciled current clean source |
| pcm_profile | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines pcm_profile as a current clean-source parameter. | mechanically reconciled current clean source |
| service | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines service as a current clean-source parameter. | mechanically reconciled current clean source |
| clock | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines clock as a current clean-source parameter. | mechanically reconciled current clean source |
| time_ops | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines time_ops as a current clean-source parameter. | mechanically reconciled current clean source |
| memory_ops | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines memory_ops as a current clean-source parameter. | mechanically reconciled current clean source |
| thread_ops | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines thread_ops as a current clean-source parameter. | mechanically reconciled current clean source |
| sync | parameter | src/audio/session.c | pstvnc_audio_session_start | local | Defines sync as a current clean-source parameter. | mechanically reconciled current clean source |
| create_result | variable | src/audio/session.c | pstvnc_audio_session_start | local | Defines create_result as a current clean-source variable. | mechanically reconciled current clean source |
| start_result | variable | src/audio/session.c | pstvnc_audio_session_start | local | Defines start_result as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_audio_session_request_stop | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_request_stop as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_request_stop | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_join | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_join as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_join | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_release | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_release as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_release | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_outcome | function | src/audio/session.c | session | file | Defines pstvnc_audio_session_outcome as a current clean-source function. | mechanically reconciled current clean source |
| session | parameter | src/audio/session.c | pstvnc_audio_session_outcome | local | Defines session as a current clean-source parameter. | mechanically reconciled current clean source |
| outcome | parameter | src/audio/session.c | pstvnc_audio_session_outcome | local | Defines outcome as a current clean-source parameter. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_H | macro | src/audio/session.h | session interface | public | Defines PSTVNC_AUDIO_SESSION_H as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_STACK_ALIGNMENT | macro | src/audio/session.h | session interface | public | Defines PSTVNC_AUDIO_SESSION_STACK_ALIGNMENT as a current clean-source macro. | mechanically reconciled current clean source |
| pstvnc_audio_session_values | structure | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_values as a current clean-source structure. | mechanically reconciled current clean source |
| worker_stack_bytes | field | src/audio/session.h | pstvnc_audio_session_values | local | Defines worker_stack_bytes as a current clean-source field. | mechanically reconciled current clean source |
| worker_priority | field | src/audio/session.h | pstvnc_audio_session_values | local | Defines worker_priority as a current clean-source field. | mechanically reconciled current clean source |
| playback_buffer_capacity | field | src/audio/session.h | pstvnc_audio_session_values | local | Defines playback_buffer_capacity as a current clean-source field. | mechanically reconciled current clean source |
| startup_reservoir_bytes | field | src/audio/session.h | pstvnc_audio_session_values | local | Defines startup_reservoir_bytes as a current clean-source field. | mechanically reconciled current clean source |
| reservoir_poll_us | field | src/audio/session.h | pstvnc_audio_session_values | local | Defines reservoir_poll_us as a current clean-source field. | mechanically reconciled current clean source |
| clock_poll_us | field | src/audio/session.h | pstvnc_audio_session_values | local | Defines clock_poll_us as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_audio_session_values_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_values_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_memory_ops | structure | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_memory_ops as a current clean-source structure. | mechanically reconciled current clean source |
| allocate | field | src/audio/session.h | pstvnc_audio_session_memory_ops | local | Defines allocate as a current clean-source field. | mechanically reconciled current clean source |
| release | field | src/audio/session.h | pstvnc_audio_session_memory_ops | local | Defines release as a current clean-source field. | mechanically reconciled current clean source |
| context | field | src/audio/session.h | pstvnc_audio_session_memory_ops | local | Defines context as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_audio_session_memory_ops_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_memory_ops_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_thread_entry_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_thread_entry_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_thread_ops | structure | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_thread_ops as a current clean-source structure. | mechanically reconciled current clean source |
| create | field | src/audio/session.h | pstvnc_audio_session_thread_ops | local | Defines create as a current clean-source field. | mechanically reconciled current clean source |
| start | field | src/audio/session.h | pstvnc_audio_session_thread_ops | local | Defines start as a current clean-source field. | mechanically reconciled current clean source |
| join | field | src/audio/session.h | pstvnc_audio_session_thread_ops | local | Defines join as a current clean-source field. | mechanically reconciled current clean source |
| destroy | field | src/audio/session.h | pstvnc_audio_session_thread_ops | local | Defines destroy as a current clean-source field. | mechanically reconciled current clean source |
| context | field | src/audio/session.h | pstvnc_audio_session_thread_ops | local | Defines context as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_audio_session_thread_ops_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_thread_ops_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_sync | structure | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_sync as a current clean-source structure. | mechanically reconciled current clean source |
| lock | field | src/audio/session.h | pstvnc_audio_session_sync | local | Defines lock as a current clean-source field. | mechanically reconciled current clean source |
| unlock | field | src/audio/session.h | pstvnc_audio_session_sync | local | Defines unlock as a current clean-source field. | mechanically reconciled current clean source |
| context | field | src/audio/session.h | pstvnc_audio_session_sync | local | Defines context as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_audio_session_sync_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_sync_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_result | enum | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_result as a current clean-source enum. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OK | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_OK as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_INVALID | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_INVALID as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_BUFFER_ALLOCATION_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_BUFFER_ALLOCATION_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_STACK_ALLOCATION_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_STACK_ALLOCATION_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_THREAD_CREATE_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_THREAD_CREATE_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_THREAD_START_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_THREAD_START_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_THREAD_JOIN_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_THREAD_JOIN_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_THREAD_DESTROY_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_THREAD_DESTROY_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_WORKER_LIVE | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_WORKER_LIVE as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_SYNC_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_SYNC_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_NOT_FINISHED | enum value | src/audio/session.h | pstvnc_audio_session_result | local | Defines PSTVNC_AUDIO_SESSION_NOT_FINISHED as a current clean-source enum value. | mechanically reconciled current clean source |
| pstvnc_audio_session_result_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_result_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_outcome_kind | enum | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_outcome_kind as a current clean-source enum. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_NONE | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_NONE as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_EMPTY as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_STOPPED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_TRANSPORT | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_TRANSPORT as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_CLOCK | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_CLOCK as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_PLAYBACK as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_RESERVOIR_DELAY_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_RESERVOIR_DELAY_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED | enum value | src/audio/session.h | pstvnc_audio_session_outcome_kind | local | Defines PSTVNC_AUDIO_SESSION_OUTCOME_SYNC_FAILED as a current clean-source enum value. | mechanically reconciled current clean source |
| pstvnc_audio_session_outcome_kind_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_outcome_kind_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_outcome | structure | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_outcome as a current clean-source structure. | mechanically reconciled current clean source |
| kind | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines kind as a current clean-source field. | mechanically reconciled current clean source |
| transport_result | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines transport_result as a current clean-source field. | mechanically reconciled current clean source |
| clock_result | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines clock_result as a current clean-source field. | mechanically reconciled current clean source |
| playback_result | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines playback_result as a current clean-source field. | mechanically reconciled current clean source |
| playback_report | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines playback_report as a current clean-source field. | mechanically reconciled current clean source |
| reservoir_bytes | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines reservoir_bytes as a current clean-source field. | mechanically reconciled current clean source |
| producer_done | field | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines producer_done as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_audio_session_outcome_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_outcome_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session | structure | src/audio/session.h | session interface | public | Defines pstvnc_audio_session as a current clean-source structure. | mechanically reconciled current clean source |
| values | field | src/audio/session.h | pstvnc_audio_session | local | Defines values as a current clean-source field. | mechanically reconciled current clean source |
| pcm_profile | field | src/audio/session.h | pstvnc_audio_session | local | Defines pcm_profile as a current clean-source field. | mechanically reconciled current clean source |
| service | field | src/audio/session.h | pstvnc_audio_session | local | Defines service as a current clean-source field. | mechanically reconciled current clean source |
| clock | field | src/audio/session.h | pstvnc_audio_session | local | Defines clock as a current clean-source field. | mechanically reconciled current clean source |
| time_ops | field | src/audio/session.h | pstvnc_audio_session | local | Defines time_ops as a current clean-source field. | mechanically reconciled current clean source |
| memory_ops | field | src/audio/session.h | pstvnc_audio_session | local | Defines memory_ops as a current clean-source field. | mechanically reconciled current clean source |
| thread_ops | field | src/audio/session.h | pstvnc_audio_session | local | Defines thread_ops as a current clean-source field. | mechanically reconciled current clean source |
| sync | field | src/audio/session.h | pstvnc_audio_session | local | Defines sync as a current clean-source field. | mechanically reconciled current clean source |
| playback_buffer | field | src/audio/session.h | pstvnc_audio_session | local | Defines playback_buffer as a current clean-source field. | mechanically reconciled current clean source |
| worker_stack | field | src/audio/session.h | pstvnc_audio_session | local | Defines worker_stack as a current clean-source field. | mechanically reconciled current clean source |
| thread_id | field | src/audio/session.h | pstvnc_audio_session | local | Defines thread_id as a current clean-source field. | mechanically reconciled current clean source |
| initialized | field | src/audio/session.h | pstvnc_audio_session | local | Defines initialized as a current clean-source field. | mechanically reconciled current clean source |
| thread_created | field | src/audio/session.h | pstvnc_audio_session | local | Defines thread_created as a current clean-source field. | mechanically reconciled current clean source |
| thread_started | field | src/audio/session.h | pstvnc_audio_session | local | Defines thread_started as a current clean-source field. | mechanically reconciled current clean source |
| thread_joined | field | src/audio/session.h | pstvnc_audio_session | local | Defines thread_joined as a current clean-source field. | mechanically reconciled current clean source |
| thread_destroyed | field | src/audio/session.h | pstvnc_audio_session | local | Defines thread_destroyed as a current clean-source field. | mechanically reconciled current clean source |
| stop_requested | field | src/audio/session.h | pstvnc_audio_session | local | Defines stop_requested as a current clean-source field. | mechanically reconciled current clean source |
| worker_finished | field | src/audio/session.h | pstvnc_audio_session | local | Defines worker_finished as a current clean-source field. | mechanically reconciled current clean source |
| worker_sync_failed | field | src/audio/session.h | pstvnc_audio_session | local | Defines worker_sync_failed as a current clean-source field. | mechanically reconciled current clean source |
| outcome | field | src/audio/session.h | pstvnc_audio_session | local | Defines outcome as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_audio_session_t | type | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_audio_session_start | function declaration | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_start as a current clean-source function declaration. | mechanically reconciled current clean source |
| session | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines session as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| values | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines values as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pcm_profile | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines pcm_profile as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| service | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines service as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| clock | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines clock as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| time_ops | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines time_ops as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| memory_ops | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines memory_ops as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| thread_ops | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines thread_ops as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| sync | prototype parameter | src/audio/session.h | pstvnc_audio_session_start | local | Defines sync as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_request_stop | function declaration | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_request_stop as a current clean-source function declaration. | mechanically reconciled current clean source |
| session | prototype parameter | src/audio/session.h | pstvnc_audio_session_request_stop | local | Defines session as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_join | function declaration | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_join as a current clean-source function declaration. | mechanically reconciled current clean source |
| session | prototype parameter | src/audio/session.h | pstvnc_audio_session_join | local | Defines session as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_release | function declaration | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_release as a current clean-source function declaration. | mechanically reconciled current clean source |
| session | prototype parameter | src/audio/session.h | pstvnc_audio_session_release | local | Defines session as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_audio_session_outcome | function declaration | src/audio/session.h | session interface | public | Defines pstvnc_audio_session_outcome as a current clean-source function declaration. | mechanically reconciled current clean source |
| session | prototype parameter | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines session as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| outcome | prototype parameter | src/audio/session.h | pstvnc_audio_session_outcome | local | Defines outcome as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| transport_access | parameter | src/audio/playback.c | pstvnc_audio_playback_run | local | Carries the running instance's opaque Transport access ticket. | Q12 audio module-lifecycle integration |
| transport_access | prototype parameter | src/audio/playback.h | pstvnc_audio_playback_run | prototype | Carries the running instance's opaque Transport access ticket. | Q12 audio module-lifecycle integration |
| transport_result | variable | src/audio/session.c | pstvnc_audio_session_start | local | Stores the Transport result produced while acquiring or using session-scoped access. | Q12 audio module-lifecycle integration |
| transport_access | field | src/audio/session.h | pstvnc_audio_session | public | Stores this running module instance's opaque Transport access ticket. | Q12 audio module-lifecycle integration |
| PSTVNC_AUDIO_SESSION_TRANSPORT_UNAVAILABLE | enum value | src/audio/session.h | pstvnc_audio_session_result | public | Reports that audio startup could not acquire valid Transport access. | Q12 audio module-lifecycle integration |
