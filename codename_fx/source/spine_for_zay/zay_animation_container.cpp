#include "zay_animation_container.h"
#include "zay_animation.h"
#include "zay_animation_state.h"
#include "zay_animation_state_set.h"

namespace ZAY
{
    AnimationContainer::AnimationContainer()
    {
    }

    AnimationContainer::~AnimationContainer()
    {
        removeAnimationsAll();
    }

    void AnimationContainer::setAnimation(const std::string& name, Animation* animation)
    {
        auto found = _animations.find(name);

        if (found != _animations.end())
        {
            if (found->second != animation)
            {
                if (found->second)
                {
                    found->second->release();
                }

                found->second = animation;

                if (found->second)
                {
                    found->second->retain();
                }
            }
        }
        else
        {
            _animations[name] = animation;
            animation->retain();
        }
    }

    void AnimationContainer::removeAnimationsAll()
    {
        for (const auto& it : _animations)
        {
            if (it.second)
            {
                it.second->release();
            }
        }
        _animations.clear();
    }

    Animation* AnimationContainer::getAnimation(const std::string& name) const
    {
        auto found = _animations.find(name);

        if (found != _animations.end())
        {
            return found->second;
        }
        else
        {
            return nullptr;
        }
    }

    void AnimationContainer::applyAnimation(Animatable* target, float lasttime, float curtime)
    {
        for (const auto& it : _animations)
        {
            it.second->applyToAnimatable(target, lasttime, curtime, false);
        }
    }

    void AnimationContainer::applyAnimationStateSetDeltaTime(Animatable* target, AnimationStateSet& animationStateSet, float deltaTime)
    {
        animationStateSet.updateAnimation(deltaTime, nullptr, nullptr);

        for (auto it : animationStateSet.getAnimationStates())
        {
            const auto& name = it.first;
            auto animationState = it.second;

            if (animationState->getEnabled())
            {
                auto animation_it = _animations.find(name);

                if (animation_it != _animations.end())
                {
                    auto animation = animation_it->second;

                    animation->applyToAnimatable(target, animationState->getLastTime(), animationState->getCurrentTime(), false);
                }
            }
        }
    }

    const std::map<std::string, Animation*>& AnimationContainer::getAnimationsAll() const
    {
        return _animations;
    }

}
