#pragma once

#include "zay_types.h"

namespace ZAY
{
    class AnyType
    {
    public:
        static const AnyType Null;

        AnyType();
        AnyType(const ValueType& valueType);

        AnyType(const AnyType& other);
        AnyType(AnyType&& other);
        ~AnyType();

        // compare operator
        bool operator==(const AnyType& v) const;
        bool operator!=(const AnyType& v) const;

        // assignment operator
        AnyType& operator= (const AnyType& other);
        AnyType& operator= (AnyType&& other);

        void clear();
        void setAsValueType(const ValueType& valueType);

        inline const ValueType& getValueType() const { return _valueType; }

    private:
        ValueType _valueType;
    };
}
