#include "zay_types.h"
#include "zay_animation_state.h"

namespace ZAY
{
    AnimationState::AnimationState()
    {
        _needToSetInitState = true;
        _enabled = true;
        _animationLoop = 1;
        _animationSeek = false;
        _lastTime = 0.0f;
        _currentTime = 0.0f;
        _animationSpeed = 1.0f;
        _animationLength = 0.0f;
    }
    
    AnimationState::~AnimationState()
    {
    }

    void AnimationState::setNeedToSetInitState(bool flag)
    {
        _needToSetInitState = flag;
    }
    
    bool AnimationState::getNeedToSetInitState() const
    {
        return _needToSetInitState;
    }

    void AnimationState::setEnabled(bool flag)
    {
        if (_enabled != flag)
        {
            _enabled = flag;
            
            _needToSetInitState = true;
        }
    }
    
    bool AnimationState::getEnabled() const
    {
        return _enabled;
    }

    void AnimationState::setLoopAndSeek(int loop, bool seek)
    {
        _animationLoop = loop;
        _animationSeek = seek;
    }

    int AnimationState::getLoop() const
    {
        return _animationLoop;
    }

    bool AnimationState::getSeek() const
    {
        return _animationSeek;
    }

    void AnimationState::setLastTime(float lastTime)
    {
        _lastTime = lastTime;
    }

    float AnimationState::getLastTime() const
    {
        return _lastTime;
    }

    void AnimationState::setCurrentTime(float currentTime)
    {
        if (_currentTime != currentTime)
        {
            _currentTime = currentTime;
            _needToSetInitState = true;
        }
    }
    
    float AnimationState::getCurrentTime() const
    {
        return _currentTime;
    }

    void AnimationState::setAnimationSpeed(float speed)
    {
        _animationSpeed = speed;
    }
    
    float AnimationState::getAnimationSpeed() const
    {
        return _animationSpeed;
    }

    void AnimationState::setAnimationLength(float length)
    {
        _animationLength = length;
    }

    float AnimationState::getAnimationLength() const
    {
        return _animationLength;
    }

    void AnimationState::seek(float sec, std::string name) //bx
    {
        if (0 < _animationLength)
        {
            _lastTime = _currentTime;

            if (_animationLoop == 1)
            {
                _currentTime = sec;
                bool HasTimeClip = false;
                while (_animationLength < _currentTime)
                {
                    _currentTime -= _animationLength;
                    HasTimeClip = true;
                }
                if(HasTimeClip)
                    _needToSetInitState = true;
            }
            else _currentTime = (sec < _animationLength)? sec : _animationLength;
        }
        else
        {
            _currentTime = 0.0f;
            _lastTime = 0.0f;
        }
    }

    void AnimationState::update(float deltaSec, MotionFinishedCB cb, void* payload, std::string name) //bx
    {
        if (0 < _animationLength)
        {
            const float DeltaTimeBySpeed = deltaSec * _animationSpeed;
            _lastTime = _currentTime;

            if (_animationLoop == 1)
            {
                _currentTime += DeltaTimeBySpeed;
                bool HasTimeClip = false;
                while (_animationLength < _currentTime)
                {
                    _currentTime -= _animationLength;
                    HasTimeClip = true;
                }
                if(HasTimeClip)
                {
                    _needToSetInitState = true;
                    _lastTime = 0;
                    if(cb) cb(payload, name.c_str(), _animationLoop);
                }
            }
            else
            {
                if (_currentTime < _animationLength)
                {
                    _currentTime += DeltaTimeBySpeed;
                    if (_animationLength <= _currentTime)
                    {
                        _currentTime = _animationLength;
                        if(cb) cb(payload, name.c_str(), _animationLoop);
                    }
                }
            }
        }
        else
        {
            _currentTime = 0.0f;
            _lastTime = 0.0f;
        }
    }
}
