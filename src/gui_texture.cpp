#include "lxgui/gui_texture.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/utils_filesystem.hpp"

#include <sstream>

namespace lxgui::gui {

texture::texture(utils::control_block& mBlock, manager& mManager) :
    layered_region(mBlock, mManager), mRenderer_(mManager.get_renderer()) {
    lType_.push_back(CLASS_NAME);
}

std::string texture::serialize(const std::string& sTab) const {
    std::ostringstream sStr;
    sStr << base::serialize(sTab);

    std::visit(
        [&](const auto& mData) {
            using content_type = std::decay_t<decltype(mData)>;

            if constexpr (std::is_same_v<content_type, std::string>) {
                sStr << sTab << "  # File        : " << mData << "\n";
            } else if constexpr (std::is_same_v<content_type, gradient>) {
                sStr << sTab << "  # Gradient    :\n";
                sStr << sTab << "  #-###\n";
                sStr << sTab << "  |   # min color   : " << mData.get_min_color() << "\n";
                sStr << sTab << "  |   # max color   : " << mData.get_max_color() << "\n";
                sStr << sTab << "  |   # orientation : ";
                switch (mData.get_orientation()) {
                case gradient::orientation::HORIZONTAL: sStr << "HORIZONTAL\n"; break;
                case gradient::orientation::VERTICAL: sStr << "VERTICAL\n"; break;
                default: sStr << "<error>\n"; break;
                }
                sStr << sTab << "  #-###\n";
            } else if constexpr (std::is_same_v<content_type, color>) {
                sStr << sTab << "  # Color       : " << mData << "\n";
            }
        },
        mContent_);

    sStr << sTab << "  # Tex. coord. :\n";
    sStr << sTab << "  #-###\n";
    sStr << sTab << "  |   # top-left     : (" << mQuad_.v[0].uvs << ")\n";
    sStr << sTab << "  |   # top-right    : (" << mQuad_.v[1].uvs << ")\n";
    sStr << sTab << "  |   # bottom-right : (" << mQuad_.v[2].uvs << ")\n";
    sStr << sTab << "  |   # bottom-left  : (" << mQuad_.v[3].uvs << ")\n";
    sStr << sTab << "  #-###\n";
    sStr << sTab << "  # TexCModRect : " << bTexCoordModifiesRect_ << "\n";

    sStr << sTab << "  # Blend mode  : ";
    switch (mBlendMode_) {
    case blend_mode::NONE: sStr << "NONE\n"; break;
    case blend_mode::BLEND: sStr << "BLEND\n"; break;
    case blend_mode::KEY: sStr << "KEY\n"; break;
    case blend_mode::ADD: sStr << "ADD\n"; break;
    case blend_mode::MOD: sStr << "MOD\n"; break;
    default: sStr << "<error>\n"; break;
    }

    sStr << sTab << "  # Filter      : ";
    switch (mFilter_) {
    case material::filter::NONE: sStr << "NONE\n"; break;
    case material::filter::LINEAR: sStr << "LINEAR\n"; break;
    default: sStr << "<error>\n"; break;
    }

    sStr << sTab << "  # Desaturated : " << bIsDesaturated_ << "\n";

    return sStr.str();
}

void texture::render() const {
    if (!is_visible())
        return;

    float fAlpha = get_effective_alpha();

    if (fAlpha != 1.0f) {
        quad mBlendedQuad = mQuad_;
        for (std::size_t i = 0; i < 4; ++i)
            mBlendedQuad.v[i].col.a *= fAlpha;

        mRenderer_.render_quad(mBlendedQuad);
    } else {
        mRenderer_.render_quad(mQuad_);
    }
}

void texture::create_glue() {
    create_glue_(this);
}

void texture::copy_from(const region& mObj) {
    base::copy_from(mObj);

    const texture* pTexture = down_cast<texture>(&mObj);
    if (!pTexture)
        return;

    if (pTexture->has_texture_file())
        this->set_texture(pTexture->get_texture_file());
    else if (pTexture->has_gradient())
        this->set_gradient(pTexture->get_gradient());
    else if (pTexture->has_solid_color())
        this->set_solid_color(pTexture->get_solid_color());

    this->set_blend_mode(pTexture->get_blend_mode());
    this->set_tex_coord(pTexture->get_tex_coord());
    this->set_tex_coord_modifies_rect(pTexture->get_tex_coord_modifies_rect());
    this->set_desaturated(pTexture->is_desaturated());
}

texture::blend_mode texture::get_blend_mode() const {
    return mBlendMode_;
}

material::filter texture::get_filter_mode() const {
    return mFilter_;
}

bool texture::has_solid_color() const {
    return std::holds_alternative<color>(mContent_);
}

const color& texture::get_solid_color() const {
    return std::get<color>(mContent_);
}

bool texture::has_gradient() const {
    return std::holds_alternative<gradient>(mContent_);
}

const gradient& texture::get_gradient() const {
    return std::get<gradient>(mContent_);
}

std::array<float, 8> texture::get_tex_coord() const {
    std::array<float, 8> mCoords{};

    if (mQuad_.mat) {
        for (std::size_t i = 0; i < 4; ++i) {
            const vector2f lUV = mQuad_.mat->get_local_uv(mQuad_.v[i].uvs, true);
            mCoords[2 * i + 0] = lUV.x;
            mCoords[2 * i + 1] = lUV.y;
        }
    } else {
        for (std::size_t i = 0; i < 4; ++i) {
            mCoords[2 * i + 0] = mQuad_.v[i].uvs.x;
            mCoords[2 * i + 1] = mQuad_.v[i].uvs.y;
        }
    }

    return mCoords;
}

bool texture::get_tex_coord_modifies_rect() const {
    return bTexCoordModifiesRect_;
}

bool texture::has_texture_file() const {
    return std::holds_alternative<std::string>(mContent_);
}

const std::string& texture::get_texture_file() const {
    return std::get<std::string>(mContent_);
}

color texture::get_vertex_color(std::size_t uiIndex) const {
    if (uiIndex >= 4) {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
                 << "Vertex index out of bound (" << uiIndex << ")." << std::endl;
        return color::WHITE;
    }

    return mQuad_.v[uiIndex].col;
}

bool texture::is_desaturated() const {
    return bIsDesaturated_;
}

void texture::set_blend_mode(blend_mode mBlendMode) {
    if (mBlendMode != blend_mode::BLEND) {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
                 << "texture::set_blend_mode other than \"BLEND\" is not yet implemented."
                 << std::endl;
        return;
    }

    if (mBlendMode_ == mBlendMode)
        return;

    mBlendMode_ = mBlendMode;

    notify_renderer_need_redraw();
}

void texture::set_blend_mode(const std::string& sBlendMode) {
    blend_mode mNewBlendMode = blend_mode::BLEND;

    if (sBlendMode == "BLEND")
        mBlendMode_ = blend_mode::BLEND;
    else if (sBlendMode == "ADD")
        mBlendMode_ = blend_mode::ADD;
    else if (sBlendMode == "MOD")
        mBlendMode_ = blend_mode::MOD;
    else if (sBlendMode == "KEY")
        mBlendMode_ = blend_mode::KEY;
    else if (sBlendMode == "NONE")
        mBlendMode_ = blend_mode::NONE;
    else {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
                 << "Unknown blending : \"" << sBlendMode << "\". Using \"BLEND\"." << std::endl;
    }

    set_blend_mode(mNewBlendMode);
}

void texture::set_filter_mode(material::filter mFilter) {
    if (mFilter_ == mFilter)
        return;

    mFilter_ = mFilter;

    if (std::holds_alternative<std::string>(mContent_)) {
        // Force re-load of the material
        std::string sFileName = std::get<std::string>(mContent_);
        mContent_             = std::string{};
        set_texture(sFileName);
    }
}

void texture::set_filter_mode(const std::string& sFilter) {
    material::filter mNewFilter = material::filter::NONE;

    if (sFilter == "NONE")
        mNewFilter = material::filter::NONE;
    else if (sFilter == "LINEAR")
        mNewFilter = material::filter::LINEAR;
    else {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
                 << "Unknown filtering : \"" << sFilter << "\". Using \"NONE\"." << std::endl;
    }

    set_filter_mode(mNewFilter);
}

void texture::set_desaturated(bool bIsDesaturated) {
    if (bIsDesaturated_ == bIsDesaturated)
        return;

    bIsDesaturated_ = bIsDesaturated;
    if (bIsDesaturated) {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
                 << "Texture de-saturation is not yet implemented." << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_gradient(const gradient& mGradient) {
    mContent_ = mGradient;

    mQuad_.mat = nullptr;

    if (mGradient.get_orientation() == gradient::orientation::HORIZONTAL) {
        mQuad_.v[0].col = mGradient.get_min_color();
        mQuad_.v[1].col = mGradient.get_max_color();
        mQuad_.v[2].col = mGradient.get_max_color();
        mQuad_.v[3].col = mGradient.get_min_color();
    } else {
        mQuad_.v[0].col = mGradient.get_min_color();
        mQuad_.v[1].col = mGradient.get_min_color();
        mQuad_.v[2].col = mGradient.get_max_color();
        mQuad_.v[3].col = mGradient.get_max_color();
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_rect(const std::array<float, 4>& lTextureRect) {
    if (mQuad_.mat) {
        mQuad_.v[0].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureRect[0], lTextureRect[1]), true);
        mQuad_.v[1].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureRect[2], lTextureRect[1]), true);
        mQuad_.v[2].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureRect[2], lTextureRect[3]), true);
        mQuad_.v[3].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureRect[0], lTextureRect[3]), true);

        if (bTexCoordModifiesRect_)
            update_dimensions_from_tex_coord_();
    } else {
        mQuad_.v[0].uvs = vector2f(lTextureRect[0], lTextureRect[1]);
        mQuad_.v[1].uvs = vector2f(lTextureRect[2], lTextureRect[1]);
        mQuad_.v[2].uvs = vector2f(lTextureRect[2], lTextureRect[3]);
        mQuad_.v[3].uvs = vector2f(lTextureRect[0], lTextureRect[3]);
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_coord(const std::array<float, 8>& lTextureCoords) {
    if (mQuad_.mat) {
        mQuad_.v[0].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureCoords[0], lTextureCoords[1]), true);
        mQuad_.v[1].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureCoords[2], lTextureCoords[3]), true);
        mQuad_.v[2].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureCoords[4], lTextureCoords[5]), true);
        mQuad_.v[3].uvs =
            mQuad_.mat->get_canvas_uv(vector2f(lTextureCoords[6], lTextureCoords[7]), true);

        if (bTexCoordModifiesRect_)
            update_dimensions_from_tex_coord_();
    } else {
        mQuad_.v[0].uvs = vector2f(lTextureCoords[0], lTextureCoords[1]);
        mQuad_.v[1].uvs = vector2f(lTextureCoords[2], lTextureCoords[3]);
        mQuad_.v[2].uvs = vector2f(lTextureCoords[4], lTextureCoords[5]);
        mQuad_.v[3].uvs = vector2f(lTextureCoords[6], lTextureCoords[7]);
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_coord_modifies_rect(bool bTexCoordModifiesRect) {
    if (bTexCoordModifiesRect_ != bTexCoordModifiesRect) {
        bTexCoordModifiesRect_ = bTexCoordModifiesRect;

        if (bTexCoordModifiesRect_ && mQuad_.mat)
            update_dimensions_from_tex_coord_();
    }
}

void texture::update_dimensions_from_tex_coord_() {
    vector2f mExtent = mQuad_.v[2].uvs - mQuad_.v[0].uvs;
    set_dimensions(mExtent * vector2f(mQuad_.mat->get_canvas_dimensions()));
}

void texture::set_texture(const std::string& sFile) {
    std::string sParsedFile = parse_file_name(sFile);
    mContent_               = sParsedFile;

    if (sParsedFile.empty())
        return;

    auto& mRenderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> pMat;
    if (utils::file_exists(sParsedFile))
        pMat = mRenderer.create_atlas_material("GUI", sParsedFile, mFilter_);

    mQuad_.mat = pMat;

    if (pMat) {
        mQuad_.v[0].uvs = mQuad_.mat->get_canvas_uv(vector2f(0, 0), true);
        mQuad_.v[1].uvs = mQuad_.mat->get_canvas_uv(vector2f(1, 0), true);
        mQuad_.v[2].uvs = mQuad_.mat->get_canvas_uv(vector2f(1, 1), true);
        mQuad_.v[3].uvs = mQuad_.mat->get_canvas_uv(vector2f(0, 1), true);

        if (!is_apparent_width_defined())
            set_width(mQuad_.mat->get_rect().width());

        if (!is_apparent_height_defined())
            set_height(mQuad_.mat->get_rect().height());
    } else {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
                 << "Cannot load file \"" << sParsedFile << "\" for \"" << sName_
                 << "\".\nUsing white texture instead." << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_texture(std::shared_ptr<render_target> pRenderTarget) {
    mContent_ = std::string{};

    auto& mRenderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> pMat;
    if (pRenderTarget)
        pMat = mRenderer.create_material(std::move(pRenderTarget));

    mQuad_.mat = pMat;

    if (pMat) {
        mQuad_.v[0].uvs = mQuad_.mat->get_canvas_uv(vector2f(0, 0), true);
        mQuad_.v[1].uvs = mQuad_.mat->get_canvas_uv(vector2f(1, 0), true);
        mQuad_.v[2].uvs = mQuad_.mat->get_canvas_uv(vector2f(1, 1), true);
        mQuad_.v[3].uvs = mQuad_.mat->get_canvas_uv(vector2f(0, 1), true);

        if (!is_apparent_width_defined())
            set_width(mQuad_.mat->get_rect().width());

        if (!is_apparent_height_defined())
            set_height(mQuad_.mat->get_rect().height());
    } else {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
                 << "Cannot create a texture from render target.\n"
                    "Using white texture instead."
                 << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_solid_color(const color& mColor) {
    mContent_ = mColor;

    mQuad_.mat      = nullptr;
    mQuad_.v[0].col = mColor;
    mQuad_.v[1].col = mColor;
    mQuad_.v[2].col = mColor;
    mQuad_.v[3].col = mColor;

    notify_renderer_need_redraw();
}

void texture::set_quad(const quad& mQuad) {
    mContent_ = std::string{};

    mQuad_           = mQuad;
    vector2f mExtent = mQuad_.v[2].pos - mQuad_.v[0].pos;
    set_dimensions(mExtent);

    notify_renderer_need_redraw();
}

void texture::set_vertex_color(const color& mColor, std::size_t uiIndex) {
    if (uiIndex == std::numeric_limits<std::size_t>::max()) {
        for (std::size_t i = 0; i < 4; ++i)
            mQuad_.v[i].col = mColor;

        notify_renderer_need_redraw();
        return;
    }

    if (uiIndex >= 4) {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
                 << "Vertex index out of bound (" << uiIndex << ")." << std::endl;
        return;
    }

    mQuad_.v[uiIndex].col = mColor;

    notify_renderer_need_redraw();
}

void texture::update_borders_() {
    base::update_borders_();

    mQuad_.v[0].pos = lBorderList_.top_left();
    mQuad_.v[1].pos = lBorderList_.top_right();
    mQuad_.v[2].pos = lBorderList_.bottom_right();
    mQuad_.v[3].pos = lBorderList_.bottom_left();
}

} // namespace lxgui::gui
