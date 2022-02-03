#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/gui_layoutnode.hpp"

#include "lxgui/utils_string.hpp"
#include "lxgui/utils_filesystem.hpp"

#include <sol/state.hpp>
#if defined(LXGUI_ENABLE_XML_PARSER)
#   include <pugixml.hpp>
#endif
#if defined(LXGUI_ENABLE_YAML_PARSER)
#   include <ryml.hpp>
#   include <ryml_std.hpp>
#endif

#include <fstream>

namespace lxgui {
namespace gui
{

class file_line_mappings
{
public:

    explicit file_line_mappings(const std::string& sFileName) : sFileName_(sFileName)
    {
        std::ifstream mStream(sFileName);
        if (!mStream.is_open())
            return;

        std::string sLine;
        std::size_t uiPrevPos = 0u;
        while (std::getline(mStream, sLine))
        {
            sFileContent_ += '\n' + sLine;
            lLineOffsets_.push_back(uiPrevPos);
            uiPrevPos += sLine.size() + 1u;
        }

        lLineOffsets_.push_back(uiPrevPos);

        bIsOpen_ = true;
    }

    bool is_open() const { return bIsOpen_; }

    const std::string& get_content() const { return sFileContent_; }

    std::pair<std::size_t,std::size_t> get_line_info(std::size_t uiOffset) const
    {
        auto mIter = std::lower_bound(lLineOffsets_.begin(), lLineOffsets_.end(), uiOffset);
        if (mIter == lLineOffsets_.end())
            return std::make_pair(0, 0);

        std::size_t uiLineNbr = mIter - lLineOffsets_.begin();
        std::size_t uiCharOffset = uiOffset - *mIter + 1u;

        return std::make_pair(uiLineNbr, uiCharOffset);
    }

    std::string get_location(std::size_t uiOffset) const
    {
        auto mLocation = get_line_info(uiOffset);
        if (mLocation.first == 0)
            return sFileName_ + ":?";
        else
            return sFileName_ + ":" + utils::to_string(mLocation.first);
    }

private:

    bool                     bIsOpen_ = false;
    std::string              sFileName_;
    std::string              sFileContent_;
    std::vector<std::size_t> lLineOffsets_;
};

std::string normalize_node_name(const std::string& sName, bool bCapitalFirst)
{
    std::string sNormalized;
    bool bNextCapitalize = bCapitalFirst;
    for (auto cChar : sName)
    {
        if (bNextCapitalize)
            cChar = std::toupper(cChar);

        bNextCapitalize = cChar == '_';
        if (bNextCapitalize)
            continue;

        sNormalized.push_back(cChar);
    }

    return sNormalized;
}

#if defined(LXGUI_ENABLE_XML_PARSER)
void set_node(const file_line_mappings& mFile,
    layout_node& mNode, const pugi::xml_node& mXMLNode)
{
    auto sLocation = mFile.get_location(mXMLNode.offset_debug());
    mNode.set_location(sLocation);
    mNode.set_value_location(sLocation);
    mNode.set_name(normalize_node_name(mXMLNode.name(), true));

    for (const auto& mAttr : mXMLNode.attributes())
    {
        std::string sName = normalize_node_name(mAttr.name(), false);
        if (const auto* pNode = mNode.try_get_attribute(sName))
        {
            gui::out << gui::warning << sLocation << " : attribute '" <<
                sName << "' duplicated; only first value will be used." << std::endl;
            pNode->mark_as_not_accessed();
            continue;
        }

        auto& mAttrib = mNode.add_attribute();
        mAttrib.set_location(sLocation);
        mAttrib.set_value_location(sLocation);
        mAttrib.set_name(std::move(sName));
        mAttrib.set_value(mAttr.value());
    }

    std::string sValue;
    for (const auto& mElemNode : mXMLNode.children())
    {
        if (mElemNode.type() == pugi::node_pcdata || mElemNode.type() == pugi::node_cdata)
        {
            sValue += mElemNode.value();
        }
        else
        {
            auto& mChild = mNode.add_child();
            set_node(mFile, mChild, mElemNode);
        }
    }

    mNode.set_value(sValue);
}
#endif

#if defined(LXGUI_ENABLE_YAML_PARSER)
std::string to_string(const c4::csubstr& mCString)
{
    return std::string(mCString.data(), mCString.size());
}

void set_node(const file_line_mappings& mFile, const ryml::Tree& mTree,
    layout_node& mNode, const ryml::NodeRef& mYAMLNode)
{
    std::string sLocation;
    if (mYAMLNode.has_key())
        sLocation = mFile.get_location(mYAMLNode.key().data() - mTree.arena().data());
    else if (mYAMLNode.has_val())
        sLocation = mFile.get_location(mYAMLNode.val().data() - mTree.arena().data());
    mNode.set_location(sLocation);
    mNode.set_value_location(sLocation);

    if (mYAMLNode.has_key())
        mNode.set_name(normalize_node_name(to_string(mYAMLNode.key()), true));

    for (const auto& mElemNode : mYAMLNode.children())
    {
        switch (mElemNode.type())
        {
            case ryml::KEYVAL:
            {
                std::string sName = normalize_node_name(to_string(mElemNode.key()), false);
                std::string sAttrLocation = mFile.get_location(mElemNode.key().data() - mTree.arena().data());
                if (const auto* pNode = mNode.try_get_attribute(sName))
                {
                    gui::out << gui::warning << sAttrLocation << " : attribute '" <<
                        sName << "' duplicated; only first value will be used." << std::endl;
                    gui::out << gui::warning << std::string(sAttrLocation.size(), ' ') <<
                        "   first occurence at: '" << std::endl;
                    gui::out << gui::warning << std::string(sAttrLocation.size(), ' ') <<
                        "   " << pNode->get_location() << std::endl;
                    pNode->mark_as_not_accessed();
                    continue;
                }

                auto& mAttrib = mNode.add_attribute();
                mAttrib.set_location(std::move(sAttrLocation));
                mAttrib.set_value_location(mFile.get_location(mElemNode.val().data() - mTree.arena().data()));
                mAttrib.set_name(std::move(sName));
                mAttrib.set_value(to_string(mElemNode.val()));
                break;
            }
            case ryml::KEYMAP: [[fallthrough]];
            case ryml::MAP: [[fallthrough]];
            case ryml::KEYSEQ:
            {
                auto& mChild = mNode.add_child();
                set_node(mFile, mTree, mChild, mElemNode);
                break;
            }
            default:
            {
                gui::out << gui::warning << sLocation << " : unsupported YAML node type: '" <<
                    mElemNode.type_str() << "'." << std::endl;
                break;
            }
        }
    }
}
#endif

void addon_registry::parse_layout_file_(const std::string& sFile, const addon& mAddOn)
{
    file_line_mappings mFile(sFile);
    if (!mFile.is_open())
    {
        gui::out << gui::error << sFile << ": could not open file for parsing." << std::endl;
        return;
    }

    layout_node mRoot;
    bool bParsed = false;

    const std::string sExtension = utils::get_file_extension(sFile);

#if defined(LXGUI_ENABLE_XML_PARSER)
    if (sExtension == ".xml")
    {
        const unsigned int uiOptions = pugi::parse_ws_pcdata_single;

        pugi::xml_document mDoc;
        pugi::xml_parse_result mResult = mDoc.load_buffer(
            mFile.get_content().c_str(), mFile.get_content().size(), uiOptions);

        if (!mResult)
        {
            gui::out << gui::error << mFile.get_location(mResult.offset) << ": "
                << mResult.description() << std::endl;
            return;
        }

        set_node(mFile, mRoot, mDoc.first_child());
        bParsed = true;
    }
#endif

#if defined(LXGUI_ENABLE_YAML_PARSER)
    if (sExtension == ".yml" || sExtension == ".yaml")
    {
        ryml::Tree mTree = ryml::parse(ryml::to_csubstr(mFile.get_content()));
        set_node(mFile, mTree, mRoot, mTree.rootref().first_child());
        bParsed = true;
    }
#endif

    if (!bParsed)
    {
        gui::out << gui::error << sFile << ": no parser registered for extension '" +
            sExtension + "'." << std::endl;
        return;
    }

    for (const auto& mNode : mRoot.get_children())
    {
        if (mNode.get_name() == "Script")
        {
            std::string sScriptFile = mAddOn.sDirectory+"/"+
                mNode.get_attribute_value<std::string>("file");

            try
            {
                mLua_.do_file(sScriptFile);
            }
            catch (const sol::error& e)
            {
                std::string sError = e.what();

                gui::out << gui::error << sError << std::endl;

                mEventEmitter_.fire_event("LUA_ERROR", {sError});
            }
        }
        else if (mNode.get_name() == "Include")
        {
            parse_layout_file_(mAddOn.sDirectory+"/"+
                mNode.get_attribute_value<std::string>("file"), mAddOn);
        }
        else
        {
            try
            {
                auto mAttr = parse_core_attributes(
                    mRoot_.get_registry(), mVirtualRoot_.get_registry(), mNode, nullptr);

                utils::observer_ptr<frame> pFrame;
                auto pParent = mAttr.pParent; // copy here to prevent use-after-move
                if (pParent)
                {
                    pFrame = pParent->create_child(std::move(mAttr));
                }
                else
                {
                    if (mAttr.bVirtual)
                        pFrame = mVirtualRoot_.create_root_frame(std::move(mAttr));
                    else
                        pFrame = mRoot_.create_root_frame(std::move(mAttr));
                }

                if (!pFrame)
                    continue;

                pFrame->set_addon(&mAddOn);
                pFrame->parse_layout(mNode);
                pFrame->notify_loaded();
            }
            catch (const utils::exception& e)
            {
                gui::out << gui::error << e.get_description() << std::endl;
            }
        }
    }

    warn_for_not_accessed_node(mRoot);
}

}
}
