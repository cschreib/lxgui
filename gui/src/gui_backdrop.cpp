#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_renderer.hpp"

#include <lxgui/utils_filesystem.hpp>

namespace lxgui {
namespace gui
{

backdrop::backdrop(frame* pParent) : pParent_(pParent)
{
}

void backdrop::copy_from(const backdrop& mBackdrop)
{
    this->set_background(mBackdrop.get_background_file());
    this->set_edge(mBackdrop.get_edge_file());

    this->set_background_tilling(mBackdrop.is_background_tilling());
    this->set_tile_size(mBackdrop.get_tile_size());

    if (mBackdrop.sBackgroundFile_.empty())
        this->set_background_color(mBackdrop.get_background_color());

    this->set_background_insets(mBackdrop.get_background_insets());

    if (mBackdrop.sEdgeFile_.empty())
        this->set_edge_color(mBackdrop.get_edge_color());

    this->set_edge_size(mBackdrop.get_edge_size());
    this->set_edge_insets(mBackdrop.get_edge_insets());
}

void backdrop::set_background(const std::string& sBackgroundFile)
{
    if (sBackgroundFile_ == sBackgroundFile) return;

    bCacheDirty_ = true;
    mBackgroundColor_ = color::EMPTY;

    if (sBackgroundFile.empty())
    {
        pBackgroundTexture_ = nullptr;
        sBackgroundFile_ = "";
        return;
    }

    if (!utils::file_exists(sBackgroundFile))
    {
        pBackgroundTexture_ = nullptr;
        sBackgroundFile_ = "";

        gui::out << gui::warning << "backdrop : "
            << "Cannot find file : \"" << sBackgroundFile << "\" for "
            << pParent_->get_name() << "'s backdrop background file.\n"
            << "No background will be drawn." << std::endl;

        return;
    }

    auto* pRenderer = pParent_->get_manager()->get_renderer();
    pBackgroundTexture_ = pRenderer->create_atlas_material("GUI", sBackgroundFile);

    fTileSize_ = fOriginalTileSize_ = static_cast<float>(pBackgroundTexture_->get_rect().width());
    sBackgroundFile_ = sBackgroundFile;
}

const std::string& backdrop::get_background_file() const
{
    return sBackgroundFile_;
}

void backdrop::set_background_color(const color& mColor)
{
    if (mBackgroundColor_ == mColor) return;

    bCacheDirty_ = true;

    pBackgroundTexture_ = nullptr;
    mBackgroundColor_ = mColor;
    sBackgroundFile_ = "";

    fTileSize_ = fOriginalTileSize_ = 256.0f;
}

color backdrop::get_background_color() const
{
    return mBackgroundColor_;
}

void backdrop::set_background_tilling(bool bBackgroundTilling)
{
    if (bBackgroundTilling_ == bBackgroundTilling) return;

    bBackgroundTilling_ = bBackgroundTilling;
    bCacheDirty_ = true;
}

bool backdrop::is_background_tilling() const
{
    return bBackgroundTilling_;
}

void backdrop::set_tile_size(float fTileSize)
{
    if (fTileSize_ == fTileSize) return;

    fTileSize_ = fTileSize;
    bCacheDirty_ = true;
}

float backdrop::get_tile_size() const
{
    return fTileSize_;
}

void backdrop::set_background_insets(const quad2f& lInsets)
{
    if (lBackgroundInsets_ == lInsets) return;

    lBackgroundInsets_ = lInsets;
    bCacheDirty_ = true;
}

void backdrop::set_background_insets(float fLeft, float fRight, float fTop, float fBottom)
{
    set_background_insets(quad2f(fLeft, fRight, fTop, fBottom));
}

const quad2f& backdrop::get_background_insets() const
{
    return lBackgroundInsets_;
}

void backdrop::set_edge_insets(const quad2f& lInsets)
{
    if (lEdgeInsets_ == lInsets) return;

    lEdgeInsets_ = lInsets;
    bCacheDirty_ = true;
}

void backdrop::set_edge_insets(float fLeft, float fRight, float fTop, float fBottom)
{
    set_edge_insets(quad2f(fLeft, fRight, fTop, fBottom));
}

const quad2f& backdrop::get_edge_insets() const
{
    return lEdgeInsets_;
}

void backdrop::set_edge(const std::string& sEdgeFile)
{
    if (sEdgeFile == sEdgeFile_) return;

    bCacheDirty_ = true;
    mEdgeColor_ = color::EMPTY;

    if (sEdgeFile.empty())
    {
        pEdgeTexture_ = nullptr;
        sEdgeFile_ = "";
        return;
    }

    if (!utils::file_exists(sEdgeFile))
    {
        pEdgeTexture_ = nullptr;
        sEdgeFile_ = "";

        gui::out << gui::warning << "backdrop : "
            << "Cannot find file : \"" << sEdgeFile << "\" for " << pParent_->get_name()
            << "'s backdrop edge.\nNo edge will be drawn." << std::endl;

        return;
    }

    auto* pRenderer = pParent_->get_manager()->get_renderer();
    pEdgeTexture_ = pRenderer->create_atlas_material("GUI", sEdgeFile);

    if (pEdgeTexture_->get_rect().width()/pEdgeTexture_->get_rect().height() != 8.0f)
    {
        pEdgeTexture_ = nullptr;
        sEdgeFile_ = "";

        gui::out << gui::error << "backdrop : "
            << "An edge texture width must be exactly 8 times greater than its height "
            << "(in " << sEdgeFile << ").\nNo edge will be drawn for "
            << pParent_->get_name() << "'s backdrop." << std::endl;

        return;
    }

    fEdgeSize_ = fOriginalEdgeSize_ = pEdgeTexture_->get_rect().height();
    sEdgeFile_ = sEdgeFile;
}

const std::string& backdrop::get_edge_file() const
{
    return sEdgeFile_;
}

void backdrop::set_edge_color(const color& mColor)
{
    if (mEdgeColor_ == mColor) return;

    bCacheDirty_ = true;
    pEdgeTexture_ = nullptr;
    mEdgeColor_ = mColor;
    sEdgeFile_ = "";

    if (fEdgeSize_ == 0.0f)
        fEdgeSize_ = 1.0f;

    fOriginalEdgeSize_ = 1.0f;
}

color backdrop::get_edge_color() const
{
    return mEdgeColor_;
}

void backdrop::set_edge_size(float fEdgeSize)
{
    if (fEdgeSize_ == fEdgeSize) return;

    fEdgeSize_ = fEdgeSize;
    bCacheDirty_ = true;
}

float backdrop::get_edge_size() const
{
    return fEdgeSize_;
}

void backdrop::set_vertex_color(const color& mColor)
{
    if (mVertexColor_ == mColor) return;

    mVertexColor_ = mColor;
    bCacheDirty_ = true;
}

void backdrop::render() const
{
    if (!pParent_) return;

    float fAlpha = pParent_->get_effective_alpha();
    if (fAlpha != fCacheAlpha_)
        bCacheDirty_ = true;

    auto* pRenderer = pParent_->get_manager()->get_renderer();
    bool bUseVertexCache = pRenderer->is_vertex_cache_enabled() &&
                           !pRenderer->is_quad_batching_enabled();

    bool bHasBackground = pBackgroundTexture_ || mBackgroundColor_ != color::EMPTY;
    bool bHasEdge = pEdgeTexture_ || mEdgeColor_ != color::EMPTY;

    if (bHasBackground)
    {
        if ((bUseVertexCache && !pBackgroundCache_) ||
            (!bUseVertexCache && lBackgroundQuads_.empty()))
            bCacheDirty_ = true;
    }

    if (bHasEdge)
    {
        if ((bUseVertexCache && !pEdgeCache_) ||
            (!bUseVertexCache && lEdgeQuads_.empty()))
            bCacheDirty_ = true;
    }

    update_cache_();

    if (bHasBackground)
    {
        if (bUseVertexCache && pBackgroundCache_)
            pRenderer->render_cache(pBackgroundTexture_.get(), *pBackgroundCache_);
        else
            pRenderer->render_quads(pBackgroundTexture_.get(), lBackgroundQuads_);
    }

    if (bHasEdge)
    {
        if (bUseVertexCache && pEdgeCache_)
            pRenderer->render_cache(pEdgeTexture_.get(), *pEdgeCache_);
        else
            pRenderer->render_quads(pEdgeTexture_.get(), lEdgeQuads_);
    }
}

void backdrop::notify_borders_updated() const
{
    bCacheDirty_ = true;
}

void backdrop::update_cache_() const
{
    if (!bCacheDirty_) return;

    lBackgroundQuads_.clear();
    lEdgeQuads_.clear();

    color mColor = mVertexColor_;

    float fAlpha = pParent_->get_effective_alpha();
    mColor.a *= fAlpha;

    update_background_(mColor);
    update_edge_(mColor);

    fCacheAlpha_ = fAlpha;
    bCacheDirty_ = false;
}

void repeat_wrap(std::vector<std::array<vertex,4>>& lOutput,
    const quad2f& mSourceUVs, float fTileSize, bool bRotated, const color mColor,
    const quad2f& mDestination)
{
    const auto mDTopLeft = mDestination.top_left();
    const auto mSTopLeft = mSourceUVs.top_left();
    const float fDestWidth = mDestination.width();
    const float fDestHeight = mDestination.height();

    float fSY = 0.0f;
    while (fSY < fDestHeight)
    {
        float fDHeight = fTileSize;
        float fSHeight = mSourceUVs.height();
        if (fSY + fTileSize > fDestHeight)
            fDHeight = fDestHeight - fSY;

        float fSX = 0.0f;
        while (fSX < fDestWidth)
        {
            float fDWidth = fTileSize;
            float fSWidth = mSourceUVs.width();
            if (fSX + fTileSize > fDestWidth)
                fDWidth = fDestWidth - fSX;

            lOutput.emplace_back();
            auto& mQuad = lOutput.back();

            mQuad[0].pos = mDTopLeft + vector2f(fSX,           fSY);
            mQuad[1].pos = mDTopLeft + vector2f(fSX + fDWidth, fSY);
            mQuad[2].pos = mDTopLeft + vector2f(fSX + fDWidth, fSY + fDHeight);
            mQuad[3].pos = mDTopLeft + vector2f(fSX,           fSY + fDHeight);

            if (bRotated)
            {
                fSHeight *= fDWidth/fTileSize;
                fSWidth *= fDHeight/fTileSize;
                mQuad[0].uvs = mSTopLeft + vector2f(0.0f,    fSHeight);
                mQuad[1].uvs = mSTopLeft + vector2f(0.0f,    0.0f);
                mQuad[2].uvs = mSTopLeft + vector2f(fSWidth, 0.0f);
                mQuad[3].uvs = mSTopLeft + vector2f(fSWidth, fSHeight);
            }
            else
            {
                fSWidth *= fDWidth/fTileSize;
                fSHeight *= fDHeight/fTileSize;
                mQuad[0].uvs = mSTopLeft + vector2f(0.0f,    0.0f);
                mQuad[1].uvs = mSTopLeft + vector2f(fSWidth, 0.0f);
                mQuad[2].uvs = mSTopLeft + vector2f(fSWidth, fSHeight);
                mQuad[3].uvs = mSTopLeft + vector2f(0.0f,    fSHeight);
            }

            mQuad[0].col = mQuad[1].col = mQuad[2].col = mQuad[3].col = mColor;

            fSX += fDWidth;
        }

        fSY += fDHeight;
    }
}

void backdrop::update_background_(color mColor) const
{
    if (!pBackgroundTexture_ && mBackgroundColor_ == color::EMPTY) return;

    if (!pBackgroundTexture_)
        mColor *= mBackgroundColor_;

    auto mBorders = pParent_->get_borders();
    mBorders.left += lBackgroundInsets_.left;
    mBorders.right -= lBackgroundInsets_.right;
    mBorders.top += lBackgroundInsets_.top;
    mBorders.bottom -= lBackgroundInsets_.bottom;

    auto* pRenderer = pParent_->get_manager()->get_renderer();

    if (pBackgroundTexture_)
    {
        const vector2f mCanvasTL = pBackgroundTexture_->get_canvas_uv(vector2f(0.0f, 0.0f), true);
        const vector2f mCanvasBR = pBackgroundTexture_->get_canvas_uv(vector2f(1.0f, 1.0f), true);
        const quad2f mCanvasUVs = quad2f(mCanvasTL.x, mCanvasBR.x, mCanvasTL.y, mCanvasBR.y);

        if (pRenderer->is_texture_atlas_enabled() && bBackgroundTilling_ && fTileSize_ > 1.0f)
        {
            repeat_wrap(lBackgroundQuads_, mCanvasUVs, fTileSize_, false, mColor, mBorders);
        }
        else
        {
            lBackgroundQuads_.emplace_back();
            auto& mQuad = lBackgroundQuads_.back();

            mQuad[0].pos = mBorders.top_left();
            mQuad[1].pos = mBorders.top_right();
            mQuad[2].pos = mBorders.bottom_right();
            mQuad[3].pos = mBorders.bottom_left();
            mQuad[0].uvs = mCanvasUVs.top_left();
            mQuad[1].uvs = mCanvasUVs.top_right();
            mQuad[2].uvs = mCanvasUVs.bottom_right();
            mQuad[3].uvs = mCanvasUVs.bottom_left();
            mQuad[0].col = mQuad[1].col = mQuad[2].col = mQuad[3].col = mColor;
        }
    }
    else
    {
        lBackgroundQuads_.emplace_back();
        auto& mQuad = lBackgroundQuads_.back();

        mQuad[0].pos = mBorders.top_left();
        mQuad[1].pos = mBorders.top_right();
        mQuad[2].pos = mBorders.bottom_right();
        mQuad[3].pos = mBorders.bottom_left();
        mQuad[0].uvs = vector2f(0.0f,0.0f);
        mQuad[1].uvs = vector2f(1.0f,0.0f);
        mQuad[2].uvs = vector2f(1.0f,1.0f);
        mQuad[3].uvs = vector2f(0.0f,1.0f);

        mQuad[0].col = mQuad[1].col = mQuad[2].col = mQuad[3].col = mColor;
    }

    if (pRenderer->is_vertex_cache_enabled() && !pRenderer->is_quad_batching_enabled())
    {
        if (!pBackgroundCache_)
            pBackgroundCache_ = pRenderer->create_vertex_cache(vertex_cache::type::QUADS);

        pBackgroundCache_->update(lBackgroundQuads_[0].data(), lBackgroundQuads_.size()*4);
        lBackgroundQuads_.clear();
    }
}

void backdrop::update_edge_(color mColor) const
{
    if (!pEdgeTexture_ && mEdgeColor_ == color::EMPTY) return;

    if (!pEdgeTexture_)
        mColor *= mEdgeColor_;

    constexpr float fUVStep = 1.0f/8.0f;
    auto mBorders = pParent_->get_borders();
    mBorders.left += lEdgeInsets_.left;
    mBorders.right -= lEdgeInsets_.right;
    mBorders.top += lEdgeInsets_.top;
    mBorders.bottom -= lEdgeInsets_.bottom;

    auto* pRenderer = pParent_->get_manager()->get_renderer();

    auto repeat_wrap_edge = [&](const quad2f& mSourceUVs, bool bRotated, const quad2f& mDestination)
    {
        if (pEdgeTexture_)
        {
            const vector2f mCanvasTL = pEdgeTexture_->get_canvas_uv(mSourceUVs.top_left(), true);
            const vector2f mCanvasBR = pEdgeTexture_->get_canvas_uv(mSourceUVs.bottom_right(), true);
            const quad2f mCanvasUVs = quad2f(mCanvasTL.x, mCanvasBR.x, mCanvasTL.y, mCanvasBR.y);
            if (pRenderer->is_texture_atlas_enabled() && fEdgeSize_ > 1.0f)
            {
                repeat_wrap(lEdgeQuads_, mCanvasUVs, fEdgeSize_, bRotated, mColor, mDestination);
            }
            else
            {
                lEdgeQuads_.emplace_back();
                auto& mQuad = lEdgeQuads_.back();

                mQuad[0].pos = mDestination.top_left();
                mQuad[1].pos = mDestination.top_right();
                mQuad[2].pos = mDestination.bottom_right();
                mQuad[3].pos = mDestination.bottom_left();

                if (fEdgeSize_ <= 1.0f)
                {
                    mQuad[0].uvs = mCanvasUVs.top_left();
                    mQuad[1].uvs = mCanvasUVs.top_right();
                    mQuad[2].uvs = mCanvasUVs.bottom_right();
                    mQuad[3].uvs = mCanvasUVs.bottom_left();
                }
                else
                {
                    if (bRotated)
                    {
                        float fFactor = mDestination.width() / fEdgeSize_;
                        mQuad[0].uvs = mCanvasUVs.top_left() + vector2f(0.0, fFactor*mCanvasUVs.height());
                        mQuad[1].uvs = mCanvasUVs.top_left();
                        mQuad[2].uvs = mCanvasUVs.top_right();
                        mQuad[3].uvs = mCanvasUVs.top_right() + vector2f(0.0, fFactor*mCanvasUVs.height());
                    }
                    else
                    {
                        float fFactor = mDestination.height() / fEdgeSize_;
                        mQuad[0].uvs = mCanvasUVs.top_left();
                        mQuad[1].uvs = mCanvasUVs.top_right();
                        mQuad[2].uvs = mCanvasUVs.top_right() + vector2f(0.0, fFactor*mCanvasUVs.height());
                        mQuad[3].uvs = mCanvasUVs.top_left() + vector2f(0.0, fFactor*mCanvasUVs.height());
                    }
                }

                mQuad[0].col = mQuad[1].col = mQuad[2].col = mQuad[3].col = mColor;
            }
        }
        else
        {
            lEdgeQuads_.emplace_back();
            auto& mQuad = lEdgeQuads_.back();

            mQuad[0].pos = mDestination.top_left();
            mQuad[1].pos = mDestination.top_right();
            mQuad[2].pos = mDestination.bottom_right();
            mQuad[3].pos = mDestination.bottom_left();
            mQuad[0].uvs = vector2f(0.0f,0.0f);
            mQuad[1].uvs = vector2f(1.0f,0.0f);
            mQuad[2].uvs = vector2f(1.0f,1.0f);
            mQuad[3].uvs = vector2f(0.0f,1.0f);
            mQuad[0].col = mQuad[1].col = mQuad[2].col = mQuad[3].col = mColor;
        }
    };

    // Left edge
    repeat_wrap_edge(quad2f(0.0f, fUVStep, 0.0f, 1.0f), false, quad2f(
        mBorders.left, mBorders.left + fEdgeSize_,
        mBorders.top + fEdgeSize_, mBorders.bottom - fEdgeSize_
    ));

    // Right edge
    repeat_wrap_edge(quad2f(fUVStep, 2.0f*fUVStep, 0.0f, 1.0f), false, quad2f(
        mBorders.right - fEdgeSize_, mBorders.right,
        mBorders.top + fEdgeSize_, mBorders.bottom - fEdgeSize_
    ));

    // Top edge
    repeat_wrap_edge(quad2f(2.0f*fUVStep, 3.0f*fUVStep, 0.0f, 1.0f), true, quad2f(
        mBorders.left + fEdgeSize_, mBorders.right - fEdgeSize_,
        mBorders.top, mBorders.top + fEdgeSize_
    ));

    // Bottom edge
    repeat_wrap_edge(quad2f(3.0f*fUVStep, 4.0f*fUVStep, 0.0f, 1.0f), true, quad2f(
        mBorders.left + fEdgeSize_, mBorders.right - fEdgeSize_,
        mBorders.bottom - fEdgeSize_, mBorders.bottom
    ));

    // Top-left corner
    repeat_wrap_edge(quad2f(4.0f*fUVStep, 5.0f*fUVStep, 0.0f, 1.0f), false, quad2f(
        mBorders.left, mBorders.left + fEdgeSize_,
        mBorders.top, mBorders.top + fEdgeSize_
    ));

    // Top-right corner
    repeat_wrap_edge(quad2f(5.0f*fUVStep, 6.0f*fUVStep, 0.0f, 1.0f), false, quad2f(
        mBorders.right - fEdgeSize_, mBorders.right,
        mBorders.top, mBorders.top + fEdgeSize_
    ));

    // Bottom-left corner
    repeat_wrap_edge(quad2f(6.0f*fUVStep, 7.0f*fUVStep, 0.0f, 1.0f), false, quad2f(
        mBorders.left, mBorders.left + fEdgeSize_,
        mBorders.bottom - fEdgeSize_, mBorders.bottom
    ));

    // Bottom-right corner
    repeat_wrap_edge(quad2f(7.0f*fUVStep, 8.0f*fUVStep, 0.0f, 1.0f), false, quad2f(
        mBorders.right - fEdgeSize_, mBorders.right,
        mBorders.bottom - fEdgeSize_, mBorders.bottom
    ));

    if (pRenderer->is_vertex_cache_enabled() && !pRenderer->is_quad_batching_enabled())
    {
        if (!pEdgeCache_)
            pEdgeCache_ = pRenderer->create_vertex_cache(vertex_cache::type::QUADS);

        pEdgeCache_->update(lEdgeQuads_[0].data(), lEdgeQuads_.size()*4);
        lEdgeQuads_.clear();
    }
}

}
}
