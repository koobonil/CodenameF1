#pragma once

#include "zay_types.h"
#include "zay_animation_keyframe.h"
#include "zay_animatable.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_vector4.h"
#include "zay_colour_value.h"

namespace ZAY
{
    class AnimationTrack
    : public Base
    {
    protected:
        AnimationTrack();
    public:
        virtual ~AnimationTrack();

    public:
        static void registerAnimationTrackType(const std::string& typeName, const std::function<AnimationTrack*()>& creator);
        static void unregisterAnimationTrackType(const std::string& typeName);
        static AnimationTrack* createAnimationTrack(const std::string& typeName);
        static void initAnimationTrackCreators();

        template<typename T>
        static T createAnimationTrackWithTypeCasting(const std::string& typeName)
        {
            auto track = createAnimationTrack(typeName);
            assert(dynamic_cast<T>(track));
            return static_cast<T>(track);
        }

    private:
        static std::map<std::string, std::function<AnimationTrack*()>> s_animationTrackCreators;

    public:
        std::function<const std::string&()> getTypeName;

    public:
        std::string boneName;
        std::string ikName;

    public:
        virtual void applyToAnimatable(Animatable* animatable, float lasttime, float curtime) = 0;
        virtual float getLastKeyFrameTime() const = 0;
    };

    template<typename T>
    class AnimationTrackTemplate
    : public AnimationTrack
    {
    public:
        AnimationTrackTemplate();
        virtual ~AnimationTrackTemplate();

    public:
        void setDefaultValue(const T& value);
    private:
        T _default;

    public:
        virtual void applyToAnimatable(Animatable* animatable, float lasttime, float curtime) override;
        virtual float getLastKeyFrameTime() const override;

    public:
        void setKeyFrame(float keyframe, AnimationKeyFrameSingleValue<T>* value);
    private:
        std::map<float, AnimationKeyFrameSingleValue<T>*> _values;

    public:
        void setSetter(const std::function<void(const AnimationTrack*, Animatable*, const T&)>& setter);
        void setGetter(const std::function<T(const AnimationTrack*, Animatable*)>& getter);

    protected:
        void set(Animatable* animatable, const T& value) const;
        T get(Animatable* animatable) const;

    private:
        std::function<void(const AnimationTrack*, Animatable*, const T&)> _setter;
        std::function<T(const AnimationTrack*, Animatable*)> _getter;
    };

    template<typename T>
    class AnimationTrackArrayTemplate
    : public AnimationTrack
    {
    public:
        AnimationTrackArrayTemplate();
        virtual ~AnimationTrackArrayTemplate();

    public:
        void setArraySize(int32_t arraySize);
        int32_t getArraySize() const;
    private:
        int32_t _arraySize;

    public:
        void setDefaultValue(const T& value);
    private:
        T _default;

    public:
        virtual void applyToAnimatable(Animatable* animatable, float lasttime, float curtime) override;
        virtual float getLastKeyFrameTime() const override;

    public:
        void setKeyFrame(float keyframe, AnimationKeyFrameArray<T>* value);
    private:
        std::map<float, AnimationKeyFrameArray<T>*> _values;

    public:
        void setSetter(const std::function<void(const AnimationTrack*, Animatable*, int32_t, const T&)>& setter);
        void setGetter(const std::function<T(const AnimationTrack*, Animatable*, int32_t)>& getter);

    private:
        std::function<void(const AnimationTrack*, Animatable*, int32_t, const T&)> _setter;
        std::function<T(const AnimationTrack*, Animatable*, int32_t)> _getter;
    };

    template<typename T>
    class AnimationTrackRangeTemplate
    : public AnimationTrack
    {
    public:
        AnimationTrackRangeTemplate();
        virtual ~AnimationTrackRangeTemplate();

    public:
        void setDefaultValue(const T& value);
    private:
        T _default;

    public:
        virtual void applyToAnimatable(Animatable* animatable, float lasttime, float curtime) override;
        virtual float getLastKeyFrameTime() const override;

    public:
        void setKeyFrame(float keyframe, AnimationKeyFrameRangeValue<T>* value);
    private:
        std::map<float, AnimationKeyFrameRangeValue<T>*> _values;

    public:
        void setSetter(const std::function<void(const AnimationTrack*, Animatable*, const T&)>& setter);
        void setGetter(const std::function<T(const AnimationTrack*, Animatable*)>& getter);

    protected:
        void set(Animatable* animatable, const T& value) const;
        T get(Animatable* animatable) const;

    private:
        std::function<void(const AnimationTrack*, Animatable*, const T&)> _setter;
        std::function<T(const AnimationTrack*, Animatable*)> _getter;
    };

    class AnimationTrackInteger
    : public AnimationTrackTemplate<int32_t>
    {
    public:
        AnimationTrackInteger()
        {
            setDefaultValue(0);
        }
    };

    class AnimationTrackFloat
    : public AnimationTrackTemplate<float>
    {
    public:
        AnimationTrackFloat()
        {
            setDefaultValue(0.0f);
        }
    };

    class AnimationTrackVector2
    : public AnimationTrackTemplate<Vector2>
    {
    public:
        AnimationTrackVector2()
        {
            setDefaultValue(Vector2::ZERO);
        }
    };

    class AnimationTrackVector3
    : public AnimationTrackTemplate<Vector3>
    {
    public:
        AnimationTrackVector3()
        {
            setDefaultValue(Vector3::ZERO);
        }
    };

    class AnimationTrackVector4
    : public AnimationTrackTemplate<Vector4>
    {
    public:
        AnimationTrackVector4()
        {
            setDefaultValue(Vector4::ZERO);
        }
    };

    class AnimationTrackColour
    : public AnimationTrackTemplate<ColourValue>
    {
    public:
        AnimationTrackColour()
        {
            setDefaultValue(ColourValue::ZERO);
        }
    };

    class AnimationTrackBoolean
    : public AnimationTrackTemplate<bool>
    {
    public:
        AnimationTrackBoolean()
        {
            setDefaultValue(false);
        }
    };

    class AnimationTrackString
    : public AnimationTrackTemplate<std::string>
    {
    public:
        AnimationTrackString()
        {
            setDefaultValue("");
        }
    };

    class AnimationTrackMapStringToFloat
    : public AnimationTrackTemplate<std::map<std::string, float>>
    {
    public:
        AnimationTrackMapStringToFloat()
        {
        }
    };

    class AnimationTrackIntegerArray
    : public AnimationTrackArrayTemplate<int32_t>
    {
    public:
        AnimationTrackIntegerArray()
        : AnimationTrackArrayTemplate<int32_t>()
        {
            setDefaultValue(0);
        }
    };

    class AnimationTrackFloatArray
    : public AnimationTrackArrayTemplate<float>
    {
    public:
        AnimationTrackFloatArray()
        : AnimationTrackArrayTemplate<float>()
        {
            setDefaultValue(0.0f);
        }
    };

    class AnimationTrackVector2Array
    : public AnimationTrackArrayTemplate<Vector2>
    {
    public:
        AnimationTrackVector2Array()
        : AnimationTrackArrayTemplate<Vector2>()
        {
            setDefaultValue(Vector2::ZERO);
        }
    };

    class AnimationTrackVector3Array
    : public AnimationTrackArrayTemplate<Vector3>
    {
    public:
        AnimationTrackVector3Array()
        : AnimationTrackArrayTemplate<Vector3>()
        {
            setDefaultValue(Vector3::ZERO);
        }
    };

    class AnimationTrackVector4Array
    : public AnimationTrackArrayTemplate<Vector4>
    {
    public:
        AnimationTrackVector4Array()
        : AnimationTrackArrayTemplate<Vector4>()
        {
            setDefaultValue(Vector4::ZERO);
        }
    };

    class AnimationTrackColourArray
    : public AnimationTrackArrayTemplate<ColourValue>
    {
    public:
        AnimationTrackColourArray()
        : AnimationTrackArrayTemplate<ColourValue>()
        {
            setDefaultValue(ColourValue::ZERO);
        }
    };

    class AnimationTrackBooleanArray
    : public AnimationTrackArrayTemplate<bool>
    {
    public:
        AnimationTrackBooleanArray()
        : AnimationTrackArrayTemplate<bool>()
        {
            setDefaultValue(false);
        }
    };

    class AnimationTrackStringArray
    : public AnimationTrackArrayTemplate<std::string>
    {
    public:
        AnimationTrackStringArray()
        : AnimationTrackArrayTemplate<std::string>()
        {
            setDefaultValue("");
        }
    };

    class AnimationTrackIntegerRange
    : public AnimationTrackRangeTemplate<int32_t>
    {
    public:
        AnimationTrackIntegerRange()
        {
            setDefaultValue(0);
        }
    };

    class AnimationTrackFloatRange
    : public AnimationTrackRangeTemplate<float>
    {
    public:
        AnimationTrackFloatRange()
        {
            setDefaultValue(0.0f);
        }
    };

    class AnimationTrackVector2Range
    : public AnimationTrackRangeTemplate<Vector2>
    {
    public:
        AnimationTrackVector2Range()
        {
            setDefaultValue(Vector2::ZERO);
        }
    };

    class AnimationTrackVector3Range
    : public AnimationTrackRangeTemplate<Vector3>
    {
    public:
        AnimationTrackVector3Range()
        {
            setDefaultValue(Vector3::ZERO);
        }
    };

    class AnimationTrackVector4Range
    : public AnimationTrackRangeTemplate<Vector4>
    {
    public:
        AnimationTrackVector4Range()
        {
            setDefaultValue(Vector4::ZERO);
        }
    };

    class AnimationTrackColourRange
    : public AnimationTrackRangeTemplate<ColourValue>
    {
    public:
        AnimationTrackColourRange()
        {
            setDefaultValue(ColourValue::ZERO);
        }
    };

    template<typename T>
    AnimationTrackTemplate<T>::AnimationTrackTemplate()
    {
    }

    template<typename T>
    AnimationTrackTemplate<T>::~AnimationTrackTemplate()
    {
        for (auto it : _values)
        {
            if(it.second)
                it.second->release();
        }
        _values.clear();
    }

    template<typename T>
    void AnimationTrackTemplate<T>::setDefaultValue(const T& value)
    {
        _default = value;
    }

    template<typename T>
    void AnimationTrackTemplate<T>::applyToAnimatable(Animatable* animatable, float lasttime, float curtime)
    {
        if (_values.size() > 0)
        {
            auto it = _values.upper_bound(curtime);
            if (it == _values.end())
            {
                --it;
                animatable->setEventPulse(lasttime == 0 || lasttime < it->first);

                if(_values.rbegin()->second == nullptr)
                    set(animatable, _default);
                else set(animatable, _values.rbegin()->second->getValue());
            }
            else if(it == _values.begin())
            {
                set(animatable, _default);
            }
            else
            {
                auto it_next = it;
                --it;
                animatable->setEventPulse(lasttime == 0 || lasttime < it->first);

                if(it->second == nullptr)
                    set(animatable, _default);
                else
                {
                    float ratio = (curtime - it->first) / (it_next->first - it->first);
                    set(animatable, it->second->getLerpValue(it_next->second, ratio));
                }
            }
        }
        else
        {
            set(animatable, _default);
        }
        animatable->setEventPulse(false);
    }

    template<typename T>
    float AnimationTrackTemplate<T>::getLastKeyFrameTime() const
    {
        if (_values.size() > 0)
        {
            return _values.rbegin()->first;
        }
        else
        {
            return 0.0f;
        }
    }

    template<typename T>
    void AnimationTrackTemplate<T>::setKeyFrame(float keyframe, AnimationKeyFrameSingleValue<T>* value)
    {
        auto it = _values.find(keyframe);

        if (it != _values.end())
        {
            if(it->second)
                it->second->release();
            _values.erase(it);
        }

        _values[keyframe] = value;
        if(value) value->retain();
    }

    template<typename T>
    void AnimationTrackTemplate<T>::setSetter(const std::function<void(const AnimationTrack*, Animatable*, const T&)>& setter)
    {
        _setter = setter;
    }

    template<typename T>
    void AnimationTrackTemplate<T>::setGetter(const std::function<T(const AnimationTrack*, Animatable*)>& getter)
    {
        _getter = getter;
    }

    template<typename T>
    void AnimationTrackTemplate<T>::set(Animatable* animatable, const T& value) const
    {
        if (_setter)
        {
            _setter(this, animatable, value);
        }
    }

    template<typename T>
    T AnimationTrackTemplate<T>::get(Animatable* animatable) const
    {
        if (_getter)
        {
            return _getter(this, animatable);
        }

        return _default;
    }

    template<typename T>
    AnimationTrackArrayTemplate<T>::AnimationTrackArrayTemplate()
    {
        _arraySize = 0;
    }

    template<typename T>
    AnimationTrackArrayTemplate<T>::~AnimationTrackArrayTemplate()
    {
        for (auto it : _values)
        {
            it.second->release();
        }
        _values.clear();
    }

    template<typename T>
    void AnimationTrackArrayTemplate<T>::setArraySize(int32_t arraySize)
    {
        if (_arraySize != arraySize)
        {
            for (auto it : _values)
            {
                it.second->release();
            }
            _values.clear();

            _arraySize = arraySize;
        }
    }

    template<typename T>
    int32_t AnimationTrackArrayTemplate<T>::getArraySize() const
    {
        return _arraySize;
    }

    template<typename T>
    void AnimationTrackArrayTemplate<T>::setDefaultValue(const T& value)
    {
        _default = value;
    }

    template<typename T>
    void AnimationTrackArrayTemplate<T>::applyToAnimatable(Animatable* animatable, float lasttime, float curtime)
    {
        if (_setter)
        {
            if (_values.size() > 0)
            {
                auto it = _values.upper_bound(curtime);

                if (it == _values.end())
                {
                    auto keyframe = _values.rbegin()->second;
                    auto p = keyframe->getArrayPointer();

                    for (auto i=0 ; i<_arraySize ; i++)
                    {
                        _setter(this, animatable, i, *(p+i));
                    }
                }
                else if (it == _values.begin())
                {
                    for (auto i=0 ; i<_arraySize ; i++)
                    {
                        _setter(this, animatable, i, _default);
                    }
                }
                else
                {
                    auto it_next = it;
                    --it;

                    float ratio = (curtime - it->first) / (it_next->first - it->first);

                    auto lerp_setter = [animatable, this](int32_t index, T value)
                    {
                        this->_setter(this, animatable, index, value);
                    };

                    it->second->getLerpWithSetter(lerp_setter, it_next->second, ratio);
                }
            }
            else
            {
                for (auto i=0 ; i<_arraySize ; i++)
                {
                    _setter(this, animatable, i, _default);
                }
            }
        }
    }

    template<typename T>
    float AnimationTrackArrayTemplate<T>::getLastKeyFrameTime() const
    {
        if (_values.size() > 0)
        {
            return _values.rbegin()->first;
        }
        else
        {
            return 0.0f;
        }
    }

    template<typename T>
    void AnimationTrackArrayTemplate<T>::setKeyFrame(float keyframe, AnimationKeyFrameArray<T>* value)
    {
        auto it = _values.find(keyframe);

        if (it != _values.end())
        {
            it->second->release();
            _values.erase(it);
        }

        if (value)
        {
            _values[keyframe] = value;
            value->retain();
        }
    }

    template<typename T>
    void AnimationTrackArrayTemplate<T>::setSetter(const std::function<void(const AnimationTrack*, Animatable*, int32_t, const T&)>& setter)
    {
        _setter = setter;
    }

    template<typename T>
    void AnimationTrackArrayTemplate<T>::setGetter(const std::function<T(const AnimationTrack*, Animatable*, int32_t)>& getter)
    {
        _getter = getter;
    }

    template<typename T>
    AnimationTrackRangeTemplate<T>::AnimationTrackRangeTemplate()
    {

    }

    template<typename T>
    AnimationTrackRangeTemplate<T>::~AnimationTrackRangeTemplate()
    {
        for (auto it : _values)
        {
            it.second->release();
        }
        _values.clear();
    }

    template<typename T>
    void AnimationTrackRangeTemplate<T>::setDefaultValue(const T& value)
    {
        _default = value;
    }

    template<typename T>
    void AnimationTrackRangeTemplate<T>::applyToAnimatable(Animatable* animatable, float lasttime, float curtime)
    {
        if (_values.size() > 0)
        {
            auto it = _values.upper_bound(curtime);

            if (it == _values.end())
            {
                auto keyframe = _values.rbegin()->second;

                T value;
                animatable->getKeyframeRangeData(this,
                                                 static_cast<int32_t>(_values.size()-1),
                                                 keyframe->getValue0(),
                                                 keyframe->getValue1(),
                                                 value);

                set(animatable, value);
            }
            else if (it == _values.begin())
            {
                set(animatable, _default);
            }
            else
            {
                auto it_next = it;
                --it;

                auto keyframe0 = it->second;
                auto keyframe1 = it_next->second;

                float ratio = (curtime - it->first) / (it_next->first - it->first);

                T value0, value1;

                animatable->getKeyframeRangeData(this,
                                                 0,
                                                 keyframe0->getValue0(),
                                                 keyframe0->getValue1(),
                                                 value0);

                animatable->getKeyframeRangeData(this,
                                                 0,
                                                 keyframe1->getValue0(),
                                                 keyframe1->getValue1(),
                                                 value1);

                set(animatable, keyframe0->getLerpValue(value0,
                                                        value1,
                                                        ratio));
            }
        }
        else
        {
            set(animatable, _default);
        }
    }

    template<typename T>
    float AnimationTrackRangeTemplate<T>::getLastKeyFrameTime() const
    {
        if (_values.size() > 0)
        {
            return _values.rbegin()->first;
        }
        else
        {
            return 0.0f;
        }
    }

    template<typename T>
    void AnimationTrackRangeTemplate<T>::setKeyFrame(float keyframe, AnimationKeyFrameRangeValue<T>* value)
    {
        auto it = _values.find(keyframe);

        if (it != _values.end())
        {
            it->second->release();
            _values.erase(it);
        }

        if (value)
        {
            _values[keyframe] = value;
            value->retain();
        }
    }

    template<typename T>
    void AnimationTrackRangeTemplate<T>::setSetter(const std::function<void(const AnimationTrack*, Animatable*, const T&)>& setter)
    {
        _setter = setter;
    }

    template<typename T>
    void AnimationTrackRangeTemplate<T>::setGetter(const std::function<T(const AnimationTrack*, Animatable*)>& getter)
    {
        _getter = getter;
    }

    template<typename T>
    void AnimationTrackRangeTemplate<T>::set(Animatable* animatable, const T& value) const
    {
        if (_setter)
        {
            _setter(this, animatable, value);
        }
    }

    template<typename T>
    T AnimationTrackRangeTemplate<T>::get(Animatable* animatable) const
    {
        if (_getter)
        {
            return _getter(this, animatable);
        }

        return _default;
    }
}
