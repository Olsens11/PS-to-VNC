# Clean symbols — `src`

DIRECTORY=src
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the executable entry point and application coordinator. Cross-domain product sequencing remains here; private input, UI, RFB, framebuffer, display, diagnostics, and platform mechanisms live in their earned domain directories.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| remote_pixels | variable | src/app.c | application coordinator | file | Stores the single authoritative CPU-side remote desktop image. | CLEAN_ARCHITECTURE: Desktop framebuffer |
| gs_pixels | variable | src/app.c | application coordinator | file | Stores disposable GS-ready presentation pixels derived from remote authority. | CLEAN_ARCHITECTURE: Display model and presentation |
| send_diagnostic_literal | function | src/app.c | application coordinator | file | Sends one fixed diagnostic record only when optional diagnostics is available. | ISSUE7_MINIMAL_CORE: Clean diagnostics |
| diagnostics_ready | parameter | src/app.c | send_diagnostic_literal | local | States whether the optional diagnostics transport is owned and usable. | application coordinator cleanup |
| text | parameter | src/app.c | send_diagnostic_literal | local | Points to the caller-owned diagnostic bytes to transmit. | clean diagnostics |
| length | parameter | src/app.c | send_diagnostic_literal | local | Gives the exact diagnostic byte count to transmit. | clean diagnostics |
| pstvnc_app_run | function | src/app.c | application coordinator | public | Runs ordered startup, the blocking live loop, and fatal cleanup convergence. | ISSUE7_MINIMAL_CORE: Complete application coordinator |
| PSTVNC_APP_H | include guard | src/app.h | application coordinator interface | file | Prevents repeated inclusion of the coordinator public declaration. | clean source interface |
| pstvnc_app_run | function declaration | src/app.h | application coordinator interface | public | Declares the single top-level clean application lifecycle entry point. | CLEAN_ARCHITECTURE: Application coordinator |
| main | function | src/main.c | process entry | process | Delegates product lifecycle to the coordinator and always converges on OSDSYS. | CLEAN_ARCHITECTURE: Startup lifecycle |
| argc | parameter | src/main.c | main | process | Receives process argument count but is intentionally unused by the clean baseline. | process entry |
| argv | parameter | src/main.c | main | process | Receives process argument vector but is intentionally unused by the clean baseline. | process entry |
| local_overlay_pixels | variable | src/app.c | application coordinator | file | Owns the fixed OSK-sized application staging surface for PS2-local overlay pixels without becoming authoritative remote framebuffer state. | Issue #39: keyboard, OSK and local UI foreground model |
| publish_semantic_keyboard_tap | function | src/app.c | application coordinator | file | Expands one resolved logical keyboard tap into a balanced native down/up sequence and publishes each event through the main-thread-owned RFB session. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | publish_semantic_keyboard_tap | local | Supplies session to publish_semantic_keyboard_tap. | Issue #39: keyboard, OSK and local UI foreground model |
| keyboard_tap | parameter | src/app.c | publish_semantic_keyboard_tap | local | Supplies keyboard tap to publish_semantic_keyboard_tap. | Issue #39: keyboard, OSK and local UI foreground model |
| sequence | variable | src/app.c | publish_semantic_keyboard_tap | local | Stores sequence while publish_semantic_keyboard_tap runs. | Issue #39: keyboard, OSK and local UI foreground model |
| event_index | variable | src/app.c | publish_semantic_keyboard_tap | local | Stores event index while publish_semantic_keyboard_tap runs. | Issue #39: keyboard, OSK and local UI foreground model |
| present_current_application_frame | function | src/app.c | application coordinator | file | Composes the authoritative remote desktop with the current optional local overlay, presents the complete frame, and acknowledges only the exact local generation that reached the platform. | Issue #39: keyboard, OSK and local UI foreground model |
| framebuffer | parameter | src/app.c | present_current_application_frame | local | Supplies framebuffer to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| remote_frame_changed | parameter | src/app.c | present_current_application_frame | local | Supplies remote frame changed to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | present_current_application_frame | local | Supplies local ui to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | present_current_application_frame | local | Supplies osk to present_current_application_frame. | Issue #39: keyboard, OSK and local UI foreground model |
| local_presentation | variable | src/app.c | present_current_application_frame | local | Stores local presentation while present_current_application_frame runs. | Issue #39: keyboard, OSK and local UI foreground model |
| platform_overlay | variable | src/app.c | present_current_application_frame | local | Stores platform overlay while present_current_application_frame runs. | Issue #39: keyboard, OSK and local UI foreground model |
| platform_overlay_ptr | variable | src/app.c | present_current_application_frame | local | Stores platform overlay ptr while present_current_application_frame runs. | Issue #39: keyboard, OSK and local UI foreground model |
| neutralize_published_pointer_for_local_foreground | function | src/app.c | application coordinator | file | Publishes a neutral click mask at the frozen cursor before local foreground ownership so a remotely held click cannot survive the transition. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | neutralize_published_pointer_for_local_foreground | local | Supplies session to neutralize_published_pointer_for_local_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | neutralize_published_pointer_for_local_foreground | local | Supplies published pointer to neutralize_published_pointer_for_local_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| apply_osk_activation_result | function | src/app.c | application coordinator | file | Converts one OSK activation result into local-generation dirtiness and, when produced, one semantic keyboard-tap publication. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | apply_osk_activation_result | local | Supplies session to apply_osk_activation_result. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | apply_osk_activation_result | local | Supplies local ui to apply_osk_activation_result. | Issue #39: keyboard, OSK and local UI foreground model |
| activation | parameter | src/app.c | apply_osk_activation_result | local | Supplies activation to apply_osk_activation_result. | Issue #39: keyboard, OSK and local UI foreground model |
| open_osk_foreground | function | src/app.c | application coordinator | file | Establishes the desktop-to-OSK boundary by suspending mouse interpretation, neutralizing and rebasing remote pointer state, resetting OSK state, and entering local foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | open_osk_foreground | local | Supplies input runtime to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | open_osk_foreground | local | Supplies session to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | open_osk_foreground | local | Supplies published pointer to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | open_osk_foreground | local | Supplies local ui to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | open_osk_foreground | local | Supplies osk to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | open_osk_foreground | local | Supplies mouse interpretation suspended to open_osk_foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| apply_local_controller_action | function | src/app.c | application coordinator | file | Executes one already-routed semantic local action without interpreting physical controller vocabulary or taking controller-device ownership. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | apply_local_controller_action | local | Supplies input runtime to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | apply_local_controller_action | local | Supplies session to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | apply_local_controller_action | local | Supplies published pointer to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | apply_local_controller_action | local | Supplies local ui to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | apply_local_controller_action | local | Supplies osk to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | apply_local_controller_action | local | Supplies mouse interpretation suspended to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| action | parameter | src/app.c | apply_local_controller_action | local | Supplies action to apply_local_controller_action. | Issue #39: keyboard, OSK and local UI foreground model |
| activation | variable | src/app.c | apply_local_controller_action | local | Stores activation while apply_local_controller_action runs. | Issue #39: keyboard, OSK and local UI foreground model |
| service_controller_state | function | src/app.c | application coordinator | file | Routes one trustworthy controller observation through local foreground policy, executes its semantic actions, and completes transition quarantine only after physical release is proven. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | service_controller_state | local | Supplies input runtime to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| session | parameter | src/app.c | service_controller_state | local | Supplies session to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| published_pointer | parameter | src/app.c | service_controller_state | local | Supplies published pointer to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller | parameter | src/app.c | service_controller_state | local | Supplies local controller to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | service_controller_state | local | Supplies local ui to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | service_controller_state | local | Supplies osk to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | service_controller_state | local | Supplies mouse interpretation suspended to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| controller_state | parameter | src/app.c | service_controller_state | local | Supplies controller state to service_controller_state. | Issue #39: keyboard, OSK and local UI foreground model |
| result | variable | src/app.c | service_controller_state | local | Stores result while service_controller_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| action_index | variable | src/app.c | service_controller_state | local | Stores action index while service_controller_state runs. | Issue #39: keyboard, OSK and local UI foreground model |
| resume_desktop_mouse_if_ready | function | src/app.c | application coordinator | file | Resumes desktop mouse interpretation only after desktop ownership is restored, quarantine is clear, and the no-overlay local generation has actually been presented. | Issue #39: keyboard, OSK and local UI foreground model |
| input_runtime | parameter | src/app.c | resume_desktop_mouse_if_ready | local | Supplies input runtime to resume_desktop_mouse_if_ready. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | resume_desktop_mouse_if_ready | local | Supplies local ui to resume_desktop_mouse_if_ready. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | resume_desktop_mouse_if_ready | local | Supplies mouse interpretation suspended to resume_desktop_mouse_if_ready. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller | parameter | src/app.c | service_semantic_input_events | local | Supplies local controller to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui | parameter | src/app.c | service_semantic_input_events | local | Supplies local ui to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/app.c | service_semantic_input_events | local | Supplies osk to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| mouse_interpretation_suspended | parameter | src/app.c | service_semantic_input_events | local | Supplies mouse interpretation suspended to service_semantic_input_events. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_APP_CONTROLLER_PORT | macro | src/app.c | application coordinator | file | Names the direct-libpad controller port owned by the Stage 1 input runtime. | Issue #38 controller input integration |
| PSTVNC_APP_CONTROLLER_SLOT | macro | src/app.c | application coordinator | file | Names the direct-libpad controller slot owned by the Stage 1 input runtime. | Issue #38 controller input integration |
| PSTVNC_APP_IDLE_POLL_DELAY_US | macro | src/app.c | application coordinator | file | Defines the short cooperative delay used when responsive RFB receive polling reports no server message ready. | Issue #38 responsive RFB scheduling |
| app_published_pointer_state | structure | src/app.c | application input publication | file | Groups the pointer coordinates and ordinary semantic click state last proven published to the remote RFB server. | Issue #38 published-state authority |
| app_published_pointer_state_t | type | src/app.c | application input publication | file | Names the application-owned record of successfully published remote pointer authority. | Issue #38 published-state authority |
| cursor_x | field | src/app.c | app_published_pointer_state | file | Stores the horizontal pointer coordinate last successfully published to the RFB server. | Issue #38 published-state authority |
| cursor_y | field | src/app.c | app_published_pointer_state | file | Stores the vertical pointer coordinate last successfully published to the RFB server. | Issue #38 published-state authority |
| click_buttons | field | src/app.c | app_published_pointer_state | file | Stores the ordinary semantic click buttons last successfully published to the RFB server. | Issue #38 published-state authority |
| map_semantic_clicks_to_rfb_buttons | function | src/app.c | application input publication | file | Maps project semantic left and right click state into the native RFB PointerEvent button mask. | Issue #38 RFB input serialization |
| semantic_clicks | parameter | src/app.c | map_semantic_clicks_to_rfb_buttons | local | Supplies the project semantic ordinary-click mask to translate into native RFB button bits. | Issue #38 RFB input serialization |
| rfb_buttons | parameter | src/app.c | map_semantic_clicks_to_rfb_buttons | local | Receives the native RFB ordinary-button mask produced from semantic click state. | Issue #38 RFB input serialization |
| mapped | variable | src/app.c | map_semantic_clicks_to_rfb_buttons | local | Accumulates the native RFB button bits corresponding to the supplied semantic click state. | Issue #38 RFB input serialization |
| map_wheel_direction_to_rfb_button | function | src/app.c | application input publication | file | Maps one semantic wheel direction to the corresponding native RFB transient wheel-button bit. | Issue #38 RFB input serialization |
| direction | parameter | src/app.c | map_wheel_direction_to_rfb_button | local | Supplies the semantic wheel direction whose native RFB button bit is requested. | Issue #38 RFB input serialization |
| rfb_button | parameter | src/app.c | map_wheel_direction_to_rfb_button | local | Receives the native RFB transient wheel-button bit for the supplied semantic direction. | Issue #38 RFB input serialization |
| publish_semantic_mouse_update | function | src/app.c | application input publication | file | Serializes one semantic mouse update as native RFB PointerEvents and advances published pointer authority only after exact send success. | Issue #38 published-state authority |
| session | parameter | src/app.c | publish_semantic_mouse_update | local | Supplies the main-thread-owned synchronized RFB session used to publish the semantic mouse update. | Issue #38 RFB input serialization |
| mouse_update | parameter | src/app.c | publish_semantic_mouse_update | local | Supplies one queued semantic mouse update for native RFB publication. | Issue #38 semantic input pipeline |
| published_pointer | parameter | src/app.c | publish_semantic_mouse_update | local | Supplies mutable application ownership of the last successfully published remote pointer state. | Issue #38 published-state authority |
| ordinary_rfb_buttons | variable | src/app.c | publish_semantic_mouse_update | local | Stores the native RFB ordinary-button mask derived from the semantic click state being published. | Issue #38 RFB input serialization |
| wheel_rfb_button | variable | src/app.c | publish_semantic_mouse_update | local | Stores the transient native RFB wheel-button bit used for one press-and-release wheel pulse. | Issue #38 RFB input serialization |
| service_semantic_input_events | function | src/app.c | application input publication | file | Drains queued controller-derived semantic events on the main thread and publishes each through the synchronized RFB session. | Issue #38 semantic input pipeline |
| input_runtime | parameter | src/app.c | service_semantic_input_events | local | Supplies the controller/input runtime whose bounded semantic queue is consumed by the main thread. | Issue #38 semantic input pipeline |
| session | parameter | src/app.c | service_semantic_input_events | local | Supplies the sole main-thread-owned RFB session through which queued semantic input effects are published. | Issue #38 RFB ownership |
| published_pointer | parameter | src/app.c | service_semantic_input_events | local | Supplies mutable remote pointer publication authority shared across drained semantic input events. | Issue #38 published-state authority |
| event | variable | src/app.c | service_semantic_input_events | local | Holds one semantic input event removed from the bounded runtime queue for main-thread processing. | Issue #38 semantic input pipeline |
| pop_result | variable | src/app.c | service_semantic_input_events | local | Stores the semantic-queue pop result used to distinguish an event, an empty queue, and runtime failure. | Issue #38 semantic input pipeline |
| pstvnc_app_run_with_transport_config | function | src/app.c | app | file | Defines pstvnc_app_run_with_transport_config as a current clean-source function. | mechanically reconciled current clean source |
| transport_config | parameter | src/app.c | pstvnc_app_run_with_transport_config | local | Defines transport_config as a current clean-source parameter. | mechanically reconciled current clean source |
| desktop_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines desktop_ready as a current clean-source variable. | mechanically reconciled current clean source |
| diagnostics_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines diagnostics_ready as a current clean-source variable. | mechanically reconciled current clean source |
| fatal | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines fatal as a current clean-source variable. | mechanically reconciled current clean source |
| framebuffer | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines framebuffer as a current clean-source variable. | mechanically reconciled current clean source |
| graphics_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines graphics_ready as a current clean-source variable. | mechanically reconciled current clean source |
| gs_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines gs_ready as a current clean-source variable. | mechanically reconciled current clean source |
| input_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines input_ready as a current clean-source variable. | mechanically reconciled current clean source |
| input_runtime | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines input_runtime as a current clean-source variable. | mechanically reconciled current clean source |
| input_runtime_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines input_runtime_ready as a current clean-source variable. | mechanically reconciled current clean source |
| local_controller | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines local_controller as a current clean-source variable. | mechanically reconciled current clean source |
| local_ui | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines local_ui as a current clean-source variable. | mechanically reconciled current clean source |
| mouse_interpretation_suspended | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines mouse_interpretation_suspended as a current clean-source variable. | mechanically reconciled current clean source |
| net_ready | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines net_ready as a current clean-source variable. | mechanically reconciled current clean source |
| osk | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines osk as a current clean-source variable. | mechanically reconciled current clean source |
| published_pointer | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines published_pointer as a current clean-source variable. | mechanically reconciled current clean source |
| receive_result | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines receive_result as a current clean-source variable. | mechanically reconciled current clean source |
| session | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines session as a current clean-source variable. | mechanically reconciled current clean source |
| socket_fd | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines socket_fd as a current clean-source variable. | mechanically reconciled current clean source |
| transport_session_active | variable | src/app.c | pstvnc_app_run_with_transport_config | local | Defines transport_session_active as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_app_run_with_transport_config | function declaration | src/app.h | app interface | public | Defines pstvnc_app_run_with_transport_config as a current clean-source function declaration. | mechanically reconciled current clean source |
| transport_config | prototype parameter | src/app.h | pstvnc_app_run_with_transport_config | local | Defines transport_config as a current clean-source prototype parameter. | mechanically reconciled current clean source |
