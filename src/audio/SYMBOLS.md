# Clean symbols — `src/audio`

DIRECTORY=src/audio
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns synchronous PCM consumption of Transport's logical AUDIO stream and the narrow PS2SDK AUDSRV stream-operation adapter. It owns exact PCM format/volume application, wait-before-play ordering, post-submit accounting, and per-session stream stop. It does not own the sole physical Transport receiver, playback worker lifecycle, startup-reservoir policy, common-clock timing, application orchestration, MPEG/presentation, or resident-service quit.

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
