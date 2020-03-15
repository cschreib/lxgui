#ifndef GUI_MATERIAL_HPP
#define GUI_MATERIAL_HPP

#include <lxgui/utils.hpp>

namespace gui
{
    enum filter
    {
        FILTER_NONE,
        FILTER_LINEAR
    };

    /// A class that holds rendering data
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::manager_impl.
    */
    class material
    {
    public :

        /// Constructor.
        material();

        /// Destructor.
        virtual ~material();

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
        *         the gui::manager_impl should create a bigger texture that has
        *         power of two dimensions (the "physical" dimensions).
        */
        virtual float get_real_width() const = 0;

        /// Returns the physical height of the underlying texture (if any).
        /** \return The physical height of the underlying texture (if any)
        *   \note Some old hardware don't support textures that have non
        *         power of two dimensions. If the user creates such a material,
        *         the gui::manager_impl should create a bigger texture that has
        *         power of two dimensions (the "physical" dimensions).
        */
        virtual float get_real_height() const = 0;
    };
}

#endif
