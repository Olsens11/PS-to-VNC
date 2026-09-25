# Clean symbols — `src/config`

DIRECTORY=src/config
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns pure production CONFIG-wire value decoding/validation, small configuration-text normalization helpers, the selected internal MPEG runtime profile assembled directly from existing owner value types, R26's immutable selected media-clock product profile, and R30's bounded human-readable desired product-action binding representation. R30 parses and canonically formats validated R28 binding values but selects no physical default, performs no persistence, installs no Input runtime state, and routes no product effect. The media-clock profile supplies only Configuration-owned lead/offset values; it does not own synchronization, timer observation, epoch/armed/deadline state, or runtime waiting. This directory also does not own live CONFIG negotiation, Transport receive mechanics, PCM playback, AUDSRV lifecycle, active MPEG geometry/generation meaning, decoder/worker execution, Presentation activation, or Application lifecycle orchestration.

The inventory below covers maintained clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| PSTVNC_CONFIG_PCM_VOLUME_MAX | macro | src/config/profile.c | Config profile | file | Maximum accepted PCM volume percentage. | A002 config/profile |
| PSTVNC_CONFIG_RECEIVER_PRIORITY_MAX | macro | src/config/profile.c | Config profile | file | Highest accepted EE receiver priority. | A002 config/profile |
| PSTVNC_CONFIG_RECEIVER_STACK_ALIGNMENT | macro | src/config/profile.c | Config profile | file | Required receiver-stack byte alignment. | A002 config/profile |
| PSTVNC_CONFIG_RECEIVER_STACK_MIN | macro | src/config/profile.c | Config profile | file | Minimum accepted receiver stack bytes. | A002 config/profile |
| PSTVNC_CONFIG_REQUIRED_MASK | macro | src/config/profile.c | Config profile | file | Requires every clean CONFIG field exactly once. | A002 config/profile |
| pstvnc_config_read_be32 | function | src/config/profile.c | Config profile | file | Reads one big-endian wire u32. | A002 config/profile |
| input | parameter | src/config/profile.c | pstvnc_config_read_be32 | local | Supplies encoded bytes. | A002 config/profile |
| pstvnc_config_signed_bits | function | src/config/profile.c | Config profile | file | Preserves exact u32 bits as signed int32. | A002 config/profile |
| raw | parameter | src/config/profile.c | pstvnc_config_signed_bits | local | Supplies raw signed-offset bits. | A002 config/profile |
| value | variable | src/config/profile.c | pstvnc_config_signed_bits | local | Stores the bit-preserved signed value. | A002 config/profile |
| pstvnc_config_set_field | function | src/config/profile.c | Config profile | file | Maps one field into its narrow profile owner. | A002 config/profile |
| profile | parameter | src/config/profile.c | pstvnc_config_set_field | local | Supplies candidate profile. | A002 config/profile |
| field_id | parameter | src/config/profile.c | pstvnc_config_set_field | local | Supplies clean field identity. | A002 config/profile |
| raw_value | parameter | src/config/profile.c | pstvnc_config_set_field | local | Supplies raw wire value. | A002 config/profile |
| pstvnc_config_transport_valid | function | src/config/profile.c | Config profile | file | Validates A001 Transport representation and relationships. | A002 config/profile |
| profile | parameter | src/config/profile.c | pstvnc_config_transport_valid | local | Supplies candidate profile. | A002 config/profile |
| transport | variable | src/config/profile.c | pstvnc_config_transport_valid | local | References the narrow Transport subprofile. | A002 config/profile |
| pstvnc_config_pcm_valid | function | src/config/profile.c | Config profile | file | Validates PCM activation and format/API values. | A002 config/profile |
| profile | parameter | src/config/profile.c | pstvnc_config_pcm_valid | local | Supplies candidate profile. | A002 config/profile |
| pcm | variable | src/config/profile.c | pstvnc_config_pcm_valid | local | References the narrow PCM subprofile. | A002 config/profile |
| pstvnc_config_profile_valid | function | src/config/profile.c | Config profile | file | Validates composition and owner subprofiles. | A002 config/profile |
| profile | parameter | src/config/profile.c | pstvnc_config_profile_valid | local | Supplies candidate profile. | A002 config/profile |
| pstvnc_config_profile_decode | function | src/config/profile.c | Config profile | public | Atomically decodes one complete production CONFIG payload. | A002 config/profile |
| profile | parameter | src/config/profile.c | pstvnc_config_profile_decode | local | Receives the validated immutable profile. | A002 config/profile |
| payload | parameter | src/config/profile.c | pstvnc_config_profile_decode | local | Supplies CONFIG wire bytes. | A002 config/profile |
| payload_length | parameter | src/config/profile.c | pstvnc_config_profile_decode | local | Supplies CONFIG wire length. | A002 config/profile |
| candidate | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Holds unpublished decoded state. | A002 config/profile |
| seen | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Tracks unique mandatory fields. | A002 config/profile |
| entry_count | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Stores wire entry count. | A002 config/profile |
| expected_length | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Stores exact payload length implied by the header. | A002 config/profile |
| offset | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Walks fixed wire entries. | A002 config/profile |
| field_id | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Stores current entry identity. | A002 config/profile |
| raw_value | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Stores current entry value. | A002 config/profile |
| bit | variable | src/config/profile.c | pstvnc_config_profile_decode | local | Stores current uniqueness-mask bit. | A002 config/profile |
| PSTVNC_CONFIG_PROFILE_H | include guard | src/config/profile.h | Config profile | file | Guards profile declarations. | A002 config/profile |
| PSTVNC_CONFIG_PROFILE_VERSION | macro | src/config/profile.h | Config profile | public | Clean production CONFIG wire version. | A002 config/profile |
| PSTVNC_CONFIG_PROFILE_HEADER_BYTES | macro | src/config/profile.h | Config profile | public | Fixed version/count header bytes. | A002 config/profile |
| PSTVNC_CONFIG_PROFILE_ENTRY_BYTES | macro | src/config/profile.h | Config profile | public | Fixed field/value entry bytes. | A002 config/profile |
| PSTVNC_CONFIG_PROFILE_FIELD_COUNT | macro | src/config/profile.h | Config profile | public | Mandatory clean CONFIG field count. | A002 config/profile |
| pstvnc_config_profile_field | enum | src/config/profile.h | Config profile | public | Defines clean field identities. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_SESSION_ID | enum value | src/config/profile.h | pstvnc_config_profile_field | public | Session identity field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RFB_MODE | enum value | src/config/profile.h | pstvnc_config_profile_field | public | RFB activation field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_AUDIO_MODE | enum value | src/config/profile.h | pstvnc_config_profile_field | public | Audio activation field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_VIDEO_MODE | enum value | src/config/profile.h | pstvnc_config_profile_field | public | Neutral video activation field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_AUDIO_RATE | enum value | src/config/profile.h | pstvnc_config_profile_field | public | PCM sample-rate field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_AUDIO_CHANNELS | enum value | src/config/profile.h | pstvnc_config_profile_field | public | PCM channel-count field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_AUDIO_BITS | enum value | src/config/profile.h | pstvnc_config_profile_field | public | PCM sample-width field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_AUDIO_VOLUME | enum value | src/config/profile.h | pstvnc_config_profile_field | public | PCM volume field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_AUDIO_PRESENTATION_OFFSET_US | enum value | src/config/profile.h | pstvnc_config_profile_field | public | Signed audio epoch-offset field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_VIDEO_PRESENTATION_OFFSET_US | enum value | src/config/profile.h | pstvnc_config_profile_field | public | Signed neutral video epoch-offset field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_MEDIA_EPOCH_LEAD_US | enum value | src/config/profile.h | pstvnc_config_profile_field | public | Common media-epoch lead field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RFB_QUEUE_CAPACITY | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 RFB queue-capacity field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RFB_INITIAL_CREDIT_BYTES | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 initial-credit field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RFB_CREDIT_BATCH_BYTES | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 credit-batch field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RFB_CREDIT_FLUSH_ON_EMPTY | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 flush-on-empty field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RFB_CREDIT_RETURN_ENABLED | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 credit-return enable field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_STACK_SIZE | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 receiver-stack field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_RECEIVER_THREAD_PRIORITY | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 receiver-priority field. | A002 config/profile |
| PSTVNC_CONFIG_FIELD_MAX_DATA_PAYLOAD | enum value | src/config/profile.h | pstvnc_config_profile_field | public | A001 maximum DATA-payload field. | A002 config/profile |
| pstvnc_config_audio_mode | enum | src/config/profile.h | Config profile | public | Defines OFF/PCM audio activation. | A002 config/profile |
| PSTVNC_CONFIG_AUDIO_OFF | enum value | src/config/profile.h | pstvnc_config_audio_mode | public | Audio disabled. | A002 config/profile |
| PSTVNC_CONFIG_AUDIO_PCM | enum value | src/config/profile.h | pstvnc_config_audio_mode | public | PCM audio enabled. | A002 config/profile |
| pstvnc_config_rfb_mode | enum | src/config/profile.h | Config profile | public | Defines RFB activation. | A002 config/profile |
| PSTVNC_CONFIG_RFB_OFF | enum value | src/config/profile.h | pstvnc_config_rfb_mode | public | RFB disabled. | A002 config/profile |
| PSTVNC_CONFIG_RFB_ON | enum value | src/config/profile.h | pstvnc_config_rfb_mode | public | RFB enabled. | A002 config/profile |
| pstvnc_config_video_mode | enum | src/config/profile.h | Config profile | public | Defines neutral video activation only. | A002 config/profile |
| PSTVNC_CONFIG_VIDEO_OFF | enum value | src/config/profile.h | pstvnc_config_video_mode | public | Video disabled. | A002 config/profile |
| PSTVNC_CONFIG_VIDEO_ACTIVE | enum value | src/config/profile.h | pstvnc_config_video_mode | public | Video active without A003 tuning semantics. | A002 config/profile |
| pstvnc_config_composition_profile | structure | src/config/profile.h | Config profile | public | Defines session composition facts. | A002 config/profile |
| session_id | field | src/config/profile.h | pstvnc_config_composition_profile | public | Session correlation identity. | A002 config/profile |
| rfb_mode | field | src/config/profile.h | pstvnc_config_composition_profile | public | RFB activation fact. | A002 config/profile |
| audio_mode | field | src/config/profile.h | pstvnc_config_composition_profile | public | Audio OFF/PCM activation fact. | A002 config/profile |
| video_mode | field | src/config/profile.h | pstvnc_config_composition_profile | public | Neutral video activation fact. | A002 config/profile |
| pstvnc_config_pcm_profile | structure | src/config/profile.h | Config profile | public | Defines PCM format/value authority. | A002 config/profile |
| rate_hz | field | src/config/profile.h | pstvnc_config_pcm_profile | public | PCM sample rate. | A002 config/profile |
| channels | field | src/config/profile.h | pstvnc_config_pcm_profile | public | PCM mono/stereo channel count. | A002 config/profile |
| bits_per_sample | field | src/config/profile.h | pstvnc_config_pcm_profile | public | PCM 8/16-bit sample width. | A002 config/profile |
| volume_percent | field | src/config/profile.h | pstvnc_config_pcm_profile | public | PCM volume percentage. | A002 config/profile |
| pstvnc_config_media_clock_profile | structure | src/config/profile.h | Config profile | public | Defines common-clock profile inputs. | A002 config/profile |
| epoch_lead_us | field | src/config/profile.h | pstvnc_config_media_clock_profile | public | Common media-epoch lead. | A002 config/profile |
| audio_presentation_offset_us | field | src/config/profile.h | pstvnc_config_media_clock_profile | public | Signed audio offset from common epoch. | A002 config/profile |
| video_presentation_offset_us | field | src/config/profile.h | pstvnc_config_media_clock_profile | public | Signed neutral video offset from common epoch. | A002 config/profile |
| pstvnc_config_session_profile | structure | src/config/profile.h | Config profile | public | Aggregates immutable narrow subprofiles after one successful decode. | A002 config/profile |
| composition | field | src/config/profile.h | pstvnc_config_session_profile | public | Composition subprofile. | A002 config/profile |
| transport | field | src/config/profile.h | pstvnc_config_session_profile | public | Existing A001 Transport subprofile. | A002 config/profile |
| pcm | field | src/config/profile.h | pstvnc_config_session_profile | public | PCM subprofile. | A002 config/profile |
| media_clock | field | src/config/profile.h | pstvnc_config_session_profile | public | Common-clock subprofile. | A002 config/profile |
| pstvnc_config_profile_decode | function declaration | src/config/profile.h | Config profile | public | Declares atomic profile decoding/validation. | A002 config/profile |
| profile | prototype parameter | src/config/profile.h | pstvnc_config_profile_decode | prototype | Receives validated profile. | A002 config/profile |
| payload | prototype parameter | src/config/profile.h | pstvnc_config_profile_decode | prototype | Supplies CONFIG bytes. | A002 config/profile |
| payload_length | prototype parameter | src/config/profile.h | pstvnc_config_profile_decode | prototype | Supplies CONFIG byte length. | A002 config/profile |
| pstvnc_config_trim_left | function | src/config/text.c | Config text | public | Skips leading spaces/tabs. | A002 config/profile |
| text | parameter | src/config/text.c | pstvnc_config_trim_left | local | Supplies mutable text. | A002 config/profile |
| pstvnc_config_trim_right | function | src/config/text.c | Config text | public | Removes trailing spaces/tabs in place. | A002 config/profile |
| text | parameter | src/config/text.c | pstvnc_config_trim_right | local | Supplies mutable text. | A002 config/profile |
| len | variable | src/config/text.c | pstvnc_config_trim_right | local | Tracks text length. | A002 config/profile |
| pstvnc_config_parse_int | function | src/config/text.c | Config text | public | Parses bounded normalized decimal int text. | A002 config/profile |
| text | parameter | src/config/text.c | pstvnc_config_parse_int | local | Supplies decimal text. | A002 config/profile |
| allow_negative | parameter | src/config/text.c | pstvnc_config_parse_int | local | Controls negative input. | A002 config/profile |
| require_positive | parameter | src/config/text.c | pstvnc_config_parse_int | local | Requires positive result when set. | A002 config/profile |
| out_value | parameter | src/config/text.c | pstvnc_config_parse_int | local | Receives parsed int. | A002 config/profile |
| p | variable | src/config/text.c | pstvnc_config_parse_int | local | Walks input digits. | A002 config/profile |
| digits | variable | src/config/text.c | pstvnc_config_parse_int | local | Marks first decimal digit. | A002 config/profile |
| endptr | variable | src/config/text.c | pstvnc_config_parse_int | local | Receives strtol end pointer. | A002 config/profile |
| magnitude | variable | src/config/text.c | pstvnc_config_parse_int | local | Accumulates bounded magnitude. | A002 config/profile |
| limit | variable | src/config/text.c | pstvnc_config_parse_int | local | Stores sign-dependent int magnitude limit. | A002 config/profile |
| negative | variable | src/config/text.c | pstvnc_config_parse_int | local | Records negative spelling. | A002 config/profile |
| value | variable | src/config/text.c | pstvnc_config_parse_int | local | Stores final strtol value. | A002 config/profile |
| digit | variable | src/config/text.c | pstvnc_config_parse_int | local | Stores current decimal digit. | A002 config/profile |
| pstvnc_config_parse_bool | function | src/config/text.c | Config text | public | Parses canonical boolean spellings. | A002 config/profile |
| value | parameter | src/config/text.c | pstvnc_config_parse_bool | local | Supplies boolean text. | A002 config/profile |
| out_value | parameter | src/config/text.c | pstvnc_config_parse_bool | local | Receives parsed boolean. | A002 config/profile |
| PSTVNC_CONFIG_TEXT_H | include guard | src/config/text.h | Config text | file | Guards text helper declarations. | A002 config/profile |
| pstvnc_config_trim_left | function declaration | src/config/text.h | Config text | public | Declares left whitespace trimming. | A002 config/profile |
| text | prototype parameter | src/config/text.h | pstvnc_config_trim_left | prototype | Supplies mutable text. | A002 config/profile |
| pstvnc_config_trim_right | function declaration | src/config/text.h | Config text | public | Declares right whitespace trimming. | A002 config/profile |
| text | prototype parameter | src/config/text.h | pstvnc_config_trim_right | prototype | Supplies mutable text. | A002 config/profile |
| pstvnc_config_parse_int | function declaration | src/config/text.h | Config text | public | Declares bounded integer parsing. | A002 config/profile |
| text | prototype parameter | src/config/text.h | pstvnc_config_parse_int | prototype | Supplies decimal text. | A002 config/profile |
| allow_negative | prototype parameter | src/config/text.h | pstvnc_config_parse_int | prototype | Controls negative input. | A002 config/profile |
| require_positive | prototype parameter | src/config/text.h | pstvnc_config_parse_int | prototype | Requires positive result when set. | A002 config/profile |
| out_value | prototype parameter | src/config/text.h | pstvnc_config_parse_int | prototype | Receives parsed int. | A002 config/profile |
| pstvnc_config_parse_bool | function declaration | src/config/text.h | Config text | public | Declares boolean parsing. | A002 config/profile |
| value | prototype parameter | src/config/text.h | pstvnc_config_parse_bool | prototype | Supplies boolean text. | A002 config/profile |
| out_value | prototype parameter | src/config/text.h | pstvnc_config_parse_bool | prototype | Receives parsed boolean. | A002 config/profile |
| pstvnc_config_audio_mode_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_audio_mode_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_composition_profile_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_composition_profile_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_media_clock_profile_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_media_clock_profile_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_pcm_profile_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_pcm_profile_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_profile_field_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_profile_field_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_rfb_mode_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_rfb_mode_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_session_profile_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_session_profile_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_video_mode_t | type | src/config/profile.h | profile interface | public | Defines pstvnc_config_video_mode_t as a current clean-source type. | mechanically reconciled current clean source |
| pstvnc_config_mpeg_runtime_profile_selected | function | src/config/mpeg_runtime_profile.c | mpeg_runtime_profile | file | Defines pstvnc_config_mpeg_runtime_profile_selected as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_config_selected_mpeg_runtime_profile | variable | src/config/mpeg_runtime_profile.c | mpeg_runtime_profile | file | Defines pstvnc_config_selected_mpeg_runtime_profile as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_config_mpeg_runtime_profile_selected | function declaration | src/config/mpeg_runtime_profile.h | mpeg_runtime_profile interface | public | Defines pstvnc_config_mpeg_runtime_profile_selected as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_H | macro | src/config/mpeg_runtime_profile.h | mpeg_runtime_profile interface | public | Defines PSTVNC_CONFIG_MPEG_RUNTIME_PROFILE_H as a current clean-source macro. | mechanically reconciled current clean source |
| pstvnc_config_mpeg_runtime_profile | structure | src/config/mpeg_runtime_profile.h | mpeg_runtime_profile interface | public | Defines pstvnc_config_mpeg_runtime_profile as a current clean-source structure. | mechanically reconciled current clean source |
| pstvnc_config_mpeg_runtime_profile_t | type | src/config/mpeg_runtime_profile.h | mpeg_runtime_profile interface | public | Defines pstvnc_config_mpeg_runtime_profile_t as a current clean-source type. | mechanically reconciled current clean source |
| decoder | field | src/config/mpeg_runtime_profile.h | pstvnc_config_mpeg_runtime_profile | local | Defines decoder as a current clean-source field. | mechanically reconciled current clean source |
| ps2_worker_runtime | field | src/config/mpeg_runtime_profile.h | pstvnc_config_mpeg_runtime_profile | local | Defines ps2_worker_runtime as a current clean-source field. | mechanically reconciled current clean source |
| scheduler | field | src/config/mpeg_runtime_profile.h | pstvnc_config_mpeg_runtime_profile | local | Defines scheduler as a current clean-source field. | mechanically reconciled current clean source |
| transport | field | src/config/mpeg_runtime_profile.h | pstvnc_config_mpeg_runtime_profile | local | Defines transport as a current clean-source field. | mechanically reconciled current clean source |
| worker | field | src/config/mpeg_runtime_profile.h | pstvnc_config_mpeg_runtime_profile | local | Defines worker as a current clean-source field. | mechanically reconciled current clean source |
| pstvnc_config_rfb_runtime_profile_project | function | src/config/rfb_runtime_profile.c | rfb_runtime_profile | file | Defines pstvnc_config_rfb_runtime_profile_project as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_config_rfb_runtime_profile_selected | function | src/config/rfb_runtime_profile.c | rfb_runtime_profile | file | Defines pstvnc_config_rfb_runtime_profile_selected as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_config_rfb_runtime_profile_selected_mode | function | src/config/rfb_runtime_profile.c | rfb_runtime_profile | file | Defines pstvnc_config_rfb_runtime_profile_selected_mode as a current clean-source function. | mechanically reconciled current clean source |
| mode | parameter | src/config/rfb_runtime_profile.c | pstvnc_config_rfb_runtime_profile_project | local | Defines mode as a current clean-source parameter. | mechanically reconciled current clean source |
| transport_config | parameter | src/config/rfb_runtime_profile.c | pstvnc_config_rfb_runtime_profile_project | local | Defines transport_config as a current clean-source parameter. | mechanically reconciled current clean source |
| candidate | variable | src/config/rfb_runtime_profile.c | pstvnc_config_rfb_runtime_profile_project | local | Defines candidate as a current clean-source variable. | mechanically reconciled current clean source |
| transport_config | parameter | src/config/rfb_runtime_profile.c | pstvnc_config_rfb_runtime_profile_selected | local | Defines transport_config as a current clean-source parameter. | mechanically reconciled current clean source |
| pstvnc_config_rfb_runtime_profile_project | function declaration | src/config/rfb_runtime_profile.h | rfb_runtime_profile interface | public | Defines pstvnc_config_rfb_runtime_profile_project as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_config_rfb_runtime_profile_selected | function declaration | src/config/rfb_runtime_profile.h | rfb_runtime_profile interface | public | Defines pstvnc_config_rfb_runtime_profile_selected as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_config_rfb_runtime_profile_selected_mode | function declaration | src/config/rfb_runtime_profile.h | rfb_runtime_profile interface | public | Defines pstvnc_config_rfb_runtime_profile_selected_mode as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_H | macro | src/config/rfb_runtime_profile.h | rfb_runtime_profile interface | public | Defines PSTVNC_CONFIG_RFB_RUNTIME_PROFILE_H as a current clean-source macro. | mechanically reconciled current clean source |
| mode | prototype parameter | src/config/rfb_runtime_profile.h | pstvnc_config_rfb_runtime_profile_project | local | Defines mode as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| transport_config | prototype parameter | src/config/rfb_runtime_profile.h | pstvnc_config_rfb_runtime_profile_project | local | Defines transport_config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| transport_config | prototype parameter | src/config/rfb_runtime_profile.h | pstvnc_config_rfb_runtime_profile_selected | local | Defines transport_config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| pstvnc_config_media_clock_profile_selected | function | src/config/media_clock_profile.c | media_clock_profile | file | Defines pstvnc_config_media_clock_profile_selected as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_config_selected_media_clock_profile | variable | src/config/media_clock_profile.c | media_clock_profile | file | Defines pstvnc_config_selected_media_clock_profile as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_config_media_clock_profile_selected | function declaration | src/config/media_clock_profile.h | media_clock_profile interface | public | Defines pstvnc_config_media_clock_profile_selected as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_MEDIA_CLOCK_PROFILE_H | macro | src/config/media_clock_profile.h | media_clock_profile interface | public | Defines PSTVNC_CONFIG_MEDIA_CLOCK_PROFILE_H as a current clean-source macro. | mechanically reconciled current clean source |
| config_binding_section | enum | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_binding_section as a current clean-source enum. | mechanically reconciled current clean source |
| config_append_text | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_append_text as a current clean-source function. | mechanically reconciled current clean source |
| config_button_token_count | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_button_token_count as a current clean-source function. | mechanically reconciled current clean source |
| config_context_name | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_context_name as a current clean-source function. | mechanically reconciled current clean source |
| config_find_button_token | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_find_button_token as a current clean-source function. | mechanically reconciled current clean source |
| config_line_is_recognized_key_without_assignment | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_line_is_recognized_key_without_assignment as a current clean-source function. | mechanically reconciled current clean source |
| config_parse_binding_value | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_parse_binding_value as a current clean-source function. | mechanically reconciled current clean source |
| config_parse_chord | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_parse_chord as a current clean-source function. | mechanically reconciled current clean source |
| config_parse_context | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_parse_context as a current clean-source function. | mechanically reconciled current clean source |
| config_parse_trigger | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_parse_trigger as a current clean-source function. | mechanically reconciled current clean source |
| config_trigger_name | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_trigger_name as a current clean-source function. | mechanically reconciled current clean source |
| config_validate_binding | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_validate_binding as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_config_product_action_bindings_format | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines pstvnc_config_product_action_bindings_format as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_config_product_action_bindings_parse | function | src/config/product_action_bindings.c | product_action_bindings | file | Defines pstvnc_config_product_action_bindings_parse as a current clean-source function. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_FORMAT_BYTES | macro | src/config/product_action_bindings.c | product_action_bindings | file | Defines PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_FORMAT_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| config_button_token | structure | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_button_token as a current clean-source structure. | mechanically reconciled current clean source |
| config_binding_section_t | type | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_binding_section_t as a current clean-source type. | mechanically reconciled current clean source |
| config_button_token_t | type | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_button_token_t as a current clean-source type. | mechanically reconciled current clean source |
| config_button_tokens | variable | src/config/product_action_bindings.c | product_action_bindings | file | Defines config_button_tokens as a current clean-source variable. | mechanically reconciled current clean source |
| buffer | parameter | src/config/product_action_bindings.c | config_append_text | local | Defines buffer as a current clean-source parameter. | mechanically reconciled current clean source |
| capacity | parameter | src/config/product_action_bindings.c | config_append_text | local | Defines capacity as a current clean-source parameter. | mechanically reconciled current clean source |
| length | parameter | src/config/product_action_bindings.c | config_append_text | local | Defines length as a current clean-source parameter. | mechanically reconciled current clean source |
| text | parameter | src/config/product_action_bindings.c | config_append_text | local | Defines text as a current clean-source parameter. | mechanically reconciled current clean source |
| text_length | variable | src/config/product_action_bindings.c | config_append_text | local | Defines text_length as a current clean-source variable. | mechanically reconciled current clean source |
| CONFIG_BINDING_SECTION_BINDINGS | enum value | src/config/product_action_bindings.c | config_binding_section | local | Defines CONFIG_BINDING_SECTION_BINDINGS as a current clean-source enum value. | mechanically reconciled current clean source |
| CONFIG_BINDING_SECTION_NONE | enum value | src/config/product_action_bindings.c | config_binding_section | local | Defines CONFIG_BINDING_SECTION_NONE as a current clean-source enum value. | mechanically reconciled current clean source |
| CONFIG_BINDING_SECTION_UNKNOWN | enum value | src/config/product_action_bindings.c | config_binding_section | local | Defines CONFIG_BINDING_SECTION_UNKNOWN as a current clean-source enum value. | mechanically reconciled current clean source |
| mask | field | src/config/product_action_bindings.c | config_button_token | local | Defines mask as a current clean-source field. | mechanically reconciled current clean source |
| name | field | src/config/product_action_bindings.c | config_button_token | local | Defines name as a current clean-source field. | mechanically reconciled current clean source |
| context | parameter | src/config/product_action_bindings.c | config_context_name | local | Defines context as a current clean-source parameter. | mechanically reconciled current clean source |
| name | parameter | src/config/product_action_bindings.c | config_find_button_token | local | Defines name as a current clean-source parameter. | mechanically reconciled current clean source |
| i | variable | src/config/product_action_bindings.c | config_find_button_token | local | Defines i as a current clean-source variable. | mechanically reconciled current clean source |
| line | parameter | src/config/product_action_bindings.c | config_line_is_recognized_key_without_assignment | local | Defines line as a current clean-source parameter. | mechanically reconciled current clean source |
| key | variable | src/config/product_action_bindings.c | config_line_is_recognized_key_without_assignment | local | Defines key as a current clean-source variable. | mechanically reconciled current clean source |
| key_length | variable | src/config/product_action_bindings.c | config_line_is_recognized_key_without_assignment | local | Defines key_length as a current clean-source variable. | mechanically reconciled current clean source |
| binding | parameter | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines binding as a current clean-source parameter. | mechanically reconciled current clean source |
| value | parameter | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines value as a current clean-source parameter. | mechanically reconciled current clean source |
| candidate | variable | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines candidate as a current clean-source variable. | mechanically reconciled current clean source |
| chord | variable | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines chord as a current clean-source variable. | mechanically reconciled current clean source |
| context_text | variable | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines context_text as a current clean-source variable. | mechanically reconciled current clean source |
| first_comma | variable | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines first_comma as a current clean-source variable. | mechanically reconciled current clean source |
| second_comma | variable | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines second_comma as a current clean-source variable. | mechanically reconciled current clean source |
| trigger_text | variable | src/config/product_action_bindings.c | config_parse_binding_value | local | Defines trigger_text as a current clean-source variable. | mechanically reconciled current clean source |
| button_mask | parameter | src/config/product_action_bindings.c | config_parse_chord | local | Defines button_mask as a current clean-source parameter. | mechanically reconciled current clean source |
| text | parameter | src/config/product_action_bindings.c | config_parse_chord | local | Defines text as a current clean-source parameter. | mechanically reconciled current clean source |
| button | variable | src/config/product_action_bindings.c | config_parse_chord | local | Defines button as a current clean-source variable. | mechanically reconciled current clean source |
| mask | variable | src/config/product_action_bindings.c | config_parse_chord | local | Defines mask as a current clean-source variable. | mechanically reconciled current clean source |
| separator | variable | src/config/product_action_bindings.c | config_parse_chord | local | Defines separator as a current clean-source variable. | mechanically reconciled current clean source |
| token | variable | src/config/product_action_bindings.c | config_parse_chord | local | Defines token as a current clean-source variable. | mechanically reconciled current clean source |
| context | parameter | src/config/product_action_bindings.c | config_parse_context | local | Defines context as a current clean-source parameter. | mechanically reconciled current clean source |
| text | parameter | src/config/product_action_bindings.c | config_parse_context | local | Defines text as a current clean-source parameter. | mechanically reconciled current clean source |
| text | parameter | src/config/product_action_bindings.c | config_parse_trigger | local | Defines text as a current clean-source parameter. | mechanically reconciled current clean source |
| trigger | parameter | src/config/product_action_bindings.c | config_parse_trigger | local | Defines trigger as a current clean-source parameter. | mechanically reconciled current clean source |
| trigger | parameter | src/config/product_action_bindings.c | config_trigger_name | local | Defines trigger as a current clean-source parameter. | mechanically reconciled current clean source |
| binding | parameter | src/config/product_action_bindings.c | config_validate_binding | local | Defines binding as a current clean-source parameter. | mechanically reconciled current clean source |
| validator | variable | src/config/product_action_bindings.c | config_validate_binding | local | Defines validator as a current clean-source variable. | mechanically reconciled current clean source |
| bindings | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines bindings as a current clean-source parameter. | mechanically reconciled current clean source |
| output | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines output as a current clean-source parameter. | mechanically reconciled current clean source |
| output_capacity | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines output_capacity as a current clean-source parameter. | mechanically reconciled current clean source |
| output_length | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines output_length as a current clean-source parameter. | mechanically reconciled current clean source |
| binding | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines binding as a current clean-source variable. | mechanically reconciled current clean source |
| button | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines button as a current clean-source variable. | mechanically reconciled current clean source |
| context_name | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines context_name as a current clean-source variable. | mechanically reconciled current clean source |
| first_button | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines first_button as a current clean-source variable. | mechanically reconciled current clean source |
| formatted | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines formatted as a current clean-source variable. | mechanically reconciled current clean source |
| i | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines i as a current clean-source variable. | mechanically reconciled current clean source |
| length | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines length as a current clean-source variable. | mechanically reconciled current clean source |
| remaining_mask | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines remaining_mask as a current clean-source variable. | mechanically reconciled current clean source |
| trigger_name | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_format | local | Defines trigger_name as a current clean-source variable. | mechanically reconciled current clean source |
| bindings | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines bindings as a current clean-source parameter. | mechanically reconciled current clean source |
| document | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines document as a current clean-source parameter. | mechanically reconciled current clean source |
| document_length | parameter | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines document_length as a current clean-source parameter. | mechanically reconciled current clean source |
| bindings_section_seen | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines bindings_section_seen as a current clean-source variable. | mechanically reconciled current clean source |
| candidate | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines candidate as a current clean-source variable. | mechanically reconciled current clean source |
| comment | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines comment as a current clean-source variable. | mechanically reconciled current clean source |
| copy_length | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines copy_length as a current clean-source variable. | mechanically reconciled current clean source |
| equals | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines equals as a current clean-source variable. | mechanically reconciled current clean source |
| key | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines key as a current clean-source variable. | mechanically reconciled current clean source |
| line | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines line as a current clean-source variable. | mechanically reconciled current clean source |
| line_length | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines line_length as a current clean-source variable. | mechanically reconciled current clean source |
| offset | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines offset as a current clean-source variable. | mechanically reconciled current clean source |
| recognized_key_seen | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines recognized_key_seen as a current clean-source variable. | mechanically reconciled current clean source |
| section | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines section as a current clean-source variable. | mechanically reconciled current clean source |
| section_name | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines section_name as a current clean-source variable. | mechanically reconciled current clean source |
| text | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines text as a current clean-source variable. | mechanically reconciled current clean source |
| text_length | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines text_length as a current clean-source variable. | mechanically reconciled current clean source |
| value | variable | src/config/product_action_bindings.c | pstvnc_config_product_action_bindings_parse | local | Defines value as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_config_product_action_bindings_format | function declaration | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines pstvnc_config_product_action_bindings_format as a current clean-source function declaration. | mechanically reconciled current clean source |
| pstvnc_config_product_action_bindings_parse | function declaration | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines pstvnc_config_product_action_bindings_parse as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_H | macro | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_H as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_COUNT | macro | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_COUNT as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES | macro | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_DOCUMENT_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_LINE_BYTES | macro | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines PSTVNC_CONFIG_PRODUCT_ACTION_BINDINGS_MAX_LINE_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| pstvnc_config_product_action_bindings | structure | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines pstvnc_config_product_action_bindings as a current clean-source structure. | mechanically reconciled current clean source |
| pstvnc_config_product_action_bindings_t | type | src/config/product_action_bindings.h | product_action_bindings interface | public | Defines pstvnc_config_product_action_bindings_t as a current clean-source type. | mechanically reconciled current clean source |
| binding_count | field | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings | local | Defines binding_count as a current clean-source field. | mechanically reconciled current clean source |
| bindings | field | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings | local | Defines bindings as a current clean-source field. | mechanically reconciled current clean source |
| bindings | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_format | local | Defines bindings as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| output | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_format | local | Defines output as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| output_capacity | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_format | local | Defines output_capacity as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| output_length | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_format | local | Defines output_length as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| bindings | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_parse | local | Defines bindings as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| document | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_parse | local | Defines document as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| document_length | prototype parameter | src/config/product_action_bindings.h | pstvnc_config_product_action_bindings_parse | local | Defines document_length as a current clean-source prototype parameter. | mechanically reconciled current clean source |
