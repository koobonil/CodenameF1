#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

class SoundElement
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(SoundElement)
public:
    SoundElement() {mIsPlayCheck = false; mDelayTimeMsec = 0;}
    ~SoundElement() {}
    SoundElement(SoundElement&& rhs) {operator=(ToReference(rhs));}
    SoundElement& operator=(SoundElement&& rhs)
    {
        mIsPlayCheck = rhs.mIsPlayCheck;
        mDelayTimeMsec = rhs.mDelayTimeMsec;
        mSoundName = ToReference(rhs.mSoundName);
        return *this;
    }

public:
    bool mIsPlayCheck;
    sint32 mDelayTimeMsec;
    String mSoundName;
};
typedef Array<SoundElement> SoundElements;

class soundtoolData : public F1Tool
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(soundtoolData)
public:
    soundtoolData();
    ~soundtoolData();

public:
    void Render(ZayPanel& panel);
    void RenderSound(ZayPanel& panel, sint32 i);
    void ScrollUpdate(sint32 height, float sec);

public:
    void OnModeChanged() override;
    void OnSelectSub(chars name) override;
    void InitSelectBox(sint32 index) override;
    void QuitSelectBox(sint32 index) override;
    void ChangeSelectBox(sint32 type, sint32 index) override;
    void OnSelectBoxMoving(sint32 index, float addx, float addy) override;
    void OnSelectBoxMoved(sint32 index) override;
    void OnSelectBoxSizing(sint32 index, float addx, float addy) override;
    void OnSelectBoxSized(sint32 index) override;
    void OnSelectBoxClone(sint32 index) override;

public:
    float mScrollUpdateCommand;
    Tween1D mScroll;
    uint64 mPlayTimeMsec;
    sint32 mCurSound;
    SoundElements mSoundElements;
};
