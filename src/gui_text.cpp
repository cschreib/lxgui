#include "lxgui/gui_text.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_vertex_cache.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_range.hpp"

#include <map>

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui::gui {

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace parser {
enum class color_action { none, set, reset };

struct format {
    color        col    = color::white;
    color_action action = color_action::none;
};

struct texture {
    std::string               file_name;
    float                     width  = 0.0f;
    float                     height = 0.0f;
    std::shared_ptr<material> mat;
};

using item = std::variant<char32_t, format, texture>;

struct line {
    std::vector<item> content;
    float             width = 0.0f;
};

std::vector<item>
parse_string(renderer& renderer, const utils::ustring_view& caption, bool formatting_enabled) {
    std::vector<item> content;
    for (auto iter_char = caption.begin(); iter_char != caption.end(); ++iter_char) {
        // Read format tags
        if (*iter_char == U'|' && formatting_enabled) {
            ++iter_char;
            if (iter_char == caption.end())
                break;

            if (*iter_char != U'|') {
                if (*iter_char == U'r') {
                    format format;
                    format.action = color_action::reset;
                    content.push_back(format);
                } else if (*iter_char == U'c') {
                    format format;
                    format.action = color_action::set;

                    auto read_two = [&](float& out_value) {
                        ++iter_char;
                        if (iter_char == caption.end())
                            return false;
                        utils::ustring color_part(2, U'0');
                        color_part[0] = *iter_char;
                        ++iter_char;
                        if (iter_char == caption.end())
                            return false;
                        color_part[1] = *iter_char;
                        out_value = utils::hex_to_uint(utils::unicode_to_utf8(color_part)) / 255.0f;
                        return true;
                    };

                    if (!read_two(format.col.a))
                        break;
                    if (!read_two(format.col.r))
                        break;
                    if (!read_two(format.col.g))
                        break;
                    if (!read_two(format.col.b))
                        break;

                    content.push_back(format);
                } else if (*iter_char == U'T') {
                    ++iter_char;

                    const auto begin = iter_char - caption.begin();
                    const auto pos   = caption.find(U"|t", begin);
                    if (pos == caption.npos)
                        break;

                    const std::string extracted =
                        utils::unicode_to_utf8(caption.substr(begin, pos - begin));

                    const auto words = utils::cut(extracted, ":");
                    if (!words.empty()) {
                        texture texture;
                        texture.mat = renderer.create_atlas_material("GUI", std::string{words[0]});
                        texture.width = texture.height = std::numeric_limits<float>::quiet_NaN();

                        if (words.size() == 2) {
                            texture.width =
                                utils::from_string<float>(words[1]).value_or(texture.width);
                            texture.height = texture.width;
                        } else if (words.size() > 2) {
                            texture.width =
                                utils::from_string<float>(words[1]).value_or(texture.width);
                            texture.height =
                                utils::from_string<float>(words[2]).value_or(texture.height);
                        }

                        content.push_back(texture);
                    }

                    iter_char += extracted.size() + 1;
                }

                continue;
            }
        }

        // Add characters
        content.push_back(*iter_char);
    }

    return content;
}

bool is_whitespace(const item& i) {
    return std::visit(
        [](const auto& value) {
            using type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<type, char32_t>) {
                return utils::is_whitespace(value);
            } else {
                return false;
            }
        },
        i);
}

bool is_word(const item& i) {
    return std::visit(
        [](const auto& value) {
            using type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<type, char32_t>) {
                return !utils::is_whitespace(value);
            } else {
                return false;
            }
        },
        i);
}

bool is_character(const item& i) {
    return i.index() == 0u;
}

bool is_format(const item& i) {
    return i.index() == 1u;
}

bool is_character(const item& i, char32_t c) {
    return i.index() == 0u && std::get<char32_t>(i) == c;
}

float get_width(const text& text, const item& i) {
    return std::visit(
        [&](const auto& value) {
            using type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<type, char32_t>) {
                return text.get_character_width(value);
            } else if constexpr (std::is_same_v<type, texture>) {
                if (std::isnan(value.width))
                    return text.get_line_height();
                else
                    return value.width * text.get_scaling_factor();
            } else {
                return 0.0f;
            }
        },
        i);
}

float get_kerning(const text& txt, const item& i1, const item& i2) {
    return std::visit(
        [&](const auto& value1) {
            using type1 = std::decay_t<decltype(value1)>;
            if constexpr (std::is_same_v<type1, char32_t>) {
                return std::visit(
                    [&](const auto& value2) {
                        using type2 = std::decay_t<decltype(value2)>;
                        if constexpr (std::is_same_v<type2, char32_t>) {
                            return txt.get_character_kerning(value1, value2);
                        } else {
                            return 0.0f;
                        }
                    },
                    i2);
            } else {
                return 0.0f;
            }
        },
        i1);
}

float get_tracking(const text& txt, const item& i) {
    return std::visit(
        [&](const auto& value) {
            using type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<type, char32_t>) {
                if (value != U'\n')
                    return txt.get_tracking();
                else
                    return 0.0f;
            } else {
                return 0.0f;
            }
        },
        i);
}

std::pair<float, float> get_advance(
    const text&                       txt,
    std::vector<item>::const_iterator iter_char,
    std::vector<item>::const_iterator iter_begin) {
    float advance = parser::get_width(txt, *iter_char);
    float kerning = 0.0f;

    auto iter_prev = iter_char;
    while (iter_prev != iter_begin) {
        --iter_prev;
        if (parser::is_format(*iter_prev))
            continue;

        kerning = parser::get_tracking(txt, *iter_char);

        if (!parser::is_whitespace(*iter_char) && !parser::is_whitespace(*iter_prev))
            kerning += parser::get_kerning(txt, *iter_prev, *iter_char);

        break;
    }

    return std::make_pair(kerning, advance);
}

float get_full_advance(
    const text&                       txt,
    std::vector<item>::const_iterator iter_char,
    std::vector<item>::const_iterator iter_begin) {
    const auto advance = get_advance(txt, iter_char, iter_begin);
    return advance.first + advance.second;
}

float get_string_width(const text& txt, const std::vector<item>& content) {
    float width     = 0.0f;
    float max_width = 0.0f;

    for (auto iter_char : utils::range::iterator(content)) {
        if (parser::is_character(*iter_char, U'\n')) {
            if (width > max_width)
                max_width = width;

            width = 0.0f;
        } else {
            width += parser::get_full_advance(txt, iter_char, content.begin());
        }
    }

    if (width > max_width)
        max_width = width;

    return max_width;
}
} // namespace parser
/** \endcond
 */

text::text(
    renderer& rdr, std::shared_ptr<const font> fnt, std::shared_ptr<const font> outline_font) :
    renderer_(rdr), font_(std::move(fnt)), outline_font_(std::move(outline_font)) {}

float text::get_line_height() const {
    if (font_)
        return font_->get_size() * scaling_factor_;
    else
        return 0.0;
}

void text::set_scaling_factor(float scaling_factor) {
    if (scaling_factor_ == scaling_factor)
        return;

    scaling_factor_ = scaling_factor;
    notify_cache_dirty_();
}

float text::get_scaling_factor() const {
    return scaling_factor_;
}

void text::set_text(const utils::ustring& content) {
    if (unicode_text_ == content)
        return;

    unicode_text_ = content;

    notify_cache_dirty_();
}

const utils::ustring& text::get_text() const {
    return unicode_text_;
}

void text::set_color(const color& c, bool force_color) {
    if (color_ == c && force_color_ == force_color)
        return;

    color_       = c;
    force_color_ = force_color;

    notify_vertex_cache_dirty_();
}

const color& text::get_color() const {
    return color_;
}

void text::set_alpha(float alpha) {
    if (alpha == alpha_)
        return;

    alpha_ = alpha;

    notify_vertex_cache_dirty_();
}

float text::get_alpha() const {
    return alpha_;
}

void text::set_box_dimensions(float box_width, float box_height) {
    if (box_width_ == box_width && box_height_ == box_height)
        return;

    box_width_  = box_width;
    box_height_ = box_height;

    notify_cache_dirty_();
}

void text::set_box_width(float box_width) {
    if (box_width_ == box_width)
        return;

    box_width_ = box_width;

    notify_cache_dirty_();
}

void text::set_box_height(float box_height) {
    if (box_height_ == box_height)
        return;

    box_height_ = box_height;

    notify_cache_dirty_();
}

float text::get_width() const {
    update_();
    return width_;
}

float text::get_height() const {
    update_();
    return height_;
}

float text::get_box_width() const {
    return box_width_;
}

float text::get_box_height() const {
    return box_height_;
}

float text::get_text_width() const {
    return get_string_width(unicode_text_);
}

float text::get_text_height() const {
    if (!font_)
        return 0.0f;

    std::size_t count  = std::count(unicode_text_.begin(), unicode_text_.end(), U'\n');
    float       height = (1.0f + count * line_spacing_) * get_line_height();

    return height;
}

std::size_t text::get_line_count() const {
    update_();
    return num_lines_;
}

float text::get_string_width(const std::string& content) const {
    return get_string_width(utils::utf8_to_unicode(content));
}

float text::get_string_width(const utils::ustring& content) const {
    if (!font_)
        return 0.0f;

    return parser::get_string_width(
        *this, parser::parse_string(renderer_, content, formatting_enabled_));
}

float text::get_character_width(char32_t c) const {
    if (!font_)
        return 0.0f;
    else if (c == U'\t')
        return 4.0f * font_->get_character_width(U' ') * scaling_factor_;
    else
        return font_->get_character_width(c) * scaling_factor_;
}

float text::get_character_kerning(char32_t c1, char32_t c2) const {
    return font_->get_character_kerning(c1, c2) * scaling_factor_;
}

void text::set_alignment_x(alignment_x align_x) {
    if (align_x_ == align_x)
        return;

    align_x_ = align_x;

    notify_cache_dirty_();
}

void text::set_alignment_y(alignment_y align_y) {
    if (align_y_ == align_y)
        return;

    align_y_ = align_y;

    notify_cache_dirty_();
}

alignment_x text::get_alignment_x() const {
    return align_x_;
}

alignment_y text::get_alignment_y() const {
    return align_y_;
}

void text::set_tracking(float tracking) {
    if (tracking_ == tracking)
        return;

    tracking_ = tracking;

    notify_cache_dirty_();
}

float text::get_tracking() const {
    return tracking_;
}

void text::set_line_spacing(float line_spacing) {
    if (line_spacing_ == line_spacing)
        return;

    line_spacing_ = line_spacing;

    notify_cache_dirty_();
}

float text::get_line_spacing() const {
    return line_spacing_;
}

void text::set_remove_starting_spaces(bool remove_starting_spaces) {
    if (remove_starting_spaces_ == remove_starting_spaces)
        return;

    remove_starting_spaces_ = remove_starting_spaces;

    notify_cache_dirty_();
}

bool text::get_remove_starting_spaces() const {
    return remove_starting_spaces_;
}

void text::set_word_wrap_enabled(bool wrap) {
    if (word_wrap_enabled_ == wrap)
        return;

    word_wrap_enabled_ = wrap;

    notify_cache_dirty_();
}

bool text::is_word_wrap_enabled() const {
    return word_wrap_enabled_;
}

void text::set_word_ellipsis_enabled(bool add_ellipsis) {
    if (ellipsis_enabled_ == add_ellipsis)
        return;

    ellipsis_enabled_ = add_ellipsis;

    notify_cache_dirty_();
}

bool text::is_word_ellipsis_enabled() const {
    return ellipsis_enabled_;
}

void text::set_formatting_enabled(bool formatting) {
    if (formatting == formatting_enabled_)
        return;

    formatting_enabled_ = formatting;

    notify_vertex_cache_dirty_();
}

void text::set_use_vertex_cache(bool use_vertex_cache) {
    use_vertex_cache_flag_ = use_vertex_cache;
}

bool text::get_use_vertex_cache() const {
    return use_vertex_cache_flag_;
}

bool text::use_vertex_cache_() const {
    return renderer_.is_vertex_cache_supported() && use_vertex_cache_flag_;
}

void text::render(const matrix4f& transform) const {
    if (!font_ || unicode_text_.empty())
        return;

    update_();

    bool use_vertex_cache = use_vertex_cache_();
    if (use_vertex_cache) {
        update_vertex_cache_();
    }

    if (outline_font_) {
        if (const auto mat = outline_font_->get_texture().lock()) {
            if (use_vertex_cache && outline_vertex_cache_) {
                renderer_.render_cache(mat.get(), *outline_vertex_cache_, transform);
            } else {
                std::vector<std::array<vertex, 4>> quads_copy = outline_quad_list_;
                for (auto& quad : quads_copy) {
                    for (std::size_t i = 0; i < 4; ++i) {
                        quad[i].pos = quad[i].pos * transform;
                        quad[i].col.a *= alpha_;
                    }
                }

                renderer_.render_quads(mat.get(), quads_copy);
            }
        }
    }

    if (const auto mat = font_->get_texture().lock()) {
        if (use_vertex_cache && vertex_cache_) {
            renderer_.render_cache(mat.get(), *vertex_cache_, transform);
        } else {
            std::vector<std::array<vertex, 4>> quads_copy = quad_list_;
            for (auto& quad : quads_copy) {
                for (std::size_t i = 0; i < 4; ++i) {
                    quad[i].pos = quad[i].pos * transform;

                    if (!formatting_enabled_ || force_color_ || quad[i].col == color::empty) {
                        quad[i].col = color_;
                    }

                    quad[i].col.a *= alpha_;
                }
            }

            renderer_.render_quads(mat.get(), quads_copy);
        }

        for (auto quad : icons_list_) {
            for (std::size_t i = 0; i < 4; ++i) {
                quad.v[i].pos = quad.v[i].pos * transform;
                quad.v[i].col.a *= alpha_;
            }

            renderer_.render_quad(quad);
        }
    }
}

void text::notify_cache_dirty_() const {
    update_cache_flag_ = true;
}

void text::notify_vertex_cache_dirty_() const {
    update_vertex_cache_flag_ = true;
}

float text::round_to_pixel_(float value, utils::rounding_method method) const {
    return utils::round(value, scaling_factor_, method);
}

void text::update_() const {
    if (!font_ || !update_cache_flag_)
        return;

    // Update the line list, read format tags, do word wrapping, ...
    std::vector<parser::line> line_list;

    DEBUG_LOG("     Get max line nbr");
    std::size_t max_line_nbr = 0;
    if (box_height_ != 0.0f && !std::isinf(box_height_)) {
        if (box_height_ < get_line_height()) {
            max_line_nbr = 0;
        } else {
            float remaining = box_height_ - get_line_height();
            max_line_nbr    = 1 + static_cast<std::size_t>(
                                   std::floor(remaining / (get_line_height() * line_spacing_)));
        }
    } else
        max_line_nbr = std::numeric_limits<std::size_t>::max();

    if (max_line_nbr != 0) {
        auto manual_line_list = utils::cut_each(unicode_text_, U"\n");
        for (auto iter_manual : utils::range::iterator(manual_line_list)) {
            DEBUG_LOG("     Line: '" + utils::unicode_to_utf8(*iterManual) + "'");

            // Parse the line
            std::vector<parser::item> parsed_content =
                parser::parse_string(renderer_, *iter_manual, formatting_enabled_);

            // Make a temporary line array
            std::vector<parser::line> lines;

            auto         iter_line_begin = parsed_content.begin();
            parser::line line;
            line.width = 0.0f;

            bool done = false;
            for (auto iter_char1 = parsed_content.begin(); iter_char1 != parsed_content.end();
                 ++iter_char1) {
                DEBUG_LOG("      Get width");
                line.width += parser::get_full_advance(*this, iter_char1, iter_line_begin);
                line.content.push_back(*iter_char1);

                if (round_to_pixel_(line.width - box_width_) > 0) {
                    DEBUG_LOG(
                        "      Box break " + utils::to_string(line.width) + " > " +
                        utils::to_string(box_width_));

                    // Whoops, the line is too long...
                    auto iter_space = std::find_if(
                        line.content.begin(), line.content.end(), &parser::is_whitespace);

                    if (iter_space != line.content.end() && word_wrap_enabled_) {
                        DEBUG_LOG("       Spaced");
                        // There are several words on this line, we'll
                        // be able to put the last one on the next line
                        auto                      iter_char2 = iter_char1 + 1;
                        std::vector<parser::item> erased_content;
                        std::size_t               chars_to_erase  = 0;
                        float                     last_word_width = 0.0f;
                        bool                      last_was_word   = false;
                        while (line.width > box_width_ && iter_char2 != iter_line_begin) {
                            --iter_char2;

                            if (parser::is_whitespace(*iter_char2)) {
                                if (!last_was_word || remove_starting_spaces_ ||
                                    line.width - last_word_width > box_width_) {
                                    last_word_width += parser::get_full_advance(
                                        *this, iter_char2, iter_line_begin);
                                    erased_content.insert(erased_content.begin(), *iter_char2);
                                    ++chars_to_erase;

                                    line.width -= last_word_width;
                                    last_word_width = 0.0f;
                                } else
                                    break;
                            } else {
                                last_word_width +=
                                    parser::get_full_advance(*this, iter_char2, iter_line_begin);
                                erased_content.insert(erased_content.begin(), *iter_char2);
                                ++chars_to_erase;

                                last_was_word = true;
                            }
                        }

                        if (remove_starting_spaces_) {
                            while (iter_char2 != iter_char1 + 1 &&
                                   parser::is_whitespace(*iter_char2)) {
                                --chars_to_erase;
                                erased_content.erase(erased_content.begin());
                                ++iter_char2;
                            }
                        }

                        line.width -= last_word_width;
                        line.content.erase(line.content.end() - chars_to_erase, line.content.end());
                        lines.push_back(line);

                        line.width      = parser::get_string_width(*this, erased_content);
                        line.content    = erased_content;
                        iter_line_begin = iter_char1 - (line.content.size() - 1u);
                    } else {
                        DEBUG_LOG("       Single word");
                        // There is only one word on this line, or word
                        // wrap is disabled. Anyway, this line is just
                        // too long for the text box: our only option
                        // is to truncate it.
                        if (ellipsis_enabled_) {
                            DEBUG_LOG("       Ellipsis");
                            // FIXME: this doesn't account for kerning between the "..." and prev
                            // char
                            float       word_width     = get_string_width(U"...");
                            auto        iter_char2     = iter_char1 + 1;
                            std::size_t chars_to_erase = 0;
                            while (line.width + word_width > box_width_ &&
                                   iter_char2 != iter_line_begin) {
                                --iter_char2;
                                line.width -=
                                    parser::get_full_advance(*this, iter_char2, iter_line_begin);
                                ++chars_to_erase;
                            }

                            DEBUG_LOG(
                                "       Char to erase: " + utils::to_string(chars_to_erase) +
                                " / " + utils::to_string(line.content.size()));

                            line.content.erase(
                                line.content.end() - chars_to_erase, line.content.end());
                            line.content.push_back(U'.');
                            line.content.push_back(U'.');
                            line.content.push_back(U'.');
                            line.width += word_width;
                        } else {
                            DEBUG_LOG("       Truncate");
                            auto        iter_char2     = iter_char1 + 1;
                            std::size_t chars_to_erase = 0;
                            while (line.width > box_width_ && iter_char2 != iter_line_begin) {
                                --iter_char2;
                                line.width -=
                                    parser::get_full_advance(*this, iter_char2, iter_line_begin);
                                ++chars_to_erase;
                            }

                            line.content.erase(
                                line.content.end() - chars_to_erase, line.content.end());
                        }

                        if (!word_wrap_enabled_) {
                            DEBUG_LOG("       Display single line");
                            // Word wrap is disabled, so we can only display one line anyway.
                            line_list.push_back(line);
                            done = true;
                            break;
                        }

                        // Add the line
                        lines.push_back(line);
                        line.width = 0.0f;
                        line.content.clear();

                        DEBUG_LOG("       Continue");

                        // Skip all following content (which we cannot display) until next
                        // whitespace
                        auto iter_temp = iter_char1;
                        iter_char1 =
                            std::find_if(iter_char1, parsed_content.end(), &parser::is_whitespace);

                        if (iter_char1 == parsed_content.end())
                            break;

                        // Apply the format tags that were cut
                        for (; iter_temp != iter_char1; ++iter_temp) {
                            std::visit(
                                [&](const auto& value) {
                                    using type = std::decay_t<decltype(value)>;
                                    if constexpr (std::is_same_v<type, parser::format>) {
                                        line.content.push_back(value);
                                    }
                                },
                                *iter_temp);
                        }

                        // Look for the next word
                        iter_char1 =
                            std::find_if(iter_char1, parsed_content.end(), &parser::is_word);
                        if (iter_char1 != parsed_content.end())
                            break;

                        --iter_char1;
                        iter_line_begin = iter_char1;
                    }
                }
            }

            if (done)
                break;

            DEBUG_LOG("     End");

            lines.push_back(line);

            // Add the maximum number of line to this text
            for (auto& l : lines) {
                if (line_list.size() == max_line_nbr) {
                    done = true;
                    break;
                }
                line_list.push_back(std::move(l));
            }

            if (done)
                break;
            DEBUG_LOG("     .");
        }
    }

    num_lines_ = line_list.size();

    quad_list_.clear();
    outline_quad_list_.clear();
    icons_list_.clear();

    if (!line_list.empty()) {
        if (box_width_ == 0.0f || std::isinf(box_width_)) {
            width_ = 0.0f;
            for (const auto& line : line_list)
                width_ = std::max(width_, line.width);
        } else
            width_ = box_width_;

        height_ =
            (1.0f + static_cast<float>(line_list.size() - 1) * line_spacing_) * get_line_height();

        float y  = 0.0f;
        float x0 = 0.0f;

        if (box_width_ != 0.0f && !std::isinf(box_width_)) {
            switch (align_x_) {
            case alignment_x::left: x0 = 0.0f; break;
            case alignment_x::center: x0 = box_width_ * 0.5f; break;
            case alignment_x::right: x0 = box_width_; break;
            }
        } else
            x0 = 0.0f;

        if (!std::isinf(box_height_)) {
            switch (align_y_) {
            case alignment_y::top: y = 0.0f; break;
            case alignment_y::middle: y = (box_height_ - height_) * 0.5f; break;
            case alignment_y::bottom: y = (box_height_ - height_); break;
            }
        } else {
            switch (align_y_) {
            case alignment_y::top: y = 0.0f; break;
            case alignment_y::middle: y = -height_ * 0.5f; break;
            case alignment_y::bottom: y = -height_; break;
            }
        }

        x0 = round_to_pixel_(x0);
        y  = round_to_pixel_(y);

        std::vector<color> color_stack;

        for (const auto& line : line_list) {
            float x = 0.0f;
            switch (align_x_) {
            case alignment_x::left: x = 0.0f; break;
            case alignment_x::center: x = -line.width * 0.5f; break;
            case alignment_x::right: x = -line.width; break;
            }

            x = round_to_pixel_(x) + x0;

            for (auto iter_char : utils::range::iterator(line.content)) {
                const auto advance = parser::get_advance(*this, iter_char, line.content.begin());

                x += advance.first;

                std::visit(
                    [&](const auto& value) {
                        using type = std::decay_t<decltype(value)>;
                        if constexpr (std::is_same_v<type, parser::format>) {
                            switch (value.action) {
                            case parser::color_action::set: color_stack.push_back(value.col); break;
                            case parser::color_action::reset: color_stack.pop_back(); break;
                            default: break;
                            }
                        } else if constexpr (std::is_same_v<type, parser::texture>) {
                            float tex_width = 0.0f, tex_height = 0.0f;
                            if (std::isnan(value.width)) {
                                tex_width  = get_line_height();
                                tex_height = get_line_height();
                            } else {
                                tex_width  = value.width * get_scaling_factor();
                                tex_height = value.height * get_scaling_factor();
                            }

                            tex_width  = round_to_pixel_(tex_width);
                            tex_height = round_to_pixel_(tex_height);

                            quad icon;
                            icon.mat      = value.mat;
                            icon.v[0].pos = vector2f(0.0f, 0.0f);
                            icon.v[1].pos = vector2f(tex_width, 0.0f);
                            icon.v[2].pos = vector2f(tex_width, tex_height);
                            icon.v[3].pos = vector2f(0.0f, tex_height);
                            if (icon.mat) {
                                icon.v[0].uvs = icon.mat->get_canvas_uv(vector2f(0.0f, 0.0f), true);
                                icon.v[1].uvs = icon.mat->get_canvas_uv(vector2f(1.0f, 0.0f), true);
                                icon.v[2].uvs = icon.mat->get_canvas_uv(vector2f(1.0f, 1.0f), true);
                                icon.v[3].uvs = icon.mat->get_canvas_uv(vector2f(0.0f, 1.0f), true);
                            }

                            for (std::size_t i = 0; i < 4; ++i) {
                                icon.v[i].pos += vector2f(round_to_pixel_(x), round_to_pixel_(y));
                            }

                            icons_list_.push_back(icon);
                        } else if constexpr (std::is_same_v<type, char32_t>) {
                            if (outline_font_) {
                                std::array<vertex, 4> vertex_list =
                                    create_outline_letter_quad_(value);
                                for (std::size_t i = 0; i < 4; ++i) {
                                    vertex_list[i].pos +=
                                        vector2f(round_to_pixel_(x), round_to_pixel_(y));
                                    vertex_list[i].col = color::black;
                                }

                                outline_quad_list_.push_back(vertex_list);
                            }

                            std::array<vertex, 4> vertex_list = create_letter_quad_(value);
                            for (std::size_t i = 0; i < 4; ++i) {
                                vertex_list[i].pos +=
                                    vector2f(round_to_pixel_(x), round_to_pixel_(y));
                                vertex_list[i].col =
                                    color_stack.empty() ? color::empty : color_stack.back();
                            }

                            quad_list_.push_back(vertex_list);
                        }
                    },
                    *iter_char);

                x += advance.second;
            }

            y += get_line_height() * line_spacing_;
        }
    } else {
        width_  = 0.0f;
        height_ = 0.0f;
    }

    update_cache_flag_ = false;

    notify_vertex_cache_dirty_();
}

void text::update_vertex_cache_() const {
    if (!update_vertex_cache_flag_)
        return;

    if (!vertex_cache_)
        vertex_cache_ = renderer_.create_vertex_cache(vertex_cache::type::quads);

    std::vector<std::array<vertex, 4>> quads_copy = quad_list_;
    for (auto& quad : quads_copy) {
        for (std::size_t i = 0; i < 4; ++i) {
            if (!formatting_enabled_ || force_color_ || quad[i].col == color::empty) {
                quad[i].col = color_;
            }

            quad[i].col.a *= alpha_;
        }
    }

    vertex_cache_->update(quads_copy[0].data(), quads_copy.size() * 4);

    if (outline_font_) {
        if (!outline_vertex_cache_)
            outline_vertex_cache_ = renderer_.create_vertex_cache(vertex_cache::type::quads);

        std::vector<std::array<vertex, 4>> outline_quads_copy = outline_quad_list_;
        for (auto& quad : outline_quads_copy) {
            for (std::size_t i = 0; i < 4; ++i) {
                quad[i].col.a *= alpha_;
            }
        }

        outline_vertex_cache_->update(outline_quads_copy[0].data(), outline_quads_copy.size() * 4);
    }

    update_vertex_cache_flag_ = false;
}

std::array<vertex, 4> text::create_letter_quad_(const gui::font& font, char32_t c) const {
    bounds2f quad = font.get_character_bounds(c) * scaling_factor_;

    std::array<vertex, 4> vertex_list;
    vertex_list[0].pos = quad.top_left();
    vertex_list[1].pos = quad.top_right();
    vertex_list[2].pos = quad.bottom_right();
    vertex_list[3].pos = quad.bottom_left();

    bounds2f uvs       = font.get_character_uvs(c);
    vertex_list[0].uvs = uvs.top_left();
    vertex_list[1].uvs = uvs.top_right();
    vertex_list[2].uvs = uvs.bottom_right();
    vertex_list[3].uvs = uvs.bottom_left();

    return vertex_list;
}

std::array<vertex, 4> text::create_letter_quad_(char32_t c) const {
    return create_letter_quad_(*font_, c);
}

std::array<vertex, 4> text::create_outline_letter_quad_(char32_t c) const {
    return create_letter_quad_(*outline_font_, c);
}

quad text::create_letter_quad(char32_t c) const {
    quad output;
    output.mat = font_->get_texture().lock();
    output.v   = create_letter_quad_(c);

    return output;
}

std::size_t text::get_letter_count() const {
    update_();
    return quad_list_.size();
}

const std::array<vertex, 4>& text::get_letter_quad(std::size_t index) const {
    update_();

    if (index >= quad_list_.size())
        throw gui::exception("text", "Trying to access letter at invalid index.");

    return quad_list_[index];
}

} // namespace lxgui::gui
