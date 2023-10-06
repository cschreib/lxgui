#ifndef LXGUI_INPUT_KEYS_HPP
#define LXGUI_INPUT_KEYS_HPP

#include "lxgui/lxgui.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace lxgui::input {

enum class mouse_button : std::uint8_t { left = 0, right, middle };

enum class mouse_button_event : std::uint8_t { up = 0, down = 1, double_click = 2 };

enum class key : std::uint8_t {
    k_unassigned   = 0x00,
    k_escape       = 0x01,
    k_1            = 0x02,
    k_2            = 0x03,
    k_3            = 0x04,
    k_4            = 0x05,
    k_5            = 0x06,
    k_6            = 0x07,
    k_7            = 0x08,
    k_8            = 0x09,
    k_9            = 0x0A,
    k_0            = 0x0B,
    k_minus        = 0x0C, /// - on main keyboard
    k_equals       = 0x0D,
    k_back         = 0x0E, /// Backspace
    k_tab          = 0x0F,
    k_q            = 0x10,
    k_w            = 0x11,
    k_e            = 0x12,
    k_r            = 0x13,
    k_t            = 0x14,
    k_y            = 0x15,
    k_u            = 0x16,
    k_i            = 0x17,
    k_o            = 0x18,
    k_p            = 0x19,
    k_lbracket     = 0x1A,
    k_rbracket     = 0x1B,
    k_return       = 0x1C, /// Enter on main keyboard
    k_lcontrol     = 0x1D,
    k_a            = 0x1E,
    k_s            = 0x1F,
    k_d            = 0x20,
    k_f            = 0x21,
    k_g            = 0x22,
    k_h            = 0x23,
    k_j            = 0x24,
    k_k            = 0x25,
    k_l            = 0x26,
    k_semicolon    = 0x27,
    k_apostrophe   = 0x28,
    k_grave        = 0x29, /// Accent
    k_lshift       = 0x2A,
    k_backslash    = 0x2B,
    k_z            = 0x2C,
    k_x            = 0x2D,
    k_c            = 0x2E,
    k_v            = 0x2F,
    k_b            = 0x30,
    k_n            = 0x31,
    k_m            = 0x32,
    k_comma        = 0x33,
    k_period       = 0x34, /// . on main keyboard
    k_slash        = 0x35, /// / on main keyboard
    k_rshift       = 0x36,
    k_multiply     = 0x37, /// * on numeric keypad
    k_lmenu        = 0x38, /// Left Alt
    k_space        = 0x39,
    k_capital      = 0x3A,
    k_f1           = 0x3B,
    k_f2           = 0x3C,
    k_f3           = 0x3D,
    k_f4           = 0x3E,
    k_f5           = 0x3F,
    k_f6           = 0x40,
    k_f7           = 0x41,
    k_f8           = 0x42,
    k_f9           = 0x43,
    k_f10          = 0x44,
    k_numlock      = 0x45,
    k_scroll       = 0x46, /// Scroll Lock
    k_numpad_7     = 0x47,
    k_numpad_8     = 0x48,
    k_numpad_9     = 0x49,
    k_subtract     = 0x4A, /// - on numeric keypad
    k_numpad_4     = 0x4B,
    k_numpad_5     = 0x4C,
    k_numpad_6     = 0x4D,
    k_add          = 0x4E, /// + on numeric keypad
    k_numpad_1     = 0x4F,
    k_numpad_2     = 0x50,
    k_numpad_3     = 0x51,
    k_numpad_0     = 0x52,
    k_decimal      = 0x53, /// . on numeric keypad
    k_oem_102      = 0x56, /// < > | on UK/Germany keyboards
    k_f11          = 0x57,
    k_f12          = 0x58,
    k_f13          = 0x64, ///                     (NEC PC98)
    k_f14          = 0x65, ///                     (NEC PC98)
    k_f15          = 0x66, ///                     (NEC PC98)
    k_kana         = 0x70, /// (Japanese keyboard)
    k_abnt_c1      = 0x73, /// / ? on Portugese (Brazilian) keyboards
    k_convert      = 0x79, /// (Japanese keyboard)
    k_noconvert    = 0x7B, /// (Japanese keyboard)
    k_yen          = 0x7D, /// (Japanese keyboard)
    k_abnt_c2      = 0x7E, /// Numpad . on Portugese (Brazilian) keyboards
    k_numpadequals = 0x8D, /// = on numeric keypad (NEC PC98)
    k_prevtrack    = 0x90, /// Previous Track (K_CIRCUMFLEX on Japanese keyboard)
    k_at           = 0x91, ///                     (NEC PC98)
    k_colon        = 0x92, ///                     (NEC PC98)
    k_underline    = 0x93, ///                     (NEC PC98)
    k_kanji        = 0x94, /// (Japanese keyboard)
    k_stop         = 0x95, ///                     (NEC PC98)
    k_ax           = 0x96, ///                     (Japan AX)
    k_unlabeled    = 0x97, ///                        (J3100)
    k_nexttrack    = 0x99, /// Next Track
    k_numpadenter  = 0x9C, /// Enter on numeric keypad
    k_rcontrol     = 0x9D,
    k_mute         = 0xA0, /// Mute
    k_calculator   = 0xA1, /// Calculator
    k_playpause    = 0xA2, /// Play / Pause
    k_mediastop    = 0xA4, /// Media Stop
    k_volumedown   = 0xAE, /// Volume -
    k_volumeup     = 0xB0, /// Volume +
    k_webhome      = 0xB2, /// Web home
    k_numpadcomma  = 0xB3, /// , on numeric keypad (NEC PC98)
    k_divide       = 0xB5, /// / on numeric keypad
    k_sysrq        = 0xB7,
    k_rmenu        = 0xB8, /// Right Alt
    k_pause        = 0xC5, /// Pause
    k_home         = 0xC7, /// Home on arrow keypad
    k_up           = 0xC8, /// UpArrow on arrow keypad
    k_pgup         = 0xC9, /// PgUp on arrow keypad
    k_left         = 0xCB, /// LeftArrow on arrow keypad
    k_right        = 0xCD, /// RightArrow on arrow keypad
    k_end          = 0xCF, /// End on arrow keypad
    k_down         = 0xD0, /// DownArrow on arrow keypad
    k_pgdown       = 0xD1, /// PgDn on arrow keypad
    k_insert       = 0xD2, /// Insert on arrow keypad
    k_delete       = 0xD3, /// Delete on arrow keypad
    k_lwin         = 0xDB, /// Left Windows key
    k_rwin         = 0xDC, /// Right Windows key
    k_apps         = 0xDD, /// AppMenu key
    k_power        = 0xDE, /// System Power
    k_sleep        = 0xDF, /// System Sleep
    k_wake         = 0xE3, /// System Wake
    k_websearch    = 0xE5, /// Web Search
    k_webfavorites = 0xE6, /// Web Favorites
    k_webrefresh   = 0xE7, /// Web Refresh
    k_webstop      = 0xE8, /// Web Stop
    k_webforward   = 0xE9, /// Web Forward
    k_webback      = 0xEA, /// Web Back
    k_mycomputer   = 0xEB, /// My Computer
    k_mail         = 0xEC, /// Mail
    k_mediaselect  = 0xED, /// Media Select
    k_maxkey       = 0xFF
};

/**
 * \brief Returns a standard English name for the provided mouse button.
 * \param button_id The ID code of the mouse button
 * \return The corresponding code name in English
 * \note This will return a standard English button name, e.g., "LeftButton" for the left mouse
 * button. This can be used for string-based key identification in scripts, where key
 * integer codes would be less usable, or for displaying debug or error messages.
 */
std::string_view get_mouse_button_codename(mouse_button button_id);

/**
 * \brief Returns the localizable name of the provided mouse button.
 * \param button_id The ID code of the mouse button
 * \return The localizable name of the provided mouse button
 * \note This will return localizable button names, e.g., "{mouse_left}" for the left mouse
 * button. Use a @ref lxgui::gui::localizer to transform this into a user-friendly name.
 */
std::string_view get_localizable_mouse_button_name(mouse_button button_id);

/**
 * \brief Returns a standard English name for the provided mouse button event.
 * \param button_event The ID code of the mouse button event
 * \return The corresponding code name in English
 * \note This will return a standard English button name, e.g., "Up" for the mouse up event.
 * This can be used for string-based key identification in scripts, where key
 * integer codes would be less usable, or for displaying debug or error messages.
 */
std::string_view get_mouse_button_event_codename(mouse_button_event button_event);

/**
 * \brief Returns the localizable name of the provided mouse button event.
 * \param button_event The ID code of the mouse button event
 * \return The localizable name of the provided mouse button event
 * \note This will return localizable button names, e.g., "{mouse_event_up}" for the mouse up event.
 * Use a @ref lxgui::gui::localizer to transform this into a user-friendly name.
 */
std::string_view get_localizable_mouse_button_event_name(mouse_button_event button_event);

/**
 * \brief Returns a standard Engilsh name for the provided mouse button and event.
 * \param button_id The ID code of the mouse button
 * \param button_event The ID code of the mouse button event
 * \return The corresponding code name in English
 * \note This will return a string with the format "<button>:<event>".
 * \see get_mouse_button_codename()
 * \see get_mouse_button_event_codename()
 */
std::string_view
get_mouse_button_and_event_codename(mouse_button button_id, mouse_button_event button_event);

/**
 * \brief Returns a mouse button and a button event ID from a string formatted as "<button>:<event>".
 * \param event_name The name of the mouse button event, as "<button>:<event>"
 * \return The pair of decoded button and event ID, or std::nullopt if parsing failed.
 */
std::optional<std::pair<mouse_button, mouse_button_event>>
get_mouse_button_and_event_from_codename(std::string_view button_and_event_name);

/**
 * \brief Returns a standard English name for the provided key.
 * \param key_id The key
 * \return The name of the provided key
 * \note This will return a standard English key name, e.g., "Enter" for the Enter key.
 * This can be used for string-based key identification in scripts, where key integer
 * codes would be less usable, or for displaying debug or error messages.
 */
std::string_view get_key_codename(key key_id);

/**
 * \brief Returns the key code from the standard English name of a key.
 * \param key_name The name of the key
 * \return The key code
 * \note The name of the key must be from get_key_codename().
 */
key get_key_from_codename(std::string_view key_name);

/**
 * \brief Returns the localizable name of the provided key.
 * \param key_id The key
 * \return The localizable name of the provided key
 * \note This will return localizable key names, e.g., "{key_enter}" for the Enter key.
 * Use a @ref lxgui::gui::localizer to transform this into a user-friendly name.
 */
std::string_view get_localizable_key_name(key key_id);

constexpr std::size_t mouse_button_number = 3u;
constexpr std::size_t key_number          = static_cast<std::size_t>(key::k_maxkey);

} // namespace lxgui::input

#endif
