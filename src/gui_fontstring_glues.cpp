#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>

/** A @{LayeredRegion} that can draw text on the screen.
 *   This class holds a string and a reference to a font, which
 *   is used to draw the string on the screen. The appearance of
 *   the string can be changed (font, size, color, alignment, wrapping).
 *   In addition, it is possible to change the color of a portion of
 *   the string, for example to highlight a particular name.
 *
 *   __Sizing.__ The @{FontString} class has a special property when it
 *   comes to determining the size of its region on the screen, hence
 *   how other object anchor to it, and how it anchors to other objects.
 *   See the documentation for @{Region} for more information on anchors.
 *   While other @{Region}s must either have a fixed size or more than two
 *   anchors constraining their size, the @{FontString} does not. If only
 *   one anchor is specified, the width and height of the @{FontString} will
 *   be determined by the area occupied by the displayed text, however long and
 *   tall this may be. If the width is already constrained by the fixed size
 *   or anchors, then the text will word wrap (if allowed) and the
 *   @{FontString}'s height will be as tall as the height of the wrapped text.
 *   Finally, if both the width and height are constrained by fixed sizes or
 *   anchors, the text will simply word wrap (if allowed) and be cut to fit
 *   in the specified area.
 *
 *   __Formatting.__ If @{FontString:is_formatting_enabled} is true (default),
 *   the string displayed by the @{FontString} class can have special
 *   formatting. This is done by entering the pipe `|` character, followed by
 *   a command:
 *
 *   - `|caarrggbb` sets the color of the text. `aa`, `rr`, `gg`, and `bb`
 *   must be two-digit hexadecimal values representing each color
 *   component. For example, an opaque red color would be `|cffff0000`,
 *   while a semi-transparent blue would be `|c880000ff`.
 *
 *   - `|r` cancels the change of color from the previous `|c[...]`
 *   command. Color change commands stack up, and can be nested. In the
 *   formatted string
 *   `|cffff0000 hello |cff00ff00 world |r example |r formatting`,
 *   the word `hello` would be displayed in red, `world` in blue,
 *   `example` in red again, and `formatting` would be of whatever color
 *   is the default for the @{FontString} object (see
 *   @{FontString:get_text_color}).
 *
 *   - `|Tpath/to/texture.png:w:h|t` displays a texture. The command
 *   parameters must be contained within an opening `|T` and a closing
 *   `|t`. The parameters are separated by colons `:`. The first parameter
 *   must be the path to the texture file, and it is mandatory.
 *   The other two parameters are optional, and define the size of the
 *   displayed texture. If omitted, the texture will be displayed as a
 *   square matching the size of the current font. If only `w` is supplied,
 *   the texture will be displayed as a square of size `w` pixels.
 *   If both `w` and `h` are supplied, they set the width and height of the
 *   displayed texture, in pixels.
 *
 *   - `||` displays a single `|` character.
 *
 *   Inherits all methods from: @{Region}, @{LayeredRegion}.
 *
 *   Child classes: none.
 *   @classmod FontString
 */

namespace lxgui::gui {

void font_string::register_on_lua(sol::state& m_lua) {
    auto m_class = m_lua.new_usertype<font_string>(
        "FontString", sol::base_classes, sol::bases<region, layered_region>(),
        sol::meta_function::index, member_function<&font_string::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&font_string::set_lua_member_>());

    /** @function get_font
     */
    m_class.set_function("get_font", member_function<&font_string::get_font_name>());

    /** @function get_alignment_x
     */
    m_class.set_function("get_alignment_x", [](const font_string& m_self) {
        alignment_x m_alignment = m_self.get_alignment_x();
        switch (m_alignment) {
        case alignment_x::left: return "LEFT";
        case alignment_x::center: return "CENTER";
        case alignment_x::right: return "RIGHT";
        default: return "UNKNOWN";
        }
    });

    /** @function get_alignment_y
     */
    m_class.set_function("get_alignment_y", [](const font_string& m_self) {
        alignment_y m_alignment = m_self.get_alignment_y();
        switch (m_alignment) {
        case alignment_y::top: return "TOP";
        case alignment_y::middle: return "MIDDLE";
        case alignment_y::bottom: return "BOTTOM";
        default: return "UNKNOWN";
        }
    });

    /** @function get_shadow_color
     */
    m_class.set_function("get_shadow_color", [](const font_string& m_self) {
        const color& m_shadow_color = m_self.get_shadow_color();
        return std::make_tuple(
            m_shadow_color.r, m_shadow_color.g, m_shadow_color.b, m_shadow_color.a);
    });

    /** @function get_shadow_offset
     */
    m_class.set_function("get_shadow_offset", [](const font_string& m_self) {
        const vector2f& m_shadow_offsets = m_self.get_shadow_offset();
        return std::make_pair(m_shadow_offsets.x, m_shadow_offsets.y);
    });

    /** @function get_spacing
     */
    m_class.set_function("get_spacing", member_function<&font_string::get_spacing>());

    /** @function get_line_spacing
     */
    m_class.set_function("get_line_spacing", member_function<&font_string::get_line_spacing>());

    /** @function get_text_color
     */
    m_class.set_function("get_text_color", [](const font_string& m_self) {
        const color& m_text_color = m_self.get_text_color();
        return std::make_tuple(m_text_color.r, m_text_color.g, m_text_color.b, m_text_color.a);
    });

    /** @function set_font
     */
    m_class.set_function(
        "set_font", [](font_string& m_self, const std::string& file, float f_height,
                       sol::optional<std::string> flags) {
            m_self.set_font(file, f_height);

            if (flags.has_value()) {
                if (flags.value().find("OUTLINE") != std::string::npos ||
                    flags.value().find("THICKOUTLINE") != std::string::npos) {
                    m_self.set_outlined(true);
                } else if (flags.value().empty()) {
                    m_self.set_outlined(false);
                } else {
                    gui::out << gui::warning << "EditBox:set_font : "
                             << "Unknown flags : \"" << flags.value() << "\"." << std::endl;
                }
            } else {
                m_self.set_outlined(false);
            }
        });

    /** @function set_alignment_x
     */
    m_class.set_function("set_alignment_x", [](font_string& m_self, const std::string& justify_h) {
        if (justify_h == "LEFT")
            m_self.set_alignment_x(alignment_x::left);
        else if (justify_h == "CENTER")
            m_self.set_alignment_x(alignment_x::center);
        else if (justify_h == "RIGHT")
            m_self.set_alignment_x(alignment_x::right);
        else {
            gui::out << gui::warning << "font_string:set_alignment_x : "
                     << "Unknown justify behavior : \"" << justify_h << "\"." << std::endl;
        }
    });

    /** @function set_alignment_y
     */
    m_class.set_function("set_alignment_y", [](font_string& m_self, const std::string& justify_v) {
        if (justify_v == "TOP")
            m_self.set_alignment_y(alignment_y::top);
        else if (justify_v == "MIDDLE")
            m_self.set_alignment_y(alignment_y::middle);
        else if (justify_v == "BOTTOM")
            m_self.set_alignment_y(alignment_y::bottom);
        else {
            gui::out << gui::warning << "font_string:set_alignment_y : "
                     << "Unknown justify behavior : \"" << justify_v << "\"." << std::endl;
        }
    });

    /** @function set_shadow_color
     */
    m_class.set_function(
        "set_shadow_color",
        sol::overload(
            [](font_string& m_self, float f_r, float f_g, float f_b, sol::optional<float> f_a) {
                m_self.set_shadow_color(color(f_r, f_g, f_b, f_a.value_or(1.0f)));
            },
            [](font_string& m_self, const std::string& s) { m_self.set_shadow_color(color(s)); }));

    /** @function set_shadow_offset
     */
    m_class.set_function(
        "set_shadow_offset", [](font_string& m_self, float f_x_offset, float f_y_offset) {
            m_self.set_shadow_offset(vector2f(f_x_offset, f_y_offset));
        });

    /** @function set_spacing
     */
    m_class.set_function("set_spacing", member_function<&font_string::set_spacing>());

    /** @function set_line_spacing
     */
    m_class.set_function("set_line_spacing", member_function<&font_string::set_line_spacing>());

    /** @function set_text_color
     */
    m_class.set_function(
        "set_text_color",
        sol::overload(
            [](font_string& m_self, float f_r, float f_g, float f_b, sol::optional<float> f_a) {
                m_self.set_text_color(color(f_r, f_g, f_b, f_a.value_or(1.0f)));
            },
            [](font_string& m_self, const std::string& s) { m_self.set_text_color(color(s)); }));

    /** @function can_non_space_wrap
     */
    m_class.set_function("can_non_space_wrap", member_function<&font_string::can_non_space_wrap>());

    /** @function can_word_wrap
     */
    m_class.set_function("can_word_wrap", member_function<&font_string::can_word_wrap>());

    /** @function enable_formatting
     */
    m_class.set_function("enable_formatting", member_function<&font_string::enable_formatting>());

    /** @function get_string_height
     */
    m_class.set_function("get_string_height", member_function<&font_string::get_string_height>());

    /** @function get_string_width
     */
    m_class.set_function(
        "get_string_width", [](const font_string& m_self, sol::optional<std::string> text) {
            if (text.has_value())
                return m_self.get_string_width(utils::utf8_to_unicode(text.value()));
            else
                return m_self.get_string_width();
        });

    /** @function get_text
     */
    m_class.set_function("get_text", [](const font_string& m_self) {
        return utils::unicode_to_utf8(m_self.get_text());
    });

    /** @function is_formatting_enabled
     */
    m_class.set_function(
        "is_formatting_enabled", member_function<&font_string::is_formatting_enabled>());

    /** @function set_non_space_wrap
     */
    m_class.set_function("set_non_space_wrap", member_function<&font_string::set_non_space_wrap>());

    /** @function set_word_wrap
     */
    m_class.set_function(
        "set_word_wrap", [](font_string& m_self, bool b_wrap, sol::optional<bool> b_ellipsis) {
            m_self.set_word_wrap(b_wrap, b_ellipsis.value_or(false));
        });

    /** @function set_text
     */
    m_class.set_function(
        "set_text",
        sol::overload(
            [](font_string& m_self, bool b_value) {
                m_self.set_text(utils::utf8_to_unicode(
                    m_self.get_manager().get_localizer().localize(b_value ? "{true}" : "{false}")));
            },
            [](font_string& m_self, int value) {
                m_self.set_text(utils::utf8_to_unicode(
                    m_self.get_manager().get_localizer().format_string("{:L}", value)));
            },
            [](font_string& m_self, double d_value) {
                m_self.set_text(utils::utf8_to_unicode(
                    m_self.get_manager().get_localizer().format_string("{:L}", d_value)));
            },
            [](font_string& m_self, const std::string& text) {
                m_self.set_text(utils::utf8_to_unicode(text));
            }));
}

} // namespace lxgui::gui
