#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/utils_filesystem.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>
#if defined(LXGUI_ENABLE_XML_PARSER)
#    include <pugixml.hpp>
#endif
#if defined(LXGUI_ENABLE_YAML_PARSER)
#    include <ryml.hpp>
#    include <ryml_std.hpp>
#endif

#include <fstream>

namespace lxgui::gui {

class file_line_mappings {
public:
    explicit file_line_mappings(const std::string& s_file_name) : s_file_name_(s_file_name) {
        std::ifstream m_stream(s_file_name);
        if (!m_stream.is_open())
            return;

        std::string s_line;
        std::size_t ui_prev_pos = 0u;
        while (std::getline(m_stream, s_line)) {
            s_file_content_ += '\n' + s_line;
            line_offsets_.push_back(ui_prev_pos);
            ui_prev_pos += s_line.size() + 1u;
        }

        line_offsets_.push_back(ui_prev_pos);

        b_is_open_ = true;
    }

    bool is_open() const {
        return b_is_open_;
    }

    const std::string& get_content() const {
        return s_file_content_;
    }

    std::pair<std::size_t, std::size_t> get_line_info(std::size_t ui_offset) const {
        auto m_iter = std::lower_bound(line_offsets_.begin(), line_offsets_.end(), ui_offset);
        if (m_iter == line_offsets_.end())
            return std::make_pair(0, 0);

        std::size_t ui_line_nbr    = m_iter - line_offsets_.begin();
        std::size_t ui_char_offset = ui_offset - *m_iter + 1u;

        return std::make_pair(ui_line_nbr, ui_char_offset);
    }

    std::string get_location(std::size_t ui_offset) const {
        auto m_location = get_line_info(ui_offset);
        if (m_location.first == 0)
            return s_file_name_ + ":?";
        else
            return s_file_name_ + ":" + utils::to_string(m_location.first);
    }

private:
    bool                     b_is_open_ = false;
    std::string              s_file_name_;
    std::string              s_file_content_;
    std::vector<std::size_t> line_offsets_;
};

std::string normalize_node_name(const std::string& s_name, bool b_capital_first) {
    std::string s_normalized;
    bool        b_next_capitalize = b_capital_first;
    for (auto c_char : s_name) {
        if (b_next_capitalize)
            c_char = std::toupper(c_char);

        b_next_capitalize = c_char == '_';
        if (b_next_capitalize)
            continue;

        s_normalized.push_back(c_char);
    }

    return s_normalized;
}

#if defined(LXGUI_ENABLE_XML_PARSER)
void set_node(
    const file_line_mappings& m_file, layout_node& m_node, const pugi::xml_node& m_xml_node) {
    auto s_location = m_file.get_location(m_xml_node.offset_debug());
    m_node.set_location(s_location);
    m_node.set_value_location(s_location);
    m_node.set_name(normalize_node_name(m_xml_node.name(), true));

    for (const auto& m_attr : m_xml_node.attributes()) {
        std::string s_name = normalize_node_name(m_attr.name(), false);
        if (const auto* p_node = m_node.try_get_attribute(s_name)) {
            gui::out << gui::warning << s_location << " : attribute '" << s_name
                     << "' duplicated; only first value will be used." << std::endl;
            p_node->mark_as_not_accessed();
            continue;
        }

        auto& m_attrib = m_node.add_attribute();
        m_attrib.set_location(s_location);
        m_attrib.set_value_location(s_location);
        m_attrib.set_name(std::move(s_name));
        m_attrib.set_value(m_attr.value());
    }

    std::string s_value;
    for (const auto& m_elem_node : m_xml_node.children()) {
        if (m_elem_node.type() == pugi::node_pcdata || m_elem_node.type() == pugi::node_cdata) {
            s_value += m_elem_node.value();
        } else {
            auto& m_child = m_node.add_child();
            set_node(m_file, m_child, m_elem_node);
        }
    }

    m_node.set_value(s_value);
}
#endif

#if defined(LXGUI_ENABLE_YAML_PARSER)
std::string to_string(const c4::csubstr& m_c_string) {
    return std::string(m_c_string.data(), m_c_string.size());
}

void set_node(
    const file_line_mappings& m_file,
    const ryml::Tree&         m_tree,
    layout_node&              m_node,
    const ryml::NodeRef&      m_yaml_node) {
    std::string s_location;
    if (m_yaml_node.has_key())
        s_location = m_file.get_location(m_yaml_node.key().data() - m_tree.arena().data());
    else if (m_yaml_node.has_val())
        s_location = m_file.get_location(m_yaml_node.val().data() - m_tree.arena().data());
    m_node.set_location(s_location);
    m_node.set_value_location(s_location);

    if (m_yaml_node.has_key())
        m_node.set_name(normalize_node_name(to_string(m_yaml_node.key()), true));

    for (const auto& m_elem_node : m_yaml_node.children()) {
        switch (m_elem_node.type()) {
        case ryml::KEYVAL: {
            std::string s_name = normalize_node_name(to_string(m_elem_node.key()), false);
            std::string s_attr_location =
                m_file.get_location(m_elem_node.key().data() - m_tree.arena().data());
            if (const auto* p_node = m_node.try_get_attribute(s_name)) {
                gui::out << gui::warning << s_attr_location << " : attribute '" << s_name
                         << "' duplicated; only first value will be used." << std::endl;
                gui::out << gui::warning << std::string(s_attr_location.size(), ' ')
                         << "   first occurence at: '" << std::endl;
                gui::out << gui::warning << std::string(s_attr_location.size(), ' ') << "   "
                         << p_node->get_location() << std::endl;
                p_node->mark_as_not_accessed();
                continue;
            }

            auto& m_attrib = m_node.add_attribute();
            m_attrib.set_location(std::move(s_attr_location));
            m_attrib.set_value_location(
                m_file.get_location(m_elem_node.val().data() - m_tree.arena().data()));
            m_attrib.set_name(std::move(s_name));
            m_attrib.set_value(to_string(m_elem_node.val()));
            break;
        }
        case ryml::KEYMAP: [[fallthrough]];
        case ryml::MAP: [[fallthrough]];
        case ryml::KEYSEQ: {
            auto& m_child = m_node.add_child();
            set_node(m_file, m_tree, m_child, m_elem_node);
            break;
        }
        default: {
            gui::out << gui::warning << s_location << " : unsupported YAML node type: '"
                     << m_elem_node.type_str() << "'." << std::endl;
            break;
        }
        }
    }
}
#endif

void addon_registry::parse_layout_file_(const std::string& s_file, const addon& m_add_on) {
    file_line_mappings m_file(s_file);
    if (!m_file.is_open()) {
        gui::out << gui::error << s_file << ": could not open file for parsing." << std::endl;
        return;
    }

    layout_node m_root;
    bool        b_parsed = false;

    const std::string s_extension = utils::get_file_extension(s_file);

#if defined(LXGUI_ENABLE_XML_PARSER)
    if (s_extension == ".xml") {
        const unsigned int ui_options = pugi::parse_ws_pcdata_single;

        pugi::xml_document     m_doc;
        pugi::xml_parse_result m_result = m_doc.load_buffer(
            m_file.get_content().c_str(), m_file.get_content().size(), ui_options);

        if (!m_result) {
            gui::out << gui::error << m_file.get_location(m_result.offset) << ": "
                     << m_result.description() << std::endl;
            return;
        }

        set_node(m_file, m_root, m_doc.first_child());
        b_parsed = true;
    }
#endif

#if defined(LXGUI_ENABLE_YAML_PARSER)
    if (s_extension == ".yml" || s_extension == ".yaml") {
        ryml::Tree m_tree = ryml::parse(ryml::to_csubstr(m_file.get_content()));
        set_node(m_file, m_tree, m_root, m_tree.rootref().first_child());
        b_parsed = true;
    }
#endif

    if (!b_parsed) {
        gui::out << gui::error << s_file
                 << ": no parser registered for extension '" + s_extension + "'." << std::endl;
        return;
    }

    for (const auto& m_node : m_root.get_children()) {
        if (m_node.get_name() == "Script") {
            std::string s_script_file =
                m_add_on.s_directory + "/" + m_node.get_attribute_value<std::string>("file");

            try {
                m_lua_.do_file(s_script_file);
            } catch (const sol::error& e) {
                std::string s_error = e.what();

                gui::out << gui::error << s_error << std::endl;

                m_event_emitter_.fire_event("LUA_ERROR", {s_error});
            }
        } else if (m_node.get_name() == "Include") {
            parse_layout_file_(
                m_add_on.s_directory + "/" + m_node.get_attribute_value<std::string>("file"),
                m_add_on);
        } else {
            try {
                auto m_attr = parse_core_attributes(
                    m_root_.get_registry(), m_virtual_root_.get_registry(), m_node, nullptr);

                utils::observer_ptr<frame> p_frame;
                auto p_parent = m_attr.p_parent; // copy here to prevent use-after-move
                if (p_parent) {
                    p_frame = p_parent->create_child(std::move(m_attr));
                } else {
                    if (m_attr.b_virtual)
                        p_frame = m_virtual_root_.create_root_frame(std::move(m_attr));
                    else
                        p_frame = m_root_.create_root_frame(std::move(m_attr));
                }

                if (!p_frame)
                    continue;

                p_frame->set_addon(&m_add_on);
                p_frame->parse_layout(m_node);
                p_frame->notify_loaded();
            } catch (const utils::exception& e) {
                gui::out << gui::error << e.get_description() << std::endl;
            }
        }
    }

    warn_for_not_accessed_node(m_root);
}

} // namespace lxgui::gui
