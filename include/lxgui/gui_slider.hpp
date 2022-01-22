#ifndef LXGUI_GUI_SLIDER_HPP
#define LXGUI_GUI_SLIDER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"

namespace lxgui {
namespace gui
{
    class texture;

    /// A #frame with a movable texture.
    /** This frame contains a special texture, the "slider thumb".
    *   It can be moved along a single axis (X or Y) and its position
    *   can be used to represent a value (for configuration menus, or
    *   scroll bars).
    *
    *   __Events.__ Hard-coded events available to all sliders,
    *   in addition to those from #frame:
    *
    *   - `OnValueChanged`: Triggered whenever the value controlled by
    *   the slider changes. This is triggered whenever the user moves
    *   the slider thumb, and by slider::set_value. This can also be
    *   triggered by slider::set_min_value, slider::set_max_value,
    *   slider::set_min_max_values, and slider::set_value_step if the
    *   previous value would not satisfy the new constraints.
    */
    class slider : public frame
    {
        using base = frame;

    public :

        enum class orientation
        {
            VERTICAL,
            HORIZONTAL
        };

        /// Constructor.
        explicit slider(utils::control_block& mBlock, manager& mManager);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        std::string serialize(const std::string& sTab) const override;

        /// Returns 'true' if this slider can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        bool can_use_script(const std::string& sScriptName) const override;

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param mData       Stores scripts arguments
        *   \note Triggered callbacks could destroy the frame. If you need
        *         to use the frame again after calling this function, use
        *         the helper class alive_checker.
        */
        void trigger(const std::string& sScriptName, const event_data& mData = event_data{}) override;

        /// Copies an uiobject's parameters into this slider (inheritance).
        /** \param mObj The uiobject to copy
        */
        void copy_from(const uiobject& mObj) override;

        /// Sets the texture to use for the thumb.
        /** \param pTexture The new texture
        */
        void set_thumb_texture(utils::observer_ptr<texture> pTexture);

        /// Returns the texture used for the thumb.
        /** \return The texture used for the thumb
        */
        const utils::observer_ptr<texture>& get_thumb_texture() { return pThumbTexture_; }

        /// Returns the texture used for the thumb.
        /** \return The texture used for the thumb
        */
        utils::observer_ptr<const texture> get_thumb_texture() const { return pThumbTexture_; }

        /// Sets the orientation of this slider.
        /** \param mOrientation The orientation of this slider
        */
        void set_orientation(orientation mOrientation);

        /// Sets the orientation of this slider.
        /** \param sOrientation The orientation of this slider ("VERTICAL" or "HORIZONTAL")
        */
        void set_orientation(const std::string& sOrientation);

        /// Returns the orientation of this slider.
        /** \return The orientation of this slider
        */
        orientation get_orientation() const;

        /// Sets the slider's value range.
        /** \param fMin The minimum value
        *   \param fMax The maximum value
        */
        void set_min_max_values(float fMin, float fMax);

        /// Sets this slider's minimum value.
        /** \param fMin The minimum value
        */
        void set_min_value(float fMin);

        /// Sets this slider's maximum value.
        /** \param fMax The maximum value
        */
        void set_max_value(float fMax);

        /// Returns this slider's minimum value.
        /** \return This slider's minimum value
        */
        float get_min_value() const;

        /// Returns this slider's maximum value.
        /** \return This slider's maximum value
        */
        float get_max_value() const;

        /// Sets this slider's value.
        /** \param fValue  The value
        *   \param bSilent 'true' to prevent OnValueChanged to be fired
        */
        void set_value(float fValue, bool bSilent = false);

        /// Returns this slider's value.
        /** \return This slider's value
        */
        float get_value() const;

        /// Sets this slider's value step.
        /** \param fValueStep The new step
        */
        void set_value_step(float fValueStep);

        /// Returns this slider's value step.
        /** \return This slider's value step
        */
        float get_value_step() const;

        /// Sets the draw layer of this slider's thumb texture.
        /** \param mThumbLayer The layer
        */
        void set_thumb_draw_layer(layer_type mThumbLayer);

        /// Sets the draw layer of this slider's thumb texture.
        /** \param sBarLayer The layer
        */
        void set_thumb_draw_layer(const std::string& sBarLayer);

        /// Returns the draw layer of this slider's thumb texture.
        /** \return The draw layer of this slider's thumb texture
        */
        layer_type get_thumb_draw_layer() const;

        /// Allows the user to click anywhere in the slider to relocate the thumb.
        /** \param bAllow 'true' to allow it, 'false' to allow clicks on the thumb only
        */
        void set_allow_clicks_outside_thumb(bool bAllow);

        /// Checks if clicks are allowed outside of the thumb.
        /** \return 'true' if it is the case
        *   \note See set_allow_clicks_outside_thumb().
        */
        bool are_clicks_outside_thumb_allowed() const;

        /// Checks if the provided coordinates are in the slider.
        /** \param mPosition The coordinate to test
        *   \return 'true' if the provided coordinates are in the slider, its title region,
        *           or its thumb texture
        */
        bool is_in_region(const vector2f& mPosition) const override;

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Tells this widget that its borders need updating.
        void notify_borders_need_update() override;

        /// Registers this widget class to the provided Lua state
        static void register_on_lua(sol::state& mLua);

        static constexpr const char* CLASS_NAME = "Slider";

    protected :

        void constrain_thumb_();
        void update_thumb_texture_();

        void notify_thumb_texture_needs_update_();

        void parse_attributes_(const layout_node& mNode) override;
        void parse_all_nodes_before_children_(const layout_node& mNode) override;

        orientation mOrientation_ = orientation::VERTICAL;

        float fValue_ = 0.0f;
        float fMinValue_ = 0.0f;
        float fMaxValue_ = 1.0f;
        float fValueStep_ = 0.1f;

        bool bAllowClicksOutsideThumb_ = true;

        layer_type mThumbLayer_ = layer_type::OVERLAY;
        utils::observer_ptr<texture> pThumbTexture_ = nullptr;

        bool bThumbMoved_ = false;
    };
}
}

#endif
