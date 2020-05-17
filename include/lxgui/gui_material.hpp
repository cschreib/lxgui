#ifndef GUI_MATERIAL_HPP
#define GUI_MATERIAL_HPP

#include <lxgui/utils.hpp>

namespace lxgui {
namespace gui
{
    /// A class that holds rendering data
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer_impl.
    */
    class material
    {
    public :

        enum class wrap
        {
            REPEAT,
            CLAMP
        };

        enum class filter
        {
            NONE,
            LINEAR
        };

        enum class type
        {
            TEXTURE,
            COLOR
        };

        /// Constructor.
        material() = default;

        /// Destructor.
        virtual ~material() = default;

        /// Returns the width of the underlying texture (if any).
        /** \return The width of the underlying texture (if any)
        */
        virtual float get_width() const = 0;

        /// Returns the height of the underlying texture (if any).
        /** \return The height of the underlying texture (if any)
        */
        virtual float get_height() const = 0;

        /// Returns the physical width of the underlying texture (if any).
        /** \return The physical width of the underlying texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material,
        *         the gui::renderer_impl should create a bigger texture that has
        *         power of two dimensions (the "physical" dimensions).
        */
        virtual float get_real_width() const = 0;

        /// Returns the physical height of the underlying texture (if any).
        /** \return The physical height of the underlying texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material,
        *         the gui::renderer_impl should create a bigger texture that has
        *         power of two dimensions (the "physical" dimensions).
        */
        virtual float get_real_height() const = 0;
    };
}
}

#endif
