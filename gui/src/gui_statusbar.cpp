#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <sstream>

namespace gui
{
std::array<float,4> select_uvs(const std::array<float,8>& uvs)
{
    std::array<float,4> u;
    u[0] = uvs[0]; u[1] = uvs[1]; u[2] = uvs[4]; u[3] = uvs[5];
    return u;
}

#ifdef NO_CPP11_CONSTEXPR
const char* status_bar::CLASS_NAME = "StatusBar";
#endif

status_bar::status_bar(manager* pManager) : frame(pManager),
    bUpdateBarTexture_(false), mOrientation_(ORIENT_HORIZONTAL), bReversed_(false),
    fValue_(0.0f), fMinValue_(0.0f), fMaxValue_(1.0f), mBarLayer_(LAYER_ARTWORK),
    pBarTexture_(nullptr)
{
    lInitialTextCoords_[0] = lInitialTextCoords_[1] = 0.0f;
    lInitialTextCoords_[2] = lInitialTextCoords_[3] = 1.0f;
    lType_.push_back(CLASS_NAME);
}

status_bar::~status_bar()
{
}

std::string status_bar::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << frame::serialize(sTab);
    sStr << sTab << "  # Orientation: ";
    switch (mOrientation_)
    {
        case ORIENT_HORIZONTAL : sStr << "HORIZONTAL"; break;
        case ORIENT_VERTICAL   : sStr << "VERTICAL";   break;
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

void status_bar::copy_from(uiobject* pObj)
{
    frame::copy_from(pObj);

    status_bar* pStatusBar = dynamic_cast<status_bar*>(pObj);

    if (pStatusBar)
    {
        this->set_min_value(pStatusBar->get_min_value());
        this->set_max_value(pStatusBar->get_max_value());
        this->set_value(pStatusBar->get_value());
        this->set_bar_draw_layer(pStatusBar->get_bar_draw_layer());
        this->set_orientation(pStatusBar->get_orientation());
        this->set_reversed(pStatusBar->is_reversed());

        texture* pBar = pStatusBar->get_bar_texture();
        if (pBar)
        {
            texture* pBarTexture = this->create_bar_texture_();

            if (this->is_virtual())
                pBarTexture->set_virtual();

            pBarTexture->set_name(pBar->get_name());
            if (!pManager_->add_uiobject(pBarTexture))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pBar->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pBarTexture->get_name()+"\". Skipped." << std::endl;
                delete pBarTexture; pBarTexture = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pBarTexture->create_glue();

                this->add_region(pBarTexture);
                pBarTexture->copy_from(pBar);
                pBarTexture->notify_loaded();
                this->set_bar_texture(pBarTexture);
            }
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
        fire_update_bar_texture_();
    }
}

void status_bar::set_max_value(float fMax)
{
    if (fMax != fMaxValue_)
    {
        fMaxValue_ = fMax;
        if (fMaxValue_ < fMinValue_) fMaxValue_ = fMinValue_;
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
        fire_update_bar_texture_();
    }
}

void status_bar::set_min_max_values(float fMin, float fMax)
{
    if (fMin != fMinValue_ || fMax != fMaxValue_)
    {
        fMinValue_ = std::min(fMin, fMax);
        fMaxValue_ = std::max(fMin, fMax);
        fValue_ = fValue_ > fMaxValue_ ? fMaxValue_ : (fValue_ < fMinValue_ ? fMinValue_ : fValue_);
        fire_update_bar_texture_();
    }
}

void status_bar::set_value(float fValue)
{
    fValue = fValue > fMaxValue_ ? fMaxValue_ : (fValue < fMinValue_ ? fMinValue_ : fValue);
    if (fValue != fValue_)
    {
        fValue_ = fValue;
        fire_update_bar_texture_();
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
        mBarLayer_ = LAYER_ARTWORK;
    else if (sBarLayer == "BACKGROUND")
        mBarLayer_ = LAYER_BACKGROUND;
    else if (sBarLayer == "BORDER")
        mBarLayer_ = LAYER_BORDER;
    else if (sBarLayer == "HIGHLIGHT")
        mBarLayer_ = LAYER_HIGHLIGHT;
    else if (sBarLayer == "OVERLAY")
        mBarLayer_ = LAYER_OVERLAY;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            "Uknown layer type : \""+sBarLayer+"\". Using \"ARTWORK\"." << std::endl;

        mBarLayer_ = LAYER_ARTWORK;
    }

    if (pBarTexture_)
        pBarTexture_->set_draw_layer(mBarLayer_);
}

void status_bar::set_bar_texture(texture* pBarTexture)
{
    pBarTexture_ = pBarTexture;
    if (pBarTexture_)
    {
        pBarTexture_->clear_all_points();

        if (bReversed_)
            pBarTexture_->set_point(anchor(pBarTexture_, ANCHOR_TOPRIGHT, "$parent", ANCHOR_TOPRIGHT));
        else
            pBarTexture_->set_point(anchor(pBarTexture_, ANCHOR_BOTTOMLEFT, "$parent", ANCHOR_BOTTOMLEFT));

        lInitialTextCoords_ = select_uvs(pBarTexture_->get_tex_coord());
        fire_update_bar_texture_();
    }
}

void status_bar::set_bar_color(const color& mBarColor)
{
    if (!pBarTexture_)
    {
        texture* pBarTexture = create_bar_texture_();
        pBarTexture->set_name("$parentBarTexture");
        if (!pManager_->add_uiobject(pBarTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to create bar texture for \""+sName_+"\",\n"
                "but the name was already taken : \""+pBarTexture->get_name()+"\". Skipped." << std::endl;
            delete pBarTexture; pBarTexture = nullptr;
            return;
        }

        if (!bVirtual_)
            pBarTexture->create_glue();

        add_region(pBarTexture);
        pBarTexture->notify_loaded();
        set_bar_texture(pBarTexture);
    }

    mBarColor_ = mBarColor;
    pBarTexture_->set_color(mBarColor_);
}

void status_bar::set_orientation(orientation mOrient)
{
    if (mOrient != mOrientation_)
    {
        mOrientation_ = mOrient;
        fire_update_bar_texture_();
    }
}

void status_bar::set_reversed(bool bReversed)
{
    if (bReversed == bReversed_)
        return;

    bReversed_ = bReversed;

    if (pBarTexture_)
    {
        if (bReversed_)
            pBarTexture_->set_point(anchor(pBarTexture_, ANCHOR_TOPRIGHT, "$parent", ANCHOR_TOPRIGHT));
        else
            pBarTexture_->set_point(anchor(pBarTexture_, ANCHOR_BOTTOMLEFT, "$parent", ANCHOR_BOTTOMLEFT));

        pBarTexture_->fire_update_borders();
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

texture* status_bar::get_bar_texture() const
{
    return pBarTexture_;
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

texture* status_bar::create_bar_texture_()
{
    texture* pBarTexture = new texture(pManager_);
    pBarTexture->set_special();
    pBarTexture->set_parent(this);
    pBarTexture->set_draw_layer(mBarLayer_);

    return pBarTexture;
}

void status_bar::create_glue()
{
    if (lGlue_) return;

    if (bVirtual_)
    {
        utils::wptr<lua::state> pLua = pManager_->get_lua();
        pLua->push_number(uiID_);
        lGlue_ = pLua->push_new<lua_virtual_glue>();
        pLua->set_global(sLuaName_);
        pLua->pop();
    }
    else
    {
        utils::wptr<lua::state> pLua = pManager_->get_lua();
        pLua->push_string(sName_);
        lGlue_ = pLua->push_new<lua_status_bar>();
        pLua->set_global(sLuaName_);
        pLua->pop();
    }
}

void status_bar::update(float fDelta)
{

    if (bUpdateBarTexture_ && pBarTexture_)
    {
        float fCoef = (fValue_ - fMinValue_)/(fMaxValue_ - fMinValue_);

        if (mOrientation_ == ORIENT_HORIZONTAL)
        {
            pBarTexture_->set_rel_width(fCoef);
            pBarTexture_->set_rel_height(1.0f);
        }
        else
        {
            pBarTexture_->set_rel_width(1.0f);
            pBarTexture_->set_rel_height(fCoef);
        }

        if (!pBarTexture_->get_texture().empty())
        {
            std::array<float,4> uvs = lInitialTextCoords_;
            if (mOrientation_ == ORIENT_HORIZONTAL)
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

            pBarTexture_->set_tex_coord(uvs);
        }

        bUpdateBarTexture_ = false;
    }

    frame::update(fDelta);
}

void status_bar::fire_update_bar_texture_()
{
    bUpdateBarTexture_ = true;
}
}
