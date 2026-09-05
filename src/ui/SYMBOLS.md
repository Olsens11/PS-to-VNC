# Clean symbols — `src/ui`

DIRECTORY=src/ui
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns PS2-local foreground behavior: controller-to-local semantic routing, foreground and quarantine state, the on-screen keyboard model and renderer, and platform-neutral local presentation description.

The inventory below covers clean-generation symbols defined directly in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| PSTVNC_LOCAL_CONTROLLER_OWNED_BUTTONS | macro | src/ui/local_controller.c | local controller router | public | Defines the local controller owned buttons value used by local controller router. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller_clear_result | function | src/ui/local_controller.c | local controller router | file | Implements local controller clear result. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/local_controller.c | local_controller_clear_result | local | Supplies result to local_controller_clear_result. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller_state_is_valid | function | src/ui/local_controller.c | local controller router | file | Implements local controller state is valid. | Issue #39: keyboard, OSK and local UI foreground model |
| state | parameter | src/ui/local_controller.c | local_controller_state_is_valid | local | Supplies state to local_controller_state_is_valid. | Issue #39: keyboard, OSK and local UI foreground model |
| not_down | variable | src/ui/local_controller.c | local_controller_state_is_valid | local | Stores not down while local_controller_state_is_valid runs. | Issue #39: keyboard, OSK and local UI foreground model |
| local_controller_append_action | function | src/ui/local_controller.c | local controller router | file | Implements local controller append action. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/local_controller.c | local_controller_append_action | local | Supplies result to local_controller_append_action. | Issue #39: keyboard, OSK and local UI foreground model |
| action | parameter | src/ui/local_controller.c | local_controller_append_action | local | Supplies action to local_controller_append_action. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_init | function | src/ui/local_controller.c | local controller router | file | Implements local controller init. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | parameter | src/ui/local_controller.c | pstvnc_local_controller_init | local | Supplies controller to pstvnc_local_controller_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_route | function | src/ui/local_controller.c | local controller router | file | Implements local controller route. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | parameter | src/ui/local_controller.c | pstvnc_local_controller_route | local | Supplies controller to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| foreground | parameter | src/ui/local_controller.c | pstvnc_local_controller_route | local | Supplies foreground to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| input_quarantined | parameter | src/ui/local_controller.c | pstvnc_local_controller_route | local | Supplies input quarantined to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| state | parameter | src/ui/local_controller.c | pstvnc_local_controller_route | local | Supplies state to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/local_controller.c | pstvnc_local_controller_route | local | Supplies result to pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| local_pressed | variable | src/ui/local_controller.c | pstvnc_local_controller_route | local | Stores local pressed while pstvnc_local_controller_route runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_release_is_proven | function | src/ui/local_controller.c | local controller router | file | Implements local controller release is proven. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | parameter | src/ui/local_controller.c | pstvnc_local_controller_release_is_proven | local | Supplies controller to pstvnc_local_controller_release_is_proven. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_H | macro | src/ui/local_controller.h | local controller interface | public | Defines the local controller h value used by local controller interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_MAX_ACTIONS | macro | src/ui/local_controller.h | local controller interface | public | Defines the local controller max actions value used by local controller interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_action | enum | src/ui/local_controller.h | local controller interface | public | Defines the local controller action semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_OPEN_OSK | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action open osk within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_LEFT | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move left within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_RIGHT | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move right within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_UP | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move up within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_MOVE_DOWN | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action move down within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_TOGGLE_SHIFT | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action toggle shift within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_ACTIVATE_SELECTED | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action activate selected within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_BACKSPACE | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action backspace within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_ENTER | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action enter within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_TAB | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action tab within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_CONTROLLER_ACTION_CLOSE_OSK | enum value | src/ui/local_controller.h | pstvnc_local_controller_action | public | Represents local controller action close osk within local controller action. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_action_t | type | src/ui/local_controller.h | local controller interface | public | Names the local controller action t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_result | structure | src/ui/local_controller.h | local controller interface | public | Defines the state carried by local controller result. | Issue #39: keyboard, OSK and local UI foreground model |
| actions | field | src/ui/local_controller.h | pstvnc_local_controller_result | public | Stores actions within local controller result. | Issue #39: keyboard, OSK and local UI foreground model |
| action_count | field | src/ui/local_controller.h | pstvnc_local_controller_result | public | Stores action count within local controller result. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_result_t | type | src/ui/local_controller.h | local controller interface | public | Names the local controller result t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller | structure | src/ui/local_controller.h | local controller interface | public | Defines the state carried by local controller. | Issue #39: keyboard, OSK and local UI foreground model |
| owned_buttons_awaiting_release | field | src/ui/local_controller.h | pstvnc_local_controller | public | Stores owned buttons awaiting release within local controller. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_t | type | src/ui/local_controller.h | local controller interface | public | Names the local controller t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_init | function declaration | src/ui/local_controller.h | local controller interface | public | Declares local controller init through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_init | prototype | Declares the controller argument accepted by pstvnc_local_controller_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_route | function declaration | src/ui/local_controller.h | local controller interface | public | Declares local controller route through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_route | prototype | Declares the controller argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| foreground | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_route | prototype | Declares the foreground argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| input_quarantined | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_route | prototype | Declares the input quarantined argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| state | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_route | prototype | Declares the state argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| result | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_route | prototype | Declares the result argument accepted by pstvnc_local_controller_route. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_controller_release_is_proven | function declaration | src/ui/local_controller.h | local controller interface | public | Declares local controller release is proven through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| controller | prototype parameter | src/ui/local_controller.h | pstvnc_local_controller_release_is_proven | prototype | Declares the controller argument accepted by pstvnc_local_controller_release_is_proven. | Issue #39: keyboard, OSK and local UI foreground model |
| local_ui_advance_generation | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui advance generation. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | local_ui_advance_generation | local | Supplies ui to local_ui_advance_generation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_init | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui init. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_init | local | Supplies ui to pstvnc_local_ui_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_open_osk | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui open osk. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_open_osk | local | Supplies ui to pstvnc_local_ui_open_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_close_osk | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui close osk. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_close_osk | local | Supplies ui to pstvnc_local_ui_close_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| destination | variable | src/ui/local_ui.c | pstvnc_local_ui_close_osk | local | Stores destination while pstvnc_local_ui_close_osk runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_input_is_quarantined | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui input is quarantined. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_input_is_quarantined | local | Supplies ui to pstvnc_local_ui_input_is_quarantined. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_complete_input_quarantine | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui complete input quarantine. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_complete_input_quarantine | local | Supplies ui to pstvnc_local_ui_complete_input_quarantine. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_local_change | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui mark local change. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_mark_local_change | local | Supplies ui to pstvnc_local_ui_mark_local_change. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_generation | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui generation. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_generation | local | Supplies ui to pstvnc_local_ui_generation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_needs_present | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui needs present. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_needs_present | local | Supplies ui to pstvnc_local_ui_needs_present. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_presented | function | src/ui/local_ui.c | local UI foreground model | file | Implements local ui mark presented. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui.c | pstvnc_local_ui_mark_presented | local | Supplies ui to pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | parameter | src/ui/local_ui.c | pstvnc_local_ui_mark_presented | local | Supplies generation to pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_H | macro | src/ui/local_ui.h | local UI interface | public | Defines the local ui h value used by local UI interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_foreground | enum | src/ui/local_ui.h | local UI interface | public | Defines the local ui foreground semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_FOREGROUND_DESKTOP | enum value | src/ui/local_ui.h | pstvnc_local_ui_foreground | public | Represents local ui foreground desktop within local ui foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_FOREGROUND_OSK | enum value | src/ui/local_ui.h | pstvnc_local_ui_foreground | public | Represents local ui foreground osk within local ui foreground. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_foreground_t | type | src/ui/local_ui.h | local UI interface | public | Names the local ui foreground t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui | structure | src/ui/local_ui.h | local UI interface | public | Defines the state carried by local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| foreground | field | src/ui/local_ui.h | pstvnc_local_ui | public | Stores foreground within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| return_foreground | field | src/ui/local_ui.h | pstvnc_local_ui | public | Stores return foreground within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| input_quarantined | field | src/ui/local_ui.h | pstvnc_local_ui | public | Stores input quarantined within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | field | src/ui/local_ui.h | pstvnc_local_ui | public | Stores generation within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| presented_generation | field | src/ui/local_ui.h | pstvnc_local_ui | public | Stores presented generation within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| dirty | field | src/ui/local_ui.h | pstvnc_local_ui | public | Stores dirty within local ui. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_t | type | src/ui/local_ui.h | local UI interface | public | Names the local ui t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_init | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui init through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_init | prototype | Declares the ui argument accepted by pstvnc_local_ui_init. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_open_osk | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui open osk through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_open_osk | prototype | Declares the ui argument accepted by pstvnc_local_ui_open_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_close_osk | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui close osk through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_close_osk | prototype | Declares the ui argument accepted by pstvnc_local_ui_close_osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_input_is_quarantined | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui input is quarantined through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_input_is_quarantined | prototype | Declares the ui argument accepted by pstvnc_local_ui_input_is_quarantined. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_complete_input_quarantine | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui complete input quarantine through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_complete_input_quarantine | prototype | Declares the ui argument accepted by pstvnc_local_ui_complete_input_quarantine. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_local_change | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui mark local change through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_mark_local_change | prototype | Declares the ui argument accepted by pstvnc_local_ui_mark_local_change. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_generation | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui generation through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_generation | prototype | Declares the ui argument accepted by pstvnc_local_ui_generation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_needs_present | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui needs present through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_needs_present | prototype | Declares the ui argument accepted by pstvnc_local_ui_needs_present. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_mark_presented | function declaration | src/ui/local_ui.h | local UI interface | public | Declares local ui mark presented through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_mark_presented | prototype | Declares the ui argument accepted by pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | prototype parameter | src/ui/local_ui.h | pstvnc_local_ui_mark_presented | prototype | Declares the generation argument accepted by pstvnc_local_ui_mark_presented. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_BOTTOM_INSET | macro | src/ui/local_ui_presentation.c | local UI presentation | public | Defines the local ui bottom inset value used by local UI presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| clear_presentation | function | src/ui/local_ui_presentation.c | local UI presentation | file | Clears presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| presentation | parameter | src/ui/local_ui_presentation.c | clear_presentation | local | Supplies presentation to clear_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_prepare_presentation | function | src/ui/local_ui_presentation.c | local UI presentation | file | Implements local ui prepare presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | parameter | src/ui/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies ui to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies osk to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_pixels | parameter | src/ui/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies overlay surface pixels to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_capacity_pixels | parameter | src/ui/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies overlay surface capacity pixels to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| presentation | parameter | src/ui/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Supplies presentation to pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| horizontal_space | variable | src/ui/local_ui_presentation.c | pstvnc_local_ui_prepare_presentation | local | Stores horizontal space while pstvnc_local_ui_prepare_presentation runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_LOCAL_UI_PRESENTATION_H | macro | src/ui/local_ui_presentation.h | local UI presentation interface | public | Defines the local ui presentation h value used by local UI presentation interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_presentation | structure | src/ui/local_ui_presentation.h | local UI presentation interface | public | Defines the state carried by local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_visible | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay visible within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_pixels | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay pixels within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_pixel_count | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay pixel count within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_width | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay width within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_height | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay height within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_x | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay x within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_y | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores overlay y within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| generation | field | src/ui/local_ui_presentation.h | pstvnc_local_ui_presentation | public | Stores generation within local ui presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_presentation_t | type | src/ui/local_ui_presentation.h | local UI presentation interface | public | Names the local ui presentation t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_local_ui_prepare_presentation | function declaration | src/ui/local_ui_presentation.h | local UI presentation interface | public | Declares local ui prepare presentation through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| ui | prototype parameter | src/ui/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the ui argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the osk argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_pixels | prototype parameter | src/ui/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the overlay surface pixels argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| overlay_surface_capacity_pixels | prototype parameter | src/ui/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the overlay surface capacity pixels argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| presentation | prototype parameter | src/ui/local_ui_presentation.h | pstvnc_local_ui_prepare_presentation | prototype | Declares the presentation argument accepted by pstvnc_local_ui_prepare_presentation. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_abc_rows | variable | src/ui/osk.c | on-screen keyboard model | file | Stores osk abc rows state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_page_row_lengths | variable | src/ui/osk.c | on-screen keyboard model | file | Stores osk page row lengths state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_func_labels | variable | src/ui/osk.c | on-screen keyboard model | file | Stores osk func labels state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_func_keysyms | variable | src/ui/osk.c | on-screen keyboard model | file | Stores osk func keysyms state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_special_labels | variable | src/ui/osk.c | on-screen keyboard model | file | Stores osk special labels state owned by on-screen keyboard model. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_shifted_ascii | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk shifted ascii. | Issue #39: keyboard, OSK and local UI foreground model |
| c | parameter | src/ui/osk.c | osk_shifted_ascii | local | Supplies c to osk_shifted_ascii. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_func_keysym | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk func keysym. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/ui/osk.c | osk_func_keysym | local | Supplies row to osk_func_keysym. | Issue #39: keyboard, OSK and local UI foreground model |
| col | parameter | src/ui/osk.c | osk_func_keysym | local | Supplies col to osk_func_keysym. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_real_modifier_mask | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk real modifier mask. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | osk_real_modifier_mask | local | Supplies osk to osk_real_modifier_mask. | Issue #39: keyboard, OSK and local UI foreground model |
| include_shift | parameter | src/ui/osk.c | osk_real_modifier_mask | local | Supplies include shift to osk_real_modifier_mask. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | variable | src/ui/osk.c | osk_real_modifier_mask | local | Stores modifiers while osk_real_modifier_mask runs. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_activation_clear | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk activation clear. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/osk.c | osk_activation_clear | local | Supplies result to osk_activation_clear. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_consume_modifiers_after_key | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk consume modifiers after key. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | osk_consume_modifiers_after_key | local | Supplies osk to osk_consume_modifiers_after_key. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/osk.c | osk_consume_modifiers_after_key | local | Supplies result to osk_consume_modifiers_after_key. | Issue #39: keyboard, OSK and local UI foreground model |
| had_modifiers | variable | src/ui/osk.c | osk_consume_modifiers_after_key | local | Stores had modifiers while osk_consume_modifiers_after_key runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_reset_for_open | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk reset for open. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_reset_for_open | local | Supplies osk to pstvnc_osk_reset_for_open. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_clear_modifiers | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk clear modifiers. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_clear_modifiers | local | Supplies osk to pstvnc_osk_clear_modifiers. | Issue #39: keyboard, OSK and local UI foreground model |
| changed | variable | src/ui/osk.c | pstvnc_osk_clear_modifiers | local | Stores changed while pstvnc_osk_clear_modifiers runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_toggle_shift_modifier | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk toggle shift modifier. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_toggle_shift_modifier | local | Supplies osk to pstvnc_osk_toggle_shift_modifier. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_direct_key | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk activate direct key. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_activate_direct_key | local | Supplies osk to pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | parameter | src/ui/osk.c | pstvnc_osk_activate_direct_key | local | Supplies keysym to pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/osk.c | pstvnc_osk_activate_direct_key | local | Supplies result to pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_row_length | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk row length. | Issue #39: keyboard, OSK and local UI foreground model |
| page | parameter | src/ui/osk.c | pstvnc_osk_row_length | local | Supplies page to pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/ui/osk.c | pstvnc_osk_row_length | local | Supplies row to pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_horizontal | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk move horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_move_horizontal | local | Supplies osk to pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | parameter | src/ui/osk.c | pstvnc_osk_move_horizontal | local | Supplies direction to pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| count | variable | src/ui/osk.c | pstvnc_osk_move_horizontal | local | Stores count while pstvnc_osk_move_horizontal runs. | Issue #39: keyboard, OSK and local UI foreground model |
| old_col | variable | src/ui/osk.c | pstvnc_osk_move_horizontal | local | Stores old col while pstvnc_osk_move_horizontal runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_vertical | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk move vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_move_vertical | local | Supplies osk to pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | parameter | src/ui/osk.c | pstvnc_osk_move_vertical | local | Supplies direction to pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| old_count | variable | src/ui/osk.c | pstvnc_osk_move_vertical | local | Stores old count while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| old_col | variable | src/ui/osk.c | pstvnc_osk_move_vertical | local | Stores old col while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| new_row | variable | src/ui/osk.c | pstvnc_osk_move_vertical | local | Stores new row while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| new_count | variable | src/ui/osk.c | pstvnc_osk_move_vertical | local | Stores new count while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| new_col | variable | src/ui/osk.c | pstvnc_osk_move_vertical | local | Stores new col while pstvnc_osk_move_vertical runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_display_char | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk display char. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_display_char | local | Supplies osk to pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/ui/osk.c | pstvnc_osk_display_char | local | Supplies row to pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| col | parameter | src/ui/osk.c | pstvnc_osk_display_char | local | Supplies col to pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| c | variable | src/ui/osk.c | pstvnc_osk_display_char | local | Stores c while pstvnc_osk_display_char runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_key_label | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk key label. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_key_label | local | Supplies osk to pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| row | parameter | src/ui/osk.c | pstvnc_osk_key_label | local | Supplies row to pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| col | parameter | src/ui/osk.c | pstvnc_osk_key_label | local | Supplies col to pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| count | variable | src/ui/osk.c | pstvnc_osk_key_label | local | Stores count while pstvnc_osk_key_label runs. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_selected | function | src/ui/osk.c | on-screen keyboard model | file | Implements osk activate selected. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk.c | pstvnc_osk_activate_selected | local | Supplies osk to pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| result | parameter | src/ui/osk.c | pstvnc_osk_activate_selected | local | Supplies result to pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | variable | src/ui/osk.c | pstvnc_osk_activate_selected | local | Stores keysym while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| modifiers | variable | src/ui/osk.c | pstvnc_osk_activate_selected | local | Stores modifiers while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| count | variable | src/ui/osk.c | pstvnc_osk_activate_selected | local | Stores count while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| display_char | variable | src/ui/osk.c | pstvnc_osk_activate_selected | local | Stores display char while pstvnc_osk_activate_selected runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_H | macro | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk h value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_WIDTH | macro | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk width value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_HEIGHT | macro | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk height value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_ROWS | macro | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk rows value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ROW | macro | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk utility row value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_KEY_COUNT | macro | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk utility key count value used by on-screen keyboard interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_page | enum | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk page semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_PAGE_ABC | enum value | src/ui/osk.h | pstvnc_osk_page | public | Represents osk page abc within osk page. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_PAGE_FUNC | enum value | src/ui/osk.h | pstvnc_osk_page | public | Represents osk page func within osk page. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_PAGE_COUNT | enum value | src/ui/osk.h | pstvnc_osk_page | public | Represents osk page count within osk page. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_page_t | type | src/ui/osk.h | on-screen keyboard interface | public | Names the osk page t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_utility_key | enum | src/ui/osk.h | on-screen keyboard interface | public | Defines the osk utility key semantic enumeration. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ABC | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility abc within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_FUNC | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility func within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_SHIFT | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility shift within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_CTRL | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility ctrl within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ALT | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility alt within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_SPACE | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility space within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_TAB | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility tab within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_BACKSPACE | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility backspace within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_DELETE | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility delete within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ENTER | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility enter within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_UTILITY_ESCAPE | enum value | src/ui/osk.h | pstvnc_osk_utility_key | public | Represents osk utility escape within osk utility key. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_utility_key_t | type | src/ui/osk.h | on-screen keyboard interface | public | Names the osk utility key t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk | structure | src/ui/osk.h | on-screen keyboard interface | public | Defines the state carried by osk. | Issue #39: keyboard, OSK and local UI foreground model |
| page | field | src/ui/osk.h | pstvnc_osk | public | Stores page within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| row | field | src/ui/osk.h | pstvnc_osk | public | Stores row within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| col | field | src/ui/osk.h | pstvnc_osk | public | Stores col within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| shift | field | src/ui/osk.h | pstvnc_osk | public | Stores shift within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| ctrl | field | src/ui/osk.h | pstvnc_osk | public | Stores ctrl within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| alt | field | src/ui/osk.h | pstvnc_osk | public | Stores alt within osk. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_t | type | src/ui/osk.h | on-screen keyboard interface | public | Names the osk t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activation | structure | src/ui/osk.h | on-screen keyboard interface | public | Defines the state carried by osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| local_state_changed | field | src/ui/osk.h | pstvnc_osk_activation | public | Stores local state changed within osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| produced_keyboard_tap | field | src/ui/osk.h | pstvnc_osk_activation | public | Stores produced keyboard tap within osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| keyboard_tap | field | src/ui/osk.h | pstvnc_osk_activation | public | Stores keyboard tap within osk activation. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activation_t | type | src/ui/osk.h | on-screen keyboard interface | public | Names the osk activation t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_reset_for_open | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk reset for open through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_reset_for_open | prototype | Declares the osk argument accepted by pstvnc_osk_reset_for_open. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_clear_modifiers | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk clear modifiers through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_clear_modifiers | prototype | Declares the osk argument accepted by pstvnc_osk_clear_modifiers. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_toggle_shift_modifier | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk toggle shift modifier through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_toggle_shift_modifier | prototype | Declares the osk argument accepted by pstvnc_osk_toggle_shift_modifier. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_direct_key | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk activate direct key through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_activate_direct_key | prototype | Declares the osk argument accepted by pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| keysym | prototype parameter | src/ui/osk.h | pstvnc_osk_activate_direct_key | prototype | Declares the keysym argument accepted by pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| result | prototype parameter | src/ui/osk.h | pstvnc_osk_activate_direct_key | prototype | Declares the result argument accepted by pstvnc_osk_activate_direct_key. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_row_length | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk row length through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| page | prototype parameter | src/ui/osk.h | pstvnc_osk_row_length | prototype | Declares the page argument accepted by pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| row | prototype parameter | src/ui/osk.h | pstvnc_osk_row_length | prototype | Declares the row argument accepted by pstvnc_osk_row_length. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_horizontal | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk move horizontal through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_move_horizontal | prototype | Declares the osk argument accepted by pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | prototype parameter | src/ui/osk.h | pstvnc_osk_move_horizontal | prototype | Declares the direction argument accepted by pstvnc_osk_move_horizontal. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_move_vertical | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk move vertical through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_move_vertical | prototype | Declares the osk argument accepted by pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| direction | prototype parameter | src/ui/osk.h | pstvnc_osk_move_vertical | prototype | Declares the direction argument accepted by pstvnc_osk_move_vertical. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_display_char | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk display char through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_display_char | prototype | Declares the osk argument accepted by pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| row | prototype parameter | src/ui/osk.h | pstvnc_osk_display_char | prototype | Declares the row argument accepted by pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| col | prototype parameter | src/ui/osk.h | pstvnc_osk_display_char | prototype | Declares the col argument accepted by pstvnc_osk_display_char. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_key_label | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk key label through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_key_label | prototype | Declares the osk argument accepted by pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| row | prototype parameter | src/ui/osk.h | pstvnc_osk_key_label | prototype | Declares the row argument accepted by pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| col | prototype parameter | src/ui/osk.h | pstvnc_osk_key_label | prototype | Declares the col argument accepted by pstvnc_osk_key_label. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_activate_selected | function declaration | src/ui/osk.h | on-screen keyboard interface | public | Declares osk activate selected through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk.h | pstvnc_osk_activate_selected | prototype | Declares the osk argument accepted by pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| result | prototype parameter | src/ui/osk.h | pstvnc_osk_activate_selected | prototype | Declares the result argument accepted by pstvnc_osk_activate_selected. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_font_glyph | structure | src/ui/osk_render.c | on-screen keyboard renderer | public | Defines the state carried by osk font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| character | field | src/ui/osk_render.c | pstvnc_osk_font_glyph | public | Stores character within osk font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| rows | field | src/ui/osk_render.c | pstvnc_osk_font_glyph | public | Stores rows within osk font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_font_glyph_t | type | src/ui/osk_render.c | on-screen keyboard renderer | public | Names the osk font glyph t value type. | Issue #39: keyboard, OSK and local UI foreground model |
| osk_font_glyphs | variable | src/ui/osk_render.c | on-screen keyboard renderer | file | Stores osk font glyphs state owned by on-screen keyboard renderer. | Issue #39: keyboard, OSK and local UI foreground model |
| pack_opaque_surface_color | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Packs opaque surface color. | Issue #39: keyboard, OSK and local UI foreground model |
| red | parameter | src/ui/osk_render.c | pack_opaque_surface_color | local | Supplies red to pack_opaque_surface_color. | Issue #39: keyboard, OSK and local UI foreground model |
| green | parameter | src/ui/osk_render.c | pack_opaque_surface_color | local | Supplies green to pack_opaque_surface_color. | Issue #39: keyboard, OSK and local UI foreground model |
| blue | parameter | src/ui/osk_render.c | pack_opaque_surface_color | local | Supplies blue to pack_opaque_surface_color. | Issue #39: keyboard, OSK and local UI foreground model |
| fill_surface_rectangle | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Fills surface rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/ui/osk_render.c | fill_surface_rectangle | local | Supplies surface pixels to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| x | parameter | src/ui/osk_render.c | fill_surface_rectangle | local | Supplies x to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| y | parameter | src/ui/osk_render.c | fill_surface_rectangle | local | Supplies y to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| width | parameter | src/ui/osk_render.c | fill_surface_rectangle | local | Supplies width to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| height | parameter | src/ui/osk_render.c | fill_surface_rectangle | local | Supplies height to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| color | parameter | src/ui/osk_render.c | fill_surface_rectangle | local | Supplies color to fill_surface_rectangle. | Issue #39: keyboard, OSK and local UI foreground model |
| row_index | variable | src/ui/osk_render.c | fill_surface_rectangle | local | Stores row index while fill_surface_rectangle runs. | Issue #39: keyboard, OSK and local UI foreground model |
| column_index | variable | src/ui/osk_render.c | fill_surface_rectangle | local | Stores column index while fill_surface_rectangle runs. | Issue #39: keyboard, OSK and local UI foreground model |
| destination | variable | src/ui/osk_render.c | fill_surface_rectangle | local | Stores destination while fill_surface_rectangle runs. | Issue #39: keyboard, OSK and local UI foreground model |
| find_font_glyph | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Finds font glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| character | parameter | src/ui/osk_render.c | find_font_glyph | local | Supplies character to find_font_glyph. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph_index | variable | src/ui/osk_render.c | find_font_glyph | local | Stores glyph index while find_font_glyph runs. | Issue #39: keyboard, OSK and local UI foreground model |
| draw_surface_character | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Draws surface character. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/ui/osk_render.c | draw_surface_character | local | Supplies surface pixels to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| x | parameter | src/ui/osk_render.c | draw_surface_character | local | Supplies x to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| y | parameter | src/ui/osk_render.c | draw_surface_character | local | Supplies y to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| character | parameter | src/ui/osk_render.c | draw_surface_character | local | Supplies character to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| scale | parameter | src/ui/osk_render.c | draw_surface_character | local | Supplies scale to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| color | parameter | src/ui/osk_render.c | draw_surface_character | local | Supplies color to draw_surface_character. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph | variable | src/ui/osk_render.c | draw_surface_character | local | Stores glyph while draw_surface_character runs. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph_row | variable | src/ui/osk_render.c | draw_surface_character | local | Stores glyph row while draw_surface_character runs. | Issue #39: keyboard, OSK and local UI foreground model |
| glyph_column | variable | src/ui/osk_render.c | draw_surface_character | local | Stores glyph column while draw_surface_character runs. | Issue #39: keyboard, OSK and local UI foreground model |
| draw_centered_surface_text | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Draws centered surface text. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies surface pixels to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| x | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies x to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| y | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies y to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| width | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies width to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| height | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies height to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| text | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies text to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| scale | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies scale to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| color | parameter | src/ui/osk_render.c | draw_centered_surface_text | local | Supplies color to draw_centered_surface_text. | Issue #39: keyboard, OSK and local UI foreground model |
| character_count | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores character count while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| character_width | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores character width while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_width | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores text width while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_height | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores text height while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_x | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores text x while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_y | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores text y while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| character_index | variable | src/ui/osk_render.c | draw_centered_surface_text | local | Stores character index while draw_centered_surface_text runs. | Issue #39: keyboard, OSK and local UI foreground model |
| utility_key_is_active | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Implements utility key is active. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk_render.c | utility_key_is_active | local | Supplies osk to utility_key_is_active. | Issue #39: keyboard, OSK and local UI foreground model |
| column | parameter | src/ui/osk_render.c | utility_key_is_active | local | Supplies column to utility_key_is_active. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_render_surface | function | src/ui/osk_render.c | on-screen keyboard renderer | file | Implements osk render surface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | parameter | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Supplies osk to pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | parameter | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Supplies surface pixels to pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_capacity_pixels | parameter | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Supplies surface capacity pixels to pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| panel_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores panel color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| ordinary_key_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores ordinary key color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected_key_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores selected key color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| active_key_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores active key color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| border_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores border color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| ordinary_text_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores ordinary text color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected_text_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores selected text color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| row | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores row while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected_row_length | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores selected row length while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_count | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores key count while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| margin | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores margin while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| gap | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores gap while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| available_width | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores available width while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_width | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores key width while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| used_width | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores used width while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| row_x | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores row x while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| row_y | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores row y while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_height | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores key height while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| column | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores column while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| key_x | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores key x while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| selected | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores selected while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| background_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores background color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_color | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores text color while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| label | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores label while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| character_label | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores character label while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| text_scale | variable | src/ui/osk_render.c | pstvnc_osk_render_surface | local | Stores text scale while pstvnc_osk_render_surface runs. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_RENDER_H | macro | src/ui/osk_render.h | on-screen keyboard renderer interface | public | Defines the osk render h value used by on-screen keyboard renderer interface. | Issue #39: keyboard, OSK and local UI foreground model |
| PSTVNC_OSK_SURFACE_PIXEL_COUNT | macro | src/ui/osk_render.h | on-screen keyboard renderer interface | public | Defines the osk surface pixel count value used by on-screen keyboard renderer interface. | Issue #39: keyboard, OSK and local UI foreground model |
| pstvnc_osk_render_surface | function declaration | src/ui/osk_render.h | on-screen keyboard renderer interface | public | Declares osk render surface through the owning public interface. | Issue #39: keyboard, OSK and local UI foreground model |
| osk | prototype parameter | src/ui/osk_render.h | pstvnc_osk_render_surface | prototype | Declares the osk argument accepted by pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_pixels | prototype parameter | src/ui/osk_render.h | pstvnc_osk_render_surface | prototype | Declares the surface pixels argument accepted by pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
| surface_capacity_pixels | prototype parameter | src/ui/osk_render.h | pstvnc_osk_render_surface | prototype | Declares the surface capacity pixels argument accepted by pstvnc_osk_render_surface. | Issue #39: keyboard, OSK and local UI foreground model |
