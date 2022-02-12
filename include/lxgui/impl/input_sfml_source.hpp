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
    explicit source(sf::Window& p_window);

    source(const source&) = delete;
    source& operator=(const source&) = delete;

    utils::ustring get_clipboard_content() override;
    void           set_clipboard_content(const utils::ustring& s_content) override;

    void on_sfml_event(const sf::Event& m_event);

    void set_mouse_cursor(const std::string& s_file_name, const gui::vector2i& m_hot_spot) override;
    void reset_mouse_cursor() override;

private:
    input::key from_sfml_(int ui_sf_key) const;

    sf::Window& m_window_;

    gui::vector2i m_old_mouse_pos_;
    bool          b_first_mouse_move_ = true;

    std::unordered_map<std::string, std::unique_ptr<sf::Cursor>> l_cursor_map_;
};

}} // namespace lxgui::input::sfml

#endif
