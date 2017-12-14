#include "zay_base.h"

namespace ZAY
{
    Base::Base()
    {
        _referenceCount = 0;
    }

    Base::~Base()
    {
        assert(_referenceCount == 0);
    }

    void Base::retain()
    {
        assert(_referenceCount >= 0);

        _referenceCount ++;
    }

    void Base::release()
    {
        assert(_referenceCount > 0);

        _referenceCount --;

        if (_referenceCount == 0)
        {
            delete this;
        }
    }

    Base* Base::autorelease()
    {
        assert(_referenceCount >= 0);
        assert(AutoreleasePool::getCurrentPool() != nullptr);

        AutoreleasePool::getCurrentPool()->addObject(this);
        return this;
    }

    int32_t Base::getReferenceCount() const
    {
        return _referenceCount;
    }

    void AutoreleasePool::createDefaultPool()
    {
        if (s_defaultPool == nullptr)
        {
            s_defaultPool = new AutoreleasePool();
        }
    }

    void AutoreleasePool::destroyDefaultPool()
    {
        if (s_defaultPool)
        {
            delete s_defaultPool;
            s_defaultPool = nullptr;
        }
    }

    void AutoreleasePool::setCurrentPool(AutoreleasePool* currentPool)
    {
        s_currentPool = currentPool;
    }

    AutoreleasePool* AutoreleasePool::getCurrentPool()
    {
        if (s_currentPool)
        {
            return s_currentPool;
        }

        return s_defaultPool;
    }

    AutoreleasePool* AutoreleasePool::s_defaultPool = nullptr;
    AutoreleasePool* AutoreleasePool::s_currentPool = nullptr;

    AutoreleasePool::AutoreleasePool()
    {
    }

    AutoreleasePool::~AutoreleasePool()
    {
        clear();

        if (s_currentPool == this)
        {
            s_currentPool = nullptr;
        }
    }

    void AutoreleasePool::addObject(Base* object)
    {
        if (!contains(object))
        {
            auto ret = _managedObjects.insert(object);

            if (ret.second)
            {
                object->retain();
            }
        }
    }


    void AutoreleasePool::clear()
    {
        for (auto object : _managedObjects)
        {
            object->release();
        }
        _managedObjects.clear();
    }

    bool AutoreleasePool::contains(Base* object) const
    {
        auto it = _managedObjects.find(object);

        if (it != _managedObjects.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
