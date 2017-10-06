#pragma once

#include "zay_types.h"

namespace ZAY
{
    class AnimationState
    {
    public:
        AnimationState();
        virtual ~AnimationState();

    public:
        void setNeedToSetInitState(bool flag);
        bool getNeedToSetInitState() const;
    private:
        bool _needToSetInitState;
        
    public:
        void setEnabled(bool flag);
        bool getEnabled() const;
    private:
        bool _enabled;

    public:
        void setLoop(int value);
        int getLoop() const;
    private:
        int _animationLoop; // -1-한번후 소멸, 0-한번, 1-반복

    public:
        void setLastTime(float lastTime);
        float getLastTime() const;
    private:
        float _lastTime;

    public:
        void setCurrentTime(float currentTime);
        float getCurrentTime() const;
    private:
        float _currentTime;

    public:
        void setAnimationSpeed(float speed);
        float getAnimationSpeed() const;
    private:
        float _animationSpeed;

    public:
        void setAnimationLength(float length);
        float getAnimationLength() const;
    private:
        float _animationLength;

    public:
        void update(float deltaTime, MotionFinishedCB cb, void* payload, std::string name); //bx
    };
}
