
void display_confirm_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(8, 8, 10);
    const unsigned short selected = osk_color(8, 18, 31);
    const unsigned short text = osk_color(31, 31, 31);
    const unsigned short warn = osk_color(31, 24, 3);
    char line[80];

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    osk_draw_text_centered(
        0, 10, OSK_WIDTH, 24,
        "KEEP THIS DISPLAY MODE?", 2, warn);

    snprintf(
        line, sizeof(line),
        "%s -> %s",
        display_confirm_previous_mode ?
            display_confirm_previous_mode->name : "?",
        active_video_mode ? active_video_mode->name : "?");

    osk_draw_text_centered(
        0, 37, OSK_WIDTH, 18,
        line, 1, text);

    osk_fill_rect(
        120, 62, OSK_WIDTH - 240, 27,
        display_confirm_selection == DISPLAY_CONFIRM_GO_BACK ?
            selected : key);

    osk_draw_text_centered(
        120, 62, OSK_WIDTH - 240, 27,
        "GO BACK", 2, text);

    osk_fill_rect(
        120, 95, OSK_WIDTH - 240, 27,
        display_confirm_selection == DISPLAY_CONFIRM_KEEP ?
            selected : key);

    osk_draw_text_centered(
        120, 95, OSK_WIDTH - 240, 27,
        "KEEP MODE", 2, text);

    snprintf(
        line, sizeof(line),
        "AUTO-REVERT IN %u SEC",
        display_confirm_seconds);

    osk_draw_text_centered(
        0, 133, OSK_WIDTH, 18,
        line, 1, text);

    osk_draw_text_centered(
        0, 153, OSK_WIDTH, 16,
        "X SELECT   O GO BACK", 1, text);
}


void display_restore_ack_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(8, 18, 31);
    const unsigned short text = osk_color(31, 31, 31);
    const unsigned short good = osk_color(8, 31, 10);
    char line[80];

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    osk_draw_text_centered(
        0, 14, OSK_WIDTH, 24,
        "ORIGINAL DISPLAY MODE RESTORED",
        2, good);

    snprintf(
        line,
        sizeof(line),
        "%s HAS BEEN RESTORED SUCCESSFULLY.",
        display_restore_ack_mode ?
            display_restore_ack_mode->name : "DISPLAY");

    osk_draw_text_centered(
        0, 54, OSK_WIDTH, 18,
        line, 1, text);

    osk_fill_rect(
        180, 91, OSK_WIDTH - 360, 29,
        key);

    osk_draw_text_centered(
        180, 91, OSK_WIDTH - 360, 29,
        "OK", 2, text);

    osk_draw_text_centered(
        0, 143, OSK_WIDTH, 18,
        "X OK", 1, text);
}



int display_menu_tab_for_mode(
    const ps2vnc_video_mode_t *mode)
{
    if (mode == NULL)
        return DISPLAY_TAB_TV_DTV;

    switch (mode->menu_group) {
        case PS2VNC_DISPLAY_GROUP_TV_DTV:
            return DISPLAY_TAB_TV_DTV;

        case PS2VNC_DISPLAY_GROUP_VGA_60:
            return DISPLAY_TAB_VGA_60;

        case PS2VNC_DISPLAY_GROUP_VGA_OTHER:
            return DISPLAY_TAB_VGA_OTHER;

        default:
            return DISPLAY_TAB_TV_DTV;
    }
}


static const char *display_menu_tab_label(int tab)
{
    switch (tab) {
        case DISPLAY_TAB_TV_DTV:
            return "TV / DTV";

        case DISPLAY_TAB_VGA_60:
            return "VGA 60";

        case DISPLAY_TAB_VGA_OTHER:
            return "VGA OTHER";

        case DISPLAY_TAB_ADVANCED:
            return "ADVANCED";

        default:
            return "?";
    }
}


static int display_menu_mode_belongs_to_tab(
    int selection,
    int tab)
{
    const ps2vnc_video_mode_t *mode =
        display_menu_mode_at_selection(selection);

    if (mode == NULL ||
        tab < DISPLAY_TAB_TV_DTV ||
        tab > DISPLAY_TAB_VGA_OTHER)
        return 0;

    return display_menu_tab_for_mode(mode) == tab;
}


static int display_menu_mode_visible(int selection)
{
    const ps2vnc_video_mode_t *mode =
        display_menu_mode_at_selection(selection);

    if (mode == NULL)
        return 0;

    /*
     * The running mode is never hidden, even if an advanced/manual config
     * edit happens to mark it locked.
     */
    if (mode == active_video_mode)
        return 1;

    if (!display_hide_locked_modes)
        return 1;

    return display_mode_locked[selection] ? 0 : 1;
}


static int display_menu_first_visible_selection(int tab)
{
    unsigned int i;

    if (tab < DISPLAY_TAB_TV_DTV ||
        tab > DISPLAY_TAB_VGA_OTHER)
        return -1;

    for (i = 0; i < VIDEO_MODE_COUNT; i++) {
        if (display_menu_mode_belongs_to_tab((int)i, tab) &&
            display_menu_mode_visible((int)i))
            return (int)i;
    }

    return -1;
}


static int display_menu_tab_visible(int tab)
{
    if (tab == DISPLAY_TAB_ADVANCED)
        return 1;

    if (tab < DISPLAY_TAB_TV_DTV ||
        tab > DISPLAY_TAB_VGA_OTHER)
        return 0;

    return display_menu_first_visible_selection(tab) >= 0;
}


static int display_menu_next_visible_tab(
    int tab,
    int direction)
{
    int step;

    if (direction == 0)
        return display_menu_tab_visible(tab)
            ? tab
            : DISPLAY_TAB_ADVANCED;

    for (step = 1;
         step <= DISPLAY_TAB_COUNT;
         step++) {

        int candidate =
            (tab +
             direction * step +
             DISPLAY_TAB_COUNT * 4) %
            DISPLAY_TAB_COUNT;

        if (display_menu_tab_visible(candidate))
            return candidate;
    }

    return DISPLAY_TAB_ADVANCED;
}


void display_menu_normalize_selection(void)
{
    int saved;

    if (!display_menu_tab_visible(display_menu_tab))
        display_menu_tab =
            display_menu_next_visible_tab(
                display_menu_tab,
                1);

    if (display_menu_tab == DISPLAY_TAB_ADVANCED) {
        display_menu_selection = -1;
        return;
    }

    if (display_menu_mode_belongs_to_tab(
            display_menu_selection,
            display_menu_tab) &&
        display_menu_mode_visible(display_menu_selection)) {

        display_menu_tab_selection[display_menu_tab] =
            display_menu_selection;

        return;
    }

    saved = display_menu_tab_selection[display_menu_tab];

    if (display_menu_mode_belongs_to_tab(
            saved,
            display_menu_tab) &&
        display_menu_mode_visible(saved)) {

        display_menu_selection = saved;
        return;
    }

    display_menu_selection =
        display_menu_first_visible_selection(
            display_menu_tab);

    if (display_menu_selection >= 0)
        display_menu_tab_selection[display_menu_tab] =
            display_menu_selection;
}


 void display_menu_open_for_active(int parent_system)
{
    int active_selection =
        display_menu_selection_for_mode(active_video_mode);

    display_menu_tab =
        display_menu_tab_for_mode(active_video_mode);

    display_menu_selection = active_selection;

    if (display_menu_tab >= DISPLAY_TAB_TV_DTV &&
        display_menu_tab <= DISPLAY_TAB_VGA_OTHER) {

        display_menu_tab_selection[display_menu_tab] =
            active_selection;
    }

    display_menu_error = 0;
    display_policy_error = 0;
    display_menu_parent_system = parent_system;
    display_menu_visible = 1;

    /*
     * Re-read authoritative policy every time the selector is opened.
     * Until the main thread completes that GET, switching into inactive
     * modes remains disabled.
     */
    if (display_policy_request ==
            DISPLAY_POLICY_REQUEST_NONE) {

        display_policy_loaded = 0;
        display_policy_request =
            DISPLAY_POLICY_REQUEST_REFRESH;
    }

    osk_generation++;
}


int display_menu_move_vertical_selection(
    int selection,
    int direction)
{
    int step;

    if (display_menu_tab == DISPLAY_TAB_ADVANCED ||
        direction == 0)
        return selection;

    if (!display_menu_mode_belongs_to_tab(
            selection,
            display_menu_tab) ||
        !display_menu_mode_visible(selection)) {

        return display_menu_first_visible_selection(
            display_menu_tab);
    }

    for (step = 1;
         step <= (int)VIDEO_MODE_COUNT;
         step++) {

        int candidate =
            (selection +
             direction * step +
             (int)VIDEO_MODE_COUNT * 4) %
            (int)VIDEO_MODE_COUNT;

        if (display_menu_mode_belongs_to_tab(
                candidate,
                display_menu_tab) &&
            display_menu_mode_visible(candidate)) {

            display_menu_tab_selection[display_menu_tab] =
                candidate;

            return candidate;
        }
    }

    return selection;
}


int display_menu_move_horizontal_selection(
    int selection,
    int direction)
{
    int old_tab = display_menu_tab;

    (void)selection;

    if (direction == 0)
        return display_menu_selection;

    if (old_tab >= DISPLAY_TAB_TV_DTV &&
        old_tab <= DISPLAY_TAB_VGA_OTHER &&
        display_menu_mode_belongs_to_tab(
            display_menu_selection,
            old_tab)) {

        display_menu_tab_selection[old_tab] =
            display_menu_selection;
    }

    display_menu_tab =
        display_menu_next_visible_tab(
            display_menu_tab,
            direction);

    display_menu_normalize_selection();

    return display_menu_selection;
}


void display_menu_render_bitmap(void)
{
    const unsigned short panel =
        osk_color(3, 3, 4);

    const unsigned short key =
        osk_color(10, 10, 11);

    const unsigned short selected =
        osk_color(31, 24, 3);

    const unsigned short active =
        osk_color(6, 20, 8);

    const unsigned short locked_bg =
        osk_color(5, 5, 6);

    const unsigned short border =
        osk_color(22, 22, 22);

    const unsigned short text_col =
        osk_color(31, 31, 31);

    const unsigned short locked_text =
        osk_color(18, 18, 18);

    const unsigned short selected_text =
        osk_color(1, 1, 1);

    const int margin = 4;
    const int tab_gap = 2;
    const int title_y = 2;
    const int tab_y = 16;
    const int tab_h = 12;

    const int list_x = 55;
    const int list_w = OSK_WIDTH - 110;
    const int top = 37;
    const int row_h = 10;
    const int cell_h = 9;

    const int footer_y = 159;

    int visible_tab_count = 0;
    int tab_w;
    int tab;
    int tab_draw_index = 0;

    for (tab = 0;
         tab < DISPLAY_TAB_COUNT;
         tab++) {

        if (display_menu_tab_visible(tab))
            visible_tab_count++;
    }

    /*
     * ADVANCED is permanently visible, so this is defensive only.
     */
    if (visible_tab_count <= 0)
        visible_tab_count = 1;

    tab_w =
        (OSK_WIDTH -
         margin * 2 -
         tab_gap * (visible_tab_count - 1)) /
        visible_tab_count;

    osk_fill_rect(
        0, 0,
        OSK_WIDTH, OSK_HEIGHT,
        panel
    );

    osk_draw_text_centered(
        0, title_y,
        OSK_WIDTH, 12,
        "DISPLAY SETTINGS",
        1,
        text_col
    );

    /*
     * Empty mode groups disappear from presentation. ADVANCED remains
     * permanently visible so Hide Locked Modes can always be reversed.
     */
    for (tab = 0;
         tab < DISPLAY_TAB_COUNT;
         tab++) {

        int x;

        if (!display_menu_tab_visible(tab))
            continue;

        x =
            margin +
            tab_draw_index * (tab_w + tab_gap);

        tab_draw_index++;

        unsigned short bg =
            (tab == display_menu_tab)
            ? selected
            : key;

        unsigned short fg =
            (tab == display_menu_tab)
            ? selected_text
            : text_col;

        osk_fill_rect(
            x, tab_y - 1,
            tab_w, tab_h + 2,
            border
        );

        osk_fill_rect(
            x + 1, tab_y,
            tab_w - 2, tab_h,
            bg
        );

        osk_draw_text_centered(
            x + 1, tab_y,
            tab_w - 2, tab_h,
            display_menu_tab_label(tab),
            1,
            fg
        );
    }

    if (display_menu_tab == DISPLAY_TAB_ADVANCED) {
        char setting[96];

        snprintf(
            setting,
            sizeof(setting),
            "HIDE LOCKED MODES: %s",
            display_hide_locked_modes
                ? "ON"
                : "OFF"
        );

        osk_fill_rect(
            list_x,
            55,
            list_w,
            20,
            border
        );

        osk_fill_rect(
            list_x + 1,
            56,
            list_w - 2,
            18,
            selected
        );

        osk_draw_text_centered(
            list_x + 1,
            56,
            list_w - 2,
            18,
            setting,
            1,
            selected_text
        );

    } else {
        unsigned int i;
        int row = 0;

        for (i = 0;
             i < VIDEO_MODE_COUNT;
             i++) {

            const ps2vnc_video_mode_t *mode =
                video_modes[i];

            unsigned short bg;
            unsigned short fg;
            char row_text[96];
            int y;

            if (!display_menu_mode_belongs_to_tab(
                    (int)i,
                    display_menu_tab))
                continue;

            if (!display_menu_mode_visible((int)i))
                continue;

            y = top + row * row_h;

            if (mode == active_video_mode) {
                bg = active;
                fg = text_col;

            } else if (display_mode_locked[i]) {
                bg = locked_bg;
                fg = locked_text;

            } else {
                bg = key;
                fg = text_col;
            }

            if ((int)i ==
                    display_menu_selection) {

                bg = selected;
                fg = selected_text;
            }

            if (mode == active_video_mode &&
                display_mode_locked[i]) {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s   CURRENT   LOCK",
                    mode->name
                );

            } else if (mode == active_video_mode) {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s   CURRENT",
                    mode->name
                );

            } else if (display_mode_locked[i]) {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s   LOCK",
                    mode->name
                );

            } else {

                snprintf(
                    row_text,
                    sizeof(row_text),
                    "%s",
                    mode->name
                );
            }

            osk_fill_rect(
                list_x,
                y - 1,
                list_w,
                cell_h + 2,
                border
            );

            osk_fill_rect(
                list_x + 1,
                y,
                list_w - 2,
                cell_h,
                bg
            );

            osk_draw_text_centered(
                list_x + 1,
                y,
                list_w - 2,
                cell_h,
                row_text,
                1,
                fg
            );

            row++;
        }

        if (row == 0) {
            osk_draw_text_centered(
                0, 70,
                OSK_WIDTH, 14,
                "NO VISIBLE MODES",
                1,
                text_col
            );
        }
    }

    /*
     * Request status owns the footer while a main-thread management
     * operation is in progress.
     */
    if (display_policy_request !=
            DISPLAY_POLICY_REQUEST_NONE) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            display_policy_request ==
                DISPLAY_POLICY_REQUEST_REFRESH
                ? "LOADING DISPLAY SETTINGS..."
                : "SAVING DISPLAY SETTINGS...",
            1,
            text_col
        );

    } else if (display_policy_error) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            "DISPLAY SETTINGS UNAVAILABLE   O BACK",
            1,
            text_col
        );

    } else if (display_menu_error) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            "PRESET UNAVAILABLE   O BACK",
            1,
            text_col
        );

    } else if (display_menu_tab ==
                   DISPLAY_TAB_ADVANCED) {

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            "X TOGGLE   O BACK",
            1,
            text_col
        );

    } else {
        const ps2vnc_video_mode_t *selected_mode =
            display_menu_mode_at_selection(
                display_menu_selection);

        char footer[128];

        if (selected_mode != NULL &&
            selected_mode == active_video_mode) {

            snprintf(
                footer,
                sizeof(footer),
                "X CALIBRATE %s SAFE AREA   O BACK",
                selected_mode->name
            );

        } else if (selected_mode != NULL &&
                   display_mode_locked[
                       display_menu_selection]) {

            snprintf(
                footer,
                sizeof(footer),
                "SQUARE UNLOCK   X LOCKED   O BACK"
            );

        } else if (selected_mode != NULL) {

            snprintf(
                footer,
                sizeof(footer),
                "SQUARE LOCK   X SWITCH TO %s   O BACK",
                selected_mode->name
            );

        } else {

            snprintf(
                footer,
                sizeof(footer),
                "O BACK"
            );
        }

        osk_draw_text_centered(
            0, footer_y,
            OSK_WIDTH, 12,
            footer,
            1,
            text_col
        );
    }
}



void system_menu_render_bitmap(void)
{
    const unsigned short panel = osk_color(3, 3, 4);
    const unsigned short key = osk_color(10, 10, 11);
    const unsigned short selected = osk_color(31, 24, 3);
    const unsigned short border = osk_color(22, 22, 22);
    const unsigned short text_col = osk_color(31, 31, 31);
    const unsigned short selected_text = osk_color(1, 1, 1);

    static const char *labels[SYSTEM_MENU_ITEM_COUNT] = {
        "REFRESH VNC SESSION",
        "DISPLAY SETTINGS",
        "EXIT TO SYSTEM MENU"
    };

    char footer[64];
    int i;

    osk_fill_rect(0, 0, OSK_WIDTH, OSK_HEIGHT, panel);

    osk_draw_text_centered(
        0, 5, OSK_WIDTH, 20,
        "PS2VNC SYSTEM", 2, text_col
    );

    for (i = 0; i < SYSTEM_MENU_ITEM_COUNT; i++) {
        int y = 32 + i * 29;
        unsigned short bg =
            (i == system_menu_selection) ? selected : key;
        unsigned short fg =
            (i == system_menu_selection) ? selected_text : text_col;

        osk_fill_rect(
            89, y - 1,
            OSK_WIDTH - 178, 25,
            border
        );

        osk_fill_rect(
            90, y,
            OSK_WIDTH - 180, 23,
            bg
        );

        osk_draw_text_centered(
            90, y,
            OSK_WIDTH - 180, 23,
            labels[i], 1, fg
        );
    }

    footer[0] = '\0';

    if (system_menu_countdown_action ==
            RUNTIME_ACTION_REFRESH_RFB &&
        system_menu_countdown_seconds > 0) {

        snprintf(
            footer, sizeof(footer),
            "HOLD FOR REFRESH: %u",
            system_menu_countdown_seconds
        );

    } else if (system_menu_countdown_action ==
                   RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU &&
               system_menu_countdown_seconds > 0) {

        snprintf(
            footer, sizeof(footer),
            "HOLD FOR EXIT: %u",
            system_menu_countdown_seconds
        );

    } else if (system_refresh_state ==
                   SYSTEM_REFRESH_REFRESHING) {

        strncpy(
            footer,
            "REFRESHING VNC SESSION...",
            sizeof(footer) - 1
        );
        footer[sizeof(footer) - 1] = '\0';

    } else if (system_refresh_state ==
                   SYSTEM_REFRESH_COOLDOWN) {

        strncpy(
            footer,
            "REFRESH COMPLETE - COOLDOWN",
            sizeof(footer) - 1
        );
        footer[sizeof(footer) - 1] = '\0';

    } else {
        strncpy(
            footer,
            "REFRESH READY   X SELECT   O BACK",
            sizeof(footer) - 1
        );
        footer[sizeof(footer) - 1] = '\0';
    }

    osk_draw_text_centered(
        0, 121, OSK_WIDTH, 16,
        footer, 1, text_col
    );
}


void osk_move_horizontal(int direction)
{
    int count = osk_row_length(osk_row);

    osk_col += direction;

    if (osk_col < 0)
        osk_col = count - 1;
    else if (osk_col >= count)
        osk_col = 0;

    osk_generation++;
}

void osk_move_vertical(int direction)
{
    int old_count = osk_row_length(osk_row);
    int old_col = osk_col;
    int new_row = osk_row + direction;
    int new_count;

    if (new_row < 0)
        new_row = OSK_ROWS - 1;
    else if (new_row >= OSK_ROWS)
        new_row = 0;

    new_count = osk_row_length(new_row);

    /*
     * Preserve approximate horizontal position when crossing rows
     * with different numbers of keys.
     */
    osk_col =
        (old_col * new_count) / old_count;

    if (osk_col >= new_count)
        osk_col = new_count - 1;

    osk_row = new_row;
    osk_generation++;
}

 unsigned int osk_shifted_ascii(unsigned int c)
{
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 'A';

    if (c >= 'A' && c <= 'Z')
        return c;

    switch (c) {
        case '1': return '!';
        case '2': return '@';
        case '3': return '#';
        case '4': return '$';
        case '5': return '%';
        case '6': return '^';
        case '7': return '&';
        case '8': return '*';
        case '9': return '(';
        case '0': return ')';
        case '-': return '_';
        case '=': return '+';
        case ';': return ':';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        case 0x5B: return 0x7B; /* [ -> { */
        case 0x5D: return 0x7D; /* ] -> } */
        case 0x5C: return 0x7C; /* backslash -> | */
        case 0x27: return 0x22; /* apostrophe -> quote */
        case 0x60: return 0x7E; /* backtick -> ~ */
        default:  return c;
    }
}

int osk_send_key_with_modifiers(
    int sock,
    unsigned int keysym)
{
    if (send_modified_key_tap(
            sock,
            keysym,
            osk_ctrl,
            osk_alt) < 0)
        return -1;

    /*
     * Shift/Ctrl/Alt are all one-shot.  After the next actual key is sent,
     * every latched modifier returns to off.
     */
    osk_clear_one_shot_modifiers();
    return 0;
}

static int osk_send_extended_key_with_modifiers(
    int sock,
    unsigned int keysym)
{
    int use_shift = osk_shift;
    int result;

    if (use_shift) {
        if (send_key_event(
                sock,
                1,
                XK_SHIFT_L) < 0)
            return -1;
    }

    result =
        send_modified_key_tap(
            sock,
            keysym,
            osk_ctrl,
            osk_alt
        );

    if (use_shift) {
        /*
         * Always attempt the release once Shift-down was queued.
         */
        if (send_key_event(
                sock,
                0,
                XK_SHIFT_L) < 0)
            return -1;
    }

    if (result < 0)
        return -1;

    osk_clear_one_shot_modifiers();
    return 0;
}


int osk_activate_selected(int sock)
{
    unsigned int keysym;

    if (osk_row < 4) {
        if (osk_page == OSK_PAGE_ABC) {
            /*
             * ABC keys send the exact displayed printable character.
             */
            keysym =
                (unsigned int)(unsigned char)
                osk_display_char(
                    osk_row,
                    osk_col
                );

            return
                osk_send_key_with_modifiers(
                    sock,
                    keysym
                );
        }

        /*
         * FUNC page contains real X11 non-printable keysyms.
         */
        keysym =
            osk_func_keysyms
                [osk_row]
                [osk_col];

        if (keysym == 0)
            return 0;

        return
            osk_send_extended_key_with_modifiers(
                sock,
                keysym
            );
    }

    /*
     * Permanent utility row:
     *
     * ABC FUNC SHIFT CTRL ALT SPACE TAB BKSP DEL ENTER ESC
     */
    switch (osk_col) {
        case 0: /* ABC */
            if (osk_page != OSK_PAGE_ABC) {
                osk_page = OSK_PAGE_ABC;
                osk_generation++;
            }
            return 0;

        case 1: /* FUNC */
            if (osk_page != OSK_PAGE_FUNC) {
                osk_page = OSK_PAGE_FUNC;
                osk_generation++;
            }
            return 0;

        case 2: /* SHIFT */
            osk_shift = !osk_shift;
            osk_generation++;
            return 0;

        case 3: /* CTRL */
            osk_ctrl = !osk_ctrl;
            osk_generation++;
            return 0;

        case 4: /* ALT */
            osk_alt = !osk_alt;
            osk_generation++;
            return 0;

        case 5: /* SPACE */
            keysym = ' ';
            break;

        case 6: /* TAB */
            keysym = XK_TAB;
            break;

        case 7: /* BKSP */
            keysym = XK_BACKSPACE;
            break;

        case 8: /* DEL */
            keysym = XK_DELETE;
            break;

        case 9: /* ENTER */
            keysym = XK_RETURN;
            break;

        case 10: /* ESC */
            keysym = XK_ESCAPE;
            break;

        default:
            return 0;
    }

    /*
     * Utility keys use the extended sender so Shift+Tab and similar
     * combinations work as true modifier combinations.
     */
    return
        osk_send_extended_key_with_modifiers(
            sock,
            keysym
        );
}

/*
 * The stable Milestone-7/8 receive loop blocks waiting for a changed
 * framebuffer.  OSK navigation itself changes only local PS2 state, so
 * nudge the remote cursor by one pixel and back to wake the outstanding
 * incremental framebuffer request.  The pointer ends exactly where it
 * started.
 */
int wake_renderer_with_pointer_jiggle(
    int sock,
    int x,
    int y)
{
    int jiggle_x;

    if (x < desktop_width - 1)
        jiggle_x = x + 1;
    else
        jiggle_x = x - 1;

    if (send_pointer_event(sock, 0,
                           (unsigned int)jiggle_x,
                           (unsigned int)y) < 0)
        return -1;

    if (send_pointer_event(sock, 0,
                           (unsigned int)x,
                           (unsigned int)y) < 0)
        return -1;

    return 0;
}

/*
 * Test11K-C shared analog response curve.
 *
 * The first ~85% of physical stick travel is deliberately shallow and
 * reaches only about 30% of maximum speed.  The final ~15% supplies the
 * remaining acceleration.
 *
 * Return value is Q12:
 *
 *     0    = stopped
 *     1229 = ~30%
 *     4096 = 100%
 */
#define ANALOG_DEADZONE       10
#define ANALOG_HALF_RAW       64   /* ~50% physical stick tilt */
#define ANALOG_KNEE_RAW       108  /* ~85% physical stick tilt */
#define ANALOG_MAX_RAW        127

#define ANALOG_CURVE_ONE      4096
#define ANALOG_CURVE_KNEE     1229  /* ~30% of 4096 */

static unsigned int analog_speed_curve_q12(int raw_magnitude)
{
    int usable;
    int knee_usable;
    int max_usable;
    unsigned int curve;

    if (raw_magnitude <= ANALOG_DEADZONE)
        return 0;

    if (raw_magnitude > ANALOG_MAX_RAW)
        raw_magnitude = ANALOG_MAX_RAW;

    usable =
        raw_magnitude - ANALOG_DEADZONE;

    knee_usable =
        ANALOG_KNEE_RAW - ANALOG_DEADZONE;

    max_usable =
        ANALOG_MAX_RAW - ANALOG_DEADZONE;

    if (usable <= knee_usable) {
        unsigned int scale_q12;

        /*
         * Base 11K-C shallow quadratic:
         *
         *     85% physical tilt = ~30% maximum speed.
         */
        curve =
            (ANALOG_CURVE_KNEE *
             (unsigned int)usable *
             (unsigned int)usable) /
            ((unsigned int)knee_usable *
             (unsigned int)knee_usable);

        /*
         * Test11K-D low-end refinement.
         *
         * Through the first 50% of physical stick travel, run at
         * exactly HALF of the previous 11K-C response.
         *
         * From 50% through the 85% knee, smoothly remove that
         * reduction so we arrive at the original 30% knee speed
         * without a discontinuity.
         */
        if (raw_magnitude <= ANALOG_HALF_RAW) {
            scale_q12 = 2048;  /* 50% */
        } else {
            scale_q12 =
                2048 +
                (2048 *
                 (unsigned int)
                 (raw_magnitude - ANALOG_HALF_RAW)) /
                (unsigned int)
                (ANALOG_KNEE_RAW - ANALOG_HALF_RAW);
        }

        curve =
            (curve * scale_q12) /
            ANALOG_CURVE_ONE;

        if (curve == 0)
            curve = 1;

        return curve;
    }

    /*
     * Final ~15% remains exactly as before:
     * 30% at the knee -> 100% at full tilt.
     */
    return
        ANALOG_CURVE_KNEE +
        ((ANALOG_CURVE_ONE - ANALOG_CURVE_KNEE) *
         (unsigned int)(usable - knee_usable)) /
        (unsigned int)(max_usable - knee_usable);
}


/*
 * Cursor velocity in Q8.
 *
 * 256  = 1 pixel per controller poll
 * 1536 = 6 pixels per poll at full tilt
 */
int stick_velocity_q8(unsigned char value)
{
    int d = (int)value - 128;
    int sign;
    int raw;
    unsigned int curve;
    int velocity;

    if (d < 0) {
        sign = -1;
        raw = -d;
    } else {
        sign = 1;
        raw = d;
    }

    curve = analog_speed_curve_q12(raw);

    if (curve == 0)
        return 0;

    velocity =
        (1536 * (int)curve) /
        ANALOG_CURVE_ONE;

    /*
     * Less than one pixel per poll is intentional.
     * Q8 accumulation below lets this become genuinely slow motion.
     */
    /*
     * Permit extremely fine motion near center.
     * Q8 value 2 is about 0.47 pixel/sec at a 60 Hz poll rate.
     */
    if (velocity < 2)
        velocity = 2;

    return sign * velocity;
}

/*
 * Test11K controller scrolling.
 *
 * RFB PointerEvent button bits follow the conventional X11 mouse mapping:
 *
 *   button 4 = wheel up       = 0x08
 *   button 5 = wheel down     = 0x10
 *   button 6 = wheel left     = 0x20
 *   button 7 = wheel right    = 0x40
 *
 * A wheel "click" is represented as a button press followed immediately
 * by release at the same cursor position.
 */
#define RFB_SCROLL_UP     0x08
#define RFB_SCROLL_DOWN   0x10
#define RFB_SCROLL_LEFT   0x20
#define RFB_SCROLL_RIGHT  0x40

#define DPAD_SCROLL_REPEAT_POLLS  5


unsigned char scroll_button_for_stick(
    unsigned char h,
    unsigned char v,
    int *magnitude)
{
    int x = (int)h - 128;
    int y = (int)v - 128;
    int ax = (x < 0) ? -x : x;
    int ay = (y < 0) ? -y : y;

    *magnitude = 0;

    if (ax <= ANALOG_DEADZONE &&
        ay <= ANALOG_DEADZONE)
        return 0;

    /*
     * Dominant axis only.
     */
    if (ay >= ax) {
        *magnitude = ay;

        if (y < 0)
            return RFB_SCROLL_UP;

        return RFB_SCROLL_DOWN;
    }

    *magnitude = ax;

    if (x < 0)
        return RFB_SCROLL_LEFT;

    return RFB_SCROLL_RIGHT;
}


/*
 * Number of 16.7 ms controller polls to wait between wheel clicks.
 *
 * Stronger stick deflection therefore scrolls faster.
 */
unsigned int scroll_repeat_delay(int magnitude)
{
    unsigned int curve;
    unsigned int clicks_per_second_x10;
    unsigned int interval_polls;

    curve = analog_speed_curve_q12(magnitude);

    if (curve == 0)
        return 59;

    /*
     * Full tilt = ~20 wheel clicks/sec.
     *
     * At the 85% knee, the common curve is 30%, giving ~6 clicks/sec.
     *
     * Near center we floor at ~1 click/sec rather than making scrolling
     * disappear completely.
     */
    clicks_per_second_x10 =
        (200 * curve) /
        ANALOG_CURVE_ONE;

    /*
     * Near center allow scrolling as slowly as one wheel notch
     * about every two seconds.
     */
    if (clicks_per_second_x10 < 5)
        clicks_per_second_x10 = 5;

    /*
     * Controller polls at about 60 Hz.
     * x10 arithmetic keeps useful precision at low scroll speeds.
     */
    interval_polls =
        (600 + clicks_per_second_x10 / 2) /
        clicks_per_second_x10;

    if (interval_polls < 3)
        interval_polls = 3;

    return interval_polls - 1;
}


int send_scroll_pulse(
    int sock,
    unsigned char base_buttons,
    unsigned char scroll_button,
    int x,
    int y)
{
    /*
     * Preserve any ordinary mouse button already held while adding the
     * momentary wheel bit.
     */
    if (send_pointer_event(
            sock,
            base_buttons | scroll_button,
            (unsigned int)x,
            (unsigned int)y) < 0)
        return -1;

    if (send_pointer_event(
            sock,
            base_buttons,
            (unsigned int)x,
            (unsigned int)y) < 0)
        return -1;

    return 0;
}


/*
 * Test11K-G2 progressive D-pad acceleration.
 *
 * Controller loop is approximately 60 Hz.
 *
 * For the first second the D-pad retains its existing precision behavior.
 * After one second, elapsed hold time is mapped onto the SAME response
 * curve used by the analog stick.
 *
 * This gives:
 *
 *   tap       = 2 pixels
 *   short hold = gentle precision repeat
 *   >1 sec    = gradual analog-like acceleration
 *   ~2.5 sec  = full analog-stick top speed
 */
#define DPAD_ACCEL_START_TICKS   60
#define DPAD_ACCEL_RAMP_TICKS    90

#define DPAD_BASE_VELOCITY_Q8    128
#define DPAD_MAX_VELOCITY_Q8     1536

int dpad_hold_velocity_q8(unsigned int hold_ticks)
{
    unsigned int t;
    int raw;
    unsigned int curve;

    if (hold_ticks <= DPAD_ACCEL_START_TICKS)
        return DPAD_BASE_VELOCITY_Q8;

    t = hold_ticks - DPAD_ACCEL_START_TICKS;

    if (t >= DPAD_ACCEL_RAMP_TICKS)
        return DPAD_MAX_VELOCITY_Q8;

    /*
     * Convert elapsed acceleration time into an equivalent
     * analog-stick magnitude.
     *
     * t = 0                  -> deadzone edge
     * t = DPAD_ACCEL_RAMP    -> full stick
     */
    raw =
        ANALOG_DEADZONE +
        ((ANALOG_MAX_RAW - ANALOG_DEADZONE) *
         (int)t) /
        DPAD_ACCEL_RAMP_TICKS;

    curve = analog_speed_curve_q12(raw);

    /*
     * Start exactly at the existing held-D-pad average speed:
     *
     *     2 px / 4 polls = 0.5 px/poll = Q8 128
     *
     * Then accelerate smoothly to the analog maximum of
     * 6 px/poll = Q8 1536.
     */
    return
        DPAD_BASE_VELOCITY_Q8 +
        ((DPAD_MAX_VELOCITY_Q8 -
          DPAD_BASE_VELOCITY_Q8) *
         (int)curve) /
        ANALOG_CURVE_ONE;
}
