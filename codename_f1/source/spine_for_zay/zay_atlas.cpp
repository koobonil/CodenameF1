#include "zay_types.h"
#include "zay_atlas.h"

namespace ZAY
{
    Atlas::Atlas()
    {
        _atlasX = 0;
        _atlasY = 0;
        _atlasWidth = 0;
        _atlasHeight = 0;
        _atlasRotated = 0;
    }

    Atlas::~Atlas()
    {
    }

    void Atlas::setAtlasX(int32_t atlasX)
    {
        _atlasX = atlasX;
    }
    
    void Atlas::setAtlasY(int32_t atlasY)
    {
        _atlasY = atlasY;
    }
    
    void Atlas::setAtlasWidth(int32_t atlasWidth)
    {
        _atlasWidth = atlasWidth;
    }
    
    void Atlas::setAtlasHeight(int32_t atlasHeight)
    {
        _atlasHeight = atlasHeight;
    }
    
    void Atlas::setAtlasRotated(int32_t atlasRotated)
    {
        _atlasRotated = atlasRotated;
    }

    int32_t Atlas::getAtlasX() const
    {
        return _atlasX;
    }
    
    int32_t Atlas::getAtlasY() const
    {
        return _atlasY;
    }
    
    int32_t Atlas::getAtlasWidth() const
    {
        return _atlasWidth;
    }
    
    int32_t Atlas::getAtlasHeight() const
    {
        return _atlasHeight;
    }
    
    int32_t Atlas::getAtlasRotated() const
    {
        return _atlasRotated;
    }
}
