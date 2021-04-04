#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
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

    this->set_backgrond_tilling(mBackdrop.is_background_tilling());
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
    if (!sBackgroundFile.empty())
    {
        if (utils::file_exists(sBackgroundFile))
        {
            auto* pTopLevelRenderer = pParent_->get_top_level_renderer();
            mBackground_ = pTopLevelRenderer->create_sprite(
                pTopLevelRenderer->create_material(sBackgroundFile)
            );
            uiTileSize_ = uiOriginalTileSize_ = mBackground_.get_width();
            mBackgroundColor_ = color::EMPTY;
            bHasBackground_ = true;
        }
        else
        {
            gui::out << gui::warning << "backdrop : "
                << "Cannot find file : \"" << sBackgroundFile << "\" for "
                << pParent_->get_name() << "'s backdrop background file.\n"
                << "No background will be drawn." << std::endl;

            sBackgroundFile_ = "";
            bHasBackground_ = false;
            return;
        }
    }
    else
        bHasBackground_ = false;

    sBackgroundFile_ = sBackgroundFile;
}

const std::string& backdrop::get_background_file() const
{
    return sBackgroundFile_;
}

void backdrop::set_background_color(const color& mColor)
{
    mBackgroundColor_ = mColor;
    sBackgroundFile_ = "";

    uiTileSize_ = uiOriginalTileSize_ = 256;
    auto* pTopLevelRenderer = pParent_->get_top_level_renderer();
    mBackground_ = pTopLevelRenderer->create_sprite(nullptr, 256, 256);
    mBackground_.set_color(mColor);

    bHasBackground_ = true;
}

color backdrop::get_background_color() const
{
    return mBackgroundColor_;
}

void backdrop::set_backgrond_tilling(bool bBackgroundTilling)
{
    bBackgroundTilling_ = bBackgroundTilling;

    if (!bBackgroundTilling_ && bHasBackground_)
        mBackground_.set_texture_rect(0.0f, 0.0f, 1.0f, 1.0f, true);
}

bool backdrop::is_background_tilling() const
{
    return bBackgroundTilling_;
}

void backdrop::set_tile_size(uint uiTileSize)
{
    uiTileSize_ = uiTileSize;
}

uint backdrop::get_tile_size() const
{
    return uiTileSize_;
}

void backdrop::set_background_insets(const quad2i& lInsets)
{
    lBackgroundInsets_ = lInsets;
}

void backdrop::set_background_insets(int iLeft, int iRight, int iTop, int iBottom)
{
    lBackgroundInsets_ = quad2i(iLeft, iRight, iTop, iBottom);
}

const quad2i& backdrop::get_background_insets() const
{
    return lBackgroundInsets_;
}

void backdrop::set_edge_insets(const quad2i& lInsets)
{
    lEdgeInsets_ = lInsets;
}

void backdrop::set_edge_insets(int iLeft, int iRight, int iTop, int iBottom)
{
    lEdgeInsets_ = quad2i(iLeft, iRight, iTop, iBottom);
}

const quad2i& backdrop::get_edge_insets() const
{
    return lEdgeInsets_;
}

void backdrop::set_edge(const std::string& sEdgeFile)
{
    if (!sEdgeFile.empty())
    {
        if (utils::file_exists(sEdgeFile))
        {
            auto* pTopLevelRenderer = pParent_->get_top_level_renderer();
            std::shared_ptr<material> pMat = pTopLevelRenderer->create_material(sEdgeFile);

            if (pMat->get_width()/pMat->get_height() == 8.0f)
            {
                uiEdgeSize_ = uiOriginalEdgeSize_ = pMat->get_height();

                for (uint i = 0; i < 8; ++i)
                {
                    lEdgeList_[i] = pTopLevelRenderer->create_sprite(
                        pMat, pMat->get_height()*i, 0.0f, uiEdgeSize_, uiEdgeSize_
                    );
                }

                get_edge(edge_type::TOPLEFT).set_hot_spot(
                    0.0f, 0.0f
                );
                get_edge(edge_type::TOPRIGHT).set_hot_spot(
                    uiOriginalEdgeSize_, 0.0f
                );
                get_edge(edge_type::BOTTOMLEFT).set_hot_spot(
                    0.0f, uiOriginalEdgeSize_
                );
                get_edge(edge_type::BOTTOMRIGHT).set_hot_spot(
                    uiOriginalEdgeSize_, uiOriginalEdgeSize_
                );

                bHasEdge_ = true;
                sEdgeFile_ = sEdgeFile;
                mEdgeColor_ = color::EMPTY;
            }
            else
            {
                bHasEdge_ = false;
                sEdgeFile_ = "";

                gui::out << gui::error << "backdrop : "
                    << "An edge file's width must be exactly 8 times greater than its height "
                    << "(in " << sEdgeFile << ").\nNo edge will be drawn for "
                    << pParent_->get_name() << "'s backdrop." << std::endl;
            }
        }
        else
        {
            bHasEdge_ = false;
            sEdgeFile_ = "";

            gui::out << gui::warning << "backdrop : "
                << "Cannot find file : \"" << sEdgeFile << "\" for " <<pParent_->get_name()
                << "'s backdrop edge.\nNo edge will be drawn." << std::endl;
        }
    }
    else
    {
        bHasEdge_ = false;
        sEdgeFile_ = "";
    }
}

const std::string& backdrop::get_edge_file() const
{
    return sEdgeFile_;
}

void backdrop::set_edge_color(const color& mColor)
{
    mEdgeColor_ = mColor;
    sEdgeFile_ = "";

    if (uiEdgeSize_ == 0)
        uiEdgeSize_ = 1;

    uiOriginalEdgeSize_ = 1;

    for (auto& mEdge : lEdgeList_)
    {
        auto* pTopLevelRenderer = pParent_->get_top_level_renderer();
        mEdge = pTopLevelRenderer->create_sprite(nullptr, 1, 1);
        mEdge.set_color(mColor);
    }

    get_edge(edge_type::TOPLEFT).set_hot_spot(0.0f, 0.0f);
    get_edge(edge_type::TOPRIGHT).set_hot_spot(1.0f, 0.0f);
    get_edge(edge_type::BOTTOMLEFT).set_hot_spot(0.0f, 1.0f);
    get_edge(edge_type::BOTTOMRIGHT).set_hot_spot(1.0f, 1.0f);
}

color backdrop::get_edge_color() const
{
    return mEdgeColor_;
}

void backdrop::set_edge_size(uint uiEdgeSize)
{
    uiEdgeSize_ = uiEdgeSize;
}

uint backdrop::get_edge_size() const
{
    return uiEdgeSize_;
}

void backdrop::set_vertex_color(const color& mColor)
{
    if (bHasBackground_)
        mBackground_.set_color(mColor);

    if (bHasEdge_)
    {
        for (auto& mEdge : lEdgeList_) mEdge.set_color(mColor);
    }
}

void backdrop::render() const
{
    if (pParent_)
    {
        const quad2i& lParentBorders = pParent_->get_borders();

        if (bHasBackground_)
        {
            if (bBackgroundTilling_)
            {
                mBackground_.set_texture_rect(
                    0.0f, 0.0f,
                    (
                        lParentBorders.right + lBackgroundInsets_.right -
                        (lParentBorders.left + lBackgroundInsets_.left)
                    )*uiOriginalTileSize_/float(uiTileSize_),
                    (
                        lParentBorders.bottom - lBackgroundInsets_.bottom -
                        (lParentBorders.top   - lBackgroundInsets_.top)
                    )*uiOriginalTileSize_/float(uiTileSize_)
                );
            }

            mBackground_.render_2v(
                lParentBorders.left   + lBackgroundInsets_.left,
                lParentBorders.top    + lBackgroundInsets_.top,
                lParentBorders.right  - lBackgroundInsets_.right,
                lParentBorders.bottom - lBackgroundInsets_.bottom
            );
        }

        if (bHasEdge_)
        {
            float fEdgeScale = float(uiEdgeSize_)/float(uiOriginalEdgeSize_);

            // render corners
            get_edge(edge_type::TOPLEFT).render_ex(
                lParentBorders.left + lEdgeInsets_.left,
                lParentBorders.top  + lEdgeInsets_.top,
                0.0f, fEdgeScale, fEdgeScale
            );

            get_edge(edge_type::TOPRIGHT).render_ex(
                lParentBorders.right - lEdgeInsets_.right,
                lParentBorders.top   + lEdgeInsets_.top,
                0.0f, fEdgeScale, fEdgeScale
            );

            get_edge(edge_type::BOTTOMLEFT).render_ex(
                lParentBorders.left   + lEdgeInsets_.left,
                lParentBorders.bottom - lEdgeInsets_.bottom,
                0.0f, fEdgeScale, fEdgeScale
            );

            get_edge(edge_type::BOTTOMRIGHT).render_ex(
                lParentBorders.right  - lEdgeInsets_.right,
                lParentBorders.bottom - lEdgeInsets_.bottom,
                0.0f, fEdgeScale, fEdgeScale
            );

            // render sides
            float fEdgeHeight = lParentBorders.bottom - lEdgeInsets_.bottom
                - lParentBorders.top - lEdgeInsets_.top - 2u*uiEdgeSize_;

            if (fEdgeHeight > 0.0f)
            {
                get_edge(edge_type::LEFT).set_texture_rect(
                    0.0f, 0.0f, uiOriginalEdgeSize_, fEdgeHeight
                );

                get_edge(edge_type::RIGHT).set_texture_rect(
                    uiOriginalEdgeSize_, 0.0f, 2u*uiOriginalEdgeSize_, fEdgeHeight
                );

                get_edge(edge_type::LEFT).render_2v(
                    lParentBorders.left   + lEdgeInsets_.left,
                    lParentBorders.top    + lEdgeInsets_.top    + uiEdgeSize_,

                    lParentBorders.left   + lEdgeInsets_.left   + uiEdgeSize_,
                    lParentBorders.bottom - lEdgeInsets_.bottom - uiEdgeSize_
                );

                get_edge(edge_type::RIGHT).render_2v(
                    lParentBorders.right  - lEdgeInsets_.right  - uiEdgeSize_,
                    lParentBorders.top    + lEdgeInsets_.top    + uiEdgeSize_,

                    lParentBorders.right  - lEdgeInsets_.right,
                    lParentBorders.bottom - lEdgeInsets_.bottom - uiEdgeSize_
                );
            }

            float fEdgeWidth = lParentBorders.right - lEdgeInsets_.right
                - lParentBorders.left - lEdgeInsets_.left - 2*uiEdgeSize_;

            if (fEdgeWidth > 0.0f)
            {
                get_edge(edge_type::TOP).set_texture_rect(
                    2u*uiOriginalEdgeSize_, 0.0f, 3u*uiOriginalEdgeSize_, fEdgeWidth
                );

                get_edge(edge_type::BOTTOM).set_texture_rect(
                    3u*uiOriginalEdgeSize_, 0.0f, 4u*uiOriginalEdgeSize_, fEdgeWidth
                );

                get_edge(edge_type::TOP).render_4v(
                    lParentBorders.right - lEdgeInsets_.right - uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top,

                    lParentBorders.right - lEdgeInsets_.right - uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top   + uiEdgeSize_,

                    lParentBorders.left  + lEdgeInsets_.left  + uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top   + uiEdgeSize_,

                    lParentBorders.left  + lEdgeInsets_.left  + uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top
                );

                get_edge(edge_type::BOTTOM).render_4v(
                    lParentBorders.right  - lEdgeInsets_.right  - uiEdgeSize_,
                    lParentBorders.bottom - lEdgeInsets_.bottom - uiEdgeSize_,

                    lParentBorders.right  - lEdgeInsets_.right  - uiEdgeSize_,
                    lParentBorders.bottom - lEdgeInsets_.bottom,

                    lParentBorders.left   + lEdgeInsets_.left   + uiEdgeSize_,
                    lParentBorders.bottom - lEdgeInsets_.bottom,

                    lParentBorders.left   + lEdgeInsets_.left   + uiEdgeSize_,
                    lParentBorders.bottom - lEdgeInsets_.bottom - uiEdgeSize_
                );
            }
        }
    }
}

sprite& backdrop::get_edge(edge_type mPoint) const
{
    return lEdgeList_[static_cast<uint>(mPoint)];
}
}
}
