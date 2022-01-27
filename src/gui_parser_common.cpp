#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/gui_frame.hpp"

namespace lxgui {
namespace gui
{

region_core_attributes parse_core_attributes(
    registry& mRegistry, virtual_registry& mVirtualRegistry,
    const layout_node& mNode, utils::observer_ptr<frame> pParent)
{
    region_core_attributes mAttr;
    mAttr.sObjectType = mNode.get_name();
    mAttr.sName = mNode.get_attribute_value<std::string>("name");

    if (pParent)
    {
        mAttr.pParent = std::move(pParent);

        if (mNode.has_attribute("virtual"))
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                << "Cannot use the \"virtual\" attribute on \"" << mAttr.sName << "\", "
                "because it is a nested region. Attribute ignored." << std::endl;
        }
        if (mNode.has_attribute("parent"))
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                << "Cannot use the \"parent\" attribute on \"" << mAttr.sName << "\", "
                "because it is a nested region. Attribute ignored." << std::endl;
        }
    }
    else
    {
        mAttr.bVirtual = mNode.get_attribute_value_or<bool>("virtual", false);

        if (const layout_attribute* pAttr = mNode.try_get_attribute("parent"))
        {
            std::string sParent = pAttr->get_value<std::string>();
            auto pParentObj = mRegistry.get_region_by_name(sParent);
            if (!sParent.empty() && !pParentObj)
            {
                gui::out << gui::warning << mNode.get_location() << " : "
                    << "Cannot find \"" << mAttr.sName << "\"'s parent : \"" << sParent << "\". "
                    "No parent given to this region." << std::endl;
            }

            mAttr.pParent = down_cast<frame>(pParentObj);
            if (pParentObj != nullptr && mAttr.pParent == nullptr)
            {
                gui::out << gui::warning << mNode.get_location() << " : "
                    << "Cannot set  \"" << mAttr.sName << "\"'s parent : \"" << sParent << "\". "
                    "This is not a frame." << std::endl;
            }
        }
    }

    if (const layout_attribute* pAttr = mNode.try_get_attribute("inherits"))
    {
        mAttr.lInheritance = mVirtualRegistry.get_virtual_region_list(
            pAttr->get_value<std::string>());
    }

    return mAttr;
}

void warn_for_not_accessed_node(const layout_node& mNode)
{
    if (mNode.is_access_check_bypassed())
        return;

    if (!mNode.was_accessed())
    {
        gui::out << gui::warning << mNode.get_location() << " : " <<
            "node '" << mNode.get_name() << "' was not read by parser; "
            "check its name is spelled correctly and that it is at the right location." << std::endl;
        return;
    }

    for (const auto& mAttr : mNode.get_attributes())
    {
        if (mAttr.is_access_check_bypassed())
            continue;

        if (!mAttr.was_accessed())
        {
            gui::out << gui::warning << mNode.get_location() << " : " <<
                "attribute '" << mNode.get_name() << "' was not read by parser; "
                "check its name is spelled correctly and that it is at the right location."
                << std::endl;
        }
    }

    for (const auto& mChild : mNode.get_children())
        warn_for_not_accessed_node(mChild);
}


}
}
