#include "lxgui/impl/input_sfml_source.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Window.hpp>

using sf::Keyboard;
using sf::Mouse;

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

        auto cursor = std::make_unique<sf::Cursor>();
        cursor->loadFromPixels(
            image.getPixelsPtr(), image.getSize(), sf::Vector2u(hot_spot.x, hot_spot.y));
        iter = cursor_map_.insert(std::make_pair(file_name, std::move(cursor))).first;
    }

    window_.setMouseCursor(*iter->second);
}

void source::reset_mouse_cursor() {
    const std::string name = "system_arrow";
    auto              iter = cursor_map_.find(name);
    if (iter == cursor_map_.end()) {
        auto cursor = std::make_unique<sf::Cursor>();
        cursor->loadFromSystem(sf::Cursor::Arrow);
        iter = cursor_map_.insert(std::make_pair(name, std::move(cursor))).first;
    }

    window_.setMouseCursor(*iter->second);
}

key source::from_sfml_(int sf_key) const {
    switch ((sf::Keyboard::Key)sf_key) {
    case Keyboard::Escape: return key::k_escape;
    case Keyboard::Num0: return key::k_0;
    case Keyboard::Num1: return key::k_1;
    case Keyboard::Num2: return key::k_2;
    case Keyboard::Num3: return key::k_3;
    case Keyboard::Num4: return key::k_4;
    case Keyboard::Num5: return key::k_5;
    case Keyboard::Num6: return key::k_6;
    case Keyboard::Num7: return key::k_7;
    case Keyboard::Num8: return key::k_8;
    case Keyboard::Num9: return key::k_9;
    case Keyboard::Dash: return key::k_minus;
    case Keyboard::Equal: return key::k_equals;
    case Keyboard::BackSpace: return key::k_back;
    case Keyboard::Tab: return key::k_tab;
    case Keyboard::Q: return key::k_q;
    case Keyboard::W: return key::k_w;
    case Keyboard::E: return key::k_e;
    case Keyboard::R: return key::k_r;
    case Keyboard::T: return key::k_t;
    case Keyboard::Y: return key::k_y;
    case Keyboard::U: return key::k_u;
    case Keyboard::I: return key::k_i;
    case Keyboard::O: return key::k_o;
    case Keyboard::P: return key::k_p;
    case Keyboard::LBracket: return key::k_lbracket;
    case Keyboard::RBracket: return key::k_rbracket;
    case Keyboard::Return: return key::k_return;
    case Keyboard::LControl: return key::k_lcontrol;
    case Keyboard::A: return key::k_a;
    case Keyboard::S: return key::k_s;
    case Keyboard::D: return key::k_d;
    case Keyboard::F: return key::k_f;
    case Keyboard::G: return key::k_g;
    case Keyboard::H: return key::k_h;
    case Keyboard::J: return key::k_j;
    case Keyboard::K: return key::k_k;
    case Keyboard::L: return key::k_l;
    case Keyboard::SemiColon: return key::k_semicolon;
    case Keyboard::Quote: return key::k_apostrophe;
    case Keyboard::LShift: return key::k_lshift;
    case Keyboard::BackSlash: return key::k_backslash;
    case Keyboard::Z: return key::k_z;
    case Keyboard::X: return key::k_x;
    case Keyboard::C: return key::k_c;
    case Keyboard::V: return key::k_v;
    case Keyboard::B: return key::k_b;
    case Keyboard::N: return key::k_n;
    case Keyboard::M: return key::k_m;
    case Keyboard::Comma: return key::k_comma;
    case Keyboard::Period: return key::k_period;
    case Keyboard::Slash: return key::k_slash;
    case Keyboard::RShift: return key::k_rshift;
    case Keyboard::Multiply: return key::k_multiply;
    case Keyboard::LAlt: return key::k_lmenu;
    case Keyboard::Space: return key::k_space;
    case Keyboard::F1: return key::k_f1;
    case Keyboard::F2: return key::k_f2;
    case Keyboard::F3: return key::k_f3;
    case Keyboard::F4: return key::k_f4;
    case Keyboard::F5: return key::k_f5;
    case Keyboard::F6: return key::k_f6;
    case Keyboard::F7: return key::k_f7;
    case Keyboard::F8: return key::k_f8;
    case Keyboard::F9: return key::k_f9;
    case Keyboard::F10: return key::k_f10;
    case Keyboard::Numpad7: return key::k_numpad_7;
    case Keyboard::Numpad8: return key::k_numpad_8;
    case Keyboard::Numpad9: return key::k_numpad_9;
    case Keyboard::Subtract: return key::k_subtract;
    case Keyboard::Numpad4: return key::k_numpad_4;
    case Keyboard::Numpad5: return key::k_numpad_5;
    case Keyboard::Numpad6: return key::k_numpad_6;
    case Keyboard::Add: return key::k_add;
    case Keyboard::Numpad1: return key::k_numpad_1;
    case Keyboard::Numpad2: return key::k_numpad_2;
    case Keyboard::Numpad3: return key::k_numpad_3;
    case Keyboard::Numpad0: return key::k_numpad_0;
    case Keyboard::F11: return key::k_f11;
    case Keyboard::F12: return key::k_f12;
    case Keyboard::F13: return key::k_f13;
    case Keyboard::F14: return key::k_f14;
    case Keyboard::F15: return key::k_f15;
    case Keyboard::RControl: return key::k_rcontrol;
    case Keyboard::Divide: return key::k_divide;
    case Keyboard::RAlt: return key::k_rmenu;
    case Keyboard::Pause: return key::k_pause;
    case Keyboard::Home: return key::k_home;
    case Keyboard::Up: return key::k_up;
    case Keyboard::PageUp: return key::k_pgup;
    case Keyboard::Left: return key::k_left;
    case Keyboard::Right: return key::k_right;
    case Keyboard::End: return key::k_end;
    case Keyboard::Down: return key::k_down;
    case Keyboard::PageDown: return key::k_pgdown;
    case Keyboard::Insert: return key::k_insert;
    case Keyboard::Delete: return key::k_delete;
    case Keyboard::LSystem: return key::k_lwin;
    case Keyboard::RSystem: return key::k_rwin;
    case Keyboard::Menu: return key::k_apps;
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
    } else if (event.type == sf::Event::MouseWheelMoved) {
        mouse_.wheel += event.mouseWheel.delta;
        const sf::Vector2i mouse_pos = Mouse::getPosition(window_);
        on_mouse_wheel(event.mouseWheel.delta, gui::vector2f(mouse_pos.x, mouse_pos.y));
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

        const sf::Vector2i mouse_pos = Mouse::getPosition(window_);
        on_mouse_pressed(button, gui::vector2f(mouse_pos.x, mouse_pos.y));
    } else if (event.type == sf::Event::MouseButtonReleased) {
        mouse_button button = mouse_from_sfml[event.mouseButton.button];
        mouse_.is_button_down[static_cast<std::size_t>(button)] = false;

        const sf::Vector2i mouse_pos = Mouse::getPosition(window_);
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
