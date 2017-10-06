#include "zay_types.h"
#include "zay_texture_atlas.h"
#include "zay_texture2d.h"

namespace ZAY
{
    TextureAtlas::TextureAtlas()
    {
        _texture = nullptr;
    }

    TextureAtlas::~TextureAtlas()
    {
        if (_texture != nullptr)
        {
            _texture->release();
        }
    }

    void TextureAtlas::setTexture(Texture2D* texture)
    {
        if (_texture != texture)
        {
            if (_texture)
            {
                _texture->release();
            }
            
            _texture = texture;
            
            if (_texture)
            {
                _texture->retain();
            }
        }
    }

    Texture2D* TextureAtlas::getTexture() const
    {
        return _texture;
    }
}
