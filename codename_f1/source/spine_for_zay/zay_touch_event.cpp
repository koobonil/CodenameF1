#include "zay_types.h"
#include "zay_touch_event.h"
#include "zay_touch.h"

namespace ZAY
{
    TouchEvent::TouchEvent()
    {
        _touchEventCode = TouchEventCode::BEGAN;
    }

    TouchEvent::~TouchEvent()
    {
        clearTouchesAll();
    }

    TouchEvent* TouchEvent::clone() const
    {
        auto newTouchEvent = new TouchEvent();
        newTouchEvent->autorelease();
        
        newTouchEvent->_touchEventCode = _touchEventCode;
        
        for (auto touch : _touches)
        {
            newTouchEvent->addTouch(touch->clone());
        }
        
        return newTouchEvent;
    }

    void TouchEvent::setTouchEventCode(TouchEventCode touchEventCode)
    {
        _touchEventCode = touchEventCode;
    }

    TouchEventCode TouchEvent::getTouchEventCode() const
    {
        return _touchEventCode;
    }

    void TouchEvent::addTouch(Touch* touch)
    {
        if (touch)
        {
            touch->retain();
            _touches.push_back(touch);
        }
    }
    
    void TouchEvent::clearTouchesAll()
    {
        for (auto touch : _touches)
        {
            touch->release();
        }
        _touches.clear();
    }
    
    const std::vector<Touch*>& TouchEvent::getTouchesAll() const
    {
        return _touches;
    }
}
