#pragma once

#include "zay_base.h"

namespace ZAY
{
    class Atlas
    : public Base
    {
    public:
        Atlas();
        virtual ~Atlas();

    public:
        void setAtlasX(int32_t atlasX);
        void setAtlasY(int32_t atlasY);
        void setAtlasWidth(int32_t atlasWidth);
        void setAtlasHeight(int32_t atlasHeight);
        void setAtlasRotated(int32_t atlasRotated);
        
    public:
        int32_t getAtlasX() const;
        int32_t getAtlasY() const;
        int32_t getAtlasWidth() const;
        int32_t getAtlasHeight() const;
        int32_t getAtlasRotated() const;
        
    private:
        int32_t _atlasX;
        int32_t _atlasY;
        int32_t _atlasWidth;
        int32_t _atlasHeight;
        int32_t _atlasRotated;
    };
}
