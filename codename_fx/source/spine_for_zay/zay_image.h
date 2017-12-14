#pragma once

#include "zay_object_base.h"

namespace ZAY
{
    class Image
    : public ObjectBase
    {
    public:
        Image();
        virtual ~Image();

    public:
        void loadImageData(const unsigned char * data, int32_t dataLen);
        ImageData* getLoadedImageData() const;
    private:
        ImageData* _loadedImageData;

    public:
        void setWidth(int32_t width);
        void setHeight(int32_t height);
        int32_t getWidth() const;
        int32_t getHeight() const;
    private:
        int32_t _width;
        int32_t _height;
        
    public:
        void setAdditive(bool additive);
        bool getAdditive() const;
    private:
        bool _additive;

    public:
        TextureAtlas* getTextureAtlas();
    public:
        static void clearCachedTextures();
    private:
        static std::map<Image*, TextureAtlas*> s_textureAtlases;
    };
}
