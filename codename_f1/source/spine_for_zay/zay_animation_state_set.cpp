#include "zay_types.h"
#include "zay_animation_state_set.h"
#include "zay_animation_container.h"
#include "zay_animation.h"
#include "zay_animation_state.h"

namespace ZAY
{
    AnimationStateSet::AnimationStateSet()
    {
    }

    AnimationStateSet::~AnimationStateSet()
    {
        removeAnimationStatesAll();
    }

    const std::map<std::string, AnimationState*>& AnimationStateSet::getAnimationStates() const
    {
        return _animationStates;
    }

    void AnimationStateSet::createAnimationState(const std::string& name)
    {
        auto it = _animationStates.find(name);

        if (it == _animationStates.end())
        {
            auto animationState = new AnimationState();
            _animationStates[name] = animationState;
        }
    }

    AnimationState* AnimationStateSet::getAnimationState(const std::string& name) const
    {
        auto it = _animationStates.find(name);

        if (it != _animationStates.end())
        {
            return it->second;
        }
        else
        {
            return nullptr;
        }
    }

    void AnimationStateSet::removeAnimationState(const std::string& name)
    {
        auto it = _animationStates.find(name);

        if (it != _animationStates.end())
        {
            delete it->second;
            _animationStates.erase(it);
        }
    }

    void AnimationStateSet::removeAnimationStatesAll()
    {
        for (auto it : _animationStates)
        {
            delete it.second;
        }

        _animationStates.clear();
    }

    void AnimationStateSet::_createAnimationStatesFromAnimationContainer(AnimationContainer* animationContainer)
    {
        for (const auto& it : animationContainer->getAnimationsAll())
        {
            _createAnimationStatesFromAnimation(it.second, it.first);
        }
    }

    void AnimationStateSet::_createAnimationStatesFromAnimation(Animation* animation, const std::string& animationName)
    {
        createAnimationState(animationName);
        auto animationState = getAnimationState(animationName);
        animationState->setLoopAndSeek(1, false);
        animationState->setAnimationLength(std::max(animationState->getAnimationLength(), animation->getAnimationLength()));
        animationState->setAnimationSpeed(1.0f);
        animationState->setCurrentTime(0.0f);
        animationState->setLastTime(0.0f);
    }

    void AnimationStateSet::seekAnimation(float sec) //bx
    {
        for (auto it : _animationStates)
        {
            if (it.second->getEnabled() && it.second->getSeek())
            {
                it.second->seek(sec, it.first);
            }
        }
    }

    void AnimationStateSet::updateAnimation(float deltaSec, MotionFinishedCB cb, void* payload) //bx
    {
        for (auto it : _animationStates)
        {
            if (it.second->getEnabled() && !it.second->getSeek())
            {
                it.second->update(deltaSec, cb, payload, it.first);
            }
        }
    }

    void AnimationStateSet::clearNeedToSetInitState()
    {
        for (auto it : _animationStates)
        {
            it.second->setNeedToSetInitState(false);
        }
    }
}
