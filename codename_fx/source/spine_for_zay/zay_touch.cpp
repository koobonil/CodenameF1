#include "zay_types.h"
#include "zay_touch.h"

namespace ZAY
{
    Touch::Touch()
    {
        _startPoint = Vector2::ZERO;
        _point = Vector2::ZERO;
        _prevPoint = Vector2::ZERO;
        _delta = Vector2::ZERO;
        
        _id = 0;
        _startPointCaptured = false;
    }

    Touch::~Touch()
    {
        
    }

    Touch* Touch::clone() const
    {
        auto touch = new Touch();
        touch->autorelease();

        touch->_startPoint = _startPoint;
        touch->_point = _point;
        touch->_prevPoint = _prevPoint;
        touch->_delta = _delta;
        
        touch->_id = _id;
        touch->_startPointCaptured = _startPointCaptured;

        return touch;
    }

    const Vector2& Touch::getLocation() const
    {
        return _point;
    }
    
    const Vector2& Touch::getPreviousLocation() const
    {
        return _prevPoint;
    }
    
    const Vector2& Touch::getStartLocation() const
    {
        return _startPoint;
    }
    
    const Vector2& Touch::getDelta() const
    {
        return _delta;
    }

    
    
    void Touch::setTouchInfo(int32_t id, float x, float y)
    {
        _id = id;
        _point.x = x;
        _point.y = y;
        _delta = _point - _prevPoint;
        _prevPoint = _point;

        if (!_startPointCaptured)
        {
            _startPoint = _point;
            _startPointCaptured = true;
            _delta = Vector2::ZERO;
            _prevPoint = _point;
        }
    }

    int32_t Touch::getID() const
    {
        return _id;
    }
}
