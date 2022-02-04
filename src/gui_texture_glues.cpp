#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <sol/state.hpp>

/** A @{LayeredRegion} that can draw images and colored rectangles.
 *   This object contains either a texture taken from a file,
 *   or a plain color (possibly with a different color on each corner).
 *
 *   Inherits all methods from: @{Region}, @{LayeredRegion}.
 *
 *   Child classes: none.
 *   @classmod Texture
 */

namespace lxgui::gui {

sol::optional<gradient::orientation> get_gradient_orientation(const std::string& sOrientation) {
    sol::optional<gradient::orientation> mOrientation;
    if (sOrientation == "HORIZONTAL")
        mOrientation = gradient::orientation::HORIZONTAL;
    else if (sOrientation == "VERTICAL")
        mOrientation = gradient::orientation::VERTICAL;
    else {
        gui::out << gui::warning
                 << "Texture:set_gradient : "
                    "Unknown gradient orientation : \"" +
                        sOrientation + "\"."
                 << std::endl;
    }

    return mOrientation;
}

void texture::register_on_lua(sol::state& mLua) {
    auto mClass = mLua.new_usertype<texture>(
        "Texture", sol::base_classes, sol::bases<region, layered_region>(),
        sol::meta_function::index, member_function<&texture::get_lua_member_>(),
        sol::meta_function::new_index, member_function<&texture::set_lua_member_>());

    /** @function get_blend_mode
     */
    mClass.set_function("get_blend_mode", [](const texture& mSelf) {
        texture::blend_mode mBlend = mSelf.get_blend_mode();
        switch (mBlend) {
        case texture::blend_mode::NONE: return "NONE";
        case texture::blend_mode::BLEND: return "BLEND";
        case texture::blend_mode::KEY: return "KEY";
        case texture::blend_mode::ADD: return "ADD";
        case texture::blend_mode::MOD: return "MOD";
        default: return "UNKNOWN";
        }
    });

    /** @function get_filter_mode
     */
    mClass.set_function("get_filter_mode", [](const texture& mSelf) {
        material::filter mFilter = mSelf.get_filter_mode();
        switch (mFilter) {
        case material::filter::NONE: return "NONE";
        case material::filter::LINEAR: return "LINEAR";
        default: return "UNKNOWN";
        }
    });

    /** @function get_tex_coord
     */
    mClass.set_function("get_tex_coord", [](const texture& mSelf) {
        const auto& lCoords = mSelf.get_tex_coord();
        return std::make_tuple(
            lCoords[0], lCoords[1], lCoords[2], lCoords[3], lCoords[4], lCoords[5], lCoords[6],
            lCoords[7]);
    });

    /** @function get_tex_coord_modifies_rect
     */
    mClass.set_function(
        "get_tex_coord_modifies_rect", member_function<&texture::get_tex_coord_modifies_rect>());

    /** @function get_texture
     */
    mClass.set_function("get_texture", [](const texture& mSelf) {
        sol::optional<std::string> mReturn;
        if (mSelf.has_texture_file())
            mReturn = mSelf.get_texture_file();
    });

    /** @function get_vertex_color
     */
    mClass.set_function("get_vertex_color", [](const texture& mSelf, std::size_t uiIndex) {
        color mColor = mSelf.get_vertex_color(uiIndex);
        return std::make_tuple(mColor.r, mColor.g, mColor.b, mColor.a);
    });

    /** @function is_desaturated
     */
    mClass.set_function("is_desaturated", member_function<&texture::is_desaturated>());

    /** @function set_blend_mode
     */
    mClass.set_function("set_blend_mode", [](texture& mSelf, const std::string& sBlend) {
        texture::blend_mode mBlend;
        if (sBlend == "NONE")
            mBlend = texture::blend_mode::NONE;
        else if (sBlend == "BLEND")
            mBlend = texture::blend_mode::BLEND;
        else if (sBlend == "KEY")
            mBlend = texture::blend_mode::KEY;
        else if (sBlend == "ADD")
            mBlend = texture::blend_mode::ADD;
        else if (sBlend == "MOD")
            mBlend = texture::blend_mode::MOD;
        else {
            gui::out << gui::warning << "Texture:set_blend_mode : "
                     << "Unknown blending mode : \"" + sBlend + "\"." << std::endl;
            return;
        }

        mSelf.set_blend_mode(mBlend);
    });

    /** @function set_filter_mode
     */
    mClass.set_function("set_filter_mode", [](texture& mSelf, const std::string& sFilter) {
        material::filter mFilter;
        if (sFilter == "NONE")
            mFilter = material::filter::NONE;
        else if (sFilter == "LINEAR")
            mFilter = material::filter::LINEAR;
        else {
            gui::out << gui::warning << "Texture:set_filter_mode : "
                     << "Unknown filtering mode : \"" + sFilter + "\"." << std::endl;
            return;
        }

        mSelf.set_filter_mode(mFilter);
    });

    /** @function set_desaturated
     */
    mClass.set_function("set_desaturated", member_function<&texture::set_desaturated>());

    /** @function set_gradient
     */
    mClass.set_function(
        "set_gradient",
        sol::overload(
            [](texture& mSelf, const std::string& sOrientation, float fMinR, float fMinG,
               float fMinB, float fMaxR, float fMaxG, float fMaxB) {
                sol::optional<gradient::orientation> mOrientation =
                    get_gradient_orientation(sOrientation);
                if (!mOrientation.has_value())
                    return;

                mSelf.set_gradient(gradient(
                    mOrientation.value(), color(fMinR, fMinG, fMinB), color(fMaxR, fMaxG, fMaxB)));
            },
            [](texture& mSelf, const std::string& sOrientation, const std::string& sMinColor,
               const std::string& sMaxColor) {
                sol::optional<gradient::orientation> mOrientation =
                    get_gradient_orientation(sOrientation);
                if (!mOrientation.has_value())
                    return;

                mSelf.set_gradient(
                    gradient(mOrientation.value(), color(sMinColor), color(sMaxColor)));
            }));

    /** @function set_gradient_alpha
     */
    mClass.set_function(
        "set_gradient_alpha",
        sol::overload(
            [](texture& mSelf, const std::string& sOrientation, float fMinR, float fMinG,
               float fMinB, float fMinA, float fMaxR, float fMaxG, float fMaxB, float fMaxA) {
                sol::optional<gradient::orientation> mOrientation =
                    get_gradient_orientation(sOrientation);
                if (!mOrientation.has_value())
                    return;

                mSelf.set_gradient(gradient(
                    mOrientation.value(), color(fMinR, fMinG, fMinB, fMinA),
                    color(fMaxR, fMaxG, fMaxB, fMaxA)));
            },
            [](texture& mSelf, const std::string& sOrientation, const std::string& sMinColor,
               const std::string& sMaxColor) {
                sol::optional<gradient::orientation> mOrientation =
                    get_gradient_orientation(sOrientation);
                if (!mOrientation.has_value())
                    return;

                mSelf.set_gradient(
                    gradient(mOrientation.value(), color(sMinColor), color(sMaxColor)));
            }));

    /** @function set_tex_coord
     */
    mClass.set_function(
        "set_tex_coord",
        sol::overload(
            [](texture& mSelf, float fLeft, float fTop, float fRight, float fBottom) {
                mSelf.set_tex_rect({fLeft, fTop, fRight, fBottom});
            },
            [](texture& mSelf, float fTopLeftX, float fTopLeftY, float fTopRightX, float fTopRightY,
               float fBottomRightX, float fBottomRightY, float fBottomLeftX, float fBottomLeftY) {
                mSelf.set_tex_coord(
                    {fTopLeftX, fTopLeftY, fTopRightX, fTopRightY, fBottomRightX, fBottomRightY,
                     fBottomLeftX, fBottomLeftY});
            }));

    /** @function set_tex_coord_modifies_rect
     */
    mClass.set_function(
        "set_tex_coord_modifies_rect", member_function<&texture::set_tex_coord_modifies_rect>());

    /** @function set_texture
     */
    mClass.set_function(
        "set_texture",
        sol::overload(
            [](texture& mSelf, const std::string& sTexture) {
                if (!sTexture.empty() && sTexture[0] == '#') {
                    // This is actually a color hash
                    mSelf.set_solid_color(color(sTexture));
                } else {
                    // Normal texture file
                    mSelf.set_texture(sTexture);
                }
            },
            [](texture& mSelf, float fR, float fG, float fB, sol::optional<float> fA) {
                mSelf.set_solid_color(color(fR, fG, fB, fA.value_or(1.0f)));
            }));

    /** @function set_vertex_color
     */
    mClass.set_function(
        "set_vertex_color",
        sol::overload(
            [](texture& mSelf, const std::string& sColor) {
                mSelf.set_vertex_color(color(sColor), std::numeric_limits<std::size_t>::max());
            },
            [](texture& mSelf, float fR, float fG, float fB, sol::optional<float> fA) {
                mSelf.set_vertex_color(
                    color(fR, fG, fB, fA.value_or(1.0f)), std::numeric_limits<std::size_t>::max());
            },
            [](texture& mSelf, std::size_t uiIndex, const std::string& sColor) {
                mSelf.set_vertex_color(color(sColor), uiIndex);
            },
            [](texture& mSelf, std::size_t uiIndex, float fR, float fG, float fB,
               sol::optional<float> fA) {
                mSelf.set_vertex_color(color(fR, fG, fB, fA.value_or(1.0f)), uiIndex);
            }));
}

} // namespace lxgui::gui
