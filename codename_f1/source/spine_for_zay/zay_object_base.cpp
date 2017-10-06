#include "zay_object_base.h"

namespace ZAY
{
    ObjectBase::ObjectBase()
    {
    }

    ObjectBase::~ObjectBase()
    {
    }

    const ObjectBase& ObjectBase::operator =(const ObjectBase& other)
    {
        return *this;
    }
}
