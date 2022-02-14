#include "lxgui/impl/input_sdl_source.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL_clipboard.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_video.h>

namespace lxgui::input { namespace sdl {

source::source(SDL_Window* p_window, SDL_Renderer* p_renderer, bool b_initialise_sdl_image) :
    p_window_(p_window), p_renderer_(p_renderer) {
    m_window_dimensions_ = get_window_pixel_size_();

    update_pixel_per_unit_();

    if (b_initialise_sdl_image) {
        int img_flags = IMG_INIT_PNG;
        if ((IMG_Init(img_flags) & img_flags) == 0) {
            throw gui::exception(
                "input::sdl::source",
                "Could not initialise SDL_image: " + std::string(IMG_GetError()) + ".");
        }
    }
}

utils::ustring source::get_clipboard_content() {
    if (SDL_HasClipboardText()) {
        char*          s_text         = SDL_GetClipboardText();
        utils::ustring s_unicode_text = utils::utf8_to_unicode(s_text);
        SDL_free(s_text);
        return s_unicode_text;
    } else {
        return {};
    }
}

void source::set_clipboard_content(const utils::ustring& s_content) {
    SDL_SetClipboardText(utils::unicode_to_utf8(s_content).c_str());
}

void source::set_mouse_cursor(const std::string& s_file_name, const gui::vector2i& m_hot_spot) {
    auto m_iter = cursor_map_.find(s_file_name);
    if (m_iter == cursor_map_.end()) {
        // Load file
        SDL_Surface* p_surface = IMG_Load(s_file_name.c_str());
        if (p_surface == nullptr) {
            throw gui::exception(
                "input::sdl::source", "Could not load image file " + s_file_name + ".");
        }

        auto p_cursor = wrapped_cursor(
            SDL_CreateColorCursor(p_surface, m_hot_spot.x, m_hot_spot.y), &SDL_FreeCursor);
        m_iter = cursor_map_.insert(std::make_pair(s_file_name, std::move(p_cursor))).first;
        SDL_FreeSurface(p_surface);
    }

    SDL_SetCursor(m_iter->second.get());
}

void source::reset_mouse_cursor() {
    const std::string s_name = "system_arrow";
    auto              m_iter = cursor_map_.find(s_name);
    if (m_iter == cursor_map_.end()) {
        auto p_cursor =
            wrapped_cursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW), &SDL_FreeCursor);
        m_iter = cursor_map_.insert(std::make_pair(s_name, std::move(p_cursor))).first;
    }

    SDL_SetCursor(m_iter->second.get());
}

float source::get_interface_scaling_factor_hint() const {
    return f_pixels_per_unit_;
}

key source::from_sdl_(int sdl_key) const {
    switch (static_cast<SDL_Keycode>(sdl_key)) {
    case SDLK_ESCAPE: return key::k_escape;
    case SDLK_0: return key::k_0;
    case SDLK_1: return key::k_1;
    case SDLK_2: return key::k_2;
    case SDLK_3: return key::k_3;
    case SDLK_4: return key::k_4;
    case SDLK_5: return key::k_5;
    case SDLK_6: return key::k_6;
    case SDLK_7: return key::k_7;
    case SDLK_8: return key::k_8;
    case SDLK_9: return key::k_9;
    case SDLK_MINUS: return key::k_minus;
    case SDLK_EQUALS: return key::k_equals;
    case SDLK_BACKSPACE: return key::k_back;
    case SDLK_TAB: return key::k_tab;
    case SDLK_q: return key::k_q;
    case SDLK_w: return key::k_w;
    case SDLK_e: return key::k_e;
    case SDLK_r: return key::k_r;
    case SDLK_t: return key::k_t;
    case SDLK_y: return key::k_y;
    case SDLK_u: return key::k_u;
    case SDLK_i: return key::k_i;
    case SDLK_o: return key::k_o;
    case SDLK_p: return key::k_p;
    case SDLK_LEFTBRACKET: return key::k_lbracket;
    case SDLK_RIGHTBRACKET: return key::k_rbracket;
    case SDLK_RETURN: return key::k_return;
    case SDLK_KP_ENTER: return key::k_numpadenter;
    case SDLK_LCTRL: return key::k_lcontrol;
    case SDLK_a: return key::k_a;
    case SDLK_s: return key::k_s;
    case SDLK_d: return key::k_d;
    case SDLK_f: return key::k_f;
    case SDLK_g: return key::k_g;
    case SDLK_h: return key::k_h;
    case SDLK_j: return key::k_j;
    case SDLK_k: return key::k_k;
    case SDLK_l: return key::k_l;
    case SDLK_SEMICOLON: return key::k_semicolon;
    case SDLK_QUOTE: return key::k_apostrophe;
    case SDLK_LSHIFT: return key::k_lshift;
    case SDLK_BACKSLASH: return key::k_backslash;
    case SDLK_z: return key::k_z;
    case SDLK_x: return key::k_x;
    case SDLK_c: return key::k_c;
    case SDLK_v: return key::k_v;
    case SDLK_b: return key::k_b;
    case SDLK_n: return key::k_n;
    case SDLK_m: return key::k_m;
    case SDLK_COMMA: return key::k_comma;
    case SDLK_PERIOD: return key::k_period;
    case SDLK_SLASH: return key::k_slash;
    case SDLK_RSHIFT: return key::k_rshift;
    case SDLK_ASTERISK: return key::k_multiply;
    case SDLK_LALT: return key::k_lmenu;
    case SDLK_SPACE: return key::k_space;
    case SDLK_F1: return key::k_f1;
    case SDLK_F2: return key::k_f2;
    case SDLK_F3: return key::k_f3;
    case SDLK_F4: return key::k_f4;
    case SDLK_F5: return key::k_f5;
    case SDLK_F6: return key::k_f6;
    case SDLK_F7: return key::k_f7;
    case SDLK_F8: return key::k_f8;
    case SDLK_F9: return key::k_f9;
    case SDLK_F10: return key::k_f10;
    case SDLK_KP_7: return key::k_numpa_d7;
    case SDLK_KP_8: return key::k_numpa_d8;
    case SDLK_KP_9: return key::k_numpa_d9;
    case SDLK_KP_MINUS: return key::k_subtract;
    case SDLK_KP_4: return key::k_numpa_d4;
    case SDLK_KP_5: return key::k_numpa_d5;
    case SDLK_KP_6: return key::k_numpa_d6;
    case SDLK_KP_PLUS: return key::k_add;
    case SDLK_KP_1: return key::k_numpa_d1;
    case SDLK_KP_2: return key::k_numpa_d2;
    case SDLK_KP_3: return key::k_numpa_d3;
    case SDLK_KP_0: return key::k_numpa_d0;
    case SDLK_F11: return key::k_f11;
    case SDLK_F12: return key::k_f12;
    case SDLK_F13: return key::k_f13;
    case SDLK_F14: return key::k_f14;
    case SDLK_F15: return key::k_f15;
    case SDLK_RCTRL: return key::k_rcontrol;
    case SDLK_KP_DIVIDE: return key::k_divide;
    case SDLK_RALT: return key::k_rmenu;
    case SDLK_PAUSE: return key::k_pause;
    case SDLK_HOME: return key::k_home;
    case SDLK_UP: return key::k_up;
    case SDLK_PAGEUP: return key::k_pgup;
    case SDLK_LEFT: return key::k_left;
    case SDLK_RIGHT: return key::k_right;
    case SDLK_END: return key::k_end;
    case SDLK_DOWN: return key::k_down;
    case SDLK_PAGEDOWN: return key::k_pgdown;
    case SDLK_INSERT: return key::k_insert;
    case SDLK_DELETE: return key::k_delete;
    case SDLK_APPLICATION: return key::k_lwin;
    case SDLK_MENU: return key::k_apps;
    default: return key::k_unassigned;
    }
}

gui::vector2ui source::get_window_pixel_size_() const {
    int pixel_width, pixel_height;
    if (p_renderer_)
        SDL_GetRendererOutputSize(p_renderer_, &pixel_width, &pixel_height);
    else
        SDL_GL_GetDrawableSize(p_window_, &pixel_width, &pixel_height);

    return gui::vector2ui(pixel_width, pixel_height);
}

void source::update_pixel_per_unit_() {
    gui::vector2ui m_pixel_size = get_window_pixel_size_();

    int unit_width, unit_height;
    SDL_GetWindowSize(p_window_, &unit_width, &unit_height);

    f_pixels_per_unit_ =
        std::min(m_pixel_size.x / float(unit_width), m_pixel_size.y / float(unit_height));
}

void source::on_sdl_event(const SDL_Event& m_event) {
    static const mouse_button mouse_from_sdl[3] = {
        mouse_button::left, mouse_button::middle, mouse_button::right};

    switch (m_event.type) {
    case SDL_KEYDOWN: {
        key m_key                                              = from_sdl_(m_event.key.keysym.sym);
        m_keyboard_.key_state[static_cast<std::size_t>(m_key)] = true;

        on_key_pressed(m_key);
        break;
    }
    case SDL_KEYUP: {
        key m_key                                              = from_sdl_(m_event.key.keysym.sym);
        m_keyboard_.key_state[static_cast<std::size_t>(m_key)] = false;

        on_key_released(m_key);
        break;
    }
    case SDL_MOUSEBUTTONDOWN: [[fallthrough]];
    case SDL_FINGERDOWN: {
        if (m_event.type == SDL_MOUSEBUTTONDOWN &&
            (m_event.button.which == SDL_TOUCH_MOUSEID || m_event.button.button == SDL_BUTTON_X1 ||
             m_event.button.button == SDL_BUTTON_X2)) {
            // Ignore these
            break;
        }

        SDL_CaptureMouse(SDL_TRUE);

        mouse_button m_button = m_event.type == SDL_MOUSEBUTTONDOWN
                                    ? mouse_from_sdl[m_event.button.button - 1]
                                    : mouse_button::left;
        m_mouse_.button_state[static_cast<std::size_t>(m_button)] = true;

        gui::vector2f m_mouse_pos;
        if (m_event.type == SDL_MOUSEBUTTONDOWN) {
            m_mouse_pos = gui::vector2f(
                m_event.button.x * f_pixels_per_unit_, m_event.button.y * f_pixels_per_unit_);
        } else {
            // Reset "previous" mouse position to avoid triggering incorrect
            // drag events. With touch devices, the mouse position does not change
            // until the finger is down on the screen.
            m_mouse_pos = gui::vector2f(
                m_event.tfinger.x * m_window_dimensions_.x,
                m_event.tfinger.y * m_window_dimensions_.y);
        }

        on_mouse_pressed(m_button, m_mouse_pos);
        break;
    }
    case SDL_MOUSEBUTTONUP: [[fallthrough]];
    case SDL_FINGERUP: {
        if (m_event.type == SDL_MOUSEBUTTONUP &&
            (m_event.button.which == SDL_TOUCH_MOUSEID || m_event.button.button == SDL_BUTTON_X1 ||
             m_event.button.button == SDL_BUTTON_X2)) {
            // Ignore these
            break;
        }

        SDL_CaptureMouse(SDL_FALSE);

        mouse_button m_button = m_event.type == SDL_MOUSEBUTTONUP
                                    ? mouse_from_sdl[m_event.button.button - 1]
                                    : mouse_button::left;

        m_mouse_.button_state[static_cast<std::size_t>(m_button)] = false;

        gui::vector2f m_mouse_pos;
        if (m_event.type == SDL_MOUSEBUTTONUP) {
            m_mouse_pos = gui::vector2f(
                m_event.button.x * f_pixels_per_unit_, m_event.button.y * f_pixels_per_unit_);
        } else {
            m_mouse_pos = gui::vector2f(
                m_event.tfinger.x * f_pixels_per_unit_, m_event.tfinger.y * f_pixels_per_unit_);
        }

        on_mouse_released(m_button, m_mouse_pos);
        break;
    }
    case SDL_MOUSEMOTION: {
        m_mouse_.m_position = gui::vector2f(m_event.motion.x, m_event.motion.y);
        on_mouse_moved(
            gui::vector2f(m_event.motion.xrel, m_event.motion.yrel), m_mouse_.m_position);
        break;
    }
    case SDL_MOUSEWHEEL: {
        float f_delta =
            (m_event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? m_event.wheel.y : -m_event.wheel.y);
        m_mouse_.f_wheel += f_delta;

        on_mouse_wheel(f_delta, m_mouse_.m_position);
        break;
    }
    case SDL_TEXTINPUT: {
        for (auto c : utils::utf8_to_unicode(m_event.text.text)) {
            // Remove non printable characters (< 32) and Del. (127)
            if (c >= 32 && c != 127)
                on_text_entered(c);
        }
        break;
    }
    case SDL_WINDOWEVENT: {
        if (m_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED &&
            m_event.window.windowID == SDL_GetWindowID(p_window_)) {
            m_window_dimensions_ = get_window_pixel_size_();
            update_pixel_per_unit_();
            on_window_resized(m_window_dimensions_);
        }
        break;
    }
    }
}

}} // namespace lxgui::input::sdl
