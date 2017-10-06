#pragma once

#include "zay_animation.h"
#include "zay_animation_state_set.h"
#include "zay_animatable.h"

namespace ZAY
{
    class AnimationContainer
    {
    public:
        AnimationContainer();
        virtual ~AnimationContainer();

    public:
        void setAnimation(const std::string& name, Animation* animation);
        void removeAnimationsAll();
        Animation* getAnimation(const std::string& name) const;
        void applyAnimation(Animatable* target, float time);
        void applyAnimationStateSetDeltaTime(Animatable* target, AnimationStateSet& animationStateSet, float deltaTime);
        const std::map<std::string, Animation*>& getAnimationsAll() const;
    private:
        std::map<std::string, Animation*> _animations;
    };
}
