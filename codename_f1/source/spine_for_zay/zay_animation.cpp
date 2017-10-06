#include "zay_types.h"
#include "zay_animation.h"
#include "zay_animation_track.h"

namespace ZAY
{
    Animation::Animation()
    {
        _length = 0.0f;
    }
    
    Animation::~Animation()
    {
        removeAnimationTracksAll();
    }

    void Animation::applyToAnimatable(Animatable* target, float time, bool colorUpdateOnly) //bx
    {
        if(colorUpdateOnly)
        {
            for(auto track : _tracks)
            {
                AnimationTrackColour* CurTrack = dynamic_cast<AnimationTrackColour*>(track);
                if(CurTrack)
                    CurTrack->applyToAnimatable(target, time);
            }
        }
        else for (auto track : _tracks)
        {
            track->applyToAnimatable(target, time);
        }
    }
    
    void Animation::addAnimationTrack(AnimationTrack* animationTrack)
    {
        if (animationTrack)
        {
            assert(std::find(_tracks.begin(), _tracks.end(), animationTrack) == _tracks.end());
            
            _tracks.push_back(animationTrack);
            animationTrack->retain();
        }
    }
    
    void Animation::removeAnimationTrack(AnimationTrack* animationTrack)
    {
        for (auto it = _tracks.begin() ;
             it != _tracks.end() ;
             )
        {
            if ((*it) == animationTrack)
            {
                animationTrack->release();
                it = _tracks.erase(it);
                return;
            }
            else
            {
                ++it;
            }
        }
    }
    
    void Animation::removeAnimationTrack(int32_t index)
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_tracks.size()))
        {
            auto it = _tracks.begin();

            std::advance(it, index);

            (*it)->release();
            _tracks.erase(it);
        }
    }
    
    void Animation::removeAnimationTracksAll()
    {
        for (auto track : _tracks)
        {
            track->release();
        }
        _tracks.clear();
    }
    
    AnimationTrack* Animation::getAnimationTrack(int32_t index) const
    {
        if (index >= 0 &&
            index < static_cast<int32_t>(_tracks.size()))
        {
            auto it = _tracks.begin();
            
            std::advance(it, index);

            return *it;
        }
        else
        {
            return nullptr;
        }
    }
    
    const std::vector<AnimationTrack*>& Animation::getAnimationTracksAll() const
    {
        return _tracks;
    }

    int32_t Animation::getAnimationTracksCount() const
    {
        return static_cast<int32_t>(_tracks.size());
    }

    void Animation::findAndSetAnimationLengthWithTracks()
    {
        _length = 0.0f;
        
        for (auto track : _tracks)
        {
            auto trackLastKeyFrameTime = track->getLastKeyFrameTime();
            
            if (trackLastKeyFrameTime > _length)
            {
                _length = trackLastKeyFrameTime;
            }
        }
    }
    
    void Animation::setAnimationLength(float length)
    {
        _length = length;
    }
    
    float Animation::getAnimationLength() const
    {
        return _length;
    }
}
