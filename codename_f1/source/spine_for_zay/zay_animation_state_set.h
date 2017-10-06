#pragma once

#include "zay_types.h"

namespace ZAY
{
    class AnimationStateSet
    {
    public:
        AnimationStateSet();
        ~AnimationStateSet();
        
    public:
        const std::map<std::string, AnimationState*>& getAnimationStates() const;
        void createAnimationState(const std::string& name);
        AnimationState* getAnimationState(const std::string& name) const;
        void removeAnimationState(const std::string& name);
        void removeAnimationStatesAll();
        
    public:
        void _createAnimationStatesFromAnimationContainer(AnimationContainer* animationContainer);
        void _createAnimationStatesFromAnimation(Animation* animation, const std::string& animationName);
        
    public:
        void updateAnimation(float deltaTime, MotionFinishedCB cb, void* payload); //bx

    public:
        void clearNeedToSetInitState();
        
    private:
        std::map<std::string, AnimationState*> _animationStates;
    };
}
