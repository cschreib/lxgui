#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/impl/gui_sfml_rendertarget.hpp"
#include "lxgui/impl/gui_sfml_font.hpp"
#include <lxgui/gui_sprite.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace gui {
namespace sfml
{
renderer::renderer(sf::RenderWindow& mWindow) : mWindow_(mWindow), pCurrentSFMLTarget_(nullptr)
{
}

renderer::~renderer()
{
}

void renderer::begin(utils::refptr<gui::render_target> pTarget) const
{
    if (pCurrentTarget_ || pCurrentSFMLTarget_)
        throw gui::exception("gui::sfml::renderer", "Missing call to end()");

    if (pTarget)
    {
        pCurrentTarget_ = utils::refptr<sfml::render_target>::cast(pTarget);
        pCurrentTarget_->begin();
        pCurrentSFMLTarget_ = pCurrentTarget_->get_render_texture();
    }
    else
    {
        pCurrentSFMLTarget_ = &mWindow_;
    }
}

void renderer::end() const
{
    if (pCurrentTarget_)
        pCurrentTarget_->end();

    pCurrentTarget_ = nullptr;
    pCurrentSFMLTarget_ = nullptr;
}

void renderer::render_quad(const quad& mQuad) const
{
    static const std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};
    static const uint n = ids.size();

    utils::refptr<sfml::material> pMat = utils::refptr<sfml::material>::cast(mQuad.mat);

    const float fTexWidth = pMat->get_real_width();
    const float fTexHeight = pMat->get_real_height();

    sf::VertexArray mArray(sf::PrimitiveType::Triangles, ids.size());
    for (uint i = 0; i < n; ++i)
    {
        const uint j = ids[i];
        sf::Vertex& mSFVertex = mArray[i];
        const vertex& mVertex = mQuad.v[j];
        const color& mColor = (pMat->get_type() == sfml::material::type::TEXTURE ?
            mVertex.col : mVertex.col*pMat->get_color());
        const float a = mColor.a;

        mSFVertex.position.x  = mVertex.pos.x;
        mSFVertex.position.y  = mVertex.pos.y;
        mSFVertex.texCoords.x = mVertex.uvs.x*fTexWidth;
        mSFVertex.texCoords.y = mVertex.uvs.y*fTexHeight;
        mSFVertex.color.r     = mColor.r*a*255; // Premultipled alpha
        mSFVertex.color.g     = mColor.g*a*255; // Premultipled alpha
        mSFVertex.color.b     = mColor.b*a*255; // Premultipled alpha
        mSFVertex.color.a     = a*255;
    }

    sf::RenderStates mState;
    mState.blendMode = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha); // Premultiplied alpha
    mState.texture = pMat->get_texture();
    pCurrentSFMLTarget_->draw(mArray, mState);
}

void renderer::render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const
{
    static const std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};
    static const uint n = ids.size();

    utils::refptr<sfml::material> pMat = utils::refptr<sfml::material>::cast(mQuad.mat);

    const float fTexWidth = pMat->get_real_width();
    const float fTexHeight = pMat->get_real_height();

    sf::VertexArray mArray(sf::PrimitiveType::Triangles, ids.size() * lQuadList.size());
    for (uint k = 0; k < lQuadList.size(); ++k)
    {
        const std::array<vertex,4>& mVertices = lQuadList[k];
        for (uint i = 0; i < n; ++i)
        {
            const uint j = ids[i];
            sf::Vertex& mSFVertex = mArray[k*n + i];
            const vertex& mVertex = mVertices[j];
            const color& mColor = (pMat->get_type() == sfml::material::type::TEXTURE ?
                mVertex.col : mVertex.col*pMat->get_color());
            const float a = mColor.a;

            mSFVertex.position.x  = mVertex.pos.x;
            mSFVertex.position.y  = mVertex.pos.y;
            mSFVertex.texCoords.x = mVertex.uvs.x*fTexWidth;
            mSFVertex.texCoords.y = mVertex.uvs.y*fTexHeight;
            mSFVertex.color.r     = mColor.r*a*255; // Premultipled alpha
            mSFVertex.color.g     = mColor.g*a*255; // Premultipled alpha
            mSFVertex.color.b     = mColor.b*a*255; // Premultipled alpha
            mSFVertex.color.a     = a*255;
        }
    }

    sf::RenderStates mState;
    mState.blendMode = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha); // Premultiplied alpha
    mState.texture = pMat->get_texture();
    pCurrentSFMLTarget_->draw(mArray, mState);
}

utils::refptr<gui::material> renderer::create_material(const std::string& sFileName, material::filter mFilter) const
{
    std::string sBackedName = utils::to_string((int)mFilter) + '|' + sFileName;
    std::map<std::string, utils::wptr<gui::material>>::iterator iter = lTextureList_.find(sBackedName);
    if (iter != lTextureList_.end())
    {
        if (utils::refptr<gui::material> pLock = iter->second.lock())
            return pLock;
        else
            lTextureList_.erase(iter);
    }

    try
    {
        utils::refptr<sfml::material> pTex = utils::refptr<sfml::material>(new sfml::material(
            sFileName, material::wrap::REPEAT, mFilter
        ));

        lTextureList_[sFileName] = pTex;
        return pTex;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

utils::refptr<gui::material> renderer::create_material(const color& mColor) const
{
    return utils::refptr<material>(new material(mColor));
}

utils::refptr<gui::material> renderer::create_material(utils::refptr<gui::render_target> pRenderTarget) const
{
    try
    {
        return utils::refptr<sfml::render_target>::cast(pRenderTarget)->get_material().lock();
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

utils::refptr<gui::render_target> renderer::create_render_target(uint uiWidth, uint uiHeight) const
{
    try
    {
        return utils::refptr<gui::render_target>(new sfml::render_target(uiWidth, uiHeight));
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

utils::refptr<gui::font> renderer::create_font(const std::string& sFontFile, uint uiSize) const
{
    std::string sFontName = sFontFile + "|" + utils::to_string(uiSize);
    std::map<std::string, utils::wptr<gui::font>>::iterator iter = lFontList_.find(sFontName);
    if (iter != lFontList_.end())
    {
        if (utils::refptr<gui::font> pLock = iter->second.lock())
            return pLock;
        else
            lFontList_.erase(iter);
    }

    utils::refptr<gui::font> pFont(new sfml::font(sFontFile, uiSize));
    lFontList_[sFontName] = pFont;
    return pFont;
}
}
}
