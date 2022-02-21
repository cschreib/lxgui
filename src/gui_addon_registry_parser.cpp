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

#include <lxgui/extern_sol2_state.hpp>
#if defined(LXGUI_ENABLE_XML_PARSER)
#    include <lxgui/extern_pugixml.hpp>
#endif
#if defined(LXGUI_ENABLE_YAML_PARSER)
#    include <lxgui/extern_ryml.hpp>
#endif

#include <fstream>

namespace lxgui::gui {

class file_line_mappings {
public:
    explicit file_line_mappings(const std::string& file_name) : file_name_(file_name) {
        std::ifstream stream(file_name);
        if (!stream.is_open())
            return;

        std::string line;
        std::size_t prev_pos = 0u;
        while (std::getline(stream, line)) {
            file_content_ += '\n' + line;
            line_offsets_.push_back(prev_pos);
            prev_pos += line.size() + 1u;
        }

        line_offsets_.push_back(prev_pos);

        is_open_ = true;
    }

    bool is_open() const {
        return is_open_;
    }

    const std::string& get_content() const {
        return file_content_;
    }

    std::pair<std::size_t, std::size_t> get_line_info(std::size_t offset) const {
        auto iter = std::lower_bound(line_offsets_.begin(), line_offsets_.end(), offset);
        if (iter == line_offsets_.end())
            return std::make_pair(0, 0);

        std::size_t line_nbr    = iter - line_offsets_.begin();
        std::size_t char_offset = offset - *iter + 1u;

        return std::make_pair(line_nbr, char_offset);
    }

    std::string get_location(std::size_t offset) const {
        auto location = get_line_info(offset);
        if (location.first == 0)
            return file_name_ + ":?";
        else
            return file_name_ + ":" + utils::to_string(location.first);
    }

private:
    bool                     is_open_ = false;
    std::string              file_name_;
    std::string              file_content_;
    std::vector<std::size_t> line_offsets_;
};

std::string normalize_node_name(const std::string& name, bool capital_first) {
    std::string normalized;
    bool        next_capitalize = capital_first;
    for (auto c : name) {
        if (next_capitalize)
            c = std::toupper(c);

        next_capitalize = c == '_';
        if (next_capitalize)
            continue;

        normalized.push_back(c);
    }

    return normalized;
}

#if defined(LXGUI_ENABLE_XML_PARSER)
void set_node(const file_line_mappings& file, layout_node& node, const pugi::xml_node& xml_node) {
    auto location = file.get_location(xml_node.offset_debug());
    node.set_location(location);
    node.set_value_location(location);
    node.set_name(normalize_node_name(xml_node.name(), true));

    for (const auto& attr : xml_node.attributes()) {
        std::string name = normalize_node_name(attr.name(), false);
        if (const auto* node_attr = node.try_get_attribute(name)) {
            gui::out << gui::warning << location << ": attribute '" << name
                     << "' duplicated; only first value will be used." << std::endl;
            node_attr->mark_as_not_accessed();
            continue;
        }

        auto& attrib = node.add_attribute();
        attrib.set_location(location);
        attrib.set_value_location(location);
        attrib.set_name(std::move(name));
        attrib.set_value(attr.value());
    }

    std::string value;
    for (const auto& elem_node : xml_node.children()) {
        if (elem_node.type() == pugi::node_pcdata || elem_node.type() == pugi::node_cdata) {
            value += elem_node.value();
        } else {
            auto& child = node.add_child();
            set_node(file, child, elem_node);
        }
    }

    node.set_value(value);
}
#endif

#if defined(LXGUI_ENABLE_YAML_PARSER)
std::string to_string(const c4::csubstr& c_string) {
    return std::string(c_string.data(), c_string.size());
}

void set_node(
    const file_line_mappings& file,
    const ryml::Tree&         tree,
    layout_node&              node,
    const ryml::NodeRef&      yaml_node) {
    std::string location;
    if (yaml_node.has_key())
        location = file.get_location(yaml_node.key().data() - tree.arena().data());
    else if (yaml_node.has_val())
        location = file.get_location(yaml_node.val().data() - tree.arena().data());
    node.set_location(location);
    node.set_value_location(location);

    if (yaml_node.has_key())
        node.set_name(normalize_node_name(to_string(yaml_node.key()), true));

    for (auto elem_node : yaml_node.children()) {
        switch (elem_node.type()) {
        case ryml::KEYVAL: {
            std::string name = normalize_node_name(to_string(elem_node.key()), false);
            std::string attr_location =
                file.get_location(elem_node.key().data() - tree.arena().data());
            if (const auto* node_attr = node.try_get_attribute(name)) {
                gui::out << gui::warning << attr_location << ": attribute '" << name
                         << "' duplicated; only first value will be used." << std::endl;
                gui::out << gui::warning << std::string(attr_location.size(), ' ')
                         << "   first occurence at: '" << std::endl;
                gui::out << gui::warning << std::string(attr_location.size(), ' ') << "   "
                         << node_attr->get_location() << std::endl;
                node_attr->mark_as_not_accessed();
                continue;
            }

            auto& attrib = node.add_attribute();
            attrib.set_location(std::move(attr_location));
            attrib.set_value_location(
                file.get_location(elem_node.val().data() - tree.arena().data()));
            attrib.set_name(std::move(name));
            attrib.set_value(to_string(elem_node.val()));
            break;
        }
        case ryml::KEYMAP: [[fallthrough]];
        case ryml::MAP: [[fallthrough]];
        case ryml::KEYSEQ: {
            auto& child = node.add_child();
            set_node(file, tree, child, elem_node);
            break;
        }
        default: {
            gui::out << gui::warning << location << ": unsupported YAML node type: '"
                     << elem_node.type_str() << "'." << std::endl;
            break;
        }
        }
    }
}
#endif

void addon_registry::parse_layout_file_(const std::string& file_name, const addon& add_on) {
    file_line_mappings file(file_name);
    if (!file.is_open()) {
        gui::out << gui::error << file_name << ": could not open file for parsing." << std::endl;
        return;
    }

    layout_node root;
    bool        parsed = false;

    const std::string extension = utils::get_file_extension(file_name);

#if defined(LXGUI_ENABLE_XML_PARSER)
    if (extension == ".xml") {
        const unsigned int options = pugi::parse_ws_pcdata_single;

        pugi::xml_document     doc;
        pugi::xml_parse_result result =
            doc.load_buffer(file.get_content().c_str(), file.get_content().size(), options);

        if (!result) {
            gui::out << gui::error << file.get_location(result.offset) << ": "
                     << result.description() << std::endl;
            return;
        }

        set_node(file, root, doc.first_child());
        parsed = true;
    }
#endif

#if defined(LXGUI_ENABLE_YAML_PARSER)
    if (extension == ".yml" || extension == ".yaml") {
        ryml::Tree tree = ryml::parse(ryml::to_csubstr(file.get_content()));
        set_node(file, tree, root, tree.rootref().first_child());
        parsed = true;
    }
#endif

    if (!parsed) {
        gui::out << gui::error << file_name
                 << ": no parser registered for extension '" + extension + "'." << std::endl;
        return;
    }

    for (const auto& node : root.get_children()) {
        if (node.get_name() == "Script") {
            std::string script_file =
                add_on.directory + "/" + node.get_attribute_value<std::string>("file");

            try {
                lua_.do_file(script_file);
            } catch (const sol::error& e) {
                std::string err = e.what();
                gui::out << gui::error << err << std::endl;
                event_emitter_.fire_event("LUA_ERROR", {err});
            }
        } else if (node.get_name() == "Include") {
            parse_layout_file_(
                add_on.directory + "/" + node.get_attribute_value<std::string>("file"), add_on);
        } else {
            try {
                auto attr = parse_core_attributes(
                    root_.get_registry(), virtual_root_.get_registry(), node, nullptr);

                utils::observer_ptr<frame> obj;
                auto parent = attr.parent; // copy here to prevent use-after-move
                if (parent) {
                    obj = parent->create_child(std::move(attr));
                } else {
                    if (attr.is_virtual)
                        obj = virtual_root_.create_root_frame(std::move(attr));
                    else
                        obj = root_.create_root_frame(std::move(attr));
                }

                if (!obj)
                    continue;

                obj->set_addon(&add_on);
                obj->parse_layout(node);
                obj->notify_loaded();
            } catch (const utils::exception& e) {
                gui::out << gui::error << e.get_description() << std::endl;
            }
        }
    }

    warn_for_not_accessed_node(root);
}

} // namespace lxgui::gui
