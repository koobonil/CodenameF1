#pragma once

#include "zay_types.h"
#include "zay_render_priority.h"
#include "zay_touch_event.h"
#include "zay_touch_area_instance.h"

namespace ZAY
{
    class TouchEventDispatcher
    : public Base
    {
    public:
        TouchEventDispatcher();
        virtual ~TouchEventDispatcher();

    public:
        void addTouchEvent(TouchEvent* touchEvent);
        void removeTouchEventsAll();
        const std::vector<TouchEvent*>& getTouchEvents() const;
    private:
        std::vector<TouchEvent*> _queuedTouchEvents;

    public:
        typedef boost::bimap<boost::bimaps::multiset_of<RenderPriority>, boost::bimaps::set_of<TouchAreaInstance*>> TouchAreaInstanceMap;
        typedef TouchAreaInstanceMap::value_type TouchAreaInstanceMapValueType;
    public:
        void dispatchTouchEvents();
    public:
        void _addTouchAreaInstance(TouchAreaInstance* touchAreaObject);
        void _removeTouchAreaInstance(TouchAreaInstance* touchAreaObject);
    public:
        void removeTouchAreaInstancesAll();
    private:
        TouchAreaInstanceMap _touchAreaInstances;

    public:
        void setEnabled(bool enabled);
        bool getEnabled() const;
    private:
        bool _enabled;
        
        
    };
}
