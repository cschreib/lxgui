#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_parser_common.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_layout_node.hpp>
#include <lxgui/utils_filesystem.hpp>

#include <sol/state.hpp>
#include <pugixml.hpp>
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
        uint uiPrevPos = 0u;
        while (std::getline(mStream, sLine))
        {
            sFileContent_ += '\n' + sLine;
            lLineOffsets_.push_back(uiPrevPos);
            uiPrevPos += sLine.size() + 1u;
        }

        bIsOpen_ = true;
    }

    bool is_open() const { return bIsOpen_; }

    const std::string& get_content() const { return sFileContent_; }

    std::pair<uint,uint> get_line_info(uint uiOffset) const
    {
        auto mIter = std::lower_bound(lLineOffsets_.begin(), lLineOffsets_.end(), uiOffset);
        if (mIter == lLineOffsets_.end())
            return std::make_pair(0, 0);

        uint uiLineNbr = mIter - lLineOffsets_.begin();
        uint uiCharOffset = uiOffset - *mIter + 1u;

        return std::make_pair(uiLineNbr, uiCharOffset);
    }

    std::string get_location(uint uiOffset) const
    {
        auto mLocation = get_line_info(uiOffset);
        if (mLocation.first == 0)
            return sFileName_ + ":?";
        else
            return sFileName_ + ":" + utils::to_string(mLocation.first);
    }

private:

    bool              bIsOpen_ = false;
    std::string       sFileName_;
    std::string       sFileContent_;
    std::vector<uint> lLineOffsets_;
};

void set_block(const file_line_mappings& mFile, utils::layout_node& mNode, const pugi::xml_node& mXMLNode)
{
    auto sLocation = mFile.get_location(mXMLNode.offset_debug());
    mNode.set_location(sLocation);
    mNode.set_name(mXMLNode.name());

    for (const auto& mAttr : mXMLNode.attributes())
    {
        auto& mAttrib = mNode.add_attribute();
        mAttrib.set_location(sLocation);
        mAttrib.set_name(mAttr.name());
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
            set_block(mFile, mChild, mElemNode);
        }
    }

    mNode.set_value(sValue);
}

void manager::parse_layout_file_(const std::string& sFile, addon* pAddOn)
{
    file_line_mappings mFile(sFile);
    if (!mFile.is_open())
    {
        gui::out << gui::error << sFile << ": could not open file for parsing." << std::endl;
        return;
    }

    utils::layout_node mRoot;
    bool bParsed = false;

    const std::string sExtension = utils::get_file_extension(sFile);

#if defined(LXGUI_ENABLE_XML_PARSER)
    if (sExtension == ".xml")
    {
        const uint uiOptions = pugi::parse_ws_pcdata_single;

        pugi::xml_document mDoc;
        pugi::xml_parse_result mResult = mDoc.load_buffer(
            mFile.get_content().c_str(), mFile.get_content().size(), uiOptions);

        if (!mResult)
        {
            gui::out << gui::error << mFile.get_location(mResult.offset) << ": " << mResult.description() << std::endl;
            return;
        }

        set_block(mFile, mRoot, mDoc.child("Ui"));
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
            std::string sScriptFile = pAddOn->sDirectory + "/" + mNode.get_attribute_value<std::string>("file");

            try
            {
                pLua_->do_file(sScriptFile);
            }
            catch (const sol::error& e)
            {
                std::string sError = e.what();

                gui::out << gui::error << sError << std::endl;

                event mEvent("LUA_ERROR");
                mEvent.add(sError);
                fire_event(mEvent);
            }
        }
        else if (mNode.get_name() == "Include")
        {
            this->parse_layout_file_(pAddOn->sDirectory + "/" + mNode.get_attribute_value<std::string>("file"), pAddOn);
        }
        else
        {
            try
            {
                auto mAttr = parse_core_attributes(*this, mNode, nullptr);

                utils::observer_ptr<frame> pFrame;
                if (mAttr.pParent)
                {
                    pFrame = mAttr.pParent->create_child(mAttr.sObjectType, mAttr.sName, mAttr.lInheritance);
                }
                else
                {
                    if (mAttr.bVirtual)
                        pFrame = create_virtual_root_frame(mAttr.sObjectType, mAttr.sName, mAttr.lInheritance);
                    else
                        pFrame = create_root_frame(mAttr.sObjectType, mAttr.sName, mAttr.lInheritance);
                }

                if (!pFrame)
                    continue;

                pFrame->set_addon(get_current_addon());
                pFrame->parse_layout(mNode);
                pFrame->notify_loaded();
            }
            catch (const utils::exception& e)
            {
                gui::out << gui::error << e.get_description() << std::endl;
            }
        }
    }
}

}
}
