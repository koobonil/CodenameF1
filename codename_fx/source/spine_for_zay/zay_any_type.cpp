#include "zay_types.h"
#include "zay_any_type.h"

namespace ZAY
{
    const AnyType AnyType::Null;

    AnyType::AnyType()
    {
        clear();
    }
    
    AnyType::AnyType(const ValueType& valueType)
    {
        setAsValueType(valueType);
    }

    AnyType::AnyType(const AnyType& other)
    {
        _valueType = other._valueType;
    }
    
    AnyType::AnyType(AnyType&& other)
    {
        _valueType = other._valueType;
    }
    
    AnyType::~AnyType()
    {
    }

    bool AnyType::operator==(const AnyType& v) const
    {
        return (_valueType == v._valueType);
    }

    bool AnyType::operator!=(const AnyType& v) const
    {
        return (_valueType != v._valueType);
    }

    AnyType& AnyType::operator= (const AnyType& other)
    {
        _valueType = other._valueType;
        return *this;
    }
    
    AnyType& AnyType::operator= (AnyType&& other)
    {
        _valueType = other._valueType;
        return *this;
    }

    void AnyType::clear()
    {
        _valueType = ValueType::NoneType;
    }

    void AnyType::setAsValueType(const ValueType& valueType)
    {
        _valueType = valueType;
    }
}
