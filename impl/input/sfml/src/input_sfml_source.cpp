#include "lxgui/impl/input_sfml_source.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

using sfKey = sf::Keyboard::Key;

namespace lxgui::input { namespace sfml {
source::source(sf::Window& win) : window_(win) {
    window_dimensions_ = gui::vector2ui(window_.getSize().x, window_.getSize().y);
}

utils::ustring source::get_clipboard_content() {
    auto utf_string = sf::Clipboard::getString().toUtf32();
    return utils::ustring(utf_string.begin(), utf_string.end());
}

void source::set_clipboard_content(const utils::ustring& content) {
    sf::Clipboard::setString(sf::String::fromUtf32(content.begin(), content.end()));
}

void source::set_mouse_cursor(const std::string& file_name, const gui::vector2i& hot_spot) {
    auto iter = cursor_map_.find(file_name);
    if (iter == cursor_map_.end()) {
        sf::Image image;
        if (!image.loadFromFile(file_name)) {
            throw gui::exception(
                "input::sfml::source", "Could not load cursor file '" + file_name + "'.");
        }

        auto cursor  = std::make_unique<sf::Cursor>();
        bool success = cursor->loadFromPixels(
            image.getPixelsPtr(), image.getSize(), sf::Vector2u(hot_spot.x, hot_spot.y));
        if (!success) {
            throw gui::exception(
                "input::sfml::source", "Could not load cursor file '" + file_name + "'.");
        }

        iter = cursor_map_.insert(std::make_pair(file_name, std::move(cursor))).first;
    }

    window_.setMouseCursor(*iter->second);
}

void source::reset_mouse_cursor() {
    const std::string name = "system_arrow";
    auto              iter = cursor_map_.find(name);
    if (iter == cursor_map_.end()) {
        auto cursor  = std::make_unique<sf::Cursor>();
        bool success = cursor->loadFromSystem(sf::Cursor::Arrow);
        if (!success) {
            throw gui::exception("input::sfml::source", "Could not reset cursor file to default.");
        }

        iter = cursor_map_.insert(std::make_pair(name, std::move(cursor))).first;
    }

    window_.setMouseCursor(*iter->second);
}

key source::from_sfml_(int sf_key) const {
    switch ((sf::Keyboard::Key)sf_key) {
    case sfKey::Escape: return key::k_escape;
    case sfKey::Num0: return key::k_0;
    case sfKey::Num1: return key::k_1;
    case sfKey::Num2: return key::k_2;
    case sfKey::Num3: return key::k_3;
    case sfKey::Num4: return key::k_4;
    case sfKey::Num5: return key::k_5;
    case sfKey::Num6: return key::k_6;
    case sfKey::Num7: return key::k_7;
    case sfKey::Num8: return key::k_8;
    case sfKey::Num9: return key::k_9;
    case sfKey::Hyphen: return key::k_minus;
    case sfKey::Equal: return key::k_equals;
    case sfKey::Backspace: return key::k_back;
    case sfKey::Tab: return key::k_tab;
    case sfKey::Q: return key::k_q;
    case sfKey::W: return key::k_w;
    case sfKey::E: return key::k_e;
    case sfKey::R: return key::k_r;
    case sfKey::T: return key::k_t;
    case sfKey::Y: return key::k_y;
    case sfKey::U: return key::k_u;
    case sfKey::I: return key::k_i;
    case sfKey::O: return key::k_o;
    case sfKey::P: return key::k_p;
    case sfKey::LBracket: return key::k_lbracket;
    case sfKey::RBracket: return key::k_rbracket;
    case sfKey::Enter: return key::k_return;
    case sfKey::LControl: return key::k_lcontrol;
    case sfKey::A: return key::k_a;
    case sfKey::S: return key::k_s;
    case sfKey::D: return key::k_d;
    case sfKey::F: return key::k_f;
    case sfKey::G: return key::k_g;
    case sfKey::H: return key::k_h;
    case sfKey::J: return key::k_j;
    case sfKey::K: return key::k_k;
    case sfKey::L: return key::k_l;
    case sfKey::Semicolon: return key::k_semicolon;
    case sfKey::Apostrophe: return key::k_apostrophe;
    case sfKey::LShift: return key::k_lshift;
    case sfKey::Backslash: return key::k_backslash;
    case sfKey::Z: return key::k_z;
    case sfKey::X: return key::k_x;
    case sfKey::C: return key::k_c;
    case sfKey::V: return key::k_v;
    case sfKey::B: return key::k_b;
    case sfKey::N: return key::k_n;
    case sfKey::M: return key::k_m;
    case sfKey::Comma: return key::k_comma;
    case sfKey::Period: return key::k_period;
    case sfKey::Slash: return key::k_slash;
    case sfKey::RShift: return key::k_rshift;
    case sfKey::Multiply: return key::k_multiply;
    case sfKey::LAlt: return key::k_lmenu;
    case sfKey::Space: return key::k_space;
    case sfKey::F1: return key::k_f1;
    case sfKey::F2: return key::k_f2;
    case sfKey::F3: return key::k_f3;
    case sfKey::F4: return key::k_f4;
    case sfKey::F5: return key::k_f5;
    case sfKey::F6: return key::k_f6;
    case sfKey::F7: return key::k_f7;
    case sfKey::F8: return key::k_f8;
    case sfKey::F9: return key::k_f9;
    case sfKey::F10: return key::k_f10;
    case sfKey::Numpad7: return key::k_numpad_7;
    case sfKey::Numpad8: return key::k_numpad_8;
    case sfKey::Numpad9: return key::k_numpad_9;
    case sfKey::Subtract: return key::k_subtract;
    case sfKey::Numpad4: return key::k_numpad_4;
    case sfKey::Numpad5: return key::k_numpad_5;
    case sfKey::Numpad6: return key::k_numpad_6;
    case sfKey::Add: return key::k_add;
    case sfKey::Numpad1: return key::k_numpad_1;
    case sfKey::Numpad2: return key::k_numpad_2;
    case sfKey::Numpad3: return key::k_numpad_3;
    case sfKey::Numpad0: return key::k_numpad_0;
    case sfKey::F11: return key::k_f11;
    case sfKey::F12: return key::k_f12;
    case sfKey::F13: return key::k_f13;
    case sfKey::F14: return key::k_f14;
    case sfKey::F15: return key::k_f15;
    case sfKey::RControl: return key::k_rcontrol;
    case sfKey::Divide: return key::k_divide;
    case sfKey::RAlt: return key::k_rmenu;
    case sfKey::Pause: return key::k_pause;
    case sfKey::Home: return key::k_home;
    case sfKey::Up: return key::k_up;
    case sfKey::PageUp: return key::k_pgup;
    case sfKey::Left: return key::k_left;
    case sfKey::Right: return key::k_right;
    case sfKey::End: return key::k_end;
    case sfKey::Down: return key::k_down;
    case sfKey::PageDown: return key::k_pgdown;
    case sfKey::Insert: return key::k_insert;
    case sfKey::Delete: return key::k_delete;
    case sfKey::LSystem: return key::k_lwin;
    case sfKey::RSystem: return key::k_rwin;
    case sfKey::Menu: return key::k_apps;
    default: return key::k_unassigned;
    }
}

void source::on_sfml_event(const sf::Event& event) {
    static const mouse_button mouse_from_sfml[3] = {
        mouse_button::left, mouse_button::right, mouse_button::middle};

    if (event.type == sf::Event::TextEntered) {
        auto c = event.text.unicode;
        // Remove non printable characters (< 32) and Del. (127)
        if (c >= 32 && c != 127)
            on_text_entered(c);
    } else if (event.type == sf::Event::MouseWheelScrolled) {
        mouse_.wheel += event.mouseWheelScroll.delta;
        const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window_);
        on_mouse_wheel(event.mouseWheelScroll.delta, gui::vector2f(mouse_pos.x, mouse_pos.y));
    } else if (event.type == sf::Event::Resized) {
        window_dimensions_ = gui::vector2ui(event.size.width, event.size.height);
        on_window_resized(window_dimensions_);
    } else if (event.type == sf::Event::KeyPressed) {
        key  key_id = from_sfml_(event.key.code);
        bool repeat = keyboard_.is_key_down[static_cast<std::size_t>(key_id)];
        keyboard_.is_key_down[static_cast<std::size_t>(key_id)] = true;
        if (repeat) {
            on_key_pressed_repeat(key_id);
        } else {
            on_key_pressed(key_id);
        }
    } else if (event.type == sf::Event::KeyReleased) {
        key key_id                                              = from_sfml_(event.key.code);
        keyboard_.is_key_down[static_cast<std::size_t>(key_id)] = false;
        on_key_released(key_id);
    } else if (event.type == sf::Event::MouseButtonPressed) {
        mouse_button button = mouse_from_sfml[event.mouseButton.button];
        mouse_.is_button_down[static_cast<std::size_t>(button)] = true;

        const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window_);
        on_mouse_pressed(button, gui::vector2f(mouse_pos.x, mouse_pos.y));
    } else if (event.type == sf::Event::MouseButtonReleased) {
        mouse_button button = mouse_from_sfml[event.mouseButton.button];
        mouse_.is_button_down[static_cast<std::size_t>(button)] = false;

        const sf::Vector2i mouse_pos = sf::Mouse::getPosition(window_);
        on_mouse_released(button, gui::vector2f(mouse_pos.x, mouse_pos.y));
    } else if (event.type == sf::Event::MouseMoved) {
        gui::vector2i mouse_pos(event.mouseMove.x, event.mouseMove.y);
        gui::vector2i mouse_delta;
        if (!first_mouse_move_) {
            mouse_delta    = mouse_pos - old_mouse_pos_;
            old_mouse_pos_ = mouse_pos;
        }

        first_mouse_move_ = false;

        mouse_.position = gui::vector2f(mouse_pos.x, mouse_pos.y);
        on_mouse_moved(gui::vector2f(mouse_delta.x, mouse_delta.y), mouse_.position);
    }
}
}} // namespace lxgui::input::sfml
