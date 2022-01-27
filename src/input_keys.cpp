#include "lxgui/input_keys.hpp"

namespace lxgui {
namespace input
{

std::string_view get_mouse_button_codename(mouse_button mID)
{
    switch (mID)
    {
    case mouse_button::LEFT :   return "LeftButton";
    case mouse_button::RIGHT :  return "RightButton";
    case mouse_button::MIDDLE : return "MiddleButton";
    default :                   return "";
    }
}

std::string_view get_localizable_mouse_button_name(mouse_button mID)
{
    switch (mID)
    {
    case mouse_button::LEFT :   return "{mouse_left}";
    case mouse_button::RIGHT :  return "{mouse_right}";
    case mouse_button::MIDDLE : return "{mouse_middle}";
    default :                   return "{mouse_unknown}";
    }
}

std::string_view get_key_codename(key mKey)
{
    switch (mKey)
    {
    case key::K_ESCAPE       : return "Escape";
    case key::K_1            : return "1";
    case key::K_2            : return "2";
    case key::K_3            : return "3";
    case key::K_4            : return "4";
    case key::K_5            : return "5";
    case key::K_6            : return "6";
    case key::K_7            : return "7";
    case key::K_8            : return "8";
    case key::K_9            : return "9";
    case key::K_0            : return "0";
    case key::K_MINUS        : return "-";
    case key::K_EQUALS       : return "=";
    case key::K_BACK         : return "Backspace";
    case key::K_TAB          : return "Tab";
    case key::K_Q            : return "Q";
    case key::K_W            : return "W";
    case key::K_E            : return "E";
    case key::K_R            : return "R";
    case key::K_T            : return "T";
    case key::K_Y            : return "Y";
    case key::K_U            : return "U";
    case key::K_I            : return "I";
    case key::K_O            : return "O";
    case key::K_P            : return "P";
    case key::K_LBRACKET     : return "<";
    case key::K_RBRACKET     : return ">";
    case key::K_RETURN       : return "Enter";
    case key::K_LCONTROL     : return "Ctrl";
    case key::K_A            : return "A";
    case key::K_S            : return "S";
    case key::K_D            : return "D";
    case key::K_F            : return "F";
    case key::K_G            : return "G";
    case key::K_H            : return "H";
    case key::K_J            : return "J";
    case key::K_K            : return "K";
    case key::K_L            : return "L";
    case key::K_SEMICOLON    : return ";";
    case key::K_APOSTROPHE   : return "'";
    case key::K_GRAVE        : return "`";
    case key::K_LSHIFT       : return "Shift";
    case key::K_BACKSLASH    : return "\\";
    case key::K_Z            : return "Z";
    case key::K_X            : return "X";
    case key::K_C            : return "C";
    case key::K_V            : return "V";
    case key::K_B            : return "B";
    case key::K_N            : return "N";
    case key::K_M            : return "M";
    case key::K_COMMA        : return ",";
    case key::K_PERIOD       : return ".";
    case key::K_SLASH        : return "/";
    case key::K_RSHIFT       : return "Shift (R)";
    case key::K_MULTIPLY     : return "*";
    case key::K_LMENU        : return "Alt";
    case key::K_SPACE        : return "Space";
    case key::K_CAPITAL      : return "";
    case key::K_F1           : return "F1";
    case key::K_F2           : return "F2";
    case key::K_F3           : return "F3";
    case key::K_F4           : return "F4";
    case key::K_F5           : return "F5";
    case key::K_F6           : return "F6";
    case key::K_F7           : return "F7";
    case key::K_F8           : return "F8";
    case key::K_F9           : return "F9";
    case key::K_F10          : return "F10";
    case key::K_NUMLOCK      : return "Num. Lock";
    case key::K_SCROLL       : return "Scr. Lock";
    case key::K_NUMPAD7      : return "7 (Num.)";
    case key::K_NUMPAD8      : return "8 (Num.)";
    case key::K_NUMPAD9      : return "9 (Num.)";
    case key::K_SUBTRACT     : return "- (Num.)";
    case key::K_NUMPAD4      : return "4 (Num.)";
    case key::K_NUMPAD5      : return "5 (Num.)";
    case key::K_NUMPAD6      : return "6 (Num.)";
    case key::K_ADD          : return "+ (Num.)";
    case key::K_NUMPAD1      : return "1 (Num.)";
    case key::K_NUMPAD2      : return "2 (Num.)";
    case key::K_NUMPAD3      : return "3 (Num.)";
    case key::K_NUMPAD0      : return "0 (Num.)";
    case key::K_DECIMAL      : return ". (Num.)";
    case key::K_F11          : return "F11";
    case key::K_F12          : return "F12";
    case key::K_F13          : return "F13";
    case key::K_F14          : return "F14";
    case key::K_F15          : return "F15";
    case key::K_NUMPADEQUALS : return "= (Num.)";
    case key::K_PREVTRACK    : return "Prev. Track";
    case key::K_NEXTTRACK    : return "Next Track";
    case key::K_NUMPADENTER  : return "Enter (Num.)";
    case key::K_RCONTROL     : return "Ctrl (R)";
    case key::K_MUTE         : return "Mute";
    case key::K_CALCULATOR   : return "Calculator";
    case key::K_PLAYPAUSE    : return "Play";
    case key::K_MEDIASTOP    : return "Stop";
    case key::K_VOLUMEDOWN   : return "Vol. Down";
    case key::K_VOLUMEUP     : return "Vol. Up";
    case key::K_WEBHOME      : return "Web Home";
    case key::K_DIVIDE       : return "/ (Num.)";
    case key::K_SYSRQ        : return "Prt. Scn.";
    case key::K_RMENU        : return "Alt (R)";
    case key::K_PAUSE        : return "Pause";
    case key::K_HOME         : return "Home";
    case key::K_UP           : return "Up";
    case key::K_PGUP         : return "Page Up";
    case key::K_LEFT         : return "Left";
    case key::K_RIGHT        : return "Right";
    case key::K_END          : return "End";
    case key::K_DOWN         : return "Down";
    case key::K_PGDOWN       : return "Page Down";
    case key::K_INSERT       : return "Insert";
    case key::K_DELETE       : return "Delete";
    case key::K_LWIN         : return "Win (L)";
    case key::K_RWIN         : return "Win (R)";
    case key::K_APPS         : return "Apps";
    case key::K_POWER        : return "Power";
    case key::K_SLEEP        : return "Sleep";
    case key::K_WAKE         : return "Wake";
    case key::K_WEBSEARCH    : return "Web Search";
    case key::K_WEBFAVORITES : return "Web Favorites";
    case key::K_WEBREFRESH   : return "Web Refresh";
    case key::K_WEBSTOP      : return "Web Stop";
    case key::K_WEBFORWARD   : return "Web Forward";
    case key::K_WEBBACK      : return "Web Back";
    case key::K_MYCOMPUTER   : return "My Computer";
    case key::K_MAIL         : return "Mail";
    case key::K_MEDIASELECT  : return "Media Select";
    default                  : return "";
    }
}

key get_key_from_codename(std::string_view sKey)
{
    using index_t = std::underlying_type_t<key>;

    for (index_t i = 0; i < static_cast<index_t>(key::K_MAXKEY); ++i)
    {
        if (sKey == get_key_codename(static_cast<key>(i)))
            return static_cast<key>(i);
    }

    return key::K_UNASSIGNED;
}

std::string_view get_localizable_key_name(key mKey)
{
    switch (mKey)
    {
    case key::K_ESCAPE       : return "{key_escape}";
    case key::K_1            : return "{key_1}";
    case key::K_2            : return "{key_2}";
    case key::K_3            : return "{key_3}";
    case key::K_4            : return "{key_4}";
    case key::K_5            : return "{key_5}";
    case key::K_6            : return "{key_6}";
    case key::K_7            : return "{key_7}";
    case key::K_8            : return "{key_8}";
    case key::K_9            : return "{key_9}";
    case key::K_0            : return "{key_0}";
    case key::K_MINUS        : return "{key_minus}";
    case key::K_EQUALS       : return "{key_equals}";
    case key::K_BACK         : return "{key_backspace}";
    case key::K_TAB          : return "{key_tab}";
    case key::K_Q            : return "{key_q}";
    case key::K_W            : return "{key_w}";
    case key::K_E            : return "{key_e}";
    case key::K_R            : return "{key_r}";
    case key::K_T            : return "{key_t}";
    case key::K_Y            : return "{key_y}";
    case key::K_U            : return "{key_u}";
    case key::K_I            : return "{key_i}";
    case key::K_O            : return "{key_o}";
    case key::K_P            : return "{key_p}";
    case key::K_LBRACKET     : return "{key_left_angle}";
    case key::K_RBRACKET     : return "{key_right_angle}";
    case key::K_RETURN       : return "{key_enter}";
    case key::K_LCONTROL     : return "{key_left_control}";
    case key::K_A            : return "{key_a}";
    case key::K_S            : return "{key_s}";
    case key::K_D            : return "{key_d}";
    case key::K_F            : return "{key_f}";
    case key::K_G            : return "{key_g}";
    case key::K_H            : return "{key_h}";
    case key::K_J            : return "{key_j}";
    case key::K_K            : return "{key_k}";
    case key::K_L            : return "{key_l}";
    case key::K_SEMICOLON    : return "{key_semicolon}";
    case key::K_APOSTROPHE   : return "{key_apostrophe}";
    case key::K_GRAVE        : return "{key_grave}";
    case key::K_LSHIFT       : return "{key_left_shift}";
    case key::K_BACKSLASH    : return "{key_backslash}";
    case key::K_Z            : return "{key_z}";
    case key::K_X            : return "{key_x}";
    case key::K_C            : return "{key_p}";
    case key::K_V            : return "{key_v}";
    case key::K_B            : return "{key_b}";
    case key::K_N            : return "{key_n}";
    case key::K_M            : return "{key_m}";
    case key::K_COMMA        : return "{key_comma}";
    case key::K_PERIOD       : return "{key_period}";
    case key::K_SLASH        : return "{key_slash}";
    case key::K_RSHIFT       : return "{key_right_shift}";
    case key::K_MULTIPLY     : return "{key_multiply}";
    case key::K_LMENU        : return "{key_left_alt}";
    case key::K_SPACE        : return "{key_space}";
    case key::K_CAPITAL      : return "{key_unknown}";
    case key::K_F1           : return "{key_f1}";
    case key::K_F2           : return "{key_f2}";
    case key::K_F3           : return "{key_f3}";
    case key::K_F4           : return "{key_f4}";
    case key::K_F5           : return "{key_f5}";
    case key::K_F6           : return "{key_f6}";
    case key::K_F7           : return "{key_f7}";
    case key::K_F8           : return "{key_f8}";
    case key::K_F9           : return "{key_f9}";
    case key::K_F10          : return "{key_f10}";
    case key::K_NUMLOCK      : return "{key_num_lock}";
    case key::K_SCROLL       : return "{key_scroll_lock}";
    case key::K_NUMPAD7      : return "{key_numpad_7}";
    case key::K_NUMPAD8      : return "{key_numpad_8}";
    case key::K_NUMPAD9      : return "{key_numpad_9}";
    case key::K_SUBTRACT     : return "{key_numpad_minus}";
    case key::K_NUMPAD4      : return "{key_numpad_4}";
    case key::K_NUMPAD5      : return "{key_numpad_5}";
    case key::K_NUMPAD6      : return "{key_numpad_6}";
    case key::K_ADD          : return "{key_numpad_plus}";
    case key::K_NUMPAD1      : return "{key_numpad_1}";
    case key::K_NUMPAD2      : return "{key_numpad_2}";
    case key::K_NUMPAD3      : return "{key_numpad_3}";
    case key::K_NUMPAD0      : return "{key_numpad_0}";
    case key::K_DECIMAL      : return "{key_numpad_period}";
    case key::K_F11          : return "{key_f11}";
    case key::K_F12          : return "{key_f12}";
    case key::K_F13          : return "{key_f13}";
    case key::K_F14          : return "{key_f14}";
    case key::K_F15          : return "{key_f15}";
    case key::K_NUMPADEQUALS : return "{key_numpad_equals}";
    case key::K_PREVTRACK    : return "{key_prev_track}";
    case key::K_NEXTTRACK    : return "{key_next_track}";
    case key::K_NUMPADENTER  : return "{key_numpad_enter}";
    case key::K_RCONTROL     : return "{key_right_control}";
    case key::K_MUTE         : return "{key_mute}";
    case key::K_CALCULATOR   : return "{key_calculator}";
    case key::K_PLAYPAUSE    : return "{key_play}";
    case key::K_MEDIASTOP    : return "{key_stop}";
    case key::K_VOLUMEDOWN   : return "{key_volume_down}";
    case key::K_VOLUMEUP     : return "{key_volume_up}";
    case key::K_WEBHOME      : return "{key_web_home}";
    case key::K_DIVIDE       : return "{key_numpad_divide}";
    case key::K_SYSRQ        : return "{key_print_screen}";
    case key::K_RMENU        : return "{key_right_alt}";
    case key::K_PAUSE        : return "{key_pause}";
    case key::K_HOME         : return "{key_home}";
    case key::K_UP           : return "{key_up}";
    case key::K_PGUP         : return "{key_page_up}";
    case key::K_LEFT         : return "{key_left}";
    case key::K_RIGHT        : return "{key_right}";
    case key::K_END          : return "{key_end}";
    case key::K_DOWN         : return "{key_down}";
    case key::K_PGDOWN       : return "{key_page_down}";
    case key::K_INSERT       : return "{key_insert}";
    case key::K_DELETE       : return "{key_delete}";
    case key::K_LWIN         : return "{key_left_windows}";
    case key::K_RWIN         : return "{key_right_windows}";
    case key::K_APPS         : return "{key_apps}";
    case key::K_POWER        : return "{key_power}";
    case key::K_SLEEP        : return "{key_sleep}";
    case key::K_WAKE         : return "{key_wake}";
    case key::K_WEBSEARCH    : return "{key_web_search}";
    case key::K_WEBFAVORITES : return "{key_web_favorites}";
    case key::K_WEBREFRESH   : return "{key_web_refresh}";
    case key::K_WEBSTOP      : return "{key_web_stop}";
    case key::K_WEBFORWARD   : return "{key_web_forward}";
    case key::K_WEBBACK      : return "{key_web_back}";
    case key::K_MYCOMPUTER   : return "{key_my_computer}";
    case key::K_MAIL         : return "{key_mail}";
    case key::K_MEDIASELECT  : return "{key_media_select}";
    default                  : return "{key_unknown}";
    }
}

}
}
