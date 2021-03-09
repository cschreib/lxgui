#ifndef LXGUI_GUI_CHECKBUTTON_HPP
#define LXGUI_GUI_CHECKBUTTON_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_button.hpp"

namespace lxgui {
namespace gui
{
    /// A button with two additional states: checked and unchecked
    /** This widget works exactly like a classic Button, but is has two
    *   additional special textures for the check sign.
    */
    class check_button : public button
    {
    public :

        /// Constructor.
        explicit check_button(manager* pManager);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        std::string serialize(const std::string& sTab) const override;

        /// Copies an uiobject's parameters into this CheckButton (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(uiobject* pObj) override;

        /// Checks this button.
        virtual void check();

        /// UnChecks this button.
        virtual void uncheck();

        /// Disables this CheckButton.
        /** \note A disabled button doesn't receive any input.
        */
        void disable() override;

        /// Enables this CheckButton.
        void enable() override;

        /// Releases this CheckButton.
        /** \note This function only has a visual impact:
        *         the OnClick() handler is not called.
        */
        void release() override;

        /// Checks if this CheckButton is checked.
        /** \return 'true' if checked, 'false' otherwise
        */
        bool is_checked();

        /// Returns this button's checked texture.
        /** \return This button's checked texture
        */
        texture* get_checked_texture();

        /// Returns this button's disabled checked texture.
        /** \return This button's disabled checked texture
        */
        texture* get_disabled_checked_texture();

        /// Sets this button's checked texture.
        /** \param pTexture The new texture
        */
        void set_checked_texture(texture* pTexture);

        /// Sets this button's disabled checked texture.
        /** \param pTexture The new texture
        */
        void set_disabled_checked_texture(texture* pTexture);

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Parses data from an xml::block.
        /** \param pBlock The Checkbutton's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state& mLua);

        static constexpr const char* CLASS_NAME = "CheckButton";

    protected :

        std::unique_ptr<texture> create_checked_texture_();
        std::unique_ptr<texture> create_disabled_checked_texture_();

        bool bChecked_ = false;

        texture* pCheckedTexture_ = nullptr;
        texture* pDisabledCheckedTexture_ = nullptr;

    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_check_button : public lua_button
    {
    public :

        explicit lua_check_button(lua_State* pLua);
        check_button* get_object() { return static_cast<check_button*>(pObject_); }

        // Glues
        int _is_checked(lua_State*);
        int _get_checked_texture(lua_State*);
        int _get_disabled_checked_texture(lua_State*);
        int _set_checked(lua_State*);
        int _set_checked_texture(lua_State*);
        int _set_disabled_checked_texture(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::lunar_binding<lua_check_button> methods[];
    };

    /** \endcond
    */
}
}

#endif
