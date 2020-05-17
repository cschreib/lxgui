#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{
#ifdef MSVC
template<typename T>
T round(T value)
{
    return (long)(value + 0.5);
}
#endif

void step_value(float& fValue, float fStep)
{
    // Makes the value a multiple of the step :
    // fValue = N*fStep, where N is an integer.
    if (fStep != 0.0f)
        fValue = round(fValue/fStep)*fStep;
}

slider::slider(manager* pManager) : frame(pManager),
    bUpdateThumbTexture_(false), mOrientation_(orientation::VERTICAL), fValue_(0.0f),
    fMinValue_(0.0f), fMaxValue_(1.0f), fValueStep_(0.1f), bAllowClicksOutsideThumb_(true),
    mThumbLayer_(layer_type::OVERLAY), pThumbTexture_(nullptr), bThumbMoved_(false), bMouseInThumb_(false)
{
    enable_mouse(true);
    lType_.push_back(CLASS_NAME);
}

slider::~slider()
{
}

std::string slider::serialize(const std::string& sTab) const
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
    sStr << sTab << "  # Value      : " << fValue_ << "\n";
    sStr << sTab << "  # Min value  : " << fMinValue_ << "\n";
    sStr << sTab << "  # Max value  : " << fMaxValue_ << "\n";
    sStr << sTab << "  # Step       : " << fValueStep_ << "\n";
    sStr << sTab << "  # Click out  : " << bAllowClicksOutsideThumb_ << "\n";

    return sStr.str();
}

bool slider::can_use_script(const std::string& sScriptName) const
{
    if (frame::can_use_script(sScriptName))
        return true;
    else if (sScriptName == "OnValueChanged")
        return true;
    else
        return false;
}

void slider::copy_from(uiobject* pObj)
{
    frame::copy_from(pObj);

    slider* pSlider = dynamic_cast<slider*>(pObj);

    if (pSlider)
    {
        this->set_value_step(pSlider->get_value_step());
        this->set_min_value(pSlider->get_min_value());
        this->set_max_value(pSlider->get_max_value());
        this->set_value(pSlider->get_value());
        this->set_thumb_draw_layer(pSlider->get_thumb_draw_layer());
        this->set_orientation(pSlider->get_orientation());
        this->set_allow_clicks_outside_thumb(pSlider->are_clicks_outside_thumb_allowed());

        texture* pThumb = pSlider->get_thumb_texture();
        if (pThumb)
        {
            this->create_thumb_texture_();
            if (this->is_virtual())
                pThumbTexture_->set_virtual();
            pThumbTexture_->set_name(pThumb->get_name());
            if (!pManager_->add_uiobject(pThumbTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pThumb->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pThumbTexture_->get_name()+"\". Skipped." << std::endl;
                delete pThumbTexture_; pThumbTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pThumbTexture_->create_glue();

                this->add_region(pThumbTexture_);
                pThumbTexture_->copy_from(pThumb);
                pThumbTexture_->notify_loaded();
            }
        }
    }
}

void slider::constrain_thumb_()
{
    if (fMaxValue_ == fMinValue_)
        return;

    if ((mOrientation_ == orientation::HORIZONTAL && uiAbsWidth_  != 0) ||
        (mOrientation_ == orientation::VERTICAL   && uiAbsHeight_ != 0))
    {
        float fValue = 0.0f;

        if (bThumbMoved_)
        {
            if (mOrientation_ == orientation::HORIZONTAL)
                fValue = float(pThumbTexture_->get_point(anchor_point::CENTER)->get_abs_offset_x())/float(uiAbsWidth_);
            else
                fValue = float(pThumbTexture_->get_point(anchor_point::CENTER)->get_abs_offset_y())/float(uiAbsHeight_);

            fValue *= (fMaxValue_ - fMinValue_);
            fValue += fMinValue_;
            fValue = fValue > fMaxValue_ ? fMaxValue_ : (fValue < fMinValue_ ? fMinValue_ : fValue);
            step_value(fValue, fValueStep_);
        }

        float fCoef = (fValue - fMinValue_)/(fMaxValue_ - fMinValue_);

        anchor* pAnchor = pThumbTexture_->modify_point(anchor_point::CENTER);
        if (mOrientation_ == orientation::HORIZONTAL)
            pAnchor->set_abs_offset(uiAbsWidth_*fCoef, 0);
        else
            pAnchor->set_abs_offset(0, uiAbsHeight_*fCoef);
    }
}

void slider::on_event(const event& mEvent)
{
    frame::on_event(mEvent);

    if (bIsMouseEnabled_ && pManager_->is_input_enabled())
    {
        if (mEvent.get_name() == "MOUSE_PRESSED")
        {
            if (bMouseInThumb_)
            {
                pManager_->start_moving(
                    pThumbTexture_, pThumbTexture_->modify_point(anchor_point::CENTER),
                    mOrientation_ == orientation::HORIZONTAL ? constraint::X : constraint::Y,
                    std::bind(&slider::constrain_thumb_, this)
                );
                bThumbMoved_ = true;
            }
            else if (bMouseInFrame_ && bAllowClicksOutsideThumb_)
            {
                float fValue;
                if (mOrientation_ == orientation::HORIZONTAL)
                {
                    float fOffset = float(iMousePosX_ - lBorderList_.left);
                    fValue = fOffset/uiAbsWidth_;
                    set_value(fValue*(fMaxValue_ - fMinValue_) + fMinValue_);
                }
                else
                {
                    float fOffset = float(iMousePosY_ - lBorderList_.top);
                    fValue = fOffset/uiAbsHeight_;
                    set_value(fValue*(fMaxValue_ - fMinValue_) + fMinValue_);
                }

                if (pThumbTexture_)
                {
                    float fCoef = (fValue_ - fMinValue_)/(fMaxValue_ - fMinValue_);

                    anchor* pAnchor = pThumbTexture_->modify_point(anchor_point::CENTER);
                    if (mOrientation_ == orientation::HORIZONTAL)
                        pAnchor->set_abs_offset(uiAbsWidth_*fCoef, 0);
                    else
                        pAnchor->set_abs_offset(0, uiAbsHeight_*fCoef);

                    pManager_->start_moving(
                        pThumbTexture_, pThumbTexture_->modify_point(anchor_point::CENTER),
                        mOrientation_ == orientation::HORIZONTAL ? constraint::X : constraint::Y
                    );
                    bThumbMoved_ = true;
                }
            }
        }
        else if (mEvent.get_name() == "MOUSE_RELEASED")
        {
            if (pThumbTexture_)
            {
                pManager_->stop_moving(pThumbTexture_);
                bThumbMoved_ = false;
            }
        }
    }
}

void slider::set_min_value(float fMin)
{
    if (fMin != fMinValue_)
    {
        fMinValue_ = fMin;
        if (fMinValue_ > fMaxValue_) fMinValue_ = fMaxValue_;
        else step_value(fMinValue_, fValueStep_);

        if (fValue_ < fMinValue_)
        {
            fValue_ = fMinValue_;
            lQueuedEventList_.push_back("ValueChanged");
        }

        fire_update_thumb_texture_();
    }
}

void slider::set_max_value(float fMax)
{
    if (fMax != fMaxValue_)
    {
        fMaxValue_ = fMax;
        if (fMaxValue_ < fMinValue_) fMaxValue_ = fMinValue_;
        else step_value(fMaxValue_, fValueStep_);

        if (fValue_ > fMaxValue_)
        {
            fValue_ = fMaxValue_;
            lQueuedEventList_.push_back("ValueChanged");
        }

        fire_update_thumb_texture_();
    }
}

void slider::set_min_max_values(float fMin, float fMax)
{
    if (fMin != fMinValue_ || fMax != fMaxValue_)
    {
        fMinValue_ = std::min(fMin, fMax);
        fMaxValue_ = std::max(fMin, fMax);
        step_value(fMinValue_, fValueStep_);
        step_value(fMaxValue_, fValueStep_);

        if (fValue_ > fMaxValue_ || fValue_ < fMinValue_)
        {
            fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
            lQueuedEventList_.push_back("ValueChanged");
        }

        fire_update_thumb_texture_();
    }
}

void slider::set_value(float fValue, bool bSilent)
{
    if (fValue != fValue_)
    {
        fValue_ = fValue;
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
        step_value(fValue_, fValueStep_);

        if (!bSilent)
            lQueuedEventList_.push_back("ValueChanged");

        fire_update_thumb_texture_();
    }
}

void slider::set_value_step(float fValueStep)
{
    if (fValueStep_ != fValueStep)
    {
        fValueStep_ = fValueStep;

        step_value(fMinValue_, fValueStep_);
        step_value(fMaxValue_, fValueStep_);

        float fOldValue = fValue_;
        step_value(fValue_, fValueStep_);
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);

        if (fValue_ != fOldValue)
            lQueuedEventList_.push_back("ValueChanged");

        fire_update_thumb_texture_();
    }
}

void slider::set_thumb_texture(texture* pTexture)
{
    pThumbTexture_ = pTexture;
    pThumbTexture_->clear_all_points();
    pThumbTexture_->set_point(anchor(
        pThumbTexture_, anchor_point::CENTER, sName_,
        mOrientation_ == orientation::HORIZONTAL ? anchor_point::LEFT : anchor_point::TOP
    ));

    fire_update_thumb_texture_();
}

void slider::set_orientation(orientation mOrientation)
{
    if (mOrientation != mOrientation_)
    {
        mOrientation_ = mOrientation;
        if (pThumbTexture_)
        {
            pThumbTexture_->set_point(anchor(
                pThumbTexture_, anchor_point::CENTER, sName_,
                mOrientation_ == orientation::HORIZONTAL ? anchor_point::LEFT : anchor_point::TOP
            ));
        }

        fire_update_thumb_texture_();
    }
}

void slider::set_thumb_draw_layer(layer_type mThumbLayer)
{
    mThumbLayer_ = mThumbLayer;
    if (pThumbTexture_)
        pThumbTexture_->set_draw_layer(mThumbLayer_);
}

void slider::set_thumb_draw_layer(const std::string& sThumbLayer)
{
    if (sThumbLayer == "ARTWORK")
        mThumbLayer_ = layer_type::ARTWORK;
    else if (sThumbLayer == "BACKGROUND")
        mThumbLayer_ = layer_type::BACKGROUND;
    else if (sThumbLayer == "BORDER")
        mThumbLayer_ = layer_type::BORDER;
    else if (sThumbLayer == "HIGHLIGHT")
        mThumbLayer_ = layer_type::HIGHLIGHT;
    else if (sThumbLayer == "OVERLAY")
        mThumbLayer_ = layer_type::OVERLAY;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            "Uknown layer type : \""+sThumbLayer+"\". Using \"OVERLAY\"." << std::endl;
        mThumbLayer_ = layer_type::OVERLAY;
    }

    if (pThumbTexture_)
        pThumbTexture_->set_draw_layer(mThumbLayer_);
}

float slider::get_min_value() const
{
    return fMinValue_;
}

float slider::get_max_value() const
{
    return fMaxValue_;
}

float slider::get_value() const
{
    return fValue_;
}

float slider::get_value_step() const
{
    return fValueStep_;
}

texture* slider::get_thumb_texture() const
{
    return pThumbTexture_;
}

slider::orientation slider::get_orientation() const
{
    return mOrientation_;
}

layer_type slider::get_thumb_draw_layer() const
{
    return mThumbLayer_;
}

void slider::set_allow_clicks_outside_thumb(bool bAllow)
{
    bAllowClicksOutsideThumb_ = bAllow;
}

bool slider::are_clicks_outside_thumb_allowed()
{
    return bAllowClicksOutsideThumb_;
}

texture* slider::create_thumb_texture_()
{
    if (!pThumbTexture_)
    {
        pThumbTexture_ = new texture(pManager_);
        pThumbTexture_->set_special();
        pThumbTexture_->set_parent(this);
        pThumbTexture_->set_draw_layer(mThumbLayer_);
    }

    return pThumbTexture_;
}

bool slider::is_in_frame(int iX, int iY) const
{
    if (bAllowClicksOutsideThumb_)
    {
        if (pThumbTexture_)
            return frame::is_in_frame(iX, iY) || pThumbTexture_->is_in_region(iX, iY);
        else
            return frame::is_in_frame(iX, iY);
    }
    else
    {
        if (pThumbTexture_)
            return pThumbTexture_->is_in_region(iX, iY);
        else
            return false;
    }
}

void slider::notify_mouse_in_frame(bool bMouseInFrame, int iX, int iY)
{
    if (bAllowClicksOutsideThumb_)
        frame::notify_mouse_in_frame(bMouseInFrame, iX, iY);

    bMouseInThumb_ = (bMouseInFrame && pThumbTexture_ && pThumbTexture_->is_in_region(iX, iY));
}

void slider::update(float fDelta)
{
    frame::update(fDelta);

    if ((bUpdateThumbTexture_ || bThumbMoved_) && pThumbTexture_)
    {
        if ((mOrientation_ == orientation::HORIZONTAL && uiAbsWidth_  != 0) ||
            (mOrientation_ == orientation::VERTICAL   && uiAbsHeight_ != 0))
        {
            if (fMaxValue_ == fMinValue_)
            {
                pThumbTexture_->hide();
                return;
            }
            else
                pThumbTexture_->show();

            float fOldValue = fValue_;

            if (bThumbMoved_)
            {
                if (mOrientation_ == orientation::HORIZONTAL)
                    fValue_ = float(pThumbTexture_->get_point(anchor_point::CENTER)->get_abs_offset_x())/float(uiAbsWidth_);
                else
                    fValue_ = float(pThumbTexture_->get_point(anchor_point::CENTER)->get_abs_offset_y())/float(uiAbsHeight_);

                fValue_ *= (fMaxValue_ - fMinValue_);
                fValue_ += fMinValue_;
                fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
                step_value(fValue_, fValueStep_);

                if (fValue_ != fOldValue)
                    lQueuedEventList_.push_back("ValueChanged");
            }

            float fCoef = (fValue_ - fMinValue_)/(fMaxValue_ - fMinValue_);

            anchor* pAnchor = pThumbTexture_->modify_point(anchor_point::CENTER);
            if (mOrientation_ == orientation::HORIZONTAL)
                pAnchor->set_abs_offset(uiAbsWidth_*fCoef, 0);
            else
                pAnchor->set_abs_offset(0, uiAbsHeight_*fCoef);

            pThumbTexture_->fire_update_borders();
            pThumbTexture_->update(fDelta);

            bUpdateThumbTexture_ = false;
        }
    }
}

void slider::fire_update_borders() const
{
    frame::fire_update_borders();
    fire_update_thumb_texture_();
}

void slider::create_glue()
{
    create_glue_<lua_slider>();
}

void slider::fire_update_thumb_texture_() const
{
    bUpdateThumbTexture_ = true;
}
}
}
