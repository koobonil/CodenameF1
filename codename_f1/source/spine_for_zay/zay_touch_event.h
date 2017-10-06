#pragma once

#include "zay_touch.h"

namespace ZAY
{
    enum TouchEventCode
    : std::uint32_t
    {
        BEGAN,
        MOVED,
        ENDED,
        CANCELLED
    };

    class TouchEvent
    : public Base
    {
    public:
        static const int32_t MAX_TOUCHES = 15;
        
    public:
        TouchEvent();
        virtual ~TouchEvent();

    public:
        TouchEvent* clone() const;
        
    public:
        void setTouchEventCode(TouchEventCode touchEventCode);
        TouchEventCode getTouchEventCode() const;
    private:
        TouchEventCode _touchEventCode;
        
    public:
        void addTouch(Touch* touch);
        void clearTouchesAll();
        const std::vector<Touch*>& getTouchesAll() const;
    private:
        std::vector<Touch*> _touches;

    };
}
