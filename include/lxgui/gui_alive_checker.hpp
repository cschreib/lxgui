#ifndef LXGUI_GUI_ALIVE_CHECKER_HPP
#define LXGUI_GUI_ALIVE_CHECKER_HPP

#include <lxgui/utils.hpp>

namespace lxgui
{
namespace gui
{
    class manager;
    class uiobject;

    /// Utility class for safe checking of widget validity
    class alive_checker
    {
    public :

        /// Contructor.
        explicit alive_checker(uiobject* pObject);

        /// Check if the wrapper widget is still alive
        /** \return 'true' if the widget is alive, 'false' otherwise
        */
        bool is_alive() const;

    private :

        uint uiID_ = 0;
        uiobject* pObject_ = nullptr;
        manager* pManager_ = nullptr;
    };
}
}

#endif
