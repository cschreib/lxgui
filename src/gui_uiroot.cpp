#include "lxgui/gui_uiroot.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/input.hpp"

#include <lxgui/utils_std.hpp>

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{

uiroot::uiroot(manager& mManager) :
    event_receiver(mManager.get_event_manager()), frame_container(mManager, this),
    mManager_(mManager), mRenderer_(mManager.get_renderer())
{
    mScreenDimensions_ = mManager.get_input_manager().get_window_dimensions();
}

vector2f uiroot::get_target_dimensions() const
{
    return vector2f(mScreenDimensions_)/get_manager().get_interface_scaling_factor();
}

void uiroot::render() const
{
    if (bEnableCaching_)
    {
        mRenderer_.render_quad(mScreenQuad_);
    }
    else
    {
        for (const auto& mStrata : lStrataList_)
        {
            render_strata_(mStrata);
        }
    }
}

void uiroot::create_caching_render_target_()
{
    try
    {
        if (pRenderTarget_)
            pRenderTarget_->set_dimensions(mScreenDimensions_);
        else
            pRenderTarget_ = mRenderer_.create_render_target(mScreenDimensions_);
    }
    catch (const utils::exception& e)
    {
        gui::out << gui::error << "gui::uiroot : "
            << "Unable to create render_target for GUI caching :\n" << e.get_description() << std::endl;

        bEnableCaching_ = false;
        return;
    }

    vector2f mScaledDimensions = get_target_dimensions();

    mScreenQuad_.mat = mRenderer_.create_material(pRenderTarget_);
    mScreenQuad_.v[0].pos = vector2f::ZERO;
    mScreenQuad_.v[1].pos = vector2f(mScaledDimensions.x, 0);
    mScreenQuad_.v[2].pos = mScaledDimensions;
    mScreenQuad_.v[3].pos = vector2f(0, mScaledDimensions.y);

    mScreenQuad_.v[0].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(0, 0), true);
    mScreenQuad_.v[1].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(1, 0), true);
    mScreenQuad_.v[2].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(1, 1), true);
    mScreenQuad_.v[3].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(0, 1), true);
}

void uiroot::create_strata_cache_render_target_(strata& mStrata)
{
    if (mStrata.pRenderTarget)
        mStrata.pRenderTarget->set_dimensions(mScreenDimensions_);
    else
        mStrata.pRenderTarget = mRenderer_.create_render_target(mScreenDimensions_);

    vector2f mScaledDimensions = get_target_dimensions();

    mStrata.mQuad.mat = mRenderer_.create_material(mStrata.pRenderTarget);
    mStrata.mQuad.v[0].pos = vector2f::ZERO;
    mStrata.mQuad.v[1].pos = vector2f(mScaledDimensions.x, 0);
    mStrata.mQuad.v[2].pos = mScaledDimensions;
    mStrata.mQuad.v[3].pos = vector2f(0, mScaledDimensions.y);

    mStrata.mQuad.v[0].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(0, 0), true);
    mStrata.mQuad.v[1].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(1, 0), true);
    mStrata.mQuad.v[2].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(1, 1), true);
    mStrata.mQuad.v[3].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(0, 1), true);
}

template<typename T>
void remove_null(T& lList)
{
    auto mIterRemove = std::remove_if(lList.begin(), lList.end(), [](auto& pObj)
    {
        return pObj == nullptr;
    });

    lList.erase(mIterRemove, lList.end());
}

void uiroot::update(float fDelta)
{
    // Update logics on root frames from parent to children.
    for (auto& mFrame : get_root_frames())
    {
        if (!mFrame.is_virtual())
            mFrame.update(fDelta);
    }

    // Removed destroyed frames
    garbage_collect();

    bool bRedraw = has_strata_list_changed_();
    reset_strata_list_changed_flag_();

    if (bRedraw)
        get_manager().notify_object_moved();

    if (bEnableCaching_)
    {
        DEBUG_LOG(" Redraw strata...");

        try
        {
            for (auto& mStrata : lStrataList_)
            {
                if (mStrata.bRedraw)
                {
                    if (!mStrata.pRenderTarget)
                        create_strata_cache_render_target_(mStrata);

                    if (mStrata.pRenderTarget)
                    {
                        get_manager().begin(mStrata.pRenderTarget);
                        mStrata.pRenderTarget->clear(color::EMPTY);
                        render_strata_(mStrata);
                        get_manager().end();
                    }

                    bRedraw = true;
                }

                mStrata.bRedraw = false;
            }

            if (!pRenderTarget_)
                create_caching_render_target_();

            if (bRedraw && pRenderTarget_)
            {
                get_manager().begin(pRenderTarget_);
                pRenderTarget_->clear(color::EMPTY);

                for (auto& mStrata : lStrataList_)
                {
                    mRenderer_.render_quad(mStrata.mQuad);
                }

                get_manager().end();
            }
        }
        catch (const utils::exception& e)
        {
            gui::out << gui::error << "gui::uiroot : "
                << "Unable to create render_target for strata :\n"
                << e.get_description() << std::endl;

            bEnableCaching_ = false;
        }
    }
}

void uiroot::toggle_caching()
{
    bEnableCaching_ = !bEnableCaching_;

    if (bEnableCaching_)
    {
        for (auto& mStrata : lStrataList_)
            mStrata.bRedraw = true;
    }
}

void uiroot::enable_caching(bool bEnable)
{
    if (bEnableCaching_ != bEnable)
        toggle_caching();
}

bool uiroot::is_caching_enabled() const
{
    return bEnableCaching_;
}

void uiroot::on_event(const event& mEvent)
{
    if (mEvent.get_name() == "WINDOW_RESIZED")
    {
        // Update internal window size
        mScreenDimensions_ = vector2ui(mEvent.get<std::uint32_t>(0), mEvent.get<std::uint32_t>(1));

        // Notify all frames anchored to the window edges
        for (auto& mFrame : get_root_frames())
        {
            if (!mFrame.is_virtual())
            {
                mFrame.notify_borders_need_update();
                mFrame.notify_renderer_need_redraw();
            }
        }

        // Resize caching render targets
        if (pRenderTarget_)
            create_caching_render_target_();

        for (auto& mStrata : lStrataList_)
        {
            if (mStrata.pRenderTarget)
                create_strata_cache_render_target_(mStrata);
        }
    }
}

void uiroot::notify_scaling_factor_updated()
{
    for (auto& mFrame : get_root_frames())
    {
        if (!mFrame.is_virtual())
            mFrame.notify_scaling_factor_updated();
    }

    if (pRenderTarget_)
        create_caching_render_target_();

    for (auto& mStrata : lStrataList_)
    {
        if (mStrata.pRenderTarget)
            create_strata_cache_render_target_(mStrata);
    }
}

}
}
