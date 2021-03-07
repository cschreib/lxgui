#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/impl/gui_sfml_rendertarget.hpp"
#include "lxgui/impl/gui_sfml_font.hpp"
#include <lxgui/gui_sprite.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace lxgui {
namespace gui {
namespace sfml
{
renderer::renderer(sf::RenderWindow& mWindow) : mWindow_(mWindow)
{
}

void renderer::begin(std::shared_ptr<gui::render_target> pTarget) const
{
    if (pCurrentTarget_ || pCurrentSFMLTarget_)
        throw gui::exception("gui::sfml::renderer", "Missing call to end()");

    if (pTarget)
    {
        pCurrentTarget_ = std::static_pointer_cast<sfml::render_target>(pTarget);
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

    std::shared_ptr<sfml::material> pMat = std::static_pointer_cast<sfml::material>(mQuad.mat);

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

    std::shared_ptr<sfml::material> pMat = std::static_pointer_cast<sfml::material>(mQuad.mat);

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

std::shared_ptr<gui::material> renderer::create_material(const std::string& sFileName, material::filter mFilter) const
{
    std::string sBackedName = utils::to_string((int)mFilter) + '|' + sFileName;
    std::map<std::string, std::weak_ptr<gui::material>>::iterator iter = lTextureList_.find(sBackedName);
    if (iter != lTextureList_.end())
    {
        if (std::shared_ptr<gui::material> pLock = iter->second.lock())
            return pLock;
        else
            lTextureList_.erase(iter);
    }

    try
    {
        std::shared_ptr<gui::material> pTex = std::make_shared<sfml::material>(
            sFileName, material::wrap::REPEAT, mFilter
        );

        lTextureList_[sFileName] = pTex;
        return pTex;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::material> renderer::create_material(const color& mColor) const
{
    return std::make_shared<sfml::material>(mColor);
}

std::shared_ptr<gui::material> renderer::create_material(std::shared_ptr<gui::render_target> pRenderTarget) const
{
    try
    {
        return std::static_pointer_cast<sfml::render_target>(pRenderTarget)->get_material().lock();
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::render_target> renderer::create_render_target(uint uiWidth, uint uiHeight) const
{
    try
    {
        return std::make_shared<sfml::render_target>(uiWidth, uiHeight);
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::font> renderer::create_font(const std::string& sFontFile, uint uiSize) const
{
    std::string sFontName = sFontFile + "|" + utils::to_string(uiSize);
    std::map<std::string, std::weak_ptr<gui::font>>::iterator iter = lFontList_.find(sFontName);
    if (iter != lFontList_.end())
    {
        if (std::shared_ptr<gui::font> pLock = iter->second.lock())
            return pLock;
        else
            lFontList_.erase(iter);
    }

    std::shared_ptr<gui::font> pFont = std::make_shared<sfml::font>(sFontFile, uiSize);
    lFontList_[sFontName] = pFont;
    return pFont;
}

void renderer::notify_window_resized(uint uiNewWidth, uint uiNewHeight)
{
    sf::FloatRect mVisibleArea(0, 0, uiNewWidth, uiNewHeight);
    mWindow_.setView(sf::View(mVisibleArea));
}

}
}
}
