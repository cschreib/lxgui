#include "lxgui/gui_font_string.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/utils_string.hpp"

#include <lxgui/extern_sol2_state.hpp>

/** A @{LayeredRegion} that can draw text on the screen.
 * This class holds a string and a reference to a font, which
 * is used to draw the string on the screen. The appearance of
 * the string can be changed (font, size, color, alignment, wrapping).
 * In addition, it is possible to change the color of a portion of
 * the string, for example to highlight a particular name.
 *
 * __Sizing.__ The @{FontString} class has a special property when it
 * comes to determining the size of its region on the screen, hence
 * how other object anchor to it, and how it anchors to other objects.
 * See the documentation for @{Region} for more information on anchors.
 * While other @{Region}s must either have a fixed size or more than two
 * anchors constraining their size, the @{FontString} does not. If only
 * one anchor is specified, the width and height of the @{FontString} will
 * be determined by the area occupied by the displayed text, however long and
 * tall this may be. If the width is already constrained by the fixed size
 * or anchors, then the text will word wrap (if allowed) and the
 * @{FontString}'s height will be as tall as the height of the wrapped text.
 * Finally, if both the width and height are constrained by fixed sizes or
 * anchors, the text will simply word wrap (if allowed) and be cut to fit
 * in the specified area.
 *
 * __Formatting.__ If @{FontString:is_formatting_enabled} is true (default),
 * the string displayed by the @{FontString} class can have special
 * formatting. This is done by entering the pipe `|` character, followed by
 * a command:
 *
 * - `|caarrggbb` sets the color of the text. `aa`, `rr`, `gg`, and `bb`
 * must be two-digit hexadecimal values representing each color
 * component. For example, an opaque red color would be `|cffff0000`,
 * while a semi-transparent blue would be `|c880000ff`.
 *
 * - `|r` cancels the change of color from the previous `|c[...]`
 * command. Color change commands stack up, and can be nested. In the
 * formatted string
 * `|cffff0000 hello |cff00ff00 world |r example |r formatting`,
 * the word `hello` would be displayed in red, `world` in blue,
 * `example` in red again, and `formatting` would be of whatever color
 * is the default for the @{FontString} object (see
 * @{FontString:get_text_color}).
 *
 * - `|Tpath/to/texture.png:w:h|t` displays a texture. The command
 * parameters must be contained within an opening `|T` and a closing
 * `|t`. The parameters are separated by colons `:`. The first parameter
 * must be the path to the texture file, and it is mandatory.
 * The other two parameters are optional, and define the size of the
 * displayed texture. If omitted, the texture will be displayed as a
 * square matching the size of the current font. If only `w` is supplied,
 * the texture will be displayed as a square of size `w` pixels.
 * If both `w` and `h` are supplied, they set the width and height of the
 * displayed texture, in pixels.
 *
 * - `||` displays a single `|` character.
 *
 * Inherits all methods from: @{Region}, @{LayeredRegion}.
 *
 * Child classes: none.
 * @classmod FontString
 */

namespace lxgui::gui {

void font_string::register_on_lua(sol::state& lua) {
    auto type = lua.new_usertype<font_string>(
        font_string::class_name, sol::base_classes, sol::bases<region, layered_region>(),
        sol::meta_function::index, member_function<&font_string::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&font_string::set_lua_member_>());

    /** @function disable_formatting
     */
    type.set_function("disable_formatting", member_function<&font_string::disable_formatting>());

    /** @function disable_non_space_wrap
     */
    type.set_function(
        "disable_non_space_wrap", member_function<&font_string::disable_non_space_wrap>());

    /** @function disable_shadow
     */
    type.set_function("disable_shadow", member_function<&font_string::disable_shadow>());

    /** @function disable_word_ellipsis
     */
    type.set_function(
        "disable_word_ellipsis", member_function<&font_string::disable_word_ellipsis>());

    /** @function disable_word_wrap
     */
    type.set_function("disable_word_wrap", member_function<&font_string::disable_word_wrap>());

    /** @function enable_formatting
     */
    type.set_function("enable_formatting", member_function<&font_string::enable_formatting>());

    /** @function enable_non_space_wrap
     */
    type.set_function(
        "enable_non_space_wrap", member_function<&font_string::enable_non_space_wrap>());

    /** @function enable_shadow
     */
    type.set_function("enable_shadow", member_function<&font_string::enable_shadow>());

    /** @function enable_word_ellipsis
     */
    type.set_function(
        "enable_word_ellipsis", member_function<&font_string::enable_word_ellipsis>());

    /** @function enable_word_wrap
     */
    type.set_function("enable_word_wrap", member_function<&font_string::enable_word_wrap>());

    /** @function get_font
     */
    type.set_function("get_font", member_function<&font_string::get_font_name>());

    /** @function get_alignment_x
     */
    type.set_function("get_alignment_x", [](const font_string& self) {
        return utils::to_string(self.get_alignment_x());
    });

    /** @function get_alignment_y
     */
    type.set_function("get_alignment_y", [](const font_string& self) {
        return utils::to_string(self.get_alignment_y());
    });

    /** @function get_shadow_color
     */
    type.set_function("get_shadow_color", [](const font_string& self) {
        const color& shadow_color = self.get_shadow_color();
        return std::make_tuple(shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a);
    });

    /** @function get_shadow_offset
     */
    type.set_function("get_shadow_offset", [](const font_string& self) {
        const vector2f& shadow_offsets = self.get_shadow_offset();
        return std::make_pair(shadow_offsets.x, shadow_offsets.y);
    });

    /** @function get_spacing
     */
    type.set_function("get_spacing", member_function<&font_string::get_spacing>());

    /** @function get_line_spacing
     */
    type.set_function("get_line_spacing", member_function<&font_string::get_line_spacing>());

    /** @function get_text_color
     */
    type.set_function("get_text_color", [](const font_string& self) {
        const color& text_color = self.get_text_color();
        return std::make_tuple(text_color.r, text_color.g, text_color.b, text_color.a);
    });

    /** @function set_font
     */
    type.set_function(
        "set_font", [](font_string& self, const std::string& file, float height,
                       sol::optional<std::string> flags) {
            self.set_font(file, height);

            if (flags.has_value()) {
                if (flags.value().find("OUTLINE") != std::string::npos ||
                    flags.value().find("THICKOUTLINE") != std::string::npos) {
                    self.set_outlined(true);
                } else if (flags.value().empty()) {
                    self.set_outlined(false);
                } else {
                    gui::out << gui::warning << "EditBox:set_font: "
                             << "Unknown flags: \"" << flags.value() << "\"." << std::endl;
                }
            } else {
                self.set_outlined(false);
            }
        });

    /** @function set_alignment_x
     */
    type.set_function("set_alignment_x", [](font_string& self, const std::string& align_h) {
        if (auto converted = utils::from_string<alignment_x>(align_h); converted.has_value()) {
            self.set_alignment_x(converted.value());
        } else {
            gui::out << gui::warning << "font_string:set_alignment_x: "
                     << "Unknown alignment: \"" << align_h << "\"." << std::endl;
        }
    });

    /** @function set_alignment_y
     */
    type.set_function("set_alignment_y", [](font_string& self, const std::string& align_v) {
        if (auto converted = utils::from_string<alignment_y>(align_v); converted.has_value()) {
            self.set_alignment_y(converted.value());
        } else {
            gui::out << gui::warning << "font_string:set_alignment_y: "
                     << "Unknown alignment: \"" << align_v << "\"." << std::endl;
        }
    });

    /** @function set_shadow_color
     */
    type.set_function(
        "set_shadow_color",
        sol::overload(
            [](font_string& self, float r, float g, float b, sol::optional<float> a) {
                self.set_shadow_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](font_string& self, const std::string& s) { self.set_shadow_color(color(s)); }));

    /** @function set_shadow_offset
     */
    type.set_function("set_shadow_offset", [](font_string& self, float x_offset, float y_offset) {
        self.set_shadow_offset(vector2f(x_offset, y_offset));
    });

    /** @function set_spacing
     */
    type.set_function("set_spacing", member_function<&font_string::set_spacing>());

    /** @function set_line_spacing
     */
    type.set_function("set_line_spacing", member_function<&font_string::set_line_spacing>());

    /** @function set_text_color
     */
    type.set_function(
        "set_text_color",
        sol::overload(
            [](font_string& self, float r, float g, float b, sol::optional<float> a) {
                self.set_text_color(color(r, g, b, a.value_or(1.0f)));
            },
            [](font_string& self, const std::string& s) { self.set_text_color(color(s)); }));

    /** @function is_non_space_wrap_enabled
     */
    type.set_function(
        "is_non_space_wrap_enabled", member_function<&font_string::is_non_space_wrap_enabled>());

    /** @function is_word_wrap_enabled
     */
    type.set_function(
        "is_word_wrap_enabled", member_function<&font_string::is_word_wrap_enabled>());

    /** @function enable_formatting
     */
    type.set_function("enable_formatting", member_function<&font_string::enable_formatting>());

    /** @function get_string_height
     */
    type.set_function("get_string_height", member_function<&font_string::get_string_height>());

    /** @function get_string_width
     */
    type.set_function(
        "get_string_width", [](const font_string& self, sol::optional<std::string> text) {
            if (text.has_value())
                return self.get_string_width(utils::utf8_to_unicode(text.value()));
            else
                return self.get_string_width();
        });

    /** @function get_text
     */
    type.set_function("get_text", [](const font_string& self) {
        return utils::unicode_to_utf8(self.get_text());
    });

    /** @function get_vertex_cache_strategy
     */
    type.set_function("get_vertex_cache_strategy", [](const font_string& self) {
        return utils::to_string(self.get_vertex_cache_strategy());
    });

    /** @function is_formatting_enabled
     */
    type.set_function(
        "is_formatting_enabled", member_function<&font_string::is_formatting_enabled>());

    /** @function set_non_space_wrap_enabled
     */
    type.set_function(
        "set_non_space_wrap_enabled", member_function<&font_string::set_non_space_wrap_enabled>());

    /** @function set_word_wrap_enabled
     */
    type.set_function(
        "set_word_wrap_enabled", member_function<&font_string::set_word_wrap_enabled>());

    /** @function set_word_ellipsis_enabled
     */
    type.set_function(
        "set_word_ellipsis_enabled", member_function<&font_string::set_word_ellipsis_enabled>());

    /** @function set_text
     */
    type.set_function(
        "set_text",
        sol::overload(
            [](font_string& self, bool value) {
                self.set_text(utils::utf8_to_unicode(
                    self.get_manager().get_localizer().localize(value ? "{true}" : "{false}")));
            },
            [](font_string& self, int value) {
                self.set_text(utils::utf8_to_unicode(
                    self.get_manager().get_localizer().format_string("{:L}", value)));
            },
            [](font_string& self, double value) {
                self.set_text(utils::utf8_to_unicode(
                    self.get_manager().get_localizer().format_string("{:L}", value)));
            },
            [](font_string& self, const std::string& text) {
                self.set_text(utils::utf8_to_unicode(text));
            }));

    /** @function set_vertex_cache_strategy
     */
    type.set_function(
        "set_vertex_cache_strategy", [](font_string& self, const std::string& strategy_name) {
            if (auto strategy = utils::from_string<vertex_cache_strategy>(strategy_name);
                strategy.has_value()) {
                self.set_vertex_cache_strategy(strategy.value());
            } else {
                gui::out << gui::warning << "font_string:set_vertex_cache_strategy: "
                         << "Unknown strategy: \"" << strategy_name << "\"." << std::endl;
            }
        });
}

} // namespace lxgui::gui
