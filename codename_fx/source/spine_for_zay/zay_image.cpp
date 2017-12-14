#include "zay_types.h"
#include "zay_image.h"
#include "zay_texture_atlas.h"
#include "zay_image_data.h"
#include "zay_texture2d.h"

namespace ZAY
{
    Image::Image()
    {
        _loadedImageData = nullptr;
        _width = 0;
        _height = 0;
        _additive = false;
    }
    
    Image::~Image()
    {
        delete _loadedImageData;
    }

    void Image::loadImageData(const unsigned char * data, int32_t dataLen)
    {
        delete _loadedImageData;
        _loadedImageData = new ImageData;
        _loadedImageData->initWithImageData(data, dataLen);
        _width = _loadedImageData->getWidth();
        _height = _loadedImageData->getHeight();
    }

    ImageData* Image::getLoadedImageData() const
    {
        return _loadedImageData;
    }

    void Image::setWidth(int32_t width)
    {
        _width = width;
    }
    
    void Image::setHeight(int32_t height)
    {
        _height = height;
    }
    
    int32_t Image::getWidth() const
    {
        return _width;
    }
    
    int32_t Image::getHeight() const
    {
        return _height;
    }
    
    void Image::setAdditive(bool additive)
    {
        _additive = additive;
    }
    
    bool Image::getAdditive() const
    {
        return _additive;
    }

    TextureAtlas* Image::getTextureAtlas()
    {
        auto it = s_textureAtlases.find(this);

        if (it == s_textureAtlases.end())
        {
            TextureAtlas* textureAtlas = nullptr;

            if(_loadedImageData)
            {
                if (getAdditive())
                {
                    _loadedImageData->preadditiveAlpha();
                }
                else
                {
                    _loadedImageData->premultiplyAlpha();
                }
                
                Texture2D* texture = new Texture2D();
                texture->autorelease();

                if (texture->initWithImageData(_loadedImageData))
                {
                    textureAtlas = new TextureAtlas();
                    textureAtlas->retain();
                    s_textureAtlases[this] = textureAtlas;

                    textureAtlas->setTexture(texture);
                    textureAtlas->setAtlasX(0);
                    textureAtlas->setAtlasY(0);
                    textureAtlas->setAtlasWidth(_width);
                    textureAtlas->setAtlasHeight(_height);
                    textureAtlas->setAtlasRotated(0);
                }
                
                delete _loadedImageData;
                _loadedImageData = nullptr;
            }
            
            return textureAtlas;
        }

        return it->second;
    }

    void Image::clearCachedTextures()
    {
        for (auto it : s_textureAtlases)
        {
            it.second->release();
        }
        
        s_textureAtlases.clear();
    }
    
    std::map<Image*, TextureAtlas*> Image::s_textureAtlases;
}
