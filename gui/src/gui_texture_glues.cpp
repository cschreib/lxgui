#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_function.hpp>

namespace gui
{
void texture::register_glue(lua::state* pLua)
{
    pLua->reg<lua_texture>();
}

lua_texture::lua_texture(lua_State* pLua) : lua_layered_region(pLua)
{
    pTextureParent_ = dynamic_cast<texture*>(pParent_);
    if (pParent_ && !pTextureParent_)
        throw exception("lua_texture", "Dynamic cast failed !");
}

int lua_texture::_get_blend_mode(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:get_blend_mode", pLua, 1);

    texture::blend_mode mBlend = pTextureParent_->get_blend_mode();

    std::string sBlend;
    switch (mBlend)
    {
        case texture::BLEND_NONE  : sBlend = "NONE";  break;
        case texture::BLEND_BLEND : sBlend = "BLEND"; break;
        case texture::BLEND_KEY   : sBlend = "KEY";   break;
        case texture::BLEND_ADD   : sBlend = "ADD";   break;
        case texture::BLEND_MOD   : sBlend = "MOD";   break;
    }

    mFunc.push(sBlend);

    return mFunc.on_return();
}

int lua_texture::_get_filter_mode(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:get_filter_mode", pLua, 1);

    filter mFilter = pTextureParent_->get_filter_mode();

    std::string sFilter;
    switch (mFilter)
    {
        case FILTER_NONE   : sFilter = "NONE";  break;
        case FILTER_LINEAR : sFilter = "LINEAR"; break;
    }

    mFunc.push(sFilter);

    return mFunc.on_return();
}

int lua_texture::_get_tex_coord(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:get_tex_coord", pLua, 8);

    const std::array<float,8>& lCoords = pTextureParent_->get_tex_coord();

    for (uint i = 0; i < 8; ++i)
        mFunc.push(lCoords[i]);

    return mFunc.on_return();
}

int lua_texture::_get_tex_coord_modifies_rect(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:get_tex_coord_modifies_rect", pLua, 1);

    mFunc.push(pTextureParent_->get_tex_coord_modifies_rect());

    return mFunc.on_return();
}

int lua_texture::_get_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:get_texture", pLua, 1);

    mFunc.push(pTextureParent_->get_texture());

    return mFunc.on_return();
}

int lua_texture::_get_vertex_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:get_vertex_color", pLua, 4);

    color mColor = pTextureParent_->get_vertex_color();

    mFunc.push(mColor.r);
    mFunc.push(mColor.g);
    mFunc.push(mColor.b);
    mFunc.push(mColor.a);

    return mFunc.on_return();
}

int lua_texture::_is_desaturated(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:is_desaturated", pLua, 1);

    mFunc.push(pTextureParent_->is_desaturated());

    return mFunc.on_return();
}

int lua_texture::_set_blend_mode(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_blend_mode", pLua);
    mFunc.add(0, "blend mode", lua::TYPE_STRING);
    if (mFunc.check())
    {
        std::string sBlend = mFunc.get(0)->get_string();
        texture::blend_mode mBlend;
        if (sBlend == "NONE")
            mBlend = texture::BLEND_NONE;
        else if (sBlend == "BLEND")
            mBlend = texture::BLEND_BLEND;
        else if (sBlend == "KEY")
            mBlend = texture::BLEND_KEY;
        else if (sBlend == "ADD")
            mBlend = texture::BLEND_ADD;
        else if (sBlend == "MOD")
            mBlend = texture::BLEND_MOD;
        else
        {
            gui::out << gui::warning << mFunc.get_name() << " : "
                << "Unknown blending mode : \""+sBlend+"\"." << std::endl;
            return mFunc.on_return();
        }

        pTextureParent_->set_blend_mode(mBlend);
    }

    return mFunc.on_return();
}

int lua_texture::_set_filter_mode(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_filter_mode", pLua);
    mFunc.add(0, "filter mode", lua::TYPE_STRING);
    if (mFunc.check())
    {
        std::string sFilter = mFunc.get(0)->get_string();
        filter mFilter;
        if (sFilter == "NONE")
            mFilter = FILTER_NONE;
        else if (sFilter == "LINEAR")
            mFilter = FILTER_LINEAR;
        else
        {
            gui::out << gui::warning << mFunc.get_name() << " : "
                << "Unknown filtering mode : \""+sFilter+"\"." << std::endl;
            return mFunc.on_return();
        }

        pTextureParent_->set_filter_mode(mFilter);
    }

    return mFunc.on_return();
}

int lua_texture::_set_desaturated(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_desaturated", pLua, 1);
    mFunc.add(0, "is desaturated", lua::TYPE_BOOLEAN);
    if (mFunc.check())
        pTextureParent_->set_desaturated(mFunc.get(0)->get_bool());

    mFunc.push(true);

    return mFunc.on_return();
}

int lua_texture::_set_gradient(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_gradient", pLua);
    mFunc.add(0, "orientation", lua::TYPE_STRING);
    mFunc.add(1, "min red", lua::TYPE_NUMBER);
    mFunc.add(2, "min green", lua::TYPE_NUMBER);
    mFunc.add(3, "min blue", lua::TYPE_NUMBER);
    mFunc.add(4, "max red", lua::TYPE_NUMBER);
    mFunc.add(5, "max green", lua::TYPE_NUMBER);
    mFunc.add(6, "max blue", lua::TYPE_NUMBER);
    mFunc.new_param_set();
    mFunc.add(0, "min color", lua::TYPE_STRING);
    mFunc.add(1, "max color", lua::TYPE_STRING);

    if (mFunc.check())
    {
        std::string sOrientation = mFunc.get(0)->get_string();
        gradient::orientation mOrientation;
        if (sOrientation == "HORIZONTAL")
            mOrientation = gradient::orientation::HORIZONTAL;
        else if (sOrientation == "VERTICAL")
            mOrientation = gradient::orientation::VERTICAL;
        else
        {
            gui::out << gui::warning << mFunc.get_name() << " : "
                "Unknown gradient orientation : \""+sOrientation+"\"." << std::endl;
            return mFunc.on_return();
        }

        if (mFunc.get_param_set_rank() == 0)
        {
            pTextureParent_->set_gradient(gradient(
                mOrientation,
                color(
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                ),
                color(
                    mFunc.get(4)->get_number(),
                    mFunc.get(5)->get_number(),
                    mFunc.get(6)->get_number()
                )
            ));
        }
        else
        {
            pTextureParent_->set_gradient(gradient(
                mOrientation,
                color(mFunc.get(0)->get_string()),
                color(mFunc.get(1)->get_string())
            ));
        }
    }

    return mFunc.on_return();
}

int lua_texture::_set_gradient_alpha(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_gradient_alpha", pLua);
    mFunc.add(0, "orientation", lua::TYPE_STRING);
    mFunc.add(1, "min red", lua::TYPE_NUMBER);
    mFunc.add(2, "min green", lua::TYPE_NUMBER);
    mFunc.add(3, "min blue", lua::TYPE_NUMBER);
    mFunc.add(4, "min alpha", lua::TYPE_NUMBER);
    mFunc.add(5, "max red", lua::TYPE_NUMBER);
    mFunc.add(6, "max green", lua::TYPE_NUMBER);
    mFunc.add(7, "max blue", lua::TYPE_NUMBER);
    mFunc.add(8, "max alpha", lua::TYPE_NUMBER);
    mFunc.new_param_set();
    mFunc.add(0, "min color", lua::TYPE_STRING);
    mFunc.add(1, "max color", lua::TYPE_STRING);

    if (mFunc.check())
    {
        std::string sOrientation = mFunc.get(0)->get_string();
        gradient::orientation mOrientation;
        if (sOrientation == "HORIZONTAL")
            mOrientation = gradient::orientation::HORIZONTAL;
        else if (sOrientation == "VERTICAL")
            mOrientation = gradient::orientation::VERTICAL;
        else
        {
            gui::out << gui::warning << mFunc.get_name() << " : "
                "Unknown gradient orientation : \""+sOrientation+"\"." << std::endl;
            return mFunc.on_return();
        }

        if (mFunc.get_param_set_rank() == 0)
        {
            pTextureParent_->set_gradient(gradient(
                mOrientation,
                color(
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number(),
                    mFunc.get(4)->get_number()
                ),
                color(
                    mFunc.get(5)->get_number(),
                    mFunc.get(6)->get_number(),
                    mFunc.get(7)->get_number(),
                    mFunc.get(8)->get_number()
                )
            ));
        }
        else
        {
            pTextureParent_->set_gradient(gradient(
                mOrientation,
                color(mFunc.get(0)->get_string()),
                color(mFunc.get(1)->get_string())
            ));
        }
    }

    return mFunc.on_return();
}

int lua_texture::_set_tex_coord(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_tex_coord", pLua);
    mFunc.add(0, "left", lua::TYPE_NUMBER);
    mFunc.add(1, "top", lua::TYPE_NUMBER);
    mFunc.add(2, "right", lua::TYPE_NUMBER);
    mFunc.add(3, "bottom", lua::TYPE_NUMBER);
    mFunc.new_param_set();
    mFunc.add(0, "top-left X", lua::TYPE_NUMBER);
    mFunc.add(1, "top-left Y", lua::TYPE_NUMBER);
    mFunc.add(2, "top-right X", lua::TYPE_NUMBER);
    mFunc.add(3, "top-right Y", lua::TYPE_NUMBER);
    mFunc.add(4, "bottom-right X", lua::TYPE_NUMBER);
    mFunc.add(5, "bottom-right Y", lua::TYPE_NUMBER);
    mFunc.add(6, "bottom-left X", lua::TYPE_NUMBER);
    mFunc.add(7, "bottom-left Y", lua::TYPE_NUMBER);

    if (mFunc.check())
    {
        if (mFunc.get_param_set_rank() == 0)
        {
            // Only 4 coordinates provided
            std::array<float,4> mRect;
            for (uint i = 0; i < 4; ++i)
                mRect[i] = mFunc.get(i)->get_number();

            pTextureParent_->set_tex_coord(mRect);
        }
        else
        {
            // Or 8
            std::array<float,4> mCoords;
            for (uint i = 0; i < 8; ++i)
                mCoords[i] = mFunc.get(i)->get_number();

            pTextureParent_->set_tex_coord(mCoords);
        }
    }

    return mFunc.on_return();
}

int lua_texture::_set_tex_coord_modifies_rect(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_tex_coord_modifies_rect", pLua);
    mFunc.add(0, "does set_tex_coord modifies size", lua::TYPE_BOOLEAN);
    if (mFunc.check())
        pTextureParent_->set_tex_coord_modifies_rect(mFunc.get(0)->get_bool());

    return mFunc.on_return();
}

int lua_texture::_set_texture(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_texture", pLua);
    mFunc.add(0, "texture", lua::TYPE_STRING);
    mFunc.new_param_set();
    mFunc.add(0, "red", lua::TYPE_NUMBER);
    mFunc.add(1, "green", lua::TYPE_NUMBER);
    mFunc.add(2, "blue", lua::TYPE_NUMBER);
    mFunc.add(3, "alpha", lua::TYPE_NUMBER, true);
    mFunc.new_param_set();
    mFunc.add(0, "color", lua::TYPE_STRING);

    if (mFunc.check())
    {
        if (mFunc.get_param_set_rank() == 0)
        {
            std::string sTexture = mFunc.get(0)->get_string();
            if (!sTexture.empty() && sTexture[0] == '#')
            {
                // #RRGGBBAA color
                pTextureParent_->set_color(color(sTexture));
            }
            else
            {
                // texture name
                pTextureParent_->set_texture(
                    pTextureParent_->get_manager()->parse_file_name(sTexture)
                );
            }
        }
        else
        {
            // texture color
            color mColor;
            if (mFunc.is_provided(3))
            {
                mColor = color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                );
            }
            else
            {
                mColor = color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number()
                );
            }
            pTextureParent_->set_color(mColor);
        }
    }

    return mFunc.on_return();
}

int lua_texture::_set_vertex_color(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("Texture:set_vertex_color", pLua);
    mFunc.add(0, "red", lua::TYPE_NUMBER);
    mFunc.add(1, "green", lua::TYPE_NUMBER);
    mFunc.add(2, "blue", lua::TYPE_NUMBER);
    mFunc.add(3, "alpha", lua::TYPE_NUMBER, true);
    mFunc.new_param_set();
    mFunc.add(0, "color", lua::TYPE_STRING);

    if (mFunc.check())
    {
        color mColor;
        if (mFunc.get_param_set_rank() == 0)
        {
            if (mFunc.is_provided(3))
            {
                mColor = color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number(),
                    mFunc.get(3)->get_number()
                );
            }
            else
            {
                mColor = color(
                    mFunc.get(0)->get_number(),
                    mFunc.get(1)->get_number(),
                    mFunc.get(2)->get_number()
                );
            }
        }
        else
            mColor = color(mFunc.get(0)->get_string());

        pTextureParent_->set_vertex_color(mColor);
    }

    return mFunc.on_return();
}
}
