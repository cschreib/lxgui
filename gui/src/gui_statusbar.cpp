#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{
std::array<float,4> select_uvs(const std::array<float,8>& uvs)
{
    std::array<float,4> u;
    u[0] = uvs[0]; u[1] = uvs[1]; u[2] = uvs[4]; u[3] = uvs[5];
    return u;
}

status_bar::status_bar(manager& mManager) : frame(mManager)
{
    lType_.push_back(CLASS_NAME);
}

std::string status_bar::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << frame::serialize(sTab);
    sStr << sTab << "  # Orientation: ";
    switch (mOrientation_)
    {
        case orientation::HORIZONTAL : sStr << "HORIZONTAL"; break;
        case orientation::VERTICAL   : sStr << "VERTICAL";   break;
    }
    sStr << "\n";
    sStr << sTab << "  # Reversed   : " << bReversed_ << "\n";
    sStr << sTab << "  # Value      : " << fValue_ << "\n";
    sStr << sTab << "  # Min value  : " << fMinValue_ << "\n";
    sStr << sTab << "  # Max value  : " << fMaxValue_ << "\n";

    return sStr.str();
}

bool status_bar::can_use_script(const std::string& sScriptName) const
{
    if (frame::can_use_script(sScriptName))
        return true;
    else if (sScriptName == "OnValueChanged")
        return true;
    else
        return false;
}

void status_bar::copy_from(const uiobject& mObj)
{
    frame::copy_from(mObj);

    const status_bar& mStatusBar = static_cast<const status_bar&>(mObj);

    this->set_min_value(mStatusBar.get_min_value());
    this->set_max_value(mStatusBar.get_max_value());
    this->set_value(mStatusBar.get_value());
    this->set_bar_draw_layer(mStatusBar.get_bar_draw_layer());
    this->set_orientation(mStatusBar.get_orientation());
    this->set_reversed(mStatusBar.is_reversed());

    if (const texture* pBar = mStatusBar.get_bar_texture().get())
    {
        auto pBarTexture = this->create_region<texture>(
            pBar->get_draw_layer(), pBar->get_name(),
            {mStatusBar.get_bar_texture()});

        if (pBarTexture)
        {
            pBarTexture->set_special();
            pBarTexture->notify_loaded();
            this->set_bar_texture(pBarTexture);
        }
    }
}

void status_bar::set_min_value(float fMin)
{
    if (fMin != fMinValue_)
    {
        fMinValue_ = fMin;
        if (fMinValue_ > fMaxValue_) fMinValue_ = fMaxValue_;
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_max_value(float fMax)
{
    if (fMax != fMaxValue_)
    {
        fMaxValue_ = fMax;
        if (fMaxValue_ < fMinValue_) fMaxValue_ = fMinValue_;
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_min_max_values(float fMin, float fMax)
{
    if (fMin != fMinValue_ || fMax != fMaxValue_)
    {
        fMinValue_ = std::min(fMin, fMax);
        fMaxValue_ = std::max(fMin, fMax);
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_value(float fValue)
{
    fValue = fValue > fMaxValue_ ? fMaxValue_ : (fValue < fMinValue_ ? fMinValue_ : fValue);
    if (fValue != fValue_)
    {
        fValue_ = fValue;
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_bar_draw_layer(layer_type mBarLayer)
{
    mBarLayer_ = mBarLayer;
    if (pBarTexture_)
        pBarTexture_->set_draw_layer(mBarLayer_);
}

void status_bar::set_bar_draw_layer(const std::string& sBarLayer)
{
    if (sBarLayer == "ARTWORK")
        mBarLayer_ = layer_type::ARTWORK;
    else if (sBarLayer == "BACKGROUND")
        mBarLayer_ = layer_type::BACKGROUND;
    else if (sBarLayer == "BORDER")
        mBarLayer_ = layer_type::BORDER;
    else if (sBarLayer == "HIGHLIGHT")
        mBarLayer_ = layer_type::HIGHLIGHT;
    else if (sBarLayer == "OVERLAY")
        mBarLayer_ = layer_type::OVERLAY;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            "Unknown layer type : \""+sBarLayer+"\". Using \"ARTWORK\"." << std::endl;

        mBarLayer_ = layer_type::ARTWORK;
    }

    if (pBarTexture_)
        pBarTexture_->set_draw_layer(mBarLayer_);
}

void status_bar::set_bar_texture(utils::observer_ptr<texture> pBarTexture)
{
    pBarTexture_ = std::move(pBarTexture);
    if (!pBarTexture_)
        return;

    pBarTexture_->set_draw_layer(mBarLayer_);
    pBarTexture_->clear_all_points();

    std::string sParent = pBarTexture_->get_parent().get() == this ? "$parent" : sName_;

    if (bReversed_)
        pBarTexture_->set_point(anchor_data(anchor_point::TOPRIGHT, sParent));
    else
        pBarTexture_->set_point(anchor_data(anchor_point::BOTTOMLEFT, sParent));

    lInitialTextCoords_ = select_uvs(pBarTexture_->get_tex_coord());
    notify_bar_texture_needs_update_();
}

void status_bar::set_bar_color(const color& mBarColor)
{
    create_bar_texture_();

    mBarColor_ = mBarColor;
    pBarTexture_->set_solid_color(mBarColor_);
}

void status_bar::set_orientation(orientation mOrientation)
{
    if (mOrientation != mOrientation_)
    {
        mOrientation_ = mOrientation;
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_orientation(const std::string& sOrientation)
{
    orientation mOrientation = orientation::HORIZONTAL;
    if (sOrientation == "VERTICAL")
        mOrientation = orientation::VERTICAL;
    else if (sOrientation == "HORIZONTAL")
        mOrientation = orientation::HORIZONTAL;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            "Unknown orientation : \""+sOrientation+"\". Using \"HORIZONTAL\"." << std::endl;
    }

    set_orientation(mOrientation);
}

void status_bar::set_reversed(bool bReversed)
{
    if (bReversed == bReversed_)
        return;

    bReversed_ = bReversed;

    if (pBarTexture_)
    {
        if (bReversed_)
            pBarTexture_->set_point(anchor_data(anchor_point::TOPRIGHT));
        else
            pBarTexture_->set_point(anchor_data(anchor_point::BOTTOMLEFT));

        if (!bVirtual_)
            pBarTexture_->notify_borders_need_update();
    }
}

float status_bar::get_min_value() const
{
    return fMinValue_;
}

float status_bar::get_max_value() const
{
    return fMaxValue_;
}

float status_bar::get_value() const
{
    return fValue_;
}

layer_type status_bar::get_bar_draw_layer() const
{
    return mBarLayer_;
}

const color& status_bar::get_bar_color() const
{
    return mBarColor_;
}

status_bar::orientation status_bar::get_orientation() const
{
    return mOrientation_;
}

bool status_bar::is_reversed() const
{
    return bReversed_;
}

void status_bar::create_bar_texture_()
{
    if (pBarTexture_)
        return;

    auto pBarTexture = create_region<texture>(mBarLayer_, "$parentBarTexture");
    pBarTexture->set_special();
    pBarTexture->notify_loaded();
    set_bar_texture(pBarTexture);
}

void status_bar::create_glue()
{
    create_glue_(this);
}

void status_bar::update(float fDelta)
{

    if (bUpdateBarTexture_ && pBarTexture_)
    {
        float fCoef = (fValue_ - fMinValue_)/(fMaxValue_ - fMinValue_);

        if (mOrientation_ == orientation::HORIZONTAL)
            pBarTexture_->set_relative_dimensions(vector2f(fCoef, 1.0f));
        else
            pBarTexture_->set_relative_dimensions(vector2f(1.0f, fCoef));

        std::array<float,4> uvs = lInitialTextCoords_;
        if (mOrientation_ == orientation::HORIZONTAL)
        {
            if (bReversed_)
                uvs[0] = (uvs[0] - uvs[2])*fCoef + uvs[2];
            else
                uvs[2] = (uvs[2] - uvs[0])*fCoef + uvs[0];
        }
        else
        {
            if (bReversed_)
                uvs[3] = (uvs[3] - uvs[1])*fCoef + uvs[1];
            else
                uvs[1] = (uvs[1] - uvs[3])*fCoef + uvs[3];
        }

        pBarTexture_->set_tex_rect(uvs);

        bUpdateBarTexture_ = false;
    }

    alive_checker mChecker(*this);
    frame::update(fDelta);
    if (!mChecker.is_alive())
        return;
}

void status_bar::notify_bar_texture_needs_update_()
{
    bUpdateBarTexture_ = true;
}
}
}
