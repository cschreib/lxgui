#include "lxgui/gui_atlas.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_vertex.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

atlas_page::atlas_page(material::filter filt) : filter_(filt) {}

std::shared_ptr<material> atlas_page::fetch_material(const std::string& file_name) const {
    auto iter = texture_list_.find(file_name);
    if (iter != texture_list_.end()) {
        if (std::shared_ptr<gui::material> lock = iter->second.lock())
            return lock;
    }

    return nullptr;
}

std::shared_ptr<gui::material>
atlas_page::add_material(const std::string& file_name, const material& mat) {
    try {
        const auto rect     = mat.get_rect();
        const auto location = find_location_(rect.width(), rect.height());
        if (!location.has_value())
            return nullptr;

        std::shared_ptr<gui::material> tex = add_material_(mat, location.value());
        texture_list_[file_name]           = tex;
        return tex;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<font> atlas_page::fetch_font(const std::string& font_name) const {
    auto iter = font_list_.find(font_name);
    if (iter != font_list_.end()) {
        if (std::shared_ptr<gui::font> lock = iter->second.lock())
            return lock;
    }

    return nullptr;
}

bool atlas_page::add_font(const std::string& font_name, std::shared_ptr<gui::font> fnt) {
    try {
        if (const auto mat = fnt->get_texture().lock()) {
            const auto rect     = mat->get_rect();
            const auto location = find_location_(rect.width(), rect.height());
            if (!location.has_value())
                return false;

            std::shared_ptr<gui::material> tex = add_material_(*mat, location.value());
            fnt->update_texture(tex);

            font_list_[font_name] = std::move(fnt);
            return true;
        } else
            return false;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return false;
    }
}

bool atlas_page::empty() const {
    for (const auto& mat : texture_list_) {
        if (std::shared_ptr<gui::material> lock = mat.second.lock())
            return false;
    }

    for (const auto& fnt : font_list_) {
        if (std::shared_ptr<gui::font> lock = fnt.second.lock())
            return false;
    }

    return true;
}

std::optional<bounds2f> atlas_page::find_location_(float width, float height) const {
    constexpr float padding = 1.0f; // pixels

    bounds2f start_quad(0, width, 0, height);
    if (empty())
        return start_quad;

    const float atlas_width   = get_width_();
    const float atlast_height = get_height_();

    std::vector<bounds2f> occupied_space;
    occupied_space.reserve(texture_list_.size());

    float max_width  = 0.0f;
    float max_height = 0.0f;

    auto apply_padding = [&](bounds2f rect) {
        rect.right += padding;
        rect.bottom += padding;
        return rect;
    };

    for (const auto& mat : texture_list_) {
        if (std::shared_ptr<gui::material> lock = mat.second.lock()) {
            occupied_space.push_back(apply_padding(lock->get_rect()));
            max_width  = std::max(max_width, occupied_space.back().right);
            max_height = std::max(max_height, occupied_space.back().bottom);
        }
    }

    for (const auto& fnt : font_list_) {
        if (std::shared_ptr<gui::font> lock = fnt.second.lock()) {
            occupied_space.push_back(apply_padding(lock->get_texture().lock()->get_rect()));
            max_width  = std::max(max_width, occupied_space.back().right);
            max_height = std::max(max_height, occupied_space.back().bottom);
        }
    }

    float    best_area = std::numeric_limits<float>::infinity();
    bounds2f best_quad;

    for (const auto& rect_source : occupied_space) {
        auto test_position = [&](const vector2f& pos) {
            const bounds2f test_quad = start_quad + pos;
            if (test_quad.right > atlas_width || test_quad.bottom > atlast_height)
                return;

            const float new_max_width  = std::max(max_width, test_quad.right);
            const float new_max_height = std::max(max_height, test_quad.bottom);
            const float new_area       = new_max_width * new_max_height;

            if (new_area >= best_area)
                return;

            for (const auto& rect_other : occupied_space) {
                if (test_quad.overlaps(rect_other))
                    return;
            }

            best_area = new_area;
            best_quad = test_quad;
        };

        test_position(rect_source.top_right());
        test_position(rect_source.bottom_left());
    }

    if (std::isfinite(best_area))
        return best_quad;
    else
        return std::nullopt;
}

atlas::atlas(renderer& rdr, material::filter filt) : renderer_(rdr), filter_(filt) {}

std::shared_ptr<gui::material> atlas::fetch_material(const std::string& file_name) const {
    for (const auto& item : page_list_) {
        auto tex = item.page->fetch_material(file_name);
        if (tex)
            return tex;
    }

    return nullptr;
}

std::shared_ptr<gui::material>
atlas::add_material(const std::string& file_name, const material& mat) {
    try {
        for (const auto& item : page_list_) {
            auto tex = item.page->add_material(file_name, mat);
            if (tex)
                return tex;

            if (item.page->empty()) {
                gui::out << gui::warning << "Could not fit texture '" << file_name
                         << "' on any atlas page." << std::endl;
                return nullptr;
            }
        }

        add_page_();
        auto tex = page_list_.back().page->add_material(file_name, mat);
        if (tex)
            return tex;

        return nullptr;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::font> atlas::fetch_font(const std::string& font_name) const {
    for (const auto& item : page_list_) {
        auto fnt = item.page->fetch_font(font_name);
        if (fnt)
            return fnt;
    }

    return nullptr;
}

bool atlas::add_font(const std::string& font_name, std::shared_ptr<gui::font> fnt) {
    try {
        for (const auto& item : page_list_) {
            if (item.page->add_font(font_name, fnt))
                return true;

            if (item.page->empty()) {
                gui::out << gui::warning << "Could not fit font '" << font_name
                         << "' on any atlas page." << std::endl;
                return false;
            }
        }

        add_page_();

        return page_list_.back().page->add_font(font_name, std::move(fnt));
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return false;
    }
}

std::size_t atlas::get_num_pages() const {
    return page_list_.size();
}

void atlas::add_page_() {
    page_item item;
    item.page = create_page_();

    // Add a white pixel as the first material in the atlas.
    // This can be used for optimizing quad batching, to render
    // quads with no texture.
    ub32color pixel(255, 255, 255, 255);
    auto      tex       = renderer_.create_material(vector2ui(1u, 1u), &pixel);
    item.no_texture_mat = item.page->add_material("", *tex);

    page_list_.push_back(std::move(item));
}

} // namespace lxgui::gui
