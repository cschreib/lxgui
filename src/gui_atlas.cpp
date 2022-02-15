#include "lxgui/gui_atlas.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_vertex.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

atlas_page::atlas_page(material::filter m_filter) : m_filter_(m_filter) {}

std::shared_ptr<material> atlas_page::fetch_material(const std::string& file_name) const {
    auto m_iter = texture_list_.find(file_name);
    if (m_iter != texture_list_.end()) {
        if (std::shared_ptr<gui::material> p_lock = m_iter->second.lock())
            return p_lock;
    }

    return nullptr;
}

std::shared_ptr<gui::material>
atlas_page::add_material(const std::string& file_name, const material& m_mat) {
    try {
        const auto m_rect     = m_mat.get_rect();
        const auto m_location = find_location_(m_rect.width(), m_rect.height());
        if (!m_location.has_value())
            return nullptr;

        std::shared_ptr<gui::material> p_tex = add_material_(m_mat, m_location.value());
        texture_list_[file_name]             = p_tex;
        return p_tex;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<font> atlas_page::fetch_font(const std::string& font_name) const {
    auto m_iter = font_list_.find(font_name);
    if (m_iter != font_list_.end()) {
        if (std::shared_ptr<gui::font> p_lock = m_iter->second.lock())
            return p_lock;
    }

    return nullptr;
}

bool atlas_page::add_font(const std::string& font_name, std::shared_ptr<gui::font> p_font) {
    try {
        if (const auto p_mat = p_font->get_texture().lock()) {
            const auto m_rect     = p_mat->get_rect();
            const auto m_location = find_location_(m_rect.width(), m_rect.height());
            if (!m_location.has_value())
                return false;

            std::shared_ptr<gui::material> p_tex = add_material_(*p_mat, m_location.value());
            p_font->update_texture(p_tex);

            font_list_[font_name] = std::move(p_font);
            return true;
        } else
            return false;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return false;
    }
}

bool atlas_page::empty() const {
    for (const auto& p_mat : texture_list_) {
        if (std::shared_ptr<gui::material> p_lock = p_mat.second.lock())
            return false;
    }

    for (const auto& p_font : font_list_) {
        if (std::shared_ptr<gui::font> p_lock = p_font.second.lock())
            return false;
    }

    return true;
}

std::optional<bounds2f> atlas_page::find_location_(float f_width, float f_height) const {
    constexpr float f_padding = 1.0f; // pixels

    bounds2f mStartQuad(0, f_width, 0, f_height);
    if (empty())
        return mStartQuad;

    const float fAtlasWidth  = get_width_();
    const float fAtlasHeight = get_height_();

    std::vector<bounds2f> occupied_space;
    occupied_space.reserve(texture_list_.size());

    float fMaxWidth  = 0.0f;
    float fMaxHeight = 0.0f;

    auto apply_padding = [&](bounds2f m_rect) {
        m_rect.right += f_padding;
        m_rect.bottom += f_padding;
        return m_rect;
    };

    for (const auto& p_mat : texture_list_) {
        if (std::shared_ptr<gui::material> p_lock = p_mat.second.lock()) {
            occupied_space.push_back(apply_padding(p_lock->get_rect()));
            fMaxWidth  = std::max(fMaxWidth, occupied_space.back().right);
            fMaxHeight = std::max(fMaxHeight, occupied_space.back().bottom);
        }
    }

    for (const auto& p_font : font_list_) {
        if (std::shared_ptr<gui::font> p_lock = p_font.second.lock()) {
            occupied_space.push_back(apply_padding(p_lock->get_texture().lock()->get_rect()));
            fMaxWidth  = std::max(fMaxWidth, occupied_space.back().right);
            fMaxHeight = std::max(fMaxHeight, occupied_space.back().bottom);
        }
    }

    float    fBestArea = std::numeric_limits<float>::infinity();
    bounds2f mBestQuad;

    for (const auto& m_rect_source : occupied_space) {
        auto m_test_position = [&](const vector2f& m_pos) {
            const bounds2f m_test_quad = mStartQuad + m_pos;
            if (m_test_quad.right > fAtlasWidth || m_test_quad.bottom > fAtlasHeight)
                return;

            const float f_new_max_width  = std::max(fMaxWidth, m_test_quad.right);
            const float f_new_max_height = std::max(fMaxHeight, m_test_quad.bottom);
            const float f_new_area       = f_new_max_width * f_new_max_height;

            if (f_new_area >= fBestArea)
                return;

            for (const auto& m_rect_other : occupied_space) {
                if (m_test_quad.overlaps(m_rect_other))
                    return;
            }

            fBestArea = f_new_area;
            mBestQuad = m_test_quad;
        };

        m_test_position(m_rect_source.top_right());
        m_test_position(m_rect_source.bottom_left());
    }

    if (std::isfinite(fBestArea))
        return mBestQuad;
    else
        return std::nullopt;
}

atlas::atlas(renderer& m_renderer, material::filter m_filter) :
    m_renderer_(m_renderer), m_filter_(m_filter) {}

std::shared_ptr<gui::material> atlas::fetch_material(const std::string& file_name) const {
    for (const auto& m_page_item : page_list_) {
        auto p_tex = m_page_item.p_page->fetch_material(file_name);
        if (p_tex)
            return p_tex;
    }

    return nullptr;
}

std::shared_ptr<gui::material>
atlas::add_material(const std::string& file_name, const material& m_mat) {
    try {
        for (const auto& m_page_item : page_list_) {
            auto p_tex = m_page_item.p_page->add_material(file_name, m_mat);
            if (p_tex)
                return p_tex;

            if (m_page_item.p_page->empty()) {
                gui::out << gui::warning << "Could not fit texture '" << file_name
                         << "' on any atlas page." << std::endl;
                return nullptr;
            }
        }

        add_page_();
        auto p_tex = page_list_.back().p_page->add_material(file_name, m_mat);
        if (p_tex)
            return p_tex;

        return nullptr;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::font> atlas::fetch_font(const std::string& font_name) const {
    for (const auto& m_page_item : page_list_) {
        auto p_font = m_page_item.p_page->fetch_font(font_name);
        if (p_font)
            return p_font;
    }

    return nullptr;
}

bool atlas::add_font(const std::string& font_name, std::shared_ptr<gui::font> p_font) {
    try {
        for (const auto& m_page_item : page_list_) {
            if (m_page_item.p_page->add_font(font_name, p_font))
                return true;

            if (m_page_item.p_page->empty()) {
                gui::out << gui::warning << "Could not fit font '" << font_name
                         << "' on any atlas page." << std::endl;
                return false;
            }
        }

        add_page_();

        return page_list_.back().p_page->add_font(font_name, std::move(p_font));
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return false;
    }
}

std::size_t atlas::get_num_pages() const {
    return page_list_.size();
}

void atlas::add_page_() {
    page_item m_page;
    m_page.p_page = create_page_();

    // Add a white pixel as the first material in the atlas.
    // This can be used for optimizing quad batching, to render
    // quads with no texture.
    ub32color m_pixel(255, 255, 255, 255);
    auto      p_tex         = m_renderer_.create_material(vector2ui(1u, 1u), &m_pixel);
    m_page.p_no_texture_mat = m_page.p_page->add_material("", *p_tex);

    page_list_.push_back(std::move(m_page));
}

} // namespace lxgui::gui
