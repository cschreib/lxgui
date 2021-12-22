#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui {
namespace gui
{

node_core_attributes parse_core_attributes(manager& mManager, const layout_node& mNode,
    utils::observer_ptr<frame> pParent)
{
    node_core_attributes mAttr;
    mAttr.sObjectType = mNode.get_name();
    mAttr.sName = mNode.get_attribute_value<std::string>("name");

    if (pParent)
    {
        mAttr.pParent = pParent;

        if (mNode.has_attribute("virtual"))
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                << "Cannot use the \"virtual\" attribute on \"" << mAttr.sName << "\", "
                "because it is a nested uiobject. Attribute ignored." << std::endl;
        }
        if (mNode.has_attribute("parent"))
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                << "Cannot use the \"parent\" attribute on \"" << mAttr.sName << "\", "
                "because it is a nested uiobject. Attribute ignored." << std::endl;
        }
    }
    else
    {
        mAttr.bVirtual = mNode.get_attribute_value_or<bool>("virtual", false);

        if (const layout_node* pAttr = mNode.try_get_attribute("parent"))
        {
            std::string sParent = pAttr->get_value<std::string>();
            auto pParent = mManager.get_uiobject_by_name(sParent);
            if (!sParent.empty() && !mAttr.pParent)
            {
                gui::out << gui::warning << mNode.get_location() << " : "
                    << "Cannot find \"" << mAttr.sName << "\"'s parent : \"" << sParent << "\". "
                    "No parent given to that widget." << std::endl;
            }

            mAttr.pParent = down_cast<frame>(pParent);
            if (pParent != nullptr && mAttr.pParent == nullptr)
            {
                gui::out << gui::warning << mNode.get_location() << " : "
                    << "Cannot set  \"" << mAttr.sName << "\"'s parent : \"" << sParent << "\". "
                    "This is not a frame." << std::endl;
            }
        }
    }

    if (const layout_node* pAttr = mNode.try_get_attribute("inherits"))
        mAttr.lInheritance = mManager.get_virtual_uiobject_list(pAttr->get_value<std::string>());

    return mAttr;
}

}
}
