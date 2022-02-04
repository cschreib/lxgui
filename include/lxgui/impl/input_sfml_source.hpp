#ifndef LXGUI_INPUT_SFML_SOURCE_HPP
#define LXGUI_INPUT_SFML_SOURCE_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/utils.hpp"

#include <SFML/Window/Cursor.hpp>
#include <memory>
#include <unordered_map>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace sf {

class Window;
class Event;

} // namespace sf
/** \endcond
 */

namespace lxgui::input { namespace sfml {

class source final : public input::source {
public:
    /// Initializes this input source.
    /** \param pWindow The window from which to receive input
     */
    explicit source(sf::Window& pWindow);

    source(const source&) = delete;
    source& operator=(const source&) = delete;

    utils::ustring get_clipboard_content() override;
    void           set_clipboard_content(const utils::ustring& sContent) override;

    void on_sfml_event(const sf::Event& mEvent);

    void set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot) override;
    void reset_mouse_cursor() override;

private:
    input::key from_sfml_(int uiSFKey) const;

    sf::Window& mWindow_;

    gui::vector2i mOldMousePos_;
    bool          bFirstMouseMove_ = true;

    std::unordered_map<std::string, std::unique_ptr<sf::Cursor>> lCursorMap_;
};

}} // namespace lxgui::input::sfml

#endif
