#include "zay_types.h"
#include "zay_any_value.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_vector4.h"
#include "zay_quaternion.h"
#include "zay_colour_value.h"

namespace ZAY
{
    const AnyValue AnyValue::Null;

    AnyValue::AnyValue()
    {
        _value.strVal = nullptr;
    }

    AnyValue::AnyValue(const AnyType& anyType)
    {
        _anyType = anyType;
        
        switch (_anyType.getValueType())
        {
            case ValueType::NoneType:
                _value.strVal = nullptr;
                break;

            case ValueType::Integer:
                _value.intVal = 0;
                break;

            case ValueType::Real:
                _value.realVal = 0.0f;
                break;

            case ValueType::Boolean:
                _value.boolVal = false;
                break;

            case ValueType::String:
                _value.strVal = new std::string();
                break;

            case ValueType::Vector2:
                _value.vector2Val = new Vector2(Vector2::ZERO);
                break;

            case ValueType::Vector3:
                _value.vector3Val = new Vector3(Vector3::ZERO);
                break;

            case ValueType::Vector4:
                _value.vector4Val = new Vector4(Vector4::ZERO);
                break;

            case ValueType::Quaternion:
                _value.quaternionVal= new Quaternion(Quaternion::IDENTITY);
                break;

            case ValueType::Colour:
                _value.colorVal = new ColourValue(ColourValue::ZERO);
                break;

            default:
            case ValueType::COUNT:
                assert(0);
                break;
        }
    }
    
    AnyValue::AnyValue(int32_t v)
    {
        _value.intVal = v;
        _anyType = ValueType::Integer;
    }

    AnyValue::AnyValue(float v)
    {
        _value.realVal = v;
        _anyType = ValueType::Real;
    }

    AnyValue::AnyValue(double v)
    {
        _value.realVal = static_cast<float>(v);
        _anyType = ValueType::Real;
    }

    AnyValue::AnyValue(bool v)
    {
        _value.boolVal = v;
        _anyType = ValueType::Boolean;
    }

    AnyValue::AnyValue(const char* v)
    {
        _value.strVal = new std::string;
        *_value.strVal = v;
        _anyType = ValueType::String;
    }

    AnyValue::AnyValue(const std::string& v)
    {
        _value.strVal = new std::string;
        *_value.strVal = v;
        _anyType = ValueType::String;
    }

    AnyValue::AnyValue(const Vector2& v)
    {
        _value.vector2Val = new Vector2(v);
        _anyType = ValueType::Vector2;
    }

    AnyValue::AnyValue(float x, float y)
    {
        _value.vector2Val = new Vector2(x, y);
        _anyType = ValueType::Vector2;
    }

    AnyValue::AnyValue(double x, double y)
    {
        _value.vector2Val = new Vector2(static_cast<float>(x), static_cast<float>(y));
        _anyType = ValueType::Vector2;
    }

    AnyValue::AnyValue(const Vector3& v)
    {
        _value.vector3Val = new Vector3(v);
        _anyType = ValueType::Vector3;
    }

    AnyValue::AnyValue(float x, float y, float z)
    {
        _value.vector3Val = new Vector3(x, y, z);
        _anyType = ValueType::Vector3;
    }

    AnyValue::AnyValue(double x, double y, double z)
    {
        _value.vector3Val = new Vector3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
        _anyType = ValueType::Vector3;
    }

    AnyValue::AnyValue(const Vector4& v)
    {
        _value.vector4Val = new Vector4(v);
        _anyType = ValueType::Vector4;
    }

    AnyValue::AnyValue(float x, float y, float z, float w)
    {
        _value.vector4Val = new Vector4(x, y, z, w);
        _anyType = ValueType::Vector4;
    }

    AnyValue::AnyValue(double x, double y, double z, double w)
    {
        _value.vector4Val = new Vector4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
        _anyType = ValueType::Vector4;
    }

    AnyValue::AnyValue(const Quaternion& v)
    {
        _value.quaternionVal = new Quaternion(v);
        _anyType = ValueType::Quaternion;
    }

    AnyValue::AnyValue(const ColourValue& v)
    {
        _value.colorVal = new ColourValue(v);
        _anyType = ValueType::Colour;
    }

    AnyValue::AnyValue(const AnyValue& other)
    {
        _anyType = other._anyType;

        switch (getValueType())
        {
        case ValueType::String:
            _value.strVal = new std::string(*other._value.strVal);
            break;

        case ValueType::Vector2:
            _value.vector2Val = new Vector2(*other._value.vector2Val);
            break;

        case ValueType::Vector3:
            _value.vector3Val = new Vector3(*other._value.vector3Val);
            break;

        case ValueType::Vector4:
            _value.vector4Val = new Vector4(*other._value.vector4Val);
            break;

        case ValueType::Integer:
            _value.intVal = other._value.intVal;
            break;

        case ValueType::Real:
            _value.realVal = other._value.realVal;
            break;

        case ValueType::Boolean:
            _value.boolVal = other._value.boolVal;
            break;

        case ValueType::Quaternion:
            _value.quaternionVal = new Quaternion(*other._value.quaternionVal);
            break;

        case ValueType::Colour:
            _value.colorVal = new ColourValue(*other._value.colorVal);
            break;

        default:
            break;
        }
    }

    AnyValue::AnyValue(AnyValue&& other)
    {
        _anyType = other._anyType;

        other._anyType = ValueType::NoneType;

        switch (getValueType())
        {
        case ValueType::String:
            _value.strVal = other._value.strVal;
            break;

        case ValueType::Vector2:
            _value.vector2Val = other._value.vector2Val;
            break;

        case ValueType::Vector3:
            _value.vector3Val = other._value.vector3Val;
            break;

        case ValueType::Vector4:
            _value.vector4Val = other._value.vector4Val;
            break;

        case ValueType::Integer:
            _value.intVal = other._value.intVal;
            break;

        case ValueType::Real:
            _value.realVal = other._value.realVal;
            break;

        case ValueType::Boolean:
            _value.boolVal = other._value.boolVal;
            break;

        case ValueType::Quaternion:
            _value.quaternionVal = other._value.quaternionVal;
            break;

        case ValueType::Colour:
            _value.colorVal = other._value.colorVal;
            break;

        default:
            break;
        }
    }

    AnyValue::~AnyValue()
    {
        clear();
    }

    bool AnyValue::operator == (const AnyValue& v) const
    {
        if (getValueType() != v.getValueType())
        {
            return false;
        }

        switch (getValueType())
        {
        case ValueType::NoneType:
            return true;
            break;

        case ValueType::Integer:
            return (_value.intVal == v._value.intVal);
            break;

        case ValueType::Real:
            return (_value.realVal == v._value.realVal);
            break;

        case ValueType::Boolean:
            return (_value.boolVal == v._value.boolVal);
            break;

        case ValueType::String:
            return (*_value.strVal == *v._value.strVal);
            break;

        case ValueType::Vector2:
            return (*_value.vector2Val == *v._value.vector2Val);
            break;

        case ValueType::Vector3:
            return (*_value.vector3Val == *v._value.vector3Val);
            break;

        case ValueType::Vector4:
            return (*_value.vector4Val == *v._value.vector4Val);
            break;

        case ValueType::Quaternion:
            return (*_value.quaternionVal == *v._value.quaternionVal);
            break;
                
        case ValueType::Colour:
            return (*_value.colorVal == *v._value.colorVal);
            break;

        default:
            assert(0);
            break;
        }

        return false;
    }

    bool AnyValue::operator!=(const AnyValue& v) const
    {
        if (getValueType() != v.getValueType())
        {
            return true;
        }

        switch (getValueType())
        {
        case ValueType::NoneType:
            return false;
            break;

        case ValueType::Integer:
            return (_value.intVal != v._value.intVal);
            break;

        case ValueType::Real:
            return (_value.realVal != v._value.realVal);
            break;

        case ValueType::Boolean:
            return (_value.boolVal != v._value.boolVal);
            break;

        case ValueType::String:
            return (*_value.strVal != *v._value.strVal);
            break;

        case ValueType::Vector2:
            return (*_value.vector2Val != *v._value.vector2Val);
            break;

        case ValueType::Vector3:
            return (*_value.vector3Val != *v._value.vector3Val);
            break;

        case ValueType::Vector4:
            return (*_value.vector4Val != *v._value.vector4Val);
            break;

        case ValueType::Quaternion:
            return (*_value.quaternionVal != *v._value.quaternionVal);
            break;

        case ValueType::Colour:
            return (*_value.colorVal != *v._value.colorVal);
            break;

        default:
            assert(0);
            break;
        }

        return false;
    }

    // assignment operator
    AnyValue& AnyValue::operator = (const AnyValue& other)
    {
        clear();

        _anyType = other._anyType;

        switch (getValueType())
        {
        case ValueType::String:
            _value.strVal = new std::string(*other._value.strVal);
            break;

        case ValueType::Vector2:
            _value.vector2Val = new Vector2(*other._value.vector2Val);
            break;

        case ValueType::Vector3:
            _value.vector3Val = new Vector3(*other._value.vector3Val);
            break;

        case ValueType::Vector4:
            _value.vector4Val = new Vector4(*other._value.vector4Val);
            break;

        case ValueType::Integer:
            _value.intVal = other._value.intVal;
            break;

        case ValueType::Real:
            _value.realVal = other._value.realVal;
            break;

        case ValueType::Boolean:
            _value.boolVal = other._value.boolVal;
            break;

        case ValueType::Quaternion:
            _value.quaternionVal = new Quaternion(*other._value.quaternionVal);
            break;

        case ValueType::Colour:
            _value.colorVal = new ColourValue(*other._value.colorVal);
            break;
                
        default:
            break;
        }

        return *this;
    }

    AnyValue& AnyValue::operator= (AnyValue&& other)
    {
        clear();

        _anyType = other._anyType;
        
        other._anyType = ValueType::NoneType;

        switch (getValueType())
        {
        case ValueType::String:
            _value.strVal = other._value.strVal;
            break;

        case ValueType::Vector2:
            _value.vector2Val = other._value.vector2Val;
            break;

        case ValueType::Vector3:
            _value.vector3Val = other._value.vector3Val;
            break;

        case ValueType::Vector4:
            _value.vector4Val = other._value.vector4Val;
            break;

        case ValueType::Integer:
            _value.intVal = other._value.intVal;
            break;

        case ValueType::Real:
            _value.realVal = other._value.realVal;
            break;

        case ValueType::Boolean:
            _value.boolVal = other._value.boolVal;
            break;

        case ValueType::Quaternion:
            _value.quaternionVal = other._value.quaternionVal;
            break;

        case ValueType::Colour:
            _value.colorVal = other._value.colorVal;
            break;

        default:
            break;

        }

        return *this;
    }

    AnyValue& AnyValue::operator= (int32_t v)
    {
        clear();

        _anyType = ValueType::Integer;
        _value.intVal = v;

        return *this;
    }

    AnyValue& AnyValue::operator= (float v)
    {
        clear();

        _anyType = ValueType::Real;
        _value.realVal = v;

        return *this;
    }

    AnyValue& AnyValue::operator= (double v)
    {
        clear();

        _anyType = ValueType::Real;
        _value.realVal = static_cast<float>(v);

        return *this;
    }

    AnyValue& AnyValue::operator= (bool v)
    {
        clear();

        _anyType = ValueType::Boolean;
        _value.boolVal = v;

        return *this;
    }

    AnyValue& AnyValue::operator= (const char* v)
    {
        clear();

        _anyType = ValueType::String;
        _value.strVal = new std::string(v);

        return *this;
    }

    AnyValue& AnyValue::operator= (const std::string& v)
    {
        clear();

        _anyType = ValueType::String;
        _value.strVal = new std::string(v);

        return *this;
    }

    AnyValue& AnyValue::operator= (const Vector2& v)
    {
        clear();

        _anyType = ValueType::Vector2;
        _value.vector2Val = new Vector2(v);

        return *this;
    }

    AnyValue& AnyValue::operator= (const Vector3& v)
    {
        clear();

        _anyType = ValueType::Vector3;
        _value.vector3Val = new Vector3(v);

        return *this;
    }

    AnyValue& AnyValue::operator= (const Vector4& v)
    {
        clear();

        _anyType = ValueType::Vector4;
        _value.vector4Val = new Vector4(v);

        return *this;
    }

    AnyValue& AnyValue::operator= (const Quaternion& v)
    {
        clear();
        
        _anyType = ValueType::Quaternion;
        _value.quaternionVal = new Quaternion(v);
        
        return *this;
    }

    AnyValue& AnyValue::operator= (const ColourValue& v)
    {
        clear();

        _anyType = ValueType::Colour;
        _value.colorVal = new ColourValue(v);

        return *this;
    }

    int32_t AnyValue::asInt() const
    {
        switch (getValueType())
        {
        case ValueType::Integer:
            return _value.intVal;
            break;

        case ValueType::Real:
            return static_cast<int32_t>(_value.realVal);
            break;

        case ValueType::Boolean:
            return _value.boolVal ? 1 : 0;
            break;

        case ValueType::String:
            return static_cast<int32_t>(_value.strVal->size());
            break;

        case ValueType::Vector2:
            return static_cast<int32_t>(_value.vector2Val->length());
            break;

        case ValueType::Vector3:
            return static_cast<int32_t>(_value.vector3Val->length());
            break;

        case ValueType::Vector4:
            return static_cast<int32_t>(_value.vector4Val->length());
            break;

        case ValueType::Quaternion:
            return static_cast<int32_t>(_value.quaternionVal->Norm());
            break;

        case ValueType::Colour:
            return static_cast<int32_t>(_value.colorVal->a);
            break;
                
        default:
            break;
        }

        return 0;
    }

    float AnyValue::asReal() const
    {
        switch (getValueType())
        {
        case ValueType::Integer:
            return static_cast<float>(_value.intVal);
            break;

        case ValueType::Real:
            return _value.realVal;
            break;

        case ValueType::Boolean:
            return _value.boolVal ? 1.0f : 0.0f;
            break;

        case ValueType::String:
            return static_cast<float>(_value.strVal->size());
            break;

        case ValueType::Vector2:
            return _value.vector2Val->length();
            break;

        case ValueType::Vector3:
            return _value.vector3Val->length();
            break;

        case ValueType::Vector4:
            return _value.vector4Val->length();
            break;

        case ValueType::Quaternion:
            return _value.quaternionVal->Norm();
            break;

        case ValueType::Colour:
            return _value.colorVal->a;
            break;

        default:
            break;
        }

        return 0.0f;
    }

    bool AnyValue::asBool() const
    {
        switch (getValueType())
        {
        case ValueType::Integer:
            return (_value.intVal != 0) ? true : false;
            break;

        case ValueType::Real:
            return (_value.realVal != 0.0f) ? true : false;
            break;

        case ValueType::Boolean:
            return _value.boolVal;
            break;

        case ValueType::String:
            return (_value.strVal->size() != 0) ? true : false;
            break;

        case ValueType::Vector2:
            return (_value.vector2Val->length() != 0.0f) ? true : false;
            break;

        case ValueType::Vector3:
            return (_value.vector3Val->length() != 0.0f) ? true : false;
            break;

        case ValueType::Vector4:
            return (_value.vector4Val->length() != 0.0f) ? true : false;
            break;

        case ValueType::Quaternion:
            return (_value.quaternionVal->Norm() != 0.0f) ? true : false;
            break;

        case ValueType::Colour:
            return (_value.colorVal->a != 0.0f) ? true : false;
            break;

        default:
            break;
        }

        return false;
    }

    std::string AnyValue::asString() const
    {
        switch (getValueType())
        {
        case ValueType::Integer:
            {
                std::stringstream ss;
                ss << _value.intVal;
                return ss.str();
            }
            break;

        case ValueType::Real:
            {
                std::stringstream ss;
                ss << _value.realVal;
                return ss.str();
            }
            break;

        case ValueType::Boolean:
            {
                std::stringstream ss;
                ss << _value.boolVal;
                return ss.str();
            }
            break;

        case ValueType::String:
            return *_value.strVal;
            break;

        case ValueType::Vector2:
            {
                std::stringstream ss;
                ss << *_value.vector2Val;
                return ss.str();
            }
            break;

        case ValueType::Vector3:
            {
                std::stringstream ss;
                ss << *_value.vector3Val;
                return ss.str();
            }
            break;

        case ValueType::Vector4:
            {
                std::stringstream ss;
                ss << *_value.vector4Val;
                return ss.str();
            }
            break;

        case ValueType::Quaternion:
            {
                std::stringstream ss;
                ss << *_value.quaternionVal;
                return ss.str();
            }
            break;

        case ValueType::Colour:
            {
                std::stringstream ss;
                ss << *_value.colorVal;
                return ss.str();
            }
            break;

        default:
            break;
        }

        return s_emptyString;
    }

    Vector2 AnyValue::asVector2() const
    {
        if (getValueType() == ValueType::Vector2)
        {
            return *_value.vector2Val;
        }

        return Vector2::ZERO;
    }

    Vector3 AnyValue::asVector3() const
    {
        if (getValueType() == ValueType::Vector3)
        {
            return *_value.vector3Val;
        }

        return Vector3::ZERO;
    }

    Vector4 AnyValue::asVector4() const
    {
        if (getValueType() == ValueType::Vector4)
        {
            return *_value.vector4Val;
        }

        return Vector4::ZERO;
    }

    Quaternion AnyValue::asQuaternion() const
    {
        if (getValueType() == ValueType::Quaternion)
        {
            return *_value.quaternionVal;
        }
        
        return Quaternion::IDENTITY;
    }

    ColourValue AnyValue::asColourValue() const
    {
        if (getValueType() == ValueType::Colour)
        {
            return *_value.colorVal;
        }

        return ColourValue::ZERO;
    }

    const AnyType& AnyValue::getAnyType() const
    {
        return _anyType;
    }

    void AnyValue::clear()
    {
        switch (getValueType())
        {
        case ValueType::Integer:
        case ValueType::Real:
        case ValueType::Boolean:
            break;

        case ValueType::String:
            delete _value.strVal;
            break;

        case ValueType::Vector2:
            delete _value.vector2Val;
            break;

        case ValueType::Vector3:
            delete _value.vector3Val;
            break;

        case ValueType::Vector4:
            delete _value.vector4Val;
            break;

        case ValueType::Quaternion:
            delete _value.quaternionVal;
            break;

        case ValueType::Colour:
            delete _value.colorVal;
            break;

        default:
        case ValueType::NoneType:
        case ValueType::COUNT:
            break;
        }

        _anyType = ValueType::NoneType;
    }

    void AnyValue::reset(const AnyType& anyType)
    {
        clear();
        
        _anyType = anyType;
        
        switch (getValueType())
        {
            case ValueType::Integer:
                _value.intVal = 0;
                break;
                
            case ValueType::Real:
                _value.realVal = 0.0f;
                break;
                
            case ValueType::Boolean:
                _value.boolVal = false;
                break;
                
            case ValueType::String:
                _value.strVal = new std::string;
                break;
                
            case ValueType::Vector2:
                _value.vector2Val = new Vector2(Vector2::ZERO);
                break;
                
            case ValueType::Vector3:
                _value.vector3Val = new Vector3(Vector3::ZERO);
                break;
                
            case ValueType::Vector4:
                _value.vector4Val = new Vector4(Vector4::ZERO);
                break;
                
            case ValueType::Quaternion:
                _value.quaternionVal = new Quaternion(Quaternion::IDENTITY);
                break;
                
            case ValueType::Colour:
                _value.colorVal = new ColourValue(ColourValue::ZERO);
                break;

            default:
            case ValueType::NoneType:
            case ValueType::COUNT:
                break;
        }
    }
    
    void AnyValue::resetToDefault()
    {
        switch (getValueType())
        {
            case ValueType::Integer:
                _value.intVal = 0;
                break;
                
            case ValueType::Real:
                _value.realVal = 0.0f;
                break;
                
            case ValueType::Boolean:
                _value.boolVal = false;
                break;
                
            case ValueType::String:
                _value.strVal->clear();
                break;
                
            case ValueType::Vector2:
                *_value.vector2Val = Vector2::ZERO;
                break;
                
            case ValueType::Vector3:
                *_value.vector3Val = Vector3::ZERO;
                break;
                
            case ValueType::Vector4:
                *_value.vector4Val = Vector4::ZERO;
                break;
                
            case ValueType::Quaternion:
                *_value.quaternionVal = Quaternion::IDENTITY;
                break;
                
            case ValueType::Colour:
                *_value.colorVal = ColourValue::ZERO;
                break;

            default:
            case ValueType::NoneType:
            case ValueType::COUNT:
                break;
        }
    }

    template<>
    int32_t AnyValue::_asPrimitiveType<int32_t>() const
    {
        return asInt();
    }

    template<>
    float AnyValue::_asPrimitiveType<float>() const
    {
        return asReal();
    }

    template<>
    bool AnyValue::_asPrimitiveType<bool>() const
    {
        return asBool();
    }

    template<>
    std::string AnyValue::_asPrimitiveType<std::string>() const
    {
        return asString();
    }
    
    template<>
    Vector2 AnyValue::_asPrimitiveType<Vector2>() const
    {
        return asVector2();
    }
    
    template<>
    Vector3 AnyValue::_asPrimitiveType<Vector3>() const
    {
        return asVector3();
    }

    template<>
    Vector4 AnyValue::_asPrimitiveType<Vector4>() const
    {
        return asVector4();
    }

    template<>
    Quaternion AnyValue::_asPrimitiveType<Quaternion>() const
    {
        return asQuaternion();
    }
    
    template<>
    ColourValue AnyValue::_asPrimitiveType<ColourValue>() const
    {
        return asColourValue();
    }
}
