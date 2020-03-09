#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_filesystem.hpp>

namespace gui
{
backdrop::backdrop(frame* pParent) :
    pParent_(pParent), mBackgroundColor_(color::EMPTY), mEdgeColor_(color::EMPTY),
    bBackgroundTilling_(false), uiTileSize_(0u), uiOriginalTileSize_(0u),
    lBackgroundInsets_(quad2i::ZERO), lEdgeInsets_(quad2i::ZERO),
    uiEdgeSize_(0u), uiOriginalEdgeSize_(0u)
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
        std::string sFile = pParent_->get_manager()->parse_file_name(sBackgroundFile);
        if (utils::file_exists(sFile))
        {
            pBackground_ = pParent_->get_manager()->create_sprite(
                pParent_->get_manager()->create_material(sFile)
            );
            uiTileSize_ = uiOriginalTileSize_ = pBackground_->get_width();
            mBackgroundColor_ = color::EMPTY;
        }
        else
        {
            gui::out << gui::warning << "backdrop : "
                << "Cannot find file : \"" << sFile << "\" for "
                << pParent_->get_name() << "'s backdrop background file.\n"
                << "No background will be drawn." << std::endl;

            sBackgroundFile_ = "";
            return;
        }
    }
    else
        pBackground_ = nullptr;

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
    pBackground_ = pParent_->get_manager()->create_sprite(
        pParent_->get_manager()->create_material(mColor), 256, 256
    );
}

color backdrop::get_background_color() const
{
    return mBackgroundColor_;
}

void backdrop::set_backgrond_tilling(bool bBackgroundTilling)
{
    bBackgroundTilling_ = bBackgroundTilling;

    if (!bBackgroundTilling_ && pBackground_)
        pBackground_->set_texture_rect(0.0f, 0.0f, 1.0f, 1.0f, true);
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
        std::string sFile = pParent_->get_manager()->parse_file_name(sEdgeFile);
        if (utils::file_exists(sFile))
        {
            utils::refptr<material> pMat = pParent_->get_manager()->create_material(sFile);

            if (pMat->get_width()/pMat->get_height() == 8.0f)
            {
                uiEdgeSize_ = uiOriginalEdgeSize_ = pMat->get_height();

                for (uint i = 0; i < 8; ++i)
                {
                    lEdgeList_[i] = pParent_->get_manager()->create_sprite(
                        pMat, pMat->get_height()*i, 0.0f, uiEdgeSize_, uiEdgeSize_
                    );
                }

                lEdgeList_[EDGE_TOPLEFT]->set_hot_spot(
                    0.0f, 0.0f
                );
                lEdgeList_[EDGE_TOPRIGHT]->set_hot_spot(
                    uiOriginalEdgeSize_, 0.0f
                );
                lEdgeList_[EDGE_BOTTOMLEFT]->set_hot_spot(
                    0.0f, uiOriginalEdgeSize_
                );
                lEdgeList_[EDGE_BOTTOMRIGHT]->set_hot_spot(
                    uiOriginalEdgeSize_, uiOriginalEdgeSize_
                );

                sEdgeFile_ = sFile;
                mEdgeColor_ = color::EMPTY;
            }
            else
            {
                for (auto& mEdge : lEdgeList_) mEdge = nullptr;
                sEdgeFile_ = "";

                gui::out << gui::error << "backdrop : "
                    << "An edge file's width must be exactly 8 times greater than its height "
                    << "(in " << sFile << ").\nNo edge will be drawn for "
                    << pParent_->get_name() << "'s backdrop." << std::endl;
            }
        }
        else
        {
            for (auto& mEdge : lEdgeList_) mEdge = nullptr;
            sEdgeFile_ = "";

            gui::out << gui::warning << "backdrop : "
                << "Cannot find file : \"" << sEdgeFile << "\" for " <<pParent_->get_name()
                << "'s backdrop edge.\nNo edge will be drawn." << std::endl;
        }
    }
    else
    {
        for (auto& mEdge : lEdgeList_) mEdge = nullptr;
    }

    sEdgeFile_ = sEdgeFile;
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

    for (uint i = 0; i < 8; ++i)
    {
        lEdgeList_[i] = pParent_->get_manager()->create_sprite(
            pParent_->get_manager()->create_material(mColor), 1, 1
        );
    }

    lEdgeList_[EDGE_TOPLEFT]->set_hot_spot(0.0f, 0.0f);
    lEdgeList_[EDGE_TOPRIGHT]->set_hot_spot(1.0f, 0.0f);
    lEdgeList_[EDGE_BOTTOMLEFT]->set_hot_spot(0.0f, 1.0f);
    lEdgeList_[EDGE_BOTTOMRIGHT]->set_hot_spot(1.0f, 1.0f);
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
    if (pBackground_)
        pBackground_->set_color(mColor);

    if (lEdgeList_[0])
    {
        for (uint i = 0; i < 8; ++i)
            lEdgeList_[i]->set_color(mColor);
    }
}

void backdrop::render() const
{
    if (pParent_)
    {
        const quad2i& lParentBorders = pParent_->get_borders();

        if (pBackground_)
        {
            if (bBackgroundTilling_)
            {
                pBackground_->set_texture_rect(
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

            pBackground_->render_2v(
                lParentBorders.left   + lBackgroundInsets_.left,
                lParentBorders.top    + lBackgroundInsets_.top,
                lParentBorders.right  - lBackgroundInsets_.right,
                lParentBorders.bottom - lBackgroundInsets_.bottom
            );
        }

        if (lEdgeList_[0])
        {
            float fEdgeScale = float(uiEdgeSize_)/float(uiOriginalEdgeSize_);

            // render corners
            lEdgeList_[EDGE_TOPLEFT]->render_ex(
                lParentBorders.left + lEdgeInsets_.left,
                lParentBorders.top  + lEdgeInsets_.top,
                0.0f, fEdgeScale, fEdgeScale
            );

            lEdgeList_[EDGE_TOPRIGHT]->render_ex(
                lParentBorders.right - lEdgeInsets_.right,
                lParentBorders.top   + lEdgeInsets_.top,
                0.0f, fEdgeScale, fEdgeScale
            );

            lEdgeList_[EDGE_BOTTOMLEFT]->render_ex(
                lParentBorders.left   + lEdgeInsets_.left,
                lParentBorders.bottom - lEdgeInsets_.bottom,
                0.0f, fEdgeScale, fEdgeScale
            );

            lEdgeList_[EDGE_BOTTOMRIGHT]->render_ex(
                lParentBorders.right  - lEdgeInsets_.right,
                lParentBorders.bottom - lEdgeInsets_.bottom,
                0.0f, fEdgeScale, fEdgeScale
            );

            // render sides
            float fEdgeHeight = lParentBorders.bottom - lEdgeInsets_.bottom
                - lParentBorders.top - lEdgeInsets_.top - 2u*uiEdgeSize_;

            if (fEdgeHeight > 0.0f)
            {
                lEdgeList_[EDGE_LEFT]->set_texture_rect(
                    0.0f, 0.0f, uiOriginalEdgeSize_, fEdgeHeight
                );

                lEdgeList_[EDGE_RIGHT]->set_texture_rect(
                    uiOriginalEdgeSize_, 0.0f, 2u*uiOriginalEdgeSize_, fEdgeHeight
                );

                lEdgeList_[EDGE_LEFT]->render_2v(
                    lParentBorders.left   + lEdgeInsets_.left,
                    lParentBorders.top    + lEdgeInsets_.top    + uiEdgeSize_,

                    lParentBorders.left   + lEdgeInsets_.left   + uiEdgeSize_,
                    lParentBorders.bottom - lEdgeInsets_.bottom - uiEdgeSize_
                );

                lEdgeList_[EDGE_RIGHT]->render_2v(
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
                lEdgeList_[EDGE_TOP]->set_texture_rect(
                    2u*uiOriginalEdgeSize_, 0.0f, 3u*uiOriginalEdgeSize_, fEdgeWidth
                );

                lEdgeList_[EDGE_BOTTOM]->set_texture_rect(
                    3u*uiOriginalEdgeSize_, 0.0f, 4u*uiOriginalEdgeSize_, fEdgeWidth
                );

                lEdgeList_[EDGE_TOP]->render_4v(
                    lParentBorders.right - lEdgeInsets_.right - uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top,

                    lParentBorders.right - lEdgeInsets_.right - uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top   + uiEdgeSize_,

                    lParentBorders.left  + lEdgeInsets_.left  + uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top   + uiEdgeSize_,

                    lParentBorders.left  + lEdgeInsets_.left  + uiEdgeSize_,
                    lParentBorders.top   + lEdgeInsets_.top
                );

                lEdgeList_[EDGE_BOTTOM]->render_4v(
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
}
