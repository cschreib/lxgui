#include "lxgui/input_keys.hpp"

namespace lxgui::input {

std::string_view get_mouse_button_codename(mouse_button button_id) {
    switch (button_id) {
    case mouse_button::left: return "LeftButton";
    case mouse_button::right: return "RightButton";
    case mouse_button::middle: return "MiddleButton";
    default: return "";
    }
}

std::string_view get_localizable_mouse_button_name(mouse_button button_id) {
    switch (button_id) {
    case mouse_button::left: return "{mouse_left}";
    case mouse_button::right: return "{mouse_right}";
    case mouse_button::middle: return "{mouse_middle}";
    default: return "{mouse_unknown}";
    }
}

std::string_view get_mouse_button_event_codename(mouse_button_event button_event) {
    switch (button_event) {
    case mouse_button_event::up: return "Up";
    case mouse_button_event::down: return "Down";
    case mouse_button_event::double_click: return "DoubleClick";
    default: return "";
    }
}

std::string_view get_localizable_mouse_button_event_name(mouse_button_event button_event) {
    switch (button_event) {
    case mouse_button_event::up: return "{mouse_event_up}";
    case mouse_button_event::down: return "{mouse_event_down}";
    case mouse_button_event::double_click: return "{mouse_event_double_click}";
    default: return "{mouse_event_unknown}";
    }
}

std::string_view get_key_codename(key key_id) {
    switch (key_id) {
    case key::k_escape: return "Escape";
    case key::k_1: return "1";
    case key::k_2: return "2";
    case key::k_3: return "3";
    case key::k_4: return "4";
    case key::k_5: return "5";
    case key::k_6: return "6";
    case key::k_7: return "7";
    case key::k_8: return "8";
    case key::k_9: return "9";
    case key::k_0: return "0";
    case key::k_minus: return "-";
    case key::k_equals: return "=";
    case key::k_back: return "Backspace";
    case key::k_tab: return "Tab";
    case key::k_q: return "Q";
    case key::k_w: return "W";
    case key::k_e: return "E";
    case key::k_r: return "R";
    case key::k_t: return "T";
    case key::k_y: return "Y";
    case key::k_u: return "U";
    case key::k_i: return "I";
    case key::k_o: return "O";
    case key::k_p: return "P";
    case key::k_lbracket: return "<";
    case key::k_rbracket: return ">";
    case key::k_return: return "Enter";
    case key::k_lcontrol: return "Ctrl";
    case key::k_a: return "A";
    case key::k_s: return "S";
    case key::k_d: return "D";
    case key::k_f: return "F";
    case key::k_g: return "G";
    case key::k_h: return "H";
    case key::k_j: return "J";
    case key::k_k: return "K";
    case key::k_l: return "L";
    case key::k_semicolon: return ";";
    case key::k_apostrophe: return "'";
    case key::k_grave: return "`";
    case key::k_lshift: return "Shift";
    case key::k_backslash: return "\\";
    case key::k_z: return "Z";
    case key::k_x: return "X";
    case key::k_c: return "C";
    case key::k_v: return "V";
    case key::k_b: return "B";
    case key::k_n: return "N";
    case key::k_m: return "M";
    case key::k_comma: return ",";
    case key::k_period: return ".";
    case key::k_slash: return "/";
    case key::k_rshift: return "Shift (R)";
    case key::k_multiply: return "*";
    case key::k_lmenu: return "Alt";
    case key::k_space: return "Space";
    case key::k_capital: return "";
    case key::k_f1: return "F1";
    case key::k_f2: return "F2";
    case key::k_f3: return "F3";
    case key::k_f4: return "F4";
    case key::k_f5: return "F5";
    case key::k_f6: return "F6";
    case key::k_f7: return "F7";
    case key::k_f8: return "F8";
    case key::k_f9: return "F9";
    case key::k_f10: return "F10";
    case key::k_numlock: return "Num. Lock";
    case key::k_scroll: return "Scr. Lock";
    case key::k_numpad_7: return "7 (Num.)";
    case key::k_numpad_8: return "8 (Num.)";
    case key::k_numpad_9: return "9 (Num.)";
    case key::k_subtract: return "- (Num.)";
    case key::k_numpad_4: return "4 (Num.)";
    case key::k_numpad_5: return "5 (Num.)";
    case key::k_numpad_6: return "6 (Num.)";
    case key::k_add: return "+ (Num.)";
    case key::k_numpad_1: return "1 (Num.)";
    case key::k_numpad_2: return "2 (Num.)";
    case key::k_numpad_3: return "3 (Num.)";
    case key::k_numpad_0: return "0 (Num.)";
    case key::k_decimal: return ". (Num.)";
    case key::k_f11: return "F11";
    case key::k_f12: return "F12";
    case key::k_f13: return "F13";
    case key::k_f14: return "F14";
    case key::k_f15: return "F15";
    case key::k_numpadequals: return "= (Num.)";
    case key::k_prevtrack: return "Prev. Track";
    case key::k_nexttrack: return "Next Track";
    case key::k_numpadenter: return "Enter (Num.)";
    case key::k_rcontrol: return "Ctrl (R)";
    case key::k_mute: return "Mute";
    case key::k_calculator: return "Calculator";
    case key::k_playpause: return "Play";
    case key::k_mediastop: return "Stop";
    case key::k_volumedown: return "Vol. Down";
    case key::k_volumeup: return "Vol. Up";
    case key::k_webhome: return "Web Home";
    case key::k_divide: return "/ (Num.)";
    case key::k_sysrq: return "Prt. Scn.";
    case key::k_rmenu: return "Alt (R)";
    case key::k_pause: return "Pause";
    case key::k_home: return "Home";
    case key::k_up: return "Up";
    case key::k_pgup: return "Page Up";
    case key::k_left: return "Left";
    case key::k_right: return "Right";
    case key::k_end: return "End";
    case key::k_down: return "Down";
    case key::k_pgdown: return "Page Down";
    case key::k_insert: return "Insert";
    case key::k_delete: return "Delete";
    case key::k_lwin: return "Win (L)";
    case key::k_rwin: return "Win (R)";
    case key::k_apps: return "Apps";
    case key::k_power: return "Power";
    case key::k_sleep: return "Sleep";
    case key::k_wake: return "Wake";
    case key::k_websearch: return "Web Search";
    case key::k_webfavorites: return "Web Favorites";
    case key::k_webrefresh: return "Web Refresh";
    case key::k_webstop: return "Web Stop";
    case key::k_webforward: return "Web Forward";
    case key::k_webback: return "Web Back";
    case key::k_mycomputer: return "My Computer";
    case key::k_mail: return "Mail";
    case key::k_mediaselect: return "Media Select";
    default: return "";
    }
}

key get_key_from_codename(std::string_view key_name) {
    using index_t = std::underlying_type_t<key>;

    for (index_t i = 0; i < static_cast<index_t>(key::k_maxkey); ++i) {
        if (key_name == get_key_codename(static_cast<key>(i)))
            return static_cast<key>(i);
    }

    return key::k_unassigned;
}

std::string_view get_localizable_key_name(key key_id) {
    switch (key_id) {
    case key::k_escape: return "{key_escape}";
    case key::k_1: return "{key_1}";
    case key::k_2: return "{key_2}";
    case key::k_3: return "{key_3}";
    case key::k_4: return "{key_4}";
    case key::k_5: return "{key_5}";
    case key::k_6: return "{key_6}";
    case key::k_7: return "{key_7}";
    case key::k_8: return "{key_8}";
    case key::k_9: return "{key_9}";
    case key::k_0: return "{key_0}";
    case key::k_minus: return "{key_minus}";
    case key::k_equals: return "{key_equals}";
    case key::k_back: return "{key_backspace}";
    case key::k_tab: return "{key_tab}";
    case key::k_q: return "{key_q}";
    case key::k_w: return "{key_w}";
    case key::k_e: return "{key_e}";
    case key::k_r: return "{key_r}";
    case key::k_t: return "{key_t}";
    case key::k_y: return "{key_y}";
    case key::k_u: return "{key_u}";
    case key::k_i: return "{key_i}";
    case key::k_o: return "{key_o}";
    case key::k_p: return "{key_p}";
    case key::k_lbracket: return "{key_left_angle}";
    case key::k_rbracket: return "{key_right_angle}";
    case key::k_return: return "{key_enter}";
    case key::k_lcontrol: return "{key_left_control}";
    case key::k_a: return "{key_a}";
    case key::k_s: return "{key_s}";
    case key::k_d: return "{key_d}";
    case key::k_f: return "{key_f}";
    case key::k_g: return "{key_g}";
    case key::k_h: return "{key_h}";
    case key::k_j: return "{key_j}";
    case key::k_k: return "{key_k}";
    case key::k_l: return "{key_l}";
    case key::k_semicolon: return "{key_semicolon}";
    case key::k_apostrophe: return "{key_apostrophe}";
    case key::k_grave: return "{key_grave}";
    case key::k_lshift: return "{key_left_shift}";
    case key::k_backslash: return "{key_backslash}";
    case key::k_z: return "{key_z}";
    case key::k_x: return "{key_x}";
    case key::k_c: return "{key_p}";
    case key::k_v: return "{key_v}";
    case key::k_b: return "{key_b}";
    case key::k_n: return "{key_n}";
    case key::k_m: return "{key_m}";
    case key::k_comma: return "{key_comma}";
    case key::k_period: return "{key_period}";
    case key::k_slash: return "{key_slash}";
    case key::k_rshift: return "{key_right_shift}";
    case key::k_multiply: return "{key_multiply}";
    case key::k_lmenu: return "{key_left_alt}";
    case key::k_space: return "{key_space}";
    case key::k_capital: return "{key_unknown}";
    case key::k_f1: return "{key_f1}";
    case key::k_f2: return "{key_f2}";
    case key::k_f3: return "{key_f3}";
    case key::k_f4: return "{key_f4}";
    case key::k_f5: return "{key_f5}";
    case key::k_f6: return "{key_f6}";
    case key::k_f7: return "{key_f7}";
    case key::k_f8: return "{key_f8}";
    case key::k_f9: return "{key_f9}";
    case key::k_f10: return "{key_f10}";
    case key::k_numlock: return "{key_num_lock}";
    case key::k_scroll: return "{key_scroll_lock}";
    case key::k_numpad_7: return "{key_numpad_7}";
    case key::k_numpad_8: return "{key_numpad_8}";
    case key::k_numpad_9: return "{key_numpad_9}";
    case key::k_subtract: return "{key_numpad_minus}";
    case key::k_numpad_4: return "{key_numpad_4}";
    case key::k_numpad_5: return "{key_numpad_5}";
    case key::k_numpad_6: return "{key_numpad_6}";
    case key::k_add: return "{key_numpad_plus}";
    case key::k_numpad_1: return "{key_numpad_1}";
    case key::k_numpad_2: return "{key_numpad_2}";
    case key::k_numpad_3: return "{key_numpad_3}";
    case key::k_numpad_0: return "{key_numpad_0}";
    case key::k_decimal: return "{key_numpad_period}";
    case key::k_f11: return "{key_f11}";
    case key::k_f12: return "{key_f12}";
    case key::k_f13: return "{key_f13}";
    case key::k_f14: return "{key_f14}";
    case key::k_f15: return "{key_f15}";
    case key::k_numpadequals: return "{key_numpad_equals}";
    case key::k_prevtrack: return "{key_prev_track}";
    case key::k_nexttrack: return "{key_next_track}";
    case key::k_numpadenter: return "{key_numpad_enter}";
    case key::k_rcontrol: return "{key_right_control}";
    case key::k_mute: return "{key_mute}";
    case key::k_calculator: return "{key_calculator}";
    case key::k_playpause: return "{key_play}";
    case key::k_mediastop: return "{key_stop}";
    case key::k_volumedown: return "{key_volume_down}";
    case key::k_volumeup: return "{key_volume_up}";
    case key::k_webhome: return "{key_web_home}";
    case key::k_divide: return "{key_numpad_divide}";
    case key::k_sysrq: return "{key_print_screen}";
    case key::k_rmenu: return "{key_right_alt}";
    case key::k_pause: return "{key_pause}";
    case key::k_home: return "{key_home}";
    case key::k_up: return "{key_up}";
    case key::k_pgup: return "{key_page_up}";
    case key::k_left: return "{key_left}";
    case key::k_right: return "{key_right}";
    case key::k_end: return "{key_end}";
    case key::k_down: return "{key_down}";
    case key::k_pgdown: return "{key_page_down}";
    case key::k_insert: return "{key_insert}";
    case key::k_delete: return "{key_delete}";
    case key::k_lwin: return "{key_left_windows}";
    case key::k_rwin: return "{key_right_windows}";
    case key::k_apps: return "{key_apps}";
    case key::k_power: return "{key_power}";
    case key::k_sleep: return "{key_sleep}";
    case key::k_wake: return "{key_wake}";
    case key::k_websearch: return "{key_web_search}";
    case key::k_webfavorites: return "{key_web_favorites}";
    case key::k_webrefresh: return "{key_web_refresh}";
    case key::k_webstop: return "{key_web_stop}";
    case key::k_webforward: return "{key_web_forward}";
    case key::k_webback: return "{key_web_back}";
    case key::k_mycomputer: return "{key_my_computer}";
    case key::k_mail: return "{key_mail}";
    case key::k_mediaselect: return "{key_media_select}";
    default: return "{key_unknown}";
    }
}

} // namespace lxgui::input
