#pragma once

#include "zay_base.h"
#include "zay_animation_state.h"
#include "zay_lerp.h"
#include "zay_random_generator.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_vector4.h"
#include "zay_colour_value.h"

namespace ZAY
{
    class AnimationKeyFrame
    : public Base
    {
    public:
        static void registerAnimationKeyFrameType(const std::string& typeName, const std::function<AnimationKeyFrame*()>& creator);
        static void unregisterAnimationKeyFrameType(const std::string& typeName);
        static AnimationKeyFrame* createAnimationKeyFrame(const std::string& typeName);
        static void initAnimationKeyFrameCreators();
    private:
        static std::map<std::string, std::function<AnimationKeyFrame*()>> s_animationKeyFrameCreators;

    public:
        template<typename T>
        static T createAnimationKeyFrameWithTypeCasting(const std::string& typeName)
        {
            auto keyFrame = createAnimationKeyFrame(typeName);
            assert(dynamic_cast<T>(keyFrame));
            return static_cast<T>(keyFrame);
        }
        
    public:
        virtual const char* getTypeName() const = 0;
    };
    
    
    
    template<typename T>
    class AnimationKeyFrameSingleValue
    : public AnimationKeyFrame
    {
    public:
        AnimationKeyFrameSingleValue();

    public:
        virtual T getLerpValue(const AnimationKeyFrameSingleValue<T>* right, float percent) const = 0;
        
    public:
        KeyFrameType getKeyFrameType() const;
    protected:
        KeyFrameType _type;
        
    public:
        void setValue(const T& value);
        const T& getValue() const;
    private:
        T _value;
    };

    template<typename T>
    class AnimationKeyFrameArray
    : public AnimationKeyFrame
    {
    public:
        AnimationKeyFrameArray();

    public:
        void setArraySize(int32_t arraySize);
        int32_t getArraySize() const;
    private:
        int32_t _arraySize;

    public:
        virtual void getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float percent) const = 0;
        virtual void getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float percent) const = 0;

    public:
        KeyFrameType getKeyFrameType() const;
    protected:
        KeyFrameType _type;

    public:
        T* getArrayPointer() const;
    private:
        T* _array;
    };

    template<typename T>
    class AnimationKeyFrameRangeValue
    : public AnimationKeyFrame
    {
    public:
        AnimationKeyFrameRangeValue();

    public:
        virtual T getLerpValue(const T& left, const T& right, float percent) const = 0;

    public:
        KeyFrameType getKeyFrameType() const;
    protected:
        KeyFrameType _type;
        
    public:
        void setValue0(const T& value0);
        void setValue1(const T& value1);
        const T& getValue0() const;
        const T& getValue1() const;
    private:
        T _value0;
        T _value1;

    public:
        T generateKeyValueToAnimationState() const;
    };

    template<typename T>
    class AnimationKeyFrame_Linear
    : public AnimationKeyFrameSingleValue<T>
    {
    public:
        AnimationKeyFrame_Linear();

    public:
        virtual T getLerpValue(const AnimationKeyFrameSingleValue<T>* right, float percent) const override;
    };

    template<typename T>
    class AnimationKeyFrame_Stepped
    : public AnimationKeyFrameSingleValue<T>
    {
    public:
        AnimationKeyFrame_Stepped();
        
    public:
        virtual T getLerpValue(const AnimationKeyFrameSingleValue<T>* right, float percent) const override;
    };

    template<typename T>
    class AnimationKeyFrame_SpineBezierToTenLinearFake
    : public AnimationKeyFrameSingleValue<T>
    {
    public:
        static const int s_bezierPointCount = 9;
        
    public:
        AnimationKeyFrame_SpineBezierToTenLinearFake();

    public:
        virtual T getLerpValue(const AnimationKeyFrameSingleValue<T>* right, float percent) const override;

    public:
        void createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2);
    private:
        float _x[s_bezierPointCount];
        float _y[s_bezierPointCount];
    };

    class AnimationKeyFrame_Rotation_Linear
    : public AnimationKeyFrameSingleValue<float>
    {
    public:
        AnimationKeyFrame_Rotation_Linear();

    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Rotation_Linear"; }

    public:
        static float _getLerpValue(float left, float right, float percent);
            
    public:
        virtual float getLerpValue(const AnimationKeyFrameSingleValue<float>* right, float percent) const override;
    };
        
    class AnimationKeyFrame_Rotation_Stepped
    : public AnimationKeyFrameSingleValue<float>
    {
    public:
        AnimationKeyFrame_Rotation_Stepped();
            
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Rotation_Stepped"; }

    public:
        virtual float getLerpValue(const AnimationKeyFrameSingleValue<float>* right, float percent) const override;
    };
        
    class AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake
    : public AnimationKeyFrameSingleValue<float>
    {
    public:
        static const int s_bezierPointCount = 9;
            
    public:
        AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake();
            
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Rotation_SpineBezierToTenLinearFake"; }

    public:
        virtual float getLerpValue(const AnimationKeyFrameSingleValue<float>* right, float percent) const override;
            
    public:
        void createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2);
    private:
        float _x[s_bezierPointCount];
        float _y[s_bezierPointCount];
    };

    template<typename T>
    class AnimationKeyFrameArray_Linear
    : public AnimationKeyFrameArray<T>
    {
    public:
        AnimationKeyFrameArray_Linear();
        
    public:
        virtual void getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float percent) const override;
        virtual void getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float percent) const override;
    };

    template<typename T>
    class AnimationKeyFrameArray_Stepped
    : public AnimationKeyFrameArray<T>
    {
    public:
        AnimationKeyFrameArray_Stepped();
        
    public:
        virtual void getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float percent) const override;
        virtual void getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float percent) const override;
    };

    template<typename T>
    class AnimationKeyFrameArray_SpineBezierToTenLinearFake
    : public AnimationKeyFrameArray<T>
    {
    public:
        static const int s_bezierPointCount = 9;

    public:
        AnimationKeyFrameArray_SpineBezierToTenLinearFake();
        
    public:
        virtual void getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float percent) const override;
        virtual void getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float percent) const override;

    public:
        void createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2);
    private:
        float _x[s_bezierPointCount];
        float _y[s_bezierPointCount];
    };

    template<typename T>
    class AnimationKeyFrameRange_Linear
    : public AnimationKeyFrameRangeValue<T>
    {
    public:
        AnimationKeyFrameRange_Linear();

    public:
        virtual T getLerpValue(const T& left, const T& right, float percent) const override;
    };

    template<typename T>
    class AnimationKeyFrameRange_Stepped
    : public AnimationKeyFrameRangeValue<T>
    {
    public:
        AnimationKeyFrameRange_Stepped();

    public:
        virtual T getLerpValue(const T& left, const T& right, float percent) const override;
    };

    template<typename T>
    class AnimationKeyFrameRange_SpineBezierToTenLinearFake
    : public AnimationKeyFrameRangeValue<T>
    {
    public:
        static const int s_bezierPointCount = 9;

    public:
        AnimationKeyFrameRange_SpineBezierToTenLinearFake();

    public:
        virtual T getLerpValue(const T& left, const T& right, float percent) const override;

    public:
        void createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2);
    private:
        float _x[s_bezierPointCount];
        float _y[s_bezierPointCount];
    };

    // float
    class AnimationKeyFrame_Float_Stepped
    : public AnimationKeyFrame_Stepped<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Float_Stepped"; }
    };
    
    class AnimationKeyFrame_Float_Linear
    : public AnimationKeyFrame_Linear<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Float_Linear"; }
    };
    
    class AnimationKeyFrame_Float_SpineBezierToTenLinearFake
    : public AnimationKeyFrame_SpineBezierToTenLinearFake<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Float_SpineBezierToTenLinearFake"; }
    };

    class AnimationKeyFrameArray_Float_Stepped
    : public AnimationKeyFrameArray_Stepped<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Float_Stepped"; }
    };
        
    class AnimationKeyFrameArray_Float_Linear
    : public AnimationKeyFrameArray_Linear<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Float_Linear"; }
    };
        
    class AnimationKeyFrameArray_Float_SpineBezierToTenLinearFake
    : public AnimationKeyFrameArray_SpineBezierToTenLinearFake<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Float_SpineBezierToTenLinearFake"; }
    };

    class AnimationKeyFrameRange_Float_Stepped
    : public AnimationKeyFrameRange_Stepped<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Float_Stepped"; }
    };
        
    class AnimationKeyFrameRange_Float_Linear
    : public AnimationKeyFrameRange_Linear<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Float_Linear"; }
    };
        
    class AnimationKeyFrameRange_Float_SpineBezierToTenLinearFake
    : public AnimationKeyFrameRange_SpineBezierToTenLinearFake<float>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Float_SpineBezierToTenLinearFake"; }
    };

        
    
    // Vector2
    
    class AnimationKeyFrame_Vector2_Stepped
    : public AnimationKeyFrame_Stepped<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector2_Stepped"; }
    };
    
    class AnimationKeyFrame_Vector2_Linear
    : public AnimationKeyFrame_Linear<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector2_Linear"; }
    };
    
    class AnimationKeyFrame_Vector2_SpineBezierToTenLinearFake
    : public AnimationKeyFrame_SpineBezierToTenLinearFake<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector2_SpineBezierToTenLinearFake"; }
    };
        
    class AnimationKeyFrameArray_Vector2_Stepped
    : public AnimationKeyFrameArray_Stepped<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector2_Stepped"; }
    };
        
    class AnimationKeyFrameArray_Vector2_Linear
    : public AnimationKeyFrameArray_Linear<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector2_Linear"; }
    };
        
    class AnimationKeyFrameArray_Vector2_SpineBezierToTenLinearFake
    : public AnimationKeyFrameArray_SpineBezierToTenLinearFake<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector2_SpineBezierToTenLinearFake"; }
    };
    
    class AnimationKeyFrameRange_Vector2_Stepped
    : public AnimationKeyFrameRange_Stepped<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector2_Stepped"; }
    };
        
    class AnimationKeyFrameRange_Vector2_Linear
    : public AnimationKeyFrameRange_Linear<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector2_Linear"; }
    };
        
    class AnimationKeyFrameRange_Vector2_SpineBezierToTenLinearFake
    : public AnimationKeyFrameRange_SpineBezierToTenLinearFake<Vector2>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector2_SpineBezierToTenLinearFake"; }
    };



    // Vector3
    
    class AnimationKeyFrame_Vector3_Stepped
    : public AnimationKeyFrame_Stepped<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector3_Stepped"; }
    };
    
    class AnimationKeyFrame_Vector3_Linear
    : public AnimationKeyFrame_Linear<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector3_Linear"; }
    };
    
    class AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake
    : public AnimationKeyFrame_SpineBezierToTenLinearFake<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector3_SpineBezierToTenLinearFake"; }
    };
    
    class AnimationKeyFrameArray_Vector3_Stepped
    : public AnimationKeyFrameArray_Stepped<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector3_Stepped"; }
    };
        
    class AnimationKeyFrameArray_Vector3_Linear
    : public AnimationKeyFrameArray_Linear<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector3_Linear"; }
    };
        
    class AnimationKeyFrameArray_Vector3_SpineBezierToTenLinearFake
    : public AnimationKeyFrameArray_SpineBezierToTenLinearFake<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector3_SpineBezierToTenLinearFake"; }
    };

    class AnimationKeyFrameRange_Vector3_Stepped
    : public AnimationKeyFrameRange_Stepped<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector3_Stepped"; }
    };

    class AnimationKeyFrameRange_Vector3_Linear
    : public AnimationKeyFrameRange_Linear<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector3_Linear"; }
    };

    class AnimationKeyFrameRange_Vector3_SpineBezierToTenLinearFake
    : public AnimationKeyFrameRange_SpineBezierToTenLinearFake<Vector3>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector3_SpineBezierToTenLinearFake"; }
    };



    // Vector4
    
    class AnimationKeyFrame_Vector4_Stepped
    : public AnimationKeyFrame_Stepped<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector4_Stepped"; }
    };
    
    class AnimationKeyFrame_Vector4_Linear
    : public AnimationKeyFrame_Linear<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector4_Linear"; }
    };
    
    class AnimationKeyFrame_Vector4_SpineBezierToTenLinearFake
    : public AnimationKeyFrame_SpineBezierToTenLinearFake<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Vector4_SpineBezierToTenLinearFake"; }
    };

    class AnimationKeyFrameArray_Vector4_Stepped
    : public AnimationKeyFrameArray_Stepped<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector4_Stepped"; }
    };

    class AnimationKeyFrameArray_Vector4_Linear
    : public AnimationKeyFrameArray_Linear<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector4_Linear"; }
    };

    class AnimationKeyFrameArray_Vector4_SpineBezierToTenLinearFake
    : public AnimationKeyFrameArray_SpineBezierToTenLinearFake<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Vector4_SpineBezierToTenLinearFake"; }
    };

    class AnimationKeyFrameRange_Vector4_Stepped
    : public AnimationKeyFrameRange_Stepped<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector4_Stepped"; }
    };

    class AnimationKeyFrameRange_Vector4_Linear
    : public AnimationKeyFrameRange_Linear<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector4_Linear"; }
    };

    class AnimationKeyFrameRange_Vector4_SpineBezierToTenLinearFake
    : public AnimationKeyFrameRange_SpineBezierToTenLinearFake<Vector4>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_Vector4_SpineBezierToTenLinearFake"; }
    };

    
    
    // ColourValue
    
    class AnimationKeyFrame_ColourValue_Stepped
    : public AnimationKeyFrame_Stepped<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_ColourValue_Stepped"; }
    };

    class AnimationKeyFrame_ColourValue_Linear
    : public AnimationKeyFrame_Linear<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_ColourValue_Linear"; }
    };

    class AnimationKeyFrame_ColourValue_SpineBezierToTenLinearFake
    : public AnimationKeyFrame_SpineBezierToTenLinearFake<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_ColourValue_SpineBezierToTenLinearFake"; }
    };
    
    class AnimationKeyFrameArray_ColourValue_Stepped
    : public AnimationKeyFrameArray_Stepped<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_ColourValue_Stepped"; }
    };
        
    class AnimationKeyFrameArray_ColourValue_Linear
    : public AnimationKeyFrameArray_Linear<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_ColourValue_Linear"; }
    };
        
    class AnimationKeyFrameArray_ColourValue_SpineBezierToTenLinearFake
    : public AnimationKeyFrameArray_SpineBezierToTenLinearFake<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_ColourValue_SpineBezierToTenLinearFake"; }
    };
        
    class AnimationKeyFrameRange_ColourValue_Stepped
    : public AnimationKeyFrameRange_Stepped<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_ColourValue_Stepped"; }
    };
        
    class AnimationKeyFrameRange_ColourValue_Linear
    : public AnimationKeyFrameRange_Linear<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_ColourValue_Linear"; }
    };
        
    class AnimationKeyFrameRange_ColourValue_SpineBezierToTenLinearFake
    : public AnimationKeyFrameRange_SpineBezierToTenLinearFake<ColourValue>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameRange_ColourValue_SpineBezierToTenLinearFake"; }
    };

        
    
    // boolean
        
    class AnimationKeyFrame_Boolean_Stepped
    : public AnimationKeyFrame_Stepped<bool>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_Boolean_Stepped"; }
    };
        
    class AnimationKeyFrameArray_Boolean_Stepped
    : public AnimationKeyFrameArray_Stepped<bool>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_Boolean_Stepped"; }
    };

        
        
    // string
    
    class AnimationKeyFrame_String_Stepped
    : public AnimationKeyFrame_Stepped<std::string>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_String_Stepped"; }
    };

    class AnimationKeyFrameArray_String_Stepped
    : public AnimationKeyFrameArray_Stepped<std::string>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_String_Stepped"; }
    };

    
    
    // std::map<std::string, float>
    
    class AnimationKeyFrame_MapStringToFloat_Stepped
    : public AnimationKeyFrame_Stepped<std::map<std::string, float>>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrame_MapStringToFloat_Stepped"; }
    };

    class AnimationKeyFrameArray_MapStringToFloat_Stepped
    : public AnimationKeyFrameArray_Stepped<std::map<std::string, float>>
    {
    public:
        virtual const char* getTypeName() const { return "AnimationKeyFrameArray_MapStringToFloat_Stepped"; }
    };



    template<typename T>
    AnimationKeyFrameSingleValue<T>::AnimationKeyFrameSingleValue()
    {
        _type = KeyFrameType::COUNT;
    }

    template<typename T>
    KeyFrameType AnimationKeyFrameSingleValue<T>::getKeyFrameType() const
    {
        return _type;
    }
    
    template<typename T>
    void AnimationKeyFrameSingleValue<T>::setValue(const T& value)
    {
        _value = value;
    }
    
    template<typename T>
    const T& AnimationKeyFrameSingleValue<T>::getValue() const
    {
        return _value;
    }

    
    
    
    
    
    
    
    template<typename T>
    AnimationKeyFrameArray<T>::AnimationKeyFrameArray()
    {
        _type = KeyFrameType::COUNT;

        _arraySize = 0;
        
        _array = nullptr;
    }

    template<typename T>
    void AnimationKeyFrameArray<T>::setArraySize(int32_t arraySize)
    {
        if (_arraySize != arraySize)
        {
            if (_array)
            {
                delete [] _array;
                _array = nullptr;
            }

            _arraySize = arraySize;

            if (_arraySize > 0)
            {
                _array = new T[_arraySize];
            }
        }
    }

    template<typename T>
    int32_t AnimationKeyFrameArray<T>::getArraySize() const
    {
        return _arraySize;
    }

    template<typename T>
    KeyFrameType AnimationKeyFrameArray<T>::getKeyFrameType() const
    {
        return _type;
    }

    template<typename T>
    T* AnimationKeyFrameArray<T>::getArrayPointer() const
    {
        return _array;
    }
    
    
        
        
        
        
    template<typename T>
    AnimationKeyFrameRangeValue<T>::AnimationKeyFrameRangeValue()
    {
        _type = KeyFrameType::COUNT;
    }
        
    template<typename T>
    KeyFrameType AnimationKeyFrameRangeValue<T>::getKeyFrameType() const
    {
        return _type;
    }
        
    template<typename T>
    void AnimationKeyFrameRangeValue<T>::setValue0(const T& value0)
    {
        _value0 = value0;
    }

    template<typename T>
    void AnimationKeyFrameRangeValue<T>::setValue1(const T& value1)
    {
        _value1 = value1;
    }

    template<typename T>
    const T& AnimationKeyFrameRangeValue<T>::getValue0() const
    {
        return _value0;
    }

    template<typename T>
    const T& AnimationKeyFrameRangeValue<T>::getValue1() const
    {
        return _value1;
    }

    template<typename T>
    T AnimationKeyFrameRangeValue<T>::generateKeyValueToAnimationState() const
    {
        float ratio = RandomGenerator::frand(0.0f, 1.0f);
        return lerp(_value0, _value1, ratio);
    }

    
    
    
    
    
    
    
    
    
    template<typename T>
    AnimationKeyFrame_Linear<T>::AnimationKeyFrame_Linear()
    {
        AnimationKeyFrameSingleValue<T>::_type = KeyFrameType::Linear;
    }

    template<typename T>
    T AnimationKeyFrame_Linear<T>::getLerpValue(const AnimationKeyFrameSingleValue<T>* right, float percent) const
    {
        return lerp(AnimationKeyFrameSingleValue<T>::getValue(), right->getValue(), percent);
    }
    
    
    
    
    
    template<typename T>
    AnimationKeyFrame_Stepped<T>::AnimationKeyFrame_Stepped()
    {
        AnimationKeyFrameSingleValue<T>::_type = KeyFrameType::Stepped;
    }

    template<typename T>
    T AnimationKeyFrame_Stepped<T>::getLerpValue(const AnimationKeyFrameSingleValue<T>* /*right*/, float /*percent*/) const
    {
        return AnimationKeyFrameSingleValue<T>::getValue();
    }

    
    
    
    
    template<typename T>
    AnimationKeyFrame_SpineBezierToTenLinearFake<T>::AnimationKeyFrame_SpineBezierToTenLinearFake()
    {
        AnimationKeyFrameSingleValue<T>::_type = KeyFrameType::SpineBezierToTenLinearFake;
    }

    template<typename T>
    T AnimationKeyFrame_SpineBezierToTenLinearFake<T>::getLerpValue(const AnimationKeyFrameSingleValue<T>* right, float percent) const
    {
        for (int32_t i=0 ; i<s_bezierPointCount ; i++)
        {
            if (_x[i] >= percent)
            {
                float prevX, prevY;

                if (i == 0)
                {
                    prevX = 0.0f;
                    prevY = 0.0f;
                }
                else
                {
                    prevX = _x[i-1];
                    prevY = _y[i-1];
                }

                float ratio = prevY + (_y[i] - prevY) * (percent - prevX) / (_x[i] - prevX);

                return lerp(AnimationKeyFrameSingleValue<T>::getValue(), right->getValue(), ratio);
            }
        }

        float x = _x[s_bezierPointCount-1];
        float y = _y[s_bezierPointCount-1];

        float ratio = y + (1.0f - y) * (percent - x) / (1.0f - x);

        return lerp(AnimationKeyFrameSingleValue<T>::getValue(), right->getValue(), ratio);
    }
    
    template<typename T>
    void AnimationKeyFrame_SpineBezierToTenLinearFake<T>::createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2)
    {
        static const int BEZIER_SEGMENTS = 10;
        
        float subdiv1 = 1.0f / BEZIER_SEGMENTS, subdiv2 = subdiv1 * subdiv1, subdiv3 = subdiv2 * subdiv1;
        float pre1 = 3 * subdiv1, pre2 = 3 * subdiv2, pre4 = 6 * subdiv2, pre5 = 6 * subdiv3;
        float tmp1x = -cx1 * 2 + cx2, tmp1y = -cy1 * 2 + cy2, tmp2x = (cx1 - cx2) * 3 + 1, tmp2y = (cy1 - cy2) * 3 + 1;
        float dfx = cx1 * pre1 + tmp1x * pre2 + tmp2x * subdiv3, dfy = cy1 * pre1 + tmp1y * pre2 + tmp2y * subdiv3;
        float ddfx = tmp1x * pre4 + tmp2x * pre5, ddfy = tmp1y * pre4 + tmp2y * pre5;
        float dddfx = tmp2x * pre5, dddfy = tmp2y * pre5;
        float x = dfx, y = dfy;
        
        for (auto i=0 ; i < s_bezierPointCount ; i ++)
        {
            _x[i] = x;
            _y[i] = y;
            
            dfx += ddfx;
            dfy += ddfy;
            ddfx += dddfx;
            ddfy += dddfy;
            x += dfx;
            y += dfy;
        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    template<typename T>
    AnimationKeyFrameArray_Linear<T>::AnimationKeyFrameArray_Linear()
    : AnimationKeyFrameArray<T>()
    {
        AnimationKeyFrameArray<T>::_type = KeyFrameType::Linear;
    }

    template<typename T>
    void AnimationKeyFrameArray_Linear<T>::getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float percent) const
    {
        assert(AnimationKeyFrameArray<T>::getArraySize() == target->getArraySize());
        assert(AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize());

        if (AnimationKeyFrameArray<T>::getArraySize() == target->getArraySize() &&
            AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize())
        {
            auto array_size = AnimationKeyFrameArray<T>::getArraySize();
            
            auto t = target->getArrayPointer();
            auto r = right->getArrayPointer();
            auto s = AnimationKeyFrameArray<T>::getArrayPointer();
            
            for (auto i=0 ; i<array_size ; i++)
            {
                *(t + i) = lerp(*(s + i), *(r + i), percent);
            }
        }
    }
    
    template<typename T>
    void AnimationKeyFrameArray_Linear<T>::getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float percent) const
    {
        assert(AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize());
        assert(setter);
        
        if (AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize())
        {
            auto array_size = AnimationKeyFrameArray<T>::getArraySize();

            auto r = right->getArrayPointer();
            auto s = AnimationKeyFrameArray<T>::getArrayPointer();
            
            for (auto i=0 ; i<array_size ; i++)
            {
                setter(i, lerp(*(s + i), *(r + i), percent));
            }
        }
    }

    
    
    
    
    template<typename T>
    AnimationKeyFrameArray_Stepped<T>::AnimationKeyFrameArray_Stepped()
    : AnimationKeyFrameArray<T>()
    {
        AnimationKeyFrameArray<T>::_type = KeyFrameType::Stepped;
    }

    template<typename T>
    void AnimationKeyFrameArray_Stepped<T>::getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float /*percent*/) const
    {
        assert(AnimationKeyFrameArray<T>::getArraySize() == target->getArraySize());
        assert(AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize());
        
        if (AnimationKeyFrameArray<T>::getArraySize() == target->getArraySize() &&
            AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize())
        {
            memcpy(target->getArrayPointer(), AnimationKeyFrameArray<T>::getArrayPointer(), sizeof(T) * AnimationKeyFrameArray<T>::getArraySize());
        }
    }
    
    template<typename T>
    void AnimationKeyFrameArray_Stepped<T>::getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float /*percent*/) const
    {
        assert(AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize());
        assert(setter);
        
        if (AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize())
        {
            auto array_size = AnimationKeyFrameArray<T>::getArraySize();

            auto s = AnimationKeyFrameArray<T>::getArrayPointer();
            
            for (auto i=0 ; i<array_size ; i++)
            {
                setter(i, *(s + i));
            }
        }
    }

    
    
    
    
    template<typename T>
    AnimationKeyFrameArray_SpineBezierToTenLinearFake<T>::AnimationKeyFrameArray_SpineBezierToTenLinearFake()
    : AnimationKeyFrameArray<T>()
    {
        AnimationKeyFrameArray<T>::_type = KeyFrameType::SpineBezierToTenLinearFake;
    }
        
    template<typename T>
    void AnimationKeyFrameArray_SpineBezierToTenLinearFake<T>::getLerpArray(AnimationKeyFrameArray<T>* target, const AnimationKeyFrameArray<T>* right, float percent) const
    {
        assert(AnimationKeyFrameArray<T>::getArraySize() == target->getArraySize());
        assert(AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize());

        if (AnimationKeyFrameArray<T>::getArraySize() == target->getArraySize() &&
            AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize())
        {
            auto array_size = AnimationKeyFrameArray<T>::getArraySize();
            
            auto t = target->getArrayPointer();
            auto r = right->getArrayPointer();
            auto s = AnimationKeyFrameArray<T>::getArrayPointer();

            for (int32_t i=0 ; i<s_bezierPointCount ; i++)
            {
                if (_x[i] >= percent)
                {
                    float prevX, prevY;
                    
                    if (i == 0)
                    {
                        prevX = 0.0f;
                        prevY = 0.0f;
                    }
                    else
                    {
                        prevX = _x[i-1];
                        prevY = _y[i-1];
                    }
                    
                    float ratio = prevY + (_y[i] - prevY) * (percent - prevX) / (_x[i] - prevX);
                    
                    for (auto i=0 ; i<array_size ; i++)
                    {
                        *(t + i) = lerp(*(s + i), *(r + i), ratio);
                    }
                    
                    return;
                }
            }
            
            float x = _x[s_bezierPointCount-1];
            float y = _y[s_bezierPointCount-1];
            
            float ratio = y + (1.0f - y) * (percent - x) / (1.0f - x);
            
            for (auto i=0 ; i<array_size ; i++)
            {
                *(t + i) = lerp(*(s + i), *(r + i), ratio);
            }
            
            return;
        }
    }
    
    template<typename T>
    void AnimationKeyFrameArray_SpineBezierToTenLinearFake<T>::getLerpWithSetter(const std::function<void(int32_t, T)>& setter, const AnimationKeyFrameArray<T>* right, float percent) const
    {
        assert(AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize());
        assert(setter);
        
        if (AnimationKeyFrameArray<T>::getArraySize() == right->getArraySize())
        {
            auto array_size = AnimationKeyFrameArray<T>::getArraySize();
            
            auto r = right->getArrayPointer();
            auto s = AnimationKeyFrameArray<T>::getArrayPointer();

            for (int32_t i=0 ; i<s_bezierPointCount ; i++)
            {
                if (_x[i] >= percent)
                {
                    float prevX, prevY;
                    
                    if (i == 0)
                    {
                        prevX = 0.0f;
                        prevY = 0.0f;
                    }
                    else
                    {
                        prevX = _x[i-1];
                        prevY = _y[i-1];
                    }
                    
                    float ratio = prevY + (_y[i] - prevY) * (percent - prevX) / (_x[i] - prevX);
                    
                    for (auto i=0 ; i<array_size ; i++)
                    {
                        setter(i, lerp(*(s + i), *(r + i), ratio));
                    }
                    
                    return;
                }
            }
            
            float x = _x[s_bezierPointCount-1];
            float y = _y[s_bezierPointCount-1];
            
            float ratio = y + (1.0f - y) * (percent - x) / (1.0f - x);
            
            for (auto i=0 ; i<array_size ; i++)
            {
                setter(i, lerp(*(s + i), *(r + i), ratio));
            }
            
            return;
        }
    }
    
    template<typename T>
    void AnimationKeyFrameArray_SpineBezierToTenLinearFake<T>::createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2)
    {
        static const int BEZIER_SEGMENTS = 10;
        
        float subdiv1 = 1.0f / BEZIER_SEGMENTS, subdiv2 = subdiv1 * subdiv1, subdiv3 = subdiv2 * subdiv1;
        float pre1 = 3 * subdiv1, pre2 = 3 * subdiv2, pre4 = 6 * subdiv2, pre5 = 6 * subdiv3;
        float tmp1x = -cx1 * 2 + cx2, tmp1y = -cy1 * 2 + cy2, tmp2x = (cx1 - cx2) * 3 + 1, tmp2y = (cy1 - cy2) * 3 + 1;
        float dfx = cx1 * pre1 + tmp1x * pre2 + tmp2x * subdiv3, dfy = cy1 * pre1 + tmp1y * pre2 + tmp2y * subdiv3;
        float ddfx = tmp1x * pre4 + tmp2x * pre5, ddfy = tmp1y * pre4 + tmp2y * pre5;
        float dddfx = tmp2x * pre5, dddfy = tmp2y * pre5;
        float x = dfx, y = dfy;
        
        for (auto i=0 ; i < s_bezierPointCount ; i ++)
        {
            _x[i] = x;
            _y[i] = y;
            
            dfx += ddfx;
            dfy += ddfy;
            ddfx += dddfx;
            ddfy += dddfy;
            x += dfx;
            y += dfy;
        }
    }
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        template<typename T>
        AnimationKeyFrameRange_Linear<T>::AnimationKeyFrameRange_Linear()
        {
            AnimationKeyFrameRangeValue<T>::_type = KeyFrameType::Linear;
        }
        
        template<typename T>
        T AnimationKeyFrameRange_Linear<T>::getLerpValue(const T& left, const T& right, float percent) const
        {
            return lerp(left, right, percent);
        }
        
        
        
        
        
        
        template<typename T>
        AnimationKeyFrameRange_Stepped<T>::AnimationKeyFrameRange_Stepped()
        {
            AnimationKeyFrameRangeValue<T>::_type = KeyFrameType::Stepped;
        }
        
        template<typename T>
        T AnimationKeyFrameRange_Stepped<T>::getLerpValue(const T& left, const T& /*right*/, float /*percent*/) const
        {
            return left;
        }
        
        
        
        
        
        template<typename T>
        AnimationKeyFrameRange_SpineBezierToTenLinearFake<T>::AnimationKeyFrameRange_SpineBezierToTenLinearFake()
        {
            AnimationKeyFrameRangeValue<T>::_type = KeyFrameType::SpineBezierToTenLinearFake;
        }
        
        template<typename T>
        T AnimationKeyFrameRange_SpineBezierToTenLinearFake<T>::getLerpValue(const T& left, const T& right, float percent) const
        {
            for (int32_t i=0 ; i<s_bezierPointCount ; i++)
            {
                if (_x[i] >= percent)
                {
                    float prevX, prevY;
                    
                    if (i == 0)
                    {
                        prevX = 0.0f;
                        prevY = 0.0f;
                    }
                    else
                    {
                        prevX = _x[i-1];
                        prevY = _y[i-1];
                    }
                    
                    float ratio = prevY + (_y[i] - prevY) * (percent - prevX) / (_x[i] - prevX);
                    
                    return lerp(left, right, ratio);
                }
            }
            
            float x = _x[s_bezierPointCount-1];
            float y = _y[s_bezierPointCount-1];
            
            float ratio = y + (1.0f - y) * (percent - x) / (1.0f - x);
            
            return lerp(left, right, ratio);
        }
        
        template<typename T>
        void AnimationKeyFrameRange_SpineBezierToTenLinearFake<T>::createSpineFakeLinears(float cx1, float cy1, float cx2, float cy2)
        {
            static const int BEZIER_SEGMENTS = 10;
            
            float subdiv1 = 1.0f / BEZIER_SEGMENTS, subdiv2 = subdiv1 * subdiv1, subdiv3 = subdiv2 * subdiv1;
            float pre1 = 3 * subdiv1, pre2 = 3 * subdiv2, pre4 = 6 * subdiv2, pre5 = 6 * subdiv3;
            float tmp1x = -cx1 * 2 + cx2, tmp1y = -cy1 * 2 + cy2, tmp2x = (cx1 - cx2) * 3 + 1, tmp2y = (cy1 - cy2) * 3 + 1;
            float dfx = cx1 * pre1 + tmp1x * pre2 + tmp2x * subdiv3, dfy = cy1 * pre1 + tmp1y * pre2 + tmp2y * subdiv3;
            float ddfx = tmp1x * pre4 + tmp2x * pre5, ddfy = tmp1y * pre4 + tmp2y * pre5;
            float dddfx = tmp2x * pre5, dddfy = tmp2y * pre5;
            float x = dfx, y = dfy;
            
            for (auto i=0 ; i < s_bezierPointCount ; i ++)
            {
                _x[i] = x;
                _y[i] = y;
                
                dfx += ddfx;
                dfy += ddfy;
                ddfx += dddfx;
                ddfy += dddfy;
                x += dfx;
                y += dfy;
            }
        }

}
