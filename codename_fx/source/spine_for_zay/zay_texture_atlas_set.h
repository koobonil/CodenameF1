#pragma once

#include "zay_object_base.h"
#include "zay_atlas.h"
#include "zay_texture_atlas.h"

namespace ZAY
{
    class ImageMappingAtlas
    : public Atlas
    {
    public:
        ImageMappingAtlas();
        virtual ~ImageMappingAtlas();
        
    public:
        void setTargetTextureIndex(int32_t index);
    public:
        int32_t getTargetTextureIndex() const;
    private:
        int32_t _targetTextureIndex;

    public:
        void setSourceX(int32_t sourceX);
        void setSourceY(int32_t sourceY);
        void setSourceWidth(int32_t sourceWidth);
        void setSourceHeight(int32_t sourceHeight);
        void setSourceRotated(int32_t sourceRotated);
        
    public:
        int32_t getSourceX() const;
        int32_t getSourceY() const;
        int32_t getSourceWidth() const;
        int32_t getSourceHeight() const;
        int32_t getSourceRotated() const;
        
    private:
        int32_t _sourceX;
        int32_t _sourceY;
        int32_t _sourceWidth;
        int32_t _sourceHeight;
        int32_t _sourceRotated;
    };

    class TextureAtlasSet
    : public ObjectBase
    {
    public:
        TextureAtlasSet();
        virtual ~TextureAtlasSet();

    public:
        TextureAtlas* getTextureAtlas(Image* image) const;

   /* public:
        void setPixelFormat(PixelFormat format);
        PixelFormat getPixelFormat() const;
    private:
        PixelFormat _pixelFormat;

    public:
        void addSourceWithImage(Image* image);
        void addSourceWithTextureAtlasSet(TextureAtlasSet* atlasSet);
        void addSourceWithSceneData(SceneData* sceneData);
        void removeSourcesAll();
    private:
        bool _atlasModified;

    public:
        TextureAtlas* getTextureAtlas(Image* image) const;
        void removeTextureAtlasesAll();

    private:
        int32_t _getBytesPerPixel() const;
        void _packImageMappingAtlases();
        bool _generateTextureAtlasImage(int32_t index, char* data, int32_t dataLen) const;
    private:
        std::vector<std::pair<int32_t, int32_t>> _textureSizes;*/
    };
}
