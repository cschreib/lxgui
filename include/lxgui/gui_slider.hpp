#ifndef GUI_SLIDER_HPP
#define GUI_SLIDER_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"

namespace gui
{
    class texture;

    /// A frame with a movable texture
    /** This widget contains a special texture, the slider thumb.
    *   It can be moved along a single axis (X or Y) and its position
    *   can be used to represent a value (for configuration menus, or
    *   scroll bars).
    */
    class slider : public frame
    {
    public :

        enum orientation
        {
            ORIENT_VERTICAL,
            ORIENT_HORIZONTAL
        };

        /// Constructor.
        explicit slider(manager* pManager);

        /// Destructor.
        virtual ~slider();

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        virtual std::string serialize(const std::string& sTab) const;

        /// Returns 'true' if this slider can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        virtual bool can_use_script(const std::string& sScriptName) const;

        /// Copies an uiobject's parameters into this slider (inheritance).
        /** \param pObj The uiobject to copy
        */
        virtual void copy_from(uiobject* pObj);

        /// Sets the texture to use for the thumb.
        /** \param pTexture The new texture
        */
        void set_thumb_texture(texture* pTexture);

        /// Returns the texture used for the thumb.
        /** \return The texture used for the thumb
        */
        texture* get_thumb_texture() const;

        /// Sets the orientation of this slider.
        /** \param mOrientation The orientation of this slider
        */
        void set_orientation(orientation mOrientation);

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
        bool  are_clicks_outside_thumb_allowed();

        /// Checks if the provided coordinates are in the frame.
        /** \param iX           The horizontal coordinate
        *   \param iY           The vertical coordinate
        *   \return 'true' if the provided coordinates are in the frame
        *   \note The slider version of this function also checks if the
        *         mouse is over the thumb texture.
        */
        virtual bool is_in_frame(int iX, int iY) const;

        /// Tells this frame it is being overed by the mouse.
        /** \param bMouseInFrame 'true' if the mouse is above this frame
        *   \param iX            The horizontal mouse coordinate
        *   \param iY            The vertical mouse coordinate
        */
        virtual void notify_mouse_in_frame(bool bMouseInFrame, int iX, int iY);

        /// Calls the on_event script.
        /** \param mEvent The Event that occured
        */
        virtual void on_event(const event& mEvent);

        /// Returns this widget's Lua glue.
        virtual void create_glue();

        /// Parses data from an xml::block.
        /** \param pBlock The slider's xml::block
        */
        virtual void parse_block(xml::block* pBlock);

        /// Tells this widget to update its borders.
        virtual void fire_update_borders() const;

        /// updates this widget's logic.
        virtual void update(float fDelta);

        /// Registers this widget to the provided lua::state
        static void register_glue(utils::wptr<lua::state> pLua);

        #ifndef NO_CPP11_CONSTEXPR
        static constexpr const char* CLASS_NAME = "Slider";
        #else
        static const char* CLASS_NAME;
        #endif

    protected :

        void constrain_thumb_();

        texture* create_thumb_texture_();
        void     fire_update_thumb_texture_() const;

        mutable bool bUpdateThumbTexture_;

        orientation mOrientation_;

        float fValue_;
        float fMinValue_;
        float fMaxValue_;
        float fValueStep_;

        bool bAllowClicksOutsideThumb_;

        layer_type mThumbLayer_;
        texture*   pThumbTexture_;
        bool       bThumbMoved_;
        bool       bMouseInThumb_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_slider : public lua_frame
    {
    public :

        explicit lua_slider(lua_State* pLua);

        // Glues
        int _allow_clicks_outside_thumb(lua_State*);
        int _get_max_value(lua_State*);
        int _get_min_value(lua_State*);
        int _get_min_max_values(lua_State*);
        int _get_orientation(lua_State*);
        int _get_thumb_texture(lua_State*);
        int _get_value(lua_State*);
        int _get_value_step(lua_State*);
        int _set_max_value(lua_State*);
        int _set_min_value(lua_State*);
        int _set_min_max_values(lua_State*);
        int _set_orientation(lua_State*);
        int _set_thumb_texture(lua_State*);
        int _set_value(lua_State*);
        int _set_value_step(lua_State*);

        static const char className[];
        static const char* classList[];
        static Lunar<lua_slider>::RegType methods[];

    protected :

        slider* pSliderParent_;
    };

    /** \endcond
    */
}

#endif
