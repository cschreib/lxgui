#include "lxgui/gui_factory.hpp"

#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_registry.hpp"

namespace lxgui {
namespace gui
{

factory::factory(manager& mManager) : mManager_(mManager)
{
}

utils::owner_ptr<uiobject> factory::create_uiobject(registry& mRegistry,
    const uiobject_core_attributes& mAttr)
{
    if (!mRegistry.check_uiobject_name(mAttr.sName))
        return nullptr;

    auto mIter = lCustomObjectList_.find(mAttr.sObjectType);
    if (mIter == lCustomObjectList_.end())
    {
        gui::out << gui::warning << "gui::factory : Unknown object class : \""
            << mAttr.sObjectType << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<uiobject> pNewObject = mIter->second(mManager_);
    if (!pNewObject)
        return nullptr;

    if (!finalize_object_(mRegistry, *pNewObject, mAttr))
        return nullptr;

    apply_inheritance_(*pNewObject, mAttr);

    return pNewObject;
}

utils::owner_ptr<frame> factory::create_frame(registry& mRegistry, frame_renderer* pRenderer,
    const uiobject_core_attributes& mAttr)
{
    if (!mRegistry.check_uiobject_name(mAttr.sName))
        return nullptr;

    auto mIter = lCustomFrameList_.find(mAttr.sObjectType);
    if (mIter == lCustomFrameList_.end())
    {
        gui::out << gui::warning << "gui::factory : Unknown frame class : \""
            << mAttr.sObjectType << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<frame> pNewFrame = mIter->second(mManager_);
    if (!pNewFrame)
        return nullptr;

    if (!finalize_object_(mRegistry, *pNewFrame, mAttr))
        return nullptr;

    if (pRenderer && !pNewFrame->is_virtual())
        pRenderer->notify_rendered_frame(pNewFrame, true);

    apply_inheritance_(*pNewFrame, mAttr);

    return pNewFrame;
}

utils::owner_ptr<layered_region> factory::create_layered_region(registry& mRegistry,
    const uiobject_core_attributes& mAttr)
{
    if (!mRegistry.check_uiobject_name(mAttr.sName))
        return nullptr;

    auto mIter = lCustomRegionList_.find(mAttr.sObjectType);
    if (mIter == lCustomRegionList_.end())
    {
        gui::out << gui::warning << "gui::factory : Unknown layered_region class : \""
            << mAttr.sObjectType << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<layered_region> pNewRegion = mIter->second(mManager_);
    if (!pNewRegion)
        return nullptr;

    if (!finalize_object_(mRegistry, *pNewRegion, mAttr))
        return nullptr;

    apply_inheritance_(*pNewRegion, mAttr);

    return pNewRegion;
}

sol::state& factory::get_lua()
{
    return mManager_.get_lua();
}

const sol::state& factory::get_lua() const
{
    return mManager_.get_lua();
}

bool factory::finalize_object_(registry& mRegistry, uiobject& mObject,
    const uiobject_core_attributes& mAttr)
{
    if (mAttr.bVirtual)
        mObject.set_virtual();

    if (mAttr.pParent)
        mObject.set_name_and_parent_(mAttr.sName, mAttr.pParent);
    else
        mObject.set_name_(mAttr.sName);

    if (!mObject.is_virtual() || mAttr.pParent == nullptr)
    {
        if (!mRegistry.add_uiobject(observer_from(&mObject)))
            return false;
    }

    if (!mObject.is_virtual())
        mObject.create_glue();

    return true;
}

void factory::apply_inheritance_(uiobject& mObject, const uiobject_core_attributes& mAttr)
{
    for (const auto& pBase : mAttr.lInheritance)
    {
        if (!mObject.is_object_type(pBase->get_object_type()))
        {
            gui::out << gui::warning << "gui::factory : "
                << "\"" << mObject.get_name() << "\" (" << mObject.get_object_type()
                << ") cannot inherit from \"" << pBase->get_name() << "\" (" << pBase->get_object_type()
                << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other object
        mObject.copy_from(*pBase);
    }
}

}
}
