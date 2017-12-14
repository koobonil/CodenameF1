#pragma once

#include "zay_base.h"
#include "zay_animatable.h"

namespace ZAY
{
    class Animation
    : public Base
    {
    public:
        Animation();
        virtual ~Animation();

    public:
        void applyToAnimatable(Animatable* target, float time, bool colorUpdateOnly); //bx

    public:
        void addAnimationTrack(AnimationTrack* animationTrack);
        void removeAnimationTrack(AnimationTrack* animationTrack);
        void removeAnimationTrack(int32_t index);
        void removeAnimationTracksAll();
        AnimationTrack* getAnimationTrack(int32_t index) const;
        const std::vector<AnimationTrack*>& getAnimationTracksAll() const;
        int32_t getAnimationTracksCount() const;
    private:
        std::vector<AnimationTrack*> _tracks;
        
    public:
        void findAndSetAnimationLengthWithTracks();
        void setAnimationLength(float length);
        float getAnimationLength() const;
    private:
        float _length;
    };
}
