#ifndef GUI_STATUSBAR_HPP
#define GUI_STATUSBAR_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_color.hpp"

namespace lxgui {
namespace gui
{
    class texture;

    /// A variable length bar
    /** This is a very simple widget : you give it three
    *   values : a minimum and a maximum, and a value
    *   between those two. It then Renders a bar that will
    *   be full, empty, or anything in between depending on
    *   the value.<br>
    *   Can be used to display health or cast bars.
    */
    class status_bar : public frame
    {
    public :

        enum class orientation
        {
            HORIZONTAL,
            VERTICAL
        };

        /// Constructor.
        explicit status_bar(manager* pManager);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        std::string serialize(const std::string& sTab) const override;

        /// Returns 'true' if this status_bar can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        bool can_use_script(const std::string& sScriptName) const override;

        /// Copies an uiobject's parameters into this status_bar (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(uiobject* pObj) override;

        /// Sets this status_bar's minimum value.
        /** \param fMin The minimum value
        */
        void set_min_value(float fMin);

        /// Sets this status_bar's maximum value.
        /** \param fMax The maximum value
        */
        void set_max_value(float fMax);

        /// Sets this status_bar's value range.
        /** \param fMin The minimum value
        *   \param fMax The maximum value
        */
        void set_min_max_values(float fMin, float fMax);

        /// Sets this status_bar's value.
        /** \param fValue The value
        */
        void set_value(float fValue);

        /// Sets the draw layer of this status_bar's bar texture.
        /** \param mBarLayer The layer
        */
        void set_bar_draw_layer(layer_type mBarLayer);

        /// Sets the draw layer of this status_bar's bar texture.
        /** \param sBarLayer The layer
        */
        void set_bar_draw_layer(const std::string& sBarLayer);

        /// Sets this status_bar's bar texture.
        /** \param pBarTexture The bar texture
        */
        void set_bar_texture(texture* pBarTexture);

        /// Sets this status_bar's bar color.
        /** \param mBarColor The bar color
        */
        void set_bar_color(const color& mBarColor);

        /// Sets this status_bar's orientation.
        /** \param mOrient The orientation
        */
        void set_orientation(orientation mOrient);

        /// Reverses this status_bar.
        /** \param bReversed 'true' to reverse it
        *   \note By default, if the status bar is oriented horizontally
        *         (vertically), if will grow from left to right (bottom to top).
        *         You can use this function to reverse the growth, that is
        *         make it grow from right to left.
        */
        void set_reversed(bool bReversed);

        /// Returns this status_bar's minimum value.
        /** \return This status_bar's minimum value
        */
        float get_min_value() const;

        /// Returns this status_bar's maximum value.
        /** \return This status_bar's maximum value
        */
        float get_max_value() const;

        /// Returns this status_bar's value.
        /** \return This status_bar's value
        */
        float get_value() const;

        /// Returns the draw layer of status_bar's bar texture.
        /** \return The draw layer of status_bar's bar texture
        */
        layer_type get_bar_draw_layer() const;

        /// Returns this status_bar's bar texture.
        /** \return This status_bar's bar texture
        */
        texture* get_bar_texture() const;

        /// Returns this status_bar's bar color.
        /** \return This status_bar's bar color
        */
        const color& get_bar_color() const;

        /// Returns this status_bar's orientation.
        /** \return This status_bar's orientation
        */
        orientation get_orientation() const;

        /// Checks if this status_bar is reversed.
        /** \return 'true' if it is the case
        */
        bool is_reversed() const;

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Parses data from an xml::block.
        /** \param pBlock The status_bar's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        /// updates this widget's logic.
        void update(float fDelta) override;

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state* pLua);

        static constexpr const char* CLASS_NAME = "StatusBar";

    protected :

        texture* create_bar_texture_();
        void     fire_update_bar_texture_();

        bool bUpdateBarTexture_ = false;

        orientation mOrientation_ = orientation::HORIZONTAL;
        bool        bReversed_ = false;

        float fValue_ = 0.0f;
        float fMinValue_ = 0.0f;
        float fMaxValue_ = 1.0f;

        color      mBarColor_ = color::WHITE;
        layer_type mBarLayer_ = layer_type::ARTWORK;
        texture*   pBarTexture_ = nullptr;
        std::array<float,4> lInitialTextCoords_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_status_bar : public lua_frame
    {
    public :

        explicit lua_status_bar(lua_State* pLua);

        // Glues
        int _get_min_max_values(lua_State*);
        int _get_orientation(lua_State*);
        int _is_reversed(lua_State*);
        int _get_status_bar_color(lua_State*);
        int _get_status_bar_texture(lua_State*);
        int _get_value(lua_State*);
        int _set_min_max_values(lua_State*);
        int _set_orientation(lua_State*);
        int _set_reversed(lua_State*);
        int _set_status_bar_color(lua_State*);
        int _set_status_bar_texture(lua_State*);
        int _set_value(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::Lunar<lua_status_bar>::RegType methods[];

    protected :

        status_bar* pStatusBarParent_;
    };

    /** \endcond
    */
}
}

#endif
