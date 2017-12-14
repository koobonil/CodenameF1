#include "zay_types.h"
#include "zay_touch_event_dispatcher.h"
#include "zay_touch_event.h"
#include "zay_touch.h"
#include "zay_touch_area_instance.h"

namespace ZAY
{
    TouchEventDispatcher::TouchEventDispatcher()
    {
    }
    
    TouchEventDispatcher::~TouchEventDispatcher()
    {
        removeTouchEventsAll();
        removeTouchAreaInstancesAll();
    }

    void TouchEventDispatcher::addTouchEvent(TouchEvent* touchEvent)
    {
        _queuedTouchEvents.push_back(touchEvent);
        touchEvent->retain();
    }
    
    void TouchEventDispatcher::removeTouchEventsAll()
    {
        for (auto touchEvent : _queuedTouchEvents)
        {
            touchEvent->release();
        }
        _queuedTouchEvents.clear();
    }
    
    const std::vector<TouchEvent*>& TouchEventDispatcher::getTouchEvents() const
    {
        return _queuedTouchEvents;
    }
    
    
    
    
    
    void TouchEventDispatcher::dispatchTouchEvents()
    {
        if (_queuedTouchEvents.size() > 0)
        {
            std::vector<TouchAreaInstance*> orderedTouchAreaInstances;

            for (auto it : _touchAreaInstances)
            {
                if (it.right->getEnabled() &&
                    it.right->getEnabledForController() &&
                    it.right->getAttachedTouchEventDispatcher() == this)
                {
                    orderedTouchAreaInstances.push_back(it.right);
                    it.right->autorelease();
                }
            }

            if (orderedTouchAreaInstances.size() > 0)
            {
                std::vector<TouchEvent*> queuedTouchEvents = _queuedTouchEvents;

                for (auto touchEvent : queuedTouchEvents)
                {
                    touchEvent->autorelease();
                }

                for (auto touchEvent : _queuedTouchEvents)
                {
                    switch (touchEvent->getTouchEventCode())
                    {
                        case TouchEventCode::BEGAN:
                            for (auto touchAreaInstance : orderedTouchAreaInstances)
                            {
                                if (touchAreaInstance->getAttachedTouchEventDispatcher() == this &&
                                    touchAreaInstance->onTouchBegan)
                                {
                                    if (touchAreaInstance->onTouchBegan(touchEvent))
                                    {
                                        break;
                                    }
                                }
                            }
                            break;
                            
                        case TouchEventCode::MOVED:
                            for (auto touchAreaInstance : orderedTouchAreaInstances)
                            {
                                if (touchAreaInstance->getAttachedTouchEventDispatcher() == this &&
                                    touchAreaInstance->onTouchMoved)
                                {
                                    touchAreaInstance->onTouchMoved(touchEvent);
                                }
                            }
                            break;
                            
                        case TouchEventCode::ENDED:
                            for (auto touchAreaInstance : orderedTouchAreaInstances)
                            {
                                if (touchAreaInstance->getAttachedTouchEventDispatcher() == this &&
                                    touchAreaInstance->onTouchEnded)
                                {
                                    touchAreaInstance->onTouchEnded(touchEvent);
                                }
                            }
                            break;
                            
                        case TouchEventCode::CANCELLED:
                            for (auto touchAreaInstance : orderedTouchAreaInstances)
                            {
                                if (touchAreaInstance->getAttachedTouchEventDispatcher() == this &&
                                    touchAreaInstance->onTouchCancelled)
                                {
                                    touchAreaInstance->onTouchCancelled(touchEvent);
                                }
                            }
                            break;
                    }

                    touchEvent->release();
                }
            }

            _queuedTouchEvents.clear();
        }
    }

    void TouchEventDispatcher::_addTouchAreaInstance(TouchAreaInstance* touchAreaInstance)
    {
        assert(_touchAreaInstances.right.find(touchAreaInstance) == _touchAreaInstances.right.end());

        _touchAreaInstances.insert(TouchAreaInstanceMapValueType(touchAreaInstance->getRenderPriority(), touchAreaInstance));
        touchAreaInstance->retain();
    }

    void TouchEventDispatcher::_removeTouchAreaInstance(TouchAreaInstance* touchAreaInstance)
    {
        auto it = _touchAreaInstances.right.find(touchAreaInstance);

        if (it != _touchAreaInstances.right.end())
        {
            touchAreaInstance->release();
            _touchAreaInstances.right.erase(it);
        }
    }

    void TouchEventDispatcher::removeTouchAreaInstancesAll()
    {
        for (auto it : _touchAreaInstances)
        {
            it.right->release();
        }
        _touchAreaInstances.clear();
    }
    
    
    
    void TouchEventDispatcher::setEnabled(bool enabled)
    {
        if (_enabled != enabled)
        {
            _enabled = enabled;

        }
    }
    
    bool TouchEventDispatcher::getEnabled() const
    {
        return _enabled;
    }
}





