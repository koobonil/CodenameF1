#pragma once

#include "zay_types.h"

namespace ZAY
{
    class Base
    {
    public:
        Base();
        virtual ~Base();

    public:
        void retain();
        void release();
        Base* autorelease();
        int32_t getReferenceCount() const;
    private:
        int32_t _referenceCount;
    };

    class AutoreleasePool
    {
    public:
        static void createDefaultPool();
        static void destroyDefaultPool();
        static void setCurrentPool(AutoreleasePool* currentPool);
        static AutoreleasePool* getCurrentPool();

    private:
        static AutoreleasePool* s_defaultPool;
        static AutoreleasePool* s_currentPool;

    public:
        AutoreleasePool();
        ~AutoreleasePool();

    public:
        void addObject(Base* object);
        void clear();
        bool contains(Base* object) const;
    private:
        std::unordered_set<Base*> _managedObjects;
    };
}
