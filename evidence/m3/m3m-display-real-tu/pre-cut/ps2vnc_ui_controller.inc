
static void display_confirm_render_bitmap(void)
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


static void display_restore_ack_render_bitmap(void)
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



static int display_menu_tab_for_mode(
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


static void display_menu_normalize_selection(void)
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


static int display_menu_move_vertical_selection(
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


static int display_menu_move_horizontal_selection(
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


static void display_menu_render_bitmap(void)
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



static void system_menu_render_bitmap(void)
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


static void osk_move_horizontal(int direction)
{
    int count = osk_row_length(osk_row);

    osk_col += direction;

    if (osk_col < 0)
        osk_col = count - 1;
    else if (osk_col >= count)
        osk_col = 0;

    osk_generation++;
}

static void osk_move_vertical(int direction)
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

static int osk_send_key_with_modifiers(
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


static int osk_activate_selected(int sock)
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
static int wake_renderer_with_pointer_jiggle(
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
static int stick_velocity_q8(unsigned char value)
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


static unsigned char scroll_button_for_stick(
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
static unsigned int scroll_repeat_delay(int magnitude)
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


static int send_scroll_pulse(
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

static int dpad_hold_velocity_q8(unsigned int hold_ticks)
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


 void controller_thread(void *arg)
{
    int sock = *(int *)arg;
    int cursor_x = desktop_width / 2;
    int cursor_y = desktop_height / 2;
    unsigned char last_button_mask = 0;
    unsigned int last_pressed = 0;

    /*
     * Precision D-pad mouse movement:
     * immediate 2px step, then delayed repeat if held.
     */
    unsigned int dpad_hold_direction = 0;
    unsigned int dpad_hold_ticks = 0;

    /*
     * Fractional Q8 movement for accelerated D-pad motion.
     */
    int dpad_x_q8 = 0;
    int dpad_y_q8 = 0;

    /*
     * Fractional Q8 cursor motion.
     *
     * This is what permits genuinely slow movement near stick center:
     * several controller polls may be required to accumulate one pixel.
     */
    int analog_x_q8 = 0;
    int analog_y_q8 = 0;

    /*
     * Scroll repeat state survives from one controller poll to the next.
     */
    unsigned char last_scroll_mask = 0;
    unsigned int scroll_repeat_countdown = 0;
    unsigned int hotkey_observed_mask = 0;
    unsigned int hotkey_stable_polls = 0;
    int hotkey_active_action = RUNTIME_ACTION_NONE;
    int hotkey_active_trigger = 0;
    unsigned int hotkey_hold_polls = 0;
    int hotkey_hold_last_seconds = -1;
    unsigned int hotkey_release_armed_mask = 0;
    int hotkey_release_armed_action = RUNTIME_ACTION_NONE;
    int hotkey_latched = 0;
    int hotkey_desktop_session = 0;

    /*
     * Buttons consumed by a local PS2VNC UI stay quarantined from
     * remote-desktop input until those physical buttons are released.
     *
     * This is stronger than same-poll consumption: a Circle press that
     * closes a menu may still be physically down on the next controller
     * poll, when the desktop branch has become active again.
     */
    unsigned int local_ui_quarantine = 0;

    const struct timespec poll_delay = {
        0,
        16667000
    };

    for (;;) {
        struct padButtonStatus buttons;
        unsigned int pressed;
        unsigned int changed;
        unsigned char button_mask;
        int old_x = cursor_x;
        int old_y = cursor_y;
        int dx = 0;
        int dy = 0;

        unsigned char scroll_mask = 0;
        unsigned int scroll_delay = 0;
        int scroll_magnitude = 0;

        /*
         * G2-C pad ownership handoff.
         *
         * The acknowledgement is published only after this thread has
         * reached a point immediately before its next libpad access.
         * While acknowledged, this thread performs no padGetState/padRead
         * calls and therefore the main thread may temporarily own libpad.
         */
        if (controller_pad_pause_requested) {
            controller_pad_pause_ack = 1;

            while (controller_pad_pause_requested &&
                   !exit_requested) {
                nanosleep(&poll_delay, NULL);
            }

            controller_pad_pause_ack = 0;

            if (exit_requested) {
                ExitThread();
                return;
            }

            /*
             * The main thread will return ownership only after a clean
             * physical release. Forget every pre-handoff edge/hold state so
             * no calibration button can become desktop input afterward.
             */
            last_pressed = 0;
            last_button_mask = 0;

            dpad_hold_direction = 0;
            dpad_hold_ticks = 0;
            dpad_x_q8 = 0;
            dpad_y_q8 = 0;

            analog_x_q8 = 0;
            analog_y_q8 = 0;

            last_scroll_mask = 0;
            scroll_repeat_countdown = 0;

            hotkey_observed_mask = 0;
            hotkey_stable_polls = 0;
            hotkey_active_action = RUNTIME_ACTION_NONE;
            hotkey_active_trigger = 0;
            hotkey_hold_polls = 0;
            hotkey_hold_last_seconds = -1;
            hotkey_release_armed_mask = 0;
            hotkey_release_armed_action = RUNTIME_ACTION_NONE;
            hotkey_latched = 0;
            hotkey_desktop_session = 0;

            local_ui_quarantine = 0;

            continue;
        }

        controller_loop_counter++;

        if (ps2vnc_system_refresh_update_state() &&
            system_menu_visible) {

            (void)wake_renderer_with_pointer_jiggle(
                sock, cursor_x, cursor_y);
        }

        if (padGetState(0, 0) == PAD_STATE_STABLE &&
            padRead(0, 0, &buttons) != 0) {

            pressed = 0xFFFFu ^ (unsigned int)buttons.btns;
            changed = pressed ^ last_pressed;

            /*
             * Release is the only thing that clears local ownership.
             * Held buttons therefore cannot leak into the remote desktop
             * after a local menu closes.
             */
            local_ui_quarantine &= pressed;

            /*
             * TEST15E3-G generic runtime-hotkey arbitration.
             *
             * Only buttons participating in runtime bindings are sampled.
             * The exact held mask must remain unchanged for a short settle
             * interval before a multi-button binding owns the session.
             *
             * Once an action actually fires, the hotkey session remains
             * latched until all hotkey buttons are released.  Therefore a
             * shrinking chord cannot accidentally fire one of its subsets.
             *
             * Desktop-only bindings never delay OSK or local-menu controls.
             * Global Refresh/Exit bindings remain observable in every context.
             */
            {
                unsigned int hk =
                    pressed & RUNTIME_HOTKEY_BUTTON_MASK;

                int desktop_hotkey_context =
                    !system_menu_visible &&
                    !display_menu_visible &&
                    !osk_visible;

                if (hotkey_latched) {
                    if (hk == 0) {
                        hotkey_latched = 0;
                        hotkey_observed_mask = 0;
                        hotkey_stable_polls = 0;
                        hotkey_active_action = RUNTIME_ACTION_NONE;
                        hotkey_active_trigger = 0;
                        hotkey_hold_polls = 0;
                        hotkey_hold_last_seconds = -1;
                        hotkey_release_armed_mask = 0;
                        hotkey_release_armed_action =
                            RUNTIME_ACTION_NONE;
                        hotkey_desktop_session = 0;
                        debug_hotkey_settle_target = 0;
                    }

                } else if (hk != hotkey_observed_mask) {
                    unsigned int old_mask = hotkey_observed_mask;
                    const ps2vnc_hotkey_binding_t *new_release = NULL;
                    int release_action = RUNTIME_ACTION_NONE;
                    int pure_growth =
                        ((old_mask & hk) == old_mask);
                    int pure_shrink =
                        ((hk & old_mask) == hk);

                    /*
                     * Record where a new hotkey session began.  A button
                     * pressed while the OSK/menu owns input must never turn
                     * into a desktop shortcut merely because that context
                     * changes before the button is released.
                     */
                    if (hotkey_observed_mask == 0 && hk != 0)
                        hotkey_desktop_session =
                            desktop_hotkey_context;

                    /*
                     * An armed RELEASE binding fires only when its exact mask
                     * is broken by releasing buttons. Growing the gesture
                     * cancels it instead, preventing subset-action leakage.
                     */
                    if (hotkey_release_armed_action !=
                            RUNTIME_ACTION_NONE) {
                        if (hotkey_release_armed_mask == old_mask &&
                            pure_shrink &&
                            desktop_hotkey_context &&
                            hotkey_desktop_session) {
                            release_action =
                                hotkey_release_armed_action;
                        }

                        hotkey_release_armed_mask = 0;
                        hotkey_release_armed_action =
                            RUNTIME_ACTION_NONE;
                    }

                    hotkey_observed_mask = hk;
                    hotkey_stable_polls = 0;
                    hotkey_active_action = RUNTIME_ACTION_NONE;
                    hotkey_active_trigger = 0;
                    hotkey_hold_polls = 0;
                    hotkey_hold_last_seconds = -1;
                    debug_hotkey_settle_target = 0;

                    /*
                     * Changing a pending hold chord cancels its countdown,
                     * but deliberately leaves the System menu visible.
                     */
                    if (system_menu_countdown_action !=
                            RUNTIME_ACTION_NONE) {
                        system_menu_countdown_action =
                            RUNTIME_ACTION_NONE;
                        system_menu_countdown_seconds = 0;
                        osk_generation++;
                    }

                    /*
                     * Arm RELEASE only when arriving at its exact mask by
                     * adding buttons. Shrinking from a larger chord cannot
                     * manufacture a release shortcut.
                     */
                    if (pure_growth && hk != 0) {
                        new_release =
                            runtime_hotkey_lookup(
                                hk,
                                desktop_hotkey_context &&
                                hotkey_desktop_session);

                        if (new_release != NULL &&
                            new_release->trigger ==
                                HOTKEY_TRIGGER_RELEASE) {
                            hotkey_release_armed_mask = hk;
                            hotkey_release_armed_action =
                                new_release->action;
                        }
                    }

                    if (release_action != RUNTIME_ACTION_NONE) {
                        unsigned int keysym = 0;

                        if (release_action ==
                                RUNTIME_ACTION_SEND_TAB)
                            keysym = XK_TAB;
                        else if (release_action ==
                                 RUNTIME_ACTION_SEND_ENTER)
                            keysym = XK_RETURN;

                        if (keysym != 0 &&
                            send_key_tap(sock, keysym) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }

                        if (keysym != 0)
                            (void)wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y);
                    }

                    if (hk == 0)
                        hotkey_desktop_session = 0;

                } else if (hk != 0) {
                    const ps2vnc_hotkey_binding_t *binding;
                    unsigned int settle_polls =
                        RUNTIME_HOTKEY_SETTLE_POLLS;

                    hotkey_stable_polls++;

                    binding =
                        runtime_hotkey_lookup(
                            hk,
                            desktop_hotkey_context &&
                            hotkey_desktop_session);

                    /*
                     * Only ambiguous settle-actions get the longer collection
                     * window.  Maximal chords and HOLD bindings retain the
                     * normal short settle.
                     */
                    if (binding != NULL &&
                        binding->trigger == HOTKEY_TRIGGER_SETTLE &&
                        runtime_hotkey_has_hold_extension(
                            hk,
                            desktop_hotkey_context &&
                            hotkey_desktop_session)) {

                        settle_polls =
                            RUNTIME_HOTKEY_EXTENDABLE_SETTLE_POLLS;
                    }

                    debug_hotkey_settle_target = settle_polls;

                    if (hotkey_active_action ==
                            RUNTIME_ACTION_NONE &&
                        binding != NULL &&
                        binding->trigger != HOTKEY_TRIGGER_RELEASE &&
                        hotkey_stable_polls >= settle_polls) {

                        hotkey_active_action = binding->action;
                        hotkey_active_trigger = binding->trigger;

                        if (binding->trigger ==
                                HOTKEY_TRIGGER_SETTLE) {

                            if (binding->action ==
                                    RUNTIME_ACTION_SEND_ENTER) {

                                if (send_key_tap(
                                        sock, XK_RETURN) < 0) {
                                    controller_thread_error = 1;
                                    ExitThread();
                                }

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);

                            } else if (binding->action ==
                                       RUNTIME_ACTION_DISPLAY_SETTINGS) {

                                system_menu_visible = 0;
                                system_menu_status =
                                    SYSTEM_MENU_STATUS_NONE;

                                display_menu_open_for_active(0);

                                if (last_button_mask != 0) {
                                    (void)send_pointer_event(
                                        sock, 0,
                                        (unsigned int)cursor_x,
                                        (unsigned int)cursor_y);
                                    last_button_mask = 0;
                                }

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);
                            }

                            hotkey_latched = 1;

                        } else if (binding->trigger ==
                                   HOTKEY_TRIGGER_HOLD) {

                            if (!ps2vnc_system_overlay_allowed()) {

                                hotkey_active_action =
                                    RUNTIME_ACTION_NONE;
                                hotkey_active_trigger = 0;
                                hotkey_latched = 1;

                            } else {

                                /*
                                 * GLOBAL OVERLAY:
                                 *
                                 * Do not clear Display Settings or the OSK.
                                 * Existing renderer/input priority makes
                                 * System temporarily own the foreground while
                                 * preserving the ordinary UI underneath.
                                 */
                                system_menu_visible = 1;

                                system_menu_status =
                                    SYSTEM_MENU_STATUS_NONE;

                                system_menu_selection =
                                    (binding->action ==
                                     RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU)
                                    ? SYSTEM_MENU_EXIT
                                    : SYSTEM_MENU_REFRESH;

                                system_menu_countdown_action =
                                    binding->action;
                                system_menu_countdown_seconds = 2;
                                hotkey_hold_polls = 0;
                                hotkey_hold_last_seconds = -1;
                                osk_generation++;

                                (void)wake_renderer_with_pointer_jiggle(
                                    sock, cursor_x, cursor_y);
                            }
                        }
                    }

                    if (hotkey_active_trigger ==
                            HOTKEY_TRIGGER_HOLD) {
                        unsigned int remaining;

                        hotkey_hold_polls++;

                        remaining =
                            (SYSTEM_CHORD_HOLD_POLLS >
                             hotkey_hold_polls)
                            ? ((SYSTEM_CHORD_HOLD_POLLS -
                                hotkey_hold_polls + 59u) / 60u)
                            : 0u;

                        if ((int)remaining !=
                                hotkey_hold_last_seconds) {
                            hotkey_hold_last_seconds = (int)remaining;
                            system_menu_countdown_seconds = remaining;
                            osk_generation++;
                        }

                        if (hotkey_hold_polls >=
                                SYSTEM_CHORD_HOLD_POLLS) {

                            system_menu_countdown_action =
                                RUNTIME_ACTION_NONE;
                            system_menu_countdown_seconds = 0;
                            osk_generation++;
                            hotkey_latched = 1;

                            if (hotkey_active_action ==
                                    RUNTIME_ACTION_REFRESH_RFB) {

                                (void)
                                    ps2vnc_system_refresh_accept_request();

                            } else if (hotkey_active_action ==
                                       RUNTIME_ACTION_EXIT_TO_SYSTEM_MENU) {

                                if (ps2vnc_system_exit_accept_request()) {

                                    debug_hotkey_mask =
                                        hotkey_observed_mask;
                                    debug_hotkey_stable_polls =
                                        hotkey_stable_polls;
                                    debug_hotkey_action =
                                        hotkey_active_action;
                                    debug_hotkey_trigger =
                                        hotkey_active_trigger;
                                    debug_hotkey_hold_polls =
                                        hotkey_hold_polls;
                                    debug_hotkey_latched =
                                        hotkey_latched;
                                    debug_hotkey_quarantine =
                                        local_ui_quarantine;

                                    ExitThread();
                                    return;
                                }
                            }
                        }
                    }
                }
            }

            debug_hotkey_mask = hotkey_observed_mask;
            debug_hotkey_stable_polls = hotkey_stable_polls;
            debug_hotkey_action = hotkey_active_action;
            debug_hotkey_trigger = hotkey_active_trigger;
            debug_hotkey_hold_polls = hotkey_hold_polls;
            debug_hotkey_latched = hotkey_latched;
            debug_hotkey_quarantine = local_ui_quarantine;

            /*
             * Display confirmation owns a real wall-clock deadline.
             * It starts before the hazardous switch; while the candidate
             * switch is still incomplete the UI remains hidden, but timeout
             * can already publish ROLLBACK and interrupt recv_exact().
             */
            if (display_confirm_pending &&
                display_confirm_action == DISPLAY_CONFIRM_ACTION_NONE &&
                (remote_display_state != REMOTE_DISPLAY_ACTIVE_TEMP ||
                 display_transition_switch_in_progress)) {

                unsigned int remaining =
                    display_confirm_remaining_seconds();

                if (remaining != display_confirm_seconds) {
                    display_confirm_seconds = remaining;

                    if (display_confirm_visible) {
                        osk_generation++;
                        (void)wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y);
                    }
                }

                if (remaining == 0 &&
                    display_confirm_pending &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE) {

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;
                    display_confirm_visible = 0;
                    osk_generation++;

                    /*
                     * H4C3D16 hard-stall fail-safe.
                     *
                     * Normally the main thread consumes ROLLBACK and restores
                     * A in-process.  If it is still trapped inside the risky
                     * synchronous mode reconstruction, it cannot consume that
                     * request.  Do not touch GS or RFB from this controller
                     * thread; replace the wedged process through the same
                     * established OSDSYS path used by main().
                     *
                     * The durable transaction intentionally remains
                     * provisional.  Existing startup reconciliation therefore
                     * makes the preserved previous profile authoritative.
                     */
                    if (display_transition_switch_in_progress)
                        LoadExecPS2("rom0:OSDSYS", 0, NULL);
                }
            }

            if (display_restore_ack_visible) {
                int ack_changed = 0;

                /*
                 * The restored acknowledgement owns these local UI buttons.
                 * Nothing may leak through to VNC or the menus underneath.
                 */
                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (!display_restore_ack_request &&
                    (changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    display_restore_ack_request = 1;
                    ack_changed = 1;
                }

                if (ack_changed) {
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }

            } else /*
         * D17AL-F8J2-B4A:
         * Remote transitions now take true libpad ownership at the
         * transition boundary. Do not quarantine ordinary desktop
         * controller input for the complete ACTIVE_TEMP dwell.
         */
        if (display_confirm_pending) {
                int menu_changed = 0;

                /*
                 * F8J2-B3C:
                 *
                 * Remote provisional display control owns the controller
                 * through the SAME decision-tree boundary as the proven local
                 * display-confirmation transaction.
                 *
                 * A non-IDLE remote transaction therefore cannot fall through
                 * to desktop pointer/key/scroll generation.  Physical pad
                 * polling and the existing local/global hotkey machinery stay
                 * alive exactly as they do for the local confirmation UI.
                 *
                 * Human KEEP / GO BACK actions remain local-only below.
                 */

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    display_confirm_visible &&
                    (((changed & PAD_UP) && (pressed & PAD_UP)) ||
                    ((changed & PAD_DOWN) && (pressed & PAD_DOWN)) ||
                    ((changed & PAD_LEFT) && (pressed & PAD_LEFT)) ||
                    ((changed & PAD_RIGHT) && (pressed & PAD_RIGHT)))) {

                    display_confirm_selection =
                        (display_confirm_selection ==
                         DISPLAY_CONFIRM_GO_BACK)
                        ? DISPLAY_CONFIRM_KEEP
                        : DISPLAY_CONFIRM_GO_BACK;

                    osk_generation++;
                    menu_changed = 1;
                }

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    (changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    display_confirm_action =
                        DISPLAY_CONFIRM_ACTION_ROLLBACK;
                    display_confirm_visible = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (remote_display_state == REMOTE_DISPLAY_IDLE &&
                    display_confirm_action ==
                        DISPLAY_CONFIRM_ACTION_NONE &&
                    (changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    display_confirm_action =
                        (display_confirm_selection ==
                         DISPLAY_CONFIRM_KEEP)
                        ? DISPLAY_CONFIRM_ACTION_KEEP
                        : DISPLAY_CONFIRM_ACTION_ROLLBACK;

                    display_confirm_visible = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (menu_changed) {
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }

            } else if (system_menu_visible) {
                int menu_changed = 0;

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS);

                if (((changed & PAD_UP) && (pressed & PAD_UP)) ||
                    ((changed & PAD_LEFT) && (pressed & PAD_LEFT))) {

                    system_menu_selection =
                        (system_menu_selection +
                         SYSTEM_MENU_ITEM_COUNT - 1) %
                        SYSTEM_MENU_ITEM_COUNT;

                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if (((changed & PAD_DOWN) && (pressed & PAD_DOWN)) ||
                    ((changed & PAD_RIGHT) && (pressed & PAD_RIGHT))) {

                    system_menu_selection =
                        (system_menu_selection + 1) %
                        SYSTEM_MENU_ITEM_COUNT;

                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    system_menu_visible = 0;
                    system_menu_status =
                        SYSTEM_MENU_STATUS_NONE;
                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    system_menu_countdown_action =
                        RUNTIME_ACTION_NONE;
                    system_menu_countdown_seconds = 0;

                    if (system_menu_selection ==
                        SYSTEM_MENU_REFRESH) {

                        (void)
                            ps2vnc_system_refresh_accept_request();

                    } else if (system_menu_selection ==
                               SYSTEM_MENU_DISPLAY) {

                        system_menu_visible = 0;
                        system_menu_status =
                            SYSTEM_MENU_STATUS_NONE;

                        if (display_menu_visible) {

                            /*
                             * Display Settings was already the preserved
                             * underlay. Reveal it exactly where it was.
                             */
                            osk_generation++;
                            menu_changed = 1;

                        } else {

                            /*
                             * Deliberate navigation from desktop/OSK into
                             * Display Settings. The previous OSK is not part
                             * of this new navigation branch.
                             */
                            osk_visible = 0;
                            display_menu_open_for_active(1);
                            menu_changed = 1;
                        }

                    } else {

                        if (ps2vnc_system_exit_accept_request()) {
                            ExitThread();
                            return;
                        }

                        osk_generation++;
                        menu_changed = 1;
                    }
                }

                if (menu_changed) {
                    /*
                     * Local overlay wake is best-effort. Never sacrifice the
                     * controller thread merely because the VNC stream is bad.
                     */
                    (void)wake_renderer_with_pointer_jiggle(
                        sock, cursor_x, cursor_y);
                }


            } else if (display_menu_visible) {

                int menu_changed = 0;

                /*
                 * Square is now owned locally while Display Settings is open.
                 * Outside this menu it remains the desktop Backspace shortcut.
                 */
                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_UP | PAD_DOWN |
                     PAD_LEFT | PAD_RIGHT |
                     PAD_CIRCLE | PAD_CROSS |
                     PAD_SQUARE);

                if ((changed & PAD_UP) &&
                    (pressed & PAD_UP)) {

                    display_menu_selection =
                        display_menu_move_vertical_selection(
                            display_menu_selection,
                            -1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_DOWN) &&
                    (pressed & PAD_DOWN)) {

                    display_menu_selection =
                        display_menu_move_vertical_selection(
                            display_menu_selection,
                            1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_LEFT) &&
                    (pressed & PAD_LEFT)) {

                    display_menu_selection =
                        display_menu_move_horizontal_selection(
                            display_menu_selection,
                            -1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_RIGHT) &&
                    (pressed & PAD_RIGHT)) {

                    display_menu_selection =
                        display_menu_move_horizontal_selection(
                            display_menu_selection,
                            1);

                    display_menu_error = 0;
                    display_policy_error = 0;
                    osk_generation++;
                    menu_changed = 1;
                }

                if ((changed & PAD_CIRCLE) &&
                    (pressed & PAD_CIRCLE)) {

                    display_menu_visible = 0;
                    display_menu_error = 0;
                    display_policy_error = 0;

                    if (display_menu_parent_system) {
                        display_menu_parent_system = 0;
                        system_menu_selection =
                            SYSTEM_MENU_DISPLAY;
                        system_menu_visible = 1;
                    }

                    osk_generation++;
                    menu_changed = 1;
                }

                /*
                 * Square toggles per-mode admission policy.
                 *
                 * The current active mode is deliberately not lockable from
                 * the UI. Unlocking does not switch into the mode.
                 */
                if ((changed & PAD_SQUARE) &&
                    (pressed & PAD_SQUARE) &&
                    display_menu_tab !=
                        DISPLAY_TAB_ADVANCED) {

                    const ps2vnc_video_mode_t *selected_mode =
                        display_menu_mode_at_selection(
                            display_menu_selection);

                    if (selected_mode != NULL &&
                        selected_mode != active_video_mode &&
                        display_policy_loaded &&
                        display_policy_request ==
                            DISPLAY_POLICY_REQUEST_NONE) {

                        display_policy_error = 0;

                        display_policy_request_selection =
                            display_menu_selection;

                        display_policy_request_value =
                            display_mode_locked[
                                display_menu_selection]
                                ? 0
                                : 1;

                        display_policy_request =
                            DISPLAY_POLICY_REQUEST_LOCK;

                        osk_generation++;
                        menu_changed = 1;
                    }
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {

                    /*
                     * ADVANCED currently contains one setting row.
                     */
                    if (display_menu_tab ==
                            DISPLAY_TAB_ADVANCED) {

                        if (display_policy_loaded &&
                            display_policy_request ==
                                DISPLAY_POLICY_REQUEST_NONE) {

                            display_policy_error = 0;

                            display_policy_request_value =
                                display_hide_locked_modes
                                    ? 0
                                    : 1;

                            display_policy_request =
                                DISPLAY_POLICY_REQUEST_HIDE_LOCKED;

                            osk_generation++;
                            menu_changed = 1;
                        }

                    } else {

                        const ps2vnc_video_mode_t *selected_mode =
                            display_menu_mode_at_selection(
                                display_menu_selection);

                        if (selected_mode == NULL) {
                            /*
                             * Empty filtered tab: hard no-op.
                             */

                        } else if (selected_mode ==
                                      active_video_mode) {

                            display_menu_visible = 0;
                            display_menu_error = 0;
                            display_policy_error = 0;
                            display_menu_parent_system = 0;

                            display_calibration_request =
                                display_mode_request_for_selection(
                                    display_menu_selection);

                            osk_generation++;
                            menu_changed = 1;

                        } else if (!display_policy_loaded) {

                            /*
                             * Never switch into an inactive mode without
                             * knowing authoritative lock policy.
                             */
                            display_policy_error = 1;
                            osk_generation++;
                            menu_changed = 1;

                        } else if (display_mode_locked[
                                      display_menu_selection]) {

                            /*
                             * HARD NO-OP:
                             *
                             * No transaction request, no management resize,
                             * no GS operation, no startup persistence.
                             */
                            osk_generation++;
                            menu_changed = 1;

                        } else {

                            display_menu_visible = 0;
                            display_menu_error = 0;
                            display_policy_error = 0;
                            display_menu_parent_system = 0;

                            display_mode_request_remote = 0;
                            display_mode_request =
                                display_mode_request_for_selection(
                                    display_menu_selection);

                            osk_generation++;
                            menu_changed = 1;
                        }
                    }
                }

                if (menu_changed) {
                    if (wake_renderer_with_pointer_jiggle(
                            sock,
                            cursor_x,
                            cursor_y) < 0) {

                        controller_thread_error = 1;
                        ExitThread();
                    }
                }

            } else if (!osk_visible) {

                unsigned int desktop_pressed =
                    pressed & ~local_ui_quarantine;
                unsigned int desktop_changed =
                    desktop_pressed ^
                    (last_pressed & ~local_ui_quarantine);

                /*
                 * Open the OSK with Select.  Reset to Q for predictable
                 * entry and ensure no mouse button remains logically held.
                 */
                if ((desktop_changed & PAD_SELECT) &&
                    (desktop_pressed & PAD_SELECT)) {

                    osk_visible = 1;
                    osk_page = OSK_PAGE_ABC;
                    osk_row = 1;
                    osk_col = 0;
                    osk_shift = 0;
                    osk_ctrl = 0;
                    osk_alt = 0;
                    osk_generation++;

                    if (last_button_mask != 0) {
                        if (send_pointer_event(
                                sock, 0,
                                (unsigned int)cursor_x,
                                (unsigned int)cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        last_button_mask = 0;
                    }

                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }

                    /*
                     * last_pressed always tracks the raw physical pad state.
                     * Quarantine affects consumers, never edge bookkeeping.
                     */
                    last_pressed = pressed;
                    nanosleep(&poll_delay, NULL);
                    continue;
                }

                /*
                 * Quick keyboard shortcuts remain available while using
                 * normal mouse mode.
                 */
                if ((desktop_changed & PAD_SQUARE) &&
                    (desktop_pressed & PAD_SQUARE)) {
                    if (send_key_tap(sock, XK_BACKSPACE) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                }

                /*
                 * R1=Tab and L1+Start=Enter are resolved by the generic
                 * runtime-hotkey arbiter above.
                 */

                /*
                 * TEST13I:
                 * Triangle + D-pad = mouse-wheel scrolling.
                 *
                 * This takes priority over L1+D-pad and ordinary D-pad
                 * precision mouse movement. The OSK has its own input branch,
                 * so Triangle continues to mean Shift while the OSK is open.
                 *
                 * Vertical directions take priority over horizontal directions
                 * if a diagonal D-pad combination is reported.
                 */
                if (desktop_pressed & PAD_TRIANGLE) {
                    unsigned int dpad =
                        desktop_pressed &
                        (PAD_LEFT | PAD_RIGHT |
                         PAD_UP | PAD_DOWN);

                    /*
                     * Do not preserve any fractional/hold state from ordinary
                     * D-pad mouse movement while Triangle owns the D-pad.
                     */
                    dpad_hold_direction = 0;
                    dpad_hold_ticks = 0;
                    dpad_x_q8 = 0;
                    dpad_y_q8 = 0;

                    if ((dpad & PAD_UP) &&
                        !(dpad & PAD_DOWN)) {
                        scroll_mask = RFB_SCROLL_UP;
                    } else if ((dpad & PAD_DOWN) &&
                               !(dpad & PAD_UP)) {
                        scroll_mask = RFB_SCROLL_DOWN;
                    } else if ((dpad & PAD_LEFT) &&
                               !(dpad & PAD_RIGHT)) {
                        scroll_mask = RFB_SCROLL_LEFT;
                    } else if ((dpad & PAD_RIGHT) &&
                               !(dpad & PAD_LEFT)) {
                        scroll_mask = RFB_SCROLL_RIGHT;
                    }

                    if (scroll_mask != 0)
                        scroll_delay = DPAD_SCROLL_REPEAT_POLLS;

                /*
                 * Test11F: L1 is a physical chord modifier, not a mode.
                 * No state is remembered and nothing can remain toggled.
                 */
                } else if (desktop_pressed & PAD_L1) {
                    if ((desktop_pressed & PAD_UP) &&
                        (desktop_changed & (PAD_L1 | PAD_UP))) {
                        if (send_key_tap(sock, XK_UP) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_DOWN) &&
                        (desktop_changed & (PAD_L1 | PAD_DOWN))) {
                        if (send_key_tap(sock, XK_DOWN) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_LEFT) &&
                        (desktop_changed & (PAD_L1 | PAD_LEFT))) {
                        if (send_key_tap(sock, XK_LEFT) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }

                    if ((desktop_pressed & PAD_RIGHT) &&
                        (desktop_changed & (PAD_L1 | PAD_RIGHT))) {
                        if (send_key_tap(sock, XK_RIGHT) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                        if (wake_renderer_with_pointer_jiggle(
                                sock, cursor_x, cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }
                    }
                } else {
                    /* Normal D-pad = precise mouse movement. */
                    {
                        unsigned int dpad =
                            desktop_pressed &
                            (PAD_LEFT | PAD_RIGHT |
                             PAD_UP | PAD_DOWN);

                        unsigned int dpad_new =
                            desktop_changed &
                            desktop_pressed &
                            (PAD_LEFT | PAD_RIGHT |
                             PAD_UP | PAD_DOWN);

                        int do_dpad_step = 0;

                        if (dpad_new != 0) {
                            /*
                             * Fresh tap: exactly two pixels.
                             */
                            dpad_hold_direction = dpad;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;
                            do_dpad_step = 1;

                        } else if (dpad == 0) {
                            dpad_hold_direction = 0;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;

                        } else if (dpad != dpad_hold_direction) {
                            /*
                             * Direction change behaves like a fresh tap.
                             */
                            dpad_hold_direction = dpad;
                            dpad_hold_ticks = 0;
                            dpad_x_q8 = 0;
                            dpad_y_q8 = 0;
                            do_dpad_step = 1;

                        } else {
                            dpad_hold_ticks++;

                            if (dpad_hold_ticks <
                                DPAD_ACCEL_START_TICKS) {

                                /*
                                 * Preserve 11K-F exactly during the
                                 * precision phase:
                                 *
                                 * ~300 ms delay, then 2 pixels every
                                 * four controller polls.
                                 */
                                if (dpad_hold_ticks >= 18 &&
                                    ((dpad_hold_ticks - 18) % 4) == 0)
                                    do_dpad_step = 1;

                            } else {
                                int velocity =
                                    dpad_hold_velocity_q8(
                                        dpad_hold_ticks);

                                /*
                                 * After one second, use fractional
                                 * continuous movement so velocity can
                                 * accelerate smoothly.
                                 */
                                if (dpad & PAD_LEFT)
                                    dpad_x_q8 -= velocity;

                                if (dpad & PAD_RIGHT)
                                    dpad_x_q8 += velocity;

                                if (dpad & PAD_UP)
                                    dpad_y_q8 -= velocity;

                                if (dpad & PAD_DOWN)
                                    dpad_y_q8 += velocity;

                                while (dpad_x_q8 >= 256) {
                                    dx++;
                                    dpad_x_q8 -= 256;
                                }

                                while (dpad_x_q8 <= -256) {
                                    dx--;
                                    dpad_x_q8 += 256;
                                }

                                while (dpad_y_q8 >= 256) {
                                    dy++;
                                    dpad_y_q8 -= 256;
                                }

                                while (dpad_y_q8 <= -256) {
                                    dy--;
                                    dpad_y_q8 += 256;
                                }
                            }
                        }

                        if (do_dpad_step) {
                            if (dpad & PAD_LEFT)  dx -= 2;
                            if (dpad & PAD_RIGHT) dx += 2;
                            if (dpad & PAD_UP)    dy -= 2;
                            if (dpad & PAD_DOWN)  dy += 2;
                        }
                    }
                }

                /*
                 * Left analog normally remains accelerated mouse movement.
                 *
                 * While L3 is held, however, the same stick becomes an
                 * analog mouse wheel and does NOT move the cursor.
                 */
                if ((buttons.mode & 0xF0) == 0x70) {
                    if (desktop_pressed & PAD_L3) {
                        /*
                         * L3 is a hard pointer freeze.
                         *
                         * Clear fractional mouse motion as soon as scrolling
                         * begins so releasing L3 cannot reveal an old partial
                         * cursor movement.
                         */
                        analog_x_q8 = 0;
                        analog_y_q8 = 0;

                        /*
                         * Triangle+D-pad has explicit priority. Only derive
                         * scrolling from the analog stick if no D-pad wheel
                         * direction was selected above.
                         */
                        if (scroll_mask == 0) {
                            scroll_mask = scroll_button_for_stick(
                                buttons.ljoy_h,
                                buttons.ljoy_v,
                                &scroll_magnitude
                            );

                            if (scroll_mask != 0) {
                                scroll_delay =
                                    scroll_repeat_delay(scroll_magnitude);
                            }
                        }
                    } else {
                        int vx =
                            stick_velocity_q8(buttons.ljoy_h);

                        int vy =
                            stick_velocity_q8(buttons.ljoy_v);

                        /*
                         * Returning an axis to center immediately clears its
                         * fractional remainder.
                         */
                        if (vx == 0)
                            analog_x_q8 = 0;
                        else
                            analog_x_q8 += vx;

                        if (vy == 0)
                            analog_y_q8 = 0;
                        else
                            analog_y_q8 += vy;

                        while (analog_x_q8 >= 256) {
                            dx++;
                            analog_x_q8 -= 256;
                        }

                        while (analog_x_q8 <= -256) {
                            dx--;
                            analog_x_q8 += 256;
                        }

                        while (analog_y_q8 >= 256) {
                            dy++;
                            analog_y_q8 -= 256;
                        }

                        while (analog_y_q8 <= -256) {
                            dy--;
                            analog_y_q8 += 256;
                        }
                    }
                }

                cursor_x += dx;
                cursor_y += dy;

                if (cursor_x < 0)
                    cursor_x = 0;
                if (cursor_x >= (int)desktop_width)
                    cursor_x = (int)desktop_width - 1;

                if (cursor_y < 0)
                    cursor_y = 0;
                if (cursor_y >= (int)desktop_height)
                    cursor_y = (int)desktop_height - 1;

                button_mask =
                    ((desktop_pressed & PAD_CROSS)  ? 0x01 : 0x00) |
                    ((desktop_pressed & PAD_CIRCLE) ? 0x04 : 0x00);

                if (cursor_x != old_x ||
                    cursor_y != old_y ||
                    button_mask != last_button_mask) {

                    if (send_pointer_event(
                            sock,
                            button_mask,
                            (unsigned int)cursor_x,
                            (unsigned int)cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }

                    last_button_mask = button_mask;
                }

                /*
                 * Test11K scroll mode.
                 *
                 * Direction changes fire immediately. Holding the same
                 * direction repeats according to analog deflection.
                 */
                if (scroll_mask != 0) {
                    if (scroll_mask != last_scroll_mask)
                        scroll_repeat_countdown = 0;

                    if (scroll_repeat_countdown == 0) {
                        if (send_scroll_pulse(
                                sock,
                                button_mask,
                                scroll_mask,
                                cursor_x,
                                cursor_y) < 0) {
                            controller_thread_error = 1;
                            ExitThread();
                        }

                        scroll_repeat_countdown = scroll_delay;
                    } else {
                        scroll_repeat_countdown--;
                    }

                    last_scroll_mask = scroll_mask;
                } else {
                    last_scroll_mask = 0;
                    scroll_repeat_countdown = 0;
                }
            } else {
                int osk_changed = 0;

                local_ui_quarantine |=
                    changed & pressed &
                    (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN |
                     PAD_TRIANGLE | PAD_CROSS | PAD_SQUARE |
                     PAD_START | PAD_R1 | PAD_CIRCLE | PAD_SELECT);

                /*
                 * While the OSK is open the D-pad controls the keyboard,
                 * not the remote mouse.  Movement is edge-triggered for
                 * this first version.
                 */
                if ((changed & PAD_LEFT) &&
                    (pressed & PAD_LEFT)) {
                    osk_move_horizontal(-1);
                    osk_changed = 1;
                }

                if ((changed & PAD_RIGHT) &&
                    (pressed & PAD_RIGHT)) {
                    osk_move_horizontal(1);
                    osk_changed = 1;
                }

                if ((changed & PAD_UP) &&
                    (pressed & PAD_UP)) {
                    osk_move_vertical(-1);
                    osk_changed = 1;
                }

                if ((changed & PAD_DOWN) &&
                    (pressed & PAD_DOWN)) {
                    osk_move_vertical(1);
                    osk_changed = 1;
                }

                if ((changed & PAD_TRIANGLE) &&
                    (pressed & PAD_TRIANGLE)) {
                    osk_shift = !osk_shift;
                    osk_generation++;
                    osk_changed = 1;
                }

                if ((changed & PAD_CROSS) &&
                    (pressed & PAD_CROSS)) {
                    if (osk_activate_selected(sock) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_SQUARE) &&
                    (pressed & PAD_SQUARE)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_BACKSPACE) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_START) &&
                    (pressed & PAD_START)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_RETURN) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if ((changed & PAD_R1) &&
                    (pressed & PAD_R1)) {
                    if (osk_send_key_with_modifiers(
                            sock, XK_TAB) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                    osk_changed = 1;
                }

                if (((changed & PAD_CIRCLE) &&
                     (pressed & PAD_CIRCLE)) ||
                    ((changed & PAD_SELECT) &&
                     (pressed & PAD_SELECT))) {

                    osk_visible = 0;
                    osk_shift = 0;
                    osk_ctrl = 0;
                    osk_alt = 0;
                    osk_generation++;
                    osk_changed = 1;
                }

                /*
                 * Purely local keyboard changes need to wake the main
                 * renderer, which may be blocked in an incremental VNC
                 * request.  Actual typed keys normally cause a remote
                 * redraw themselves.
                 */
                if (osk_changed) {
                    if (wake_renderer_with_pointer_jiggle(
                            sock, cursor_x, cursor_y) < 0) {
                        controller_thread_error = 1;
                        ExitThread();
                    }
                }
            }

            last_pressed = pressed;
        }

        nanosleep(&poll_delay, NULL);
    }
}
