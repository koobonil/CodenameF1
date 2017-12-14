#pragma once

#include "zay_base.h"
#include "zay_vector2.h"

namespace ZAY
{
    class Touch
    : public Base
    {
    public:
        Touch();
        virtual ~Touch();

    public:
        Touch* clone() const;
        
    public:
        const Vector2& getLocation() const;
        const Vector2& getPreviousLocation() const;
        const Vector2& getStartLocation() const;
        const Vector2& getDelta() const;
    private:
        Vector2 _startPoint;
        Vector2 _point;
        Vector2 _prevPoint;
        Vector2 _delta;
        
    public:
        void setTouchInfo(int32_t id, float x, float y);
        int32_t getID() const;
        
    private:
        int32_t _id;
        bool _startPointCaptured;
    };
}
