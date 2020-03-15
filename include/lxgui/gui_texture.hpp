#ifndef GUI_TEXTURE_HPP
#define GUI_TEXTURE_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_gradient.hpp"
#include "lxgui/gui_sprite.hpp"

namespace gui
{
    /// The base of the GUI's appearence
    /** This object contains either a texture taken from a file,
    *   or a plain color.
    */
    class texture : public layered_region
    {
    public :

        enum blend_mode
        {
            BLEND_NONE,
            BLEND_BLEND,
            BLEND_KEY,
            BLEND_ADD,
            BLEND_MOD
        };

        /// Constructor.
        explicit texture(manager* pManager);

        /// Destructor.
        virtual ~texture();

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        virtual std::string serialize(const std::string& sTab) const;

        /// Renders this widget on the current render target.
        virtual void render();

        /// Copies an uiobject's parameters into this texture (inheritance).
        /** \param pObj The uiobject to copy
        */
        virtual void copy_from(uiobject* pObj);

        /// Returns this texture's blending mode.
        /** \return This texture's blending mode
        */
        blend_mode get_blend_mode() const;

        /// Returns this texture's filtering algorithm.
        /** \return This texture's filtering algorithm
        */
        filter get_filter_mode() const;

        /// Returns this texture's color.
        /** \return This texture's color (color::EMPTY if none)
        */
        const color& get_color() const;

        /// Returns this texture's gradient.
        /** \return This texture's gradient (Gradient::NONE if none)
        */
        const gradient& get_gradient() const;

        /// Returns this texture's texture coordinates.
        /** \return This texture's texture coordinates
        *   \note The texture coordinates are arranged as a rectangle, which is made
        *         of four points : 1 (top left), 2 (top right), 3 (bottom right) and
        *         4 (bottom left).<br>
        *         The returned array is composed like this :
        *         (x1, y1, x2, y2, x3, y3, x4, y4).
        */
        const std::array<float,8>& get_tex_coord() const;

        /// Checks if this texture's dimensions are affected by texture coordinates.
        /** \return 'true' if this texture's dimensions are affected by texture
        *           coordinates
        */
        bool get_tex_coord_modifies_rect() const;

        /// Returns this textures's texture file.
        /** \return This textures's texture file (empty string if none).
        */
        const std::string& get_texture() const;

        /// Returns this textures's vertex color.
        /** \return This textures's vertex color
        *   \note This color is used to filter the texture's colors :
        *         for each pixel, the original color is multiplied
        *         by this vertex color.
        */
        color get_vertex_color() const;

        /// Checks if this texture is desaturated.
        /** \return 'true' if the texture is desaturated
        *   \note Only available on certain graphic cards (most of modern ones
        *         are capable of this).
        */
        bool is_desaturated() const;

        /// Sets this texture's blending mode.
        /** \param mBlendMode The new blending mode
        */
        void set_blend_mode(blend_mode mBlendMode);

        /// Sets this texture's blending mode.
        /** \param sBlendMode The new blending mode
        */
        void set_blend_mode(const std::string& sBlendMode);

        /// Sets this texture's filtering mode.
        /** \param mFilter The new filtering mode
        */
        void set_filter_mode(filter mFilter);

        /// Sets this texture's blending mode.
        /** \param sFilter The new filtering mode
        */
        void set_filter_mode(const std::string& sFilter);

        /// Makes this texture appear without any color.
        /** \param bIsDesaturated 'true' if you want to remove colors
        */
        void set_desaturated(bool bIsDesaturated);

        /// Adds a gradient effect to this texture.
        /** \param mGradient The gradient to add
        *   \note To remove a gradient, call set_gradient(Gradient::NONE).
        */
        void set_gradient(const gradient& mGradient);

        /// Sets this texture's texture coordinates.
        /** \param lCoordinates This texture's texture coordinates
        *   \note The texture coordinates are arranged as a rectangle, which is made
        *         of four points : 1 (top left), 2 (top right), 3 (bottom right) and
        *         4 (bottom left).<br>
        *         The array must be arranged like this : (x1, y1, x3, y3), or (left,
        *         top, right, bottom). Other corners are calculated using these coordinates.
        *   \note This function only allows horizontal/rectangle texture coordinates.
        */
        void set_tex_coord(const std::array<float,4>& lCoordinates);

        /// Sets this texture's texture coordinates.
        /** \param lCoordinates This texture's texture coordinates
        *   \note The texture coordinates are arranged as a rectangle, which is made
        *         of four points : 1 (top left), 2 (top right), 3 (bottom right) and
        *         4 (bottom left).<br>
        *         The array must be arranged like this :
        *         (x1, y1, x2, y2, x3, y3, x4, y4).
        *   \note This function allows rotated/deformed texture coordinates.
        */
        void set_tex_coord(const std::array<float,8>& lCoordinates);

        /// Sets whether this texture's dimensions are affected by texture coordinates.
        /** \param bTexCoordModifiesRect 'true' to make dimensions change with tex coords
        */
        void set_tex_coord_modifies_rect(bool bTexCoordModifiesRect);

        /// Sets this texture's texture file.
        /** \param sFile The file from which to read data
        *   \note This function takes care of checking that the file can be opened.
        *   \note This function is not compatible with set_color() : only the latest
        *         you have called will apply.
        */
        void set_texture(const std::string& sFile);

        /// Reads texture data from a render_target.
        /** \param pRenderTarget The render_target from which to read the data
        *   \note This function is only meant for internal use and is not available
        *         to the Lua API.
        *   \note This function is not compatible with set_color() : only the latest
        *         you have called will apply.
        */
        void set_texture(utils::refptr<render_target> pRenderTarget);

        /// Sets this texture's color.
        /** \param mColor The color to use
        *   \note This function is not compatible with set_texture() : only the latest
        *         you have called will apply.
        */
        void set_color(const color& mColor);

        /// Directly sets this texture's underlying sprite.
        /** \param pSprite The new sprite to use
        *   \note The texture's dimensions will be adjusted to fit those
        *         of the provided sprite, and same goes for texture
        *         coordinates.
        *   \note Be sure to know what you're doing when you call this
        *         function.
        */
        void set_sprite(std::unique_ptr<sprite> pSprite);

        /// Sets this texture's vertex color.
        /** \param mColor This textures's new vertex color
        *   \note This color is used to filter the texture's colors :
        *         for each pixel, the original color is multiplied
        *         by this vertex color.
        */
        void set_vertex_color(const color& mColor);

        /// Creates the associated Lua glue.
        virtual void create_glue();

        /// Parses data from an xml::block.
        /** \param pBlock The texture's xml::block
        */
        virtual void parse_block(xml::block* pBlock);

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state* pLua);

        #ifndef NO_CPP11_CONSTEXPR
        static constexpr const char* CLASS_NAME = "Texture";
        #else
        static const char* CLASS_NAME;
        #endif

    private :

        virtual void parse_attributes_(xml::block* pBlock);
        void parse_tex_coords_block_(xml::block* pBlock);
        void parse_gradient_block_(xml::block* pBlock);

        std::unique_ptr<sprite> pSprite_;
        std::string             sTextureFile_;

        blend_mode mBlendMode_;
        filter     mFilter_;
        bool       bIsDesaturated_;
        gradient   mGradient_;
        color      mColor_;

        std::array<float,8> lTexCoord_;
        bool                bTexCoordModifiesRect_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_texture : public lua_layered_region
    {
    public :

        explicit lua_texture(lua_State* pLua);

        // texture
        int _set_vertex_color(lua_State*);
        int _get_blend_mode(lua_State*);
        int _get_filter_mode(lua_State*);
        int _get_tex_coord(lua_State*);
        int _get_tex_coord_modifies_rect(lua_State*);
        int _get_texture(lua_State*);
        int _get_vertex_color(lua_State*);
        int _is_desaturated(lua_State*);
        int _set_blend_mode(lua_State*);
        int _set_filter_mode(lua_State*);
        int _set_desaturated(lua_State*);
        int _set_gradient(lua_State*);
        int _set_gradient_alpha(lua_State*);
        int _set_tex_coord(lua_State*);
        int _set_tex_coord_modifies_rect(lua_State*);
        int _set_texture(lua_State*);

        static const char className[];
        static const char* classList[];
        static Lunar<lua_texture>::RegType methods[];

    protected :

        texture* pTextureParent_;
    };

    /** \endcond
    */
}

#endif
