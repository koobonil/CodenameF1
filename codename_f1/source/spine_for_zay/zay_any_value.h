#pragma once

#include "zay_types.h"
#include "zay_any_type.h"

namespace ZAY
{
    class AnyValue
    {
    public:
        static const AnyValue Null;

        AnyValue();
        explicit AnyValue(const AnyType& anyType);

        explicit AnyValue(int32_t v);
        explicit AnyValue(float v);
        explicit AnyValue(double v);
        explicit AnyValue(bool v);
        explicit AnyValue(const char* v);
        AnyValue(const std::string& v);

        AnyValue(const Vector2& v);
        explicit AnyValue(float x, float y);
        explicit AnyValue(double x, double y);

        AnyValue(const Vector3& v);
        explicit AnyValue(float x, float y, float z);
        explicit AnyValue(double x, double y, double z);

        AnyValue(const Vector4& v);
        explicit AnyValue(float x, float y, float z, float w);
        explicit AnyValue(double x, double y, double z, double w);

        AnyValue(const Quaternion& v);

        AnyValue(const ColourValue& v);

        AnyValue(const AnyValue& other);
        AnyValue(AnyValue&& other);
        ~AnyValue();

        // compare operator
        bool operator==(const AnyValue& v) const;
        bool operator!=(const AnyValue& v) const;

        // assignment operator
        AnyValue& operator= (const AnyValue& other);
        AnyValue& operator= (AnyValue&& other);

        AnyValue& operator= (int32_t v);
        AnyValue& operator= (float v);
        AnyValue& operator= (double v);
        AnyValue& operator= (bool v);
        AnyValue& operator= (const char* v);
        AnyValue& operator= (const std::string& v);

        AnyValue& operator= (const Vector2& v);

        AnyValue& operator= (const Vector3& v);

        AnyValue& operator= (const Vector4& v);

        AnyValue& operator= (const Quaternion& v);

        AnyValue& operator= (const ColourValue& v);

        int32_t asInt() const;
        float asReal() const;
        bool asBool() const;
        std::string asString() const;
        Vector2 asVector2() const;
        Vector3 asVector3() const;
        Vector4 asVector4() const;
        Quaternion asQuaternion() const;
        ColourValue asColourValue() const;
        ObjectBase* asObjectPtr() const;
        
        const AnyType& getAnyType() const;
        
        void cacheObjectPtr();
        
        template<typename T>
        typename std::enable_if<std::is_base_of<ObjectBase*, T>::value, T>::type asType() const
        {
            return _asObjectType<T>();
        }

        template<typename T>
        typename std::enable_if<!std::is_base_of<ObjectBase*, T>::value, T>::type asType() const
        {
            return _asPrimitiveType<T>();
        }

        template<typename T>
        T _asObjectType() const
        {
            assert(asObjectPtr() == nullptr ||
                   dynamic_cast<T>(asObjectPtr()));
            return static_cast<T>(asObjectPtr());
        }

        template<typename T>
        T _asPrimitiveType() const;

        inline bool isNull() const { return _anyType.getValueType() == ValueType::NoneType; }

        inline const ValueType& getValueType() const { return _anyType.getValueType(); };

        void clear();
        void reset(const AnyType& anyType);
        void resetToDefault();

        union UnionValueType
        {
            int32_t intVal;
            float realVal;
            bool boolVal;

            std::string *strVal;
            Vector2 *vector2Val;
            Vector3 *vector3Val;
            Vector4 *vector4Val;
            Quaternion *quaternionVal;
            ColourValue *colorVal;
        };

        UnionValueType _value;

        AnyType _anyType;
    };
}
