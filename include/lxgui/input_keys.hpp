#ifndef LXGUI_KEYS_HPP
#define LXGUI_KEYS_HPP

#include <lxgui/lxgui.hpp>

namespace lxgui {
namespace input
{
    enum class mouse_state
    {
        UP = 0,
        DRAGGED,
        CLICKED,
        RELEASED,
        DOUBLE,
        LONG
    };

    enum class mouse_button
    {
        LEFT = 0,
        RIGHT,
        MIDDLE
    };

    enum class key
    {
        K_UNASSIGNED  = 0x00,
        K_ESCAPE      = 0x01,
        K_1           = 0x02,
        K_2           = 0x03,
        K_3           = 0x04,
        K_4           = 0x05,
        K_5           = 0x06,
        K_6           = 0x07,
        K_7           = 0x08,
        K_8           = 0x09,
        K_9           = 0x0A,
        K_0           = 0x0B,
        K_MINUS       = 0x0C,    /// - on main keyboard
        K_EQUALS      = 0x0D,
        K_BACK        = 0x0E,    /// Backspace
        K_TAB         = 0x0F,
        K_Q           = 0x10,
        K_W           = 0x11,
        K_E           = 0x12,
        K_R           = 0x13,
        K_T           = 0x14,
        K_Y           = 0x15,
        K_U           = 0x16,
        K_I           = 0x17,
        K_O           = 0x18,
        K_P           = 0x19,
        K_LBRACKET    = 0x1A,
        K_RBRACKET    = 0x1B,
        K_RETURN      = 0x1C,    /// Enter on main keyboard
        K_LCONTROL    = 0x1D,
        K_A           = 0x1E,
        K_S           = 0x1F,
        K_D           = 0x20,
        K_F           = 0x21,
        K_G           = 0x22,
        K_H           = 0x23,
        K_J           = 0x24,
        K_K           = 0x25,
        K_L           = 0x26,
        K_SEMICOLON   = 0x27,
        K_APOSTROPHE  = 0x28,
        K_GRAVE       = 0x29,    /// Accent
        K_LSHIFT      = 0x2A,
        K_BACKSLASH   = 0x2B,
        K_Z           = 0x2C,
        K_X           = 0x2D,
        K_C           = 0x2E,
        K_V           = 0x2F,
        K_B           = 0x30,
        K_N           = 0x31,
        K_M           = 0x32,
        K_COMMA       = 0x33,
        K_PERIOD      = 0x34,    /// . on main keyboard
        K_SLASH       = 0x35,    /// / on main keyboard
        K_RSHIFT      = 0x36,
        K_MULTIPLY    = 0x37,    /// * on numeric keypad
        K_LMENU       = 0x38,    /// Left Alt
        K_SPACE       = 0x39,
        K_CAPITAL     = 0x3A,
        K_F1          = 0x3B,
        K_F2          = 0x3C,
        K_F3          = 0x3D,
        K_F4          = 0x3E,
        K_F5          = 0x3F,
        K_F6          = 0x40,
        K_F7          = 0x41,
        K_F8          = 0x42,
        K_F9          = 0x43,
        K_F10         = 0x44,
        K_NUMLOCK     = 0x45,
        K_SCROLL      = 0x46,    /// Scroll Lock
        K_NUMPAD7     = 0x47,
        K_NUMPAD8     = 0x48,
        K_NUMPAD9     = 0x49,
        K_SUBTRACT    = 0x4A,    /// - on numeric keypad
        K_NUMPAD4     = 0x4B,
        K_NUMPAD5     = 0x4C,
        K_NUMPAD6     = 0x4D,
        K_ADD         = 0x4E,    /// + on numeric keypad
        K_NUMPAD1     = 0x4F,
        K_NUMPAD2     = 0x50,
        K_NUMPAD3     = 0x51,
        K_NUMPAD0     = 0x52,
        K_DECIMAL     = 0x53,    /// . on numeric keypad
        K_OEM_102     = 0x56,    /// < > | on UK/Germany keyboards
        K_F11         = 0x57,
        K_F12         = 0x58,
        K_F13         = 0x64,    ///                     (NEC PC98)
        K_F14         = 0x65,    ///                     (NEC PC98)
        K_F15         = 0x66,    ///                     (NEC PC98)
        K_KANA        = 0x70,    /// (Japanese keyboard)
        K_ABNT_C1     = 0x73,    /// / ? on Portugese (Brazilian) keyboards
        K_CONVERT     = 0x79,    /// (Japanese keyboard)
        K_NOCONVERT   = 0x7B,    /// (Japanese keyboard)
        K_YEN         = 0x7D,    /// (Japanese keyboard)
        K_ABNT_C2     = 0x7E,    /// Numpad . on Portugese (Brazilian) keyboards
        K_NUMPADEQUALS= 0x8D,    /// = on numeric keypad (NEC PC98)
        K_PREVTRACK   = 0x90,    /// Previous Track (K_CIRCUMFLEX on Japanese keyboard)
        K_AT          = 0x91,    ///                     (NEC PC98)
        K_COLON       = 0x92,    ///                     (NEC PC98)
        K_UNDERLINE   = 0x93,    ///                     (NEC PC98)
        K_KANJI       = 0x94,    /// (Japanese keyboard)
        K_STOP        = 0x95,    ///                     (NEC PC98)
        K_AX          = 0x96,    ///                     (Japan AX)
        K_UNLABELED   = 0x97,    ///                        (J3100)
        K_NEXTTRACK   = 0x99,    /// Next Track
        K_NUMPADENTER = 0x9C,    /// Enter on numeric keypad
        K_RCONTROL    = 0x9D,
        K_MUTE        = 0xA0,    /// Mute
        K_CALCULATOR  = 0xA1,    /// Calculator
        K_PLAYPAUSE   = 0xA2,    /// Play / Pause
        K_MEDIASTOP   = 0xA4,    /// Media Stop
        K_VOLUMEDOWN  = 0xAE,    /// Volume -
        K_VOLUMEUP    = 0xB0,    /// Volume +
        K_WEBHOME     = 0xB2,    /// Web home
        K_NUMPADCOMMA = 0xB3,    /// , on numeric keypad (NEC PC98)
        K_DIVIDE      = 0xB5,    /// / on numeric keypad
        K_SYSRQ       = 0xB7,
        K_RMENU       = 0xB8,    /// Right Alt
        K_PAUSE       = 0xC5,    /// Pause
        K_HOME        = 0xC7,    /// Home on arrow keypad
        K_UP          = 0xC8,    /// UpArrow on arrow keypad
        K_PGUP        = 0xC9,    /// PgUp on arrow keypad
        K_LEFT        = 0xCB,    /// LeftArrow on arrow keypad
        K_RIGHT       = 0xCD,    /// RightArrow on arrow keypad
        K_END         = 0xCF,    /// End on arrow keypad
        K_DOWN        = 0xD0,    /// DownArrow on arrow keypad
        K_PGDOWN      = 0xD1,    /// PgDn on arrow keypad
        K_INSERT      = 0xD2,    /// Insert on arrow keypad
        K_DELETE      = 0xD3,    /// Delete on arrow keypad
        K_LWIN        = 0xDB,    /// Left Windows key
        K_RWIN        = 0xDC,    /// Right Windows key
        K_APPS        = 0xDD,    /// AppMenu key
        K_POWER       = 0xDE,    /// System Power
        K_SLEEP       = 0xDF,    /// System Sleep
        K_WAKE        = 0xE3,    /// System Wake
        K_WEBSEARCH   = 0xE5,    /// Web Search
        K_WEBFAVORITES= 0xE6,    /// Web Favorites
        K_WEBREFRESH  = 0xE7,    /// Web Refresh
        K_WEBSTOP     = 0xE8,    /// Web Stop
        K_WEBFORWARD  = 0xE9,    /// Web Forward
        K_WEBBACK     = 0xEA,    /// Web Back
        K_MYCOMPUTER  = 0xEB,    /// My Computer
        K_MAIL        = 0xEC,    /// Mail
        K_MEDIASELECT = 0xED,    /// Media Select
        K_MAXKEY      = 0xFF
    };
}
}

#endif
