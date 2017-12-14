#pragma once

#include "zay_base.h"

namespace ZAY
{
    class ObjectBase
    : public Base
    {
    public:
        ObjectBase();
        virtual ~ObjectBase();

    public:
        const ObjectBase& operator =(const ObjectBase& other);
    };
}
