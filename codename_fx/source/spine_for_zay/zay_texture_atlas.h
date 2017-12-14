#pragma once

#include "zay_types.h"
#include "zay_atlas.h"
#include "zay_texture2d.h"

namespace ZAY
{
    class TextureAtlas
    : public Atlas
    {
    public:
        TextureAtlas();
        virtual ~TextureAtlas();

    public:
        void setTexture(Texture2D* texture);
    public:
        Texture2D* getTexture() const;
    private:
        Texture2D* _texture;
    };
}
