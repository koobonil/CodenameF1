#pragma once

#include "zay_types.h"
#include "zay_lerp.h"
#include "zay_random_generator.h"
#include "zay_animation_state_set.h"
#include "zay_any_value.h"

namespace ZAY
{
    class Animatable
    {
    public:
        Animatable();
        virtual ~Animatable();
        
    public:
        virtual void applyInitialAnimationPose(bool inheritChild, bool colorUpdateOnly) = 0; //bx
        virtual void applyAnimationStateSet(AnimationStateSet* animationStateSet, bool inheritChild, bool colorUpdateOnly) = 0; //bx

    public:
        template<typename T>
        void getKeyframeRangeData(const AnimationTrack* animationTrack, int32_t keyframeIndex, const T& valueMin, const T& valueMax, T& returnValue)
        {
            auto it = _keyframeRangeDatas.find(animationTrack);
            
            if (it != _keyframeRangeDatas.end())
            {
                auto it2 = it->second.find(keyframeIndex);
                
                if (it2 != it->second.end())
                {
                    
                }
                else
                {
                    returnValue = lerp(valueMin, valueMax, RandomGenerator::frand(0.0f, 1.0f));
                    it->second[keyframeIndex] = returnValue;
                }
            }
            else
            {
                returnValue = lerp(valueMin, valueMax, RandomGenerator::frand(0.0f, 1.0f));
                _keyframeRangeDatas[animationTrack][keyframeIndex] = returnValue;
            }
        }

        void clearKeyframeRangeDatas();
    private:
        std::map<const AnimationTrack*, std::map<int32_t, AnyValue>> _keyframeRangeDatas;

    };
}
