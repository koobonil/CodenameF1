#include "zay_types.h"
#include "zay_texture_atlas_set.h"
#include "zay_atlas.h"
#include "zay_texture_atlas.h"
#include "zay_image.h"
#include "zay_skeleton_data.h"
#include "zay_scene_slot_data.h"
#include "zay_texture2d.h"
#include "zay_image_data.h"
#include "zay_renderer.h"
#include "cheetah_texture_packer/imagepacker.h"

namespace ZAY
{
    ImageMappingAtlas::ImageMappingAtlas()
    {
        _targetTextureIndex = -1;

        _sourceX = 0;
        _sourceY = 0;
        _sourceWidth = 0;
        _sourceHeight = 0;
        _sourceRotated = 0;
    }

    ImageMappingAtlas::~ImageMappingAtlas()
    {
    }

    void ImageMappingAtlas::setTargetTextureIndex(int32_t index)
    {
        _targetTextureIndex = index;
    }

    int32_t ImageMappingAtlas::getTargetTextureIndex() const
    {
        return _targetTextureIndex;
    }

    void ImageMappingAtlas::setSourceX(int32_t sourceX)
    {
        _sourceX = sourceX;
    }
    
    void ImageMappingAtlas::setSourceY(int32_t sourceY)
    {
        _sourceY = sourceY;
    }
    
    void ImageMappingAtlas::setSourceWidth(int32_t sourceWidth)
    {
        _sourceWidth = sourceWidth;
    }
    
    void ImageMappingAtlas::setSourceHeight(int32_t sourceHeight)
    {
        _sourceHeight = sourceHeight;
    }
    
    void ImageMappingAtlas::setSourceRotated(int32_t sourceRotated)
    {
        _sourceRotated = sourceRotated;
    }
    
    int32_t ImageMappingAtlas::getSourceX() const
    {
        return _sourceY;
    }
    
    int32_t ImageMappingAtlas::getSourceY() const
    {
        return _sourceX;
    }
    
    int32_t ImageMappingAtlas::getSourceWidth() const
    {
        return _sourceWidth;
    }
    
    int32_t ImageMappingAtlas::getSourceHeight() const
    {
        return _sourceHeight;
    }
    
    int32_t ImageMappingAtlas::getSourceRotated() const
    {
        return _sourceRotated;
    }

    TextureAtlasSet::TextureAtlasSet()
    {
    }
    
    TextureAtlasSet::~TextureAtlasSet()
    {
    }

    TextureAtlas* TextureAtlasSet::getTextureAtlas(Image* image) const
    {
        return nullptr;
    }
}
