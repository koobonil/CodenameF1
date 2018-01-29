#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

class MapBreath
{
public:
    MapBreath()
    {
        mAniTimeMsec = 0;
        mEndTimeMsec = 0;
        mSizeR = 0;
        mGaugeTime = 0;
        mDamage = 0;
    }
    ~MapBreath() {}
    MapBreath(const MapBreath& rhs) {operator=(rhs);}
    MapBreath& operator=(const MapBreath& rhs)
    {
        mAniTimeMsec = rhs.mAniTimeMsec;
        mEndTimeMsec = rhs.mEndTimeMsec;
        mPos = rhs.mPos;
        mSizeR = rhs.mSizeR;
        mGaugeTime = rhs.mGaugeTime;
        mDamage = rhs.mDamage;
        return *this;
    }

public:
    uint64 mAniTimeMsec;
    uint64 mEndTimeMsec;
    Point mPos;
    sint32 mSizeR;
    sint32 mGaugeTime;
    sint32 mDamage;
};

class MapBreathAttack
{
public:
    MapBreathAttack()
    {
        mBreathSizeR = 0;
        mBreathAniTimeMsec = 0;
        mDragonFlip = false;
        mDragonSizeR = 0;
        mDragonBreathBeginTimeMsec = 0;
        mDragonBreathEndTimeMsec = 0;
    }
    ~MapBreathAttack() {}

public:
    Point mBreathPos;
    sint32 mBreathSizeR;
    uint64 mBreathAniTimeMsec;
    bool mDragonFlip;
    Point mDragonPos;
    sint32 mDragonSizeR;
    uint64 mDragonBreathBeginTimeMsec;
    uint64 mDragonBreathEndTimeMsec;
};

class ingameData : public ZayObject, public F1State
{
public:
    ingameData();
    ~ingameData();

private:
    void Targeting(MapMonster& monster, const TryWorldZone& tryworld);
    sint32 GetContactObject(const MapMonster& monster, const Point& curPos, const Point& nextPos);
    sint32 GetValidNextObject(const MapMonster& monster, const Point& curPos, const Point& nextPos, Point& resultPos, Point& reflectPos);

public:
    void InitForSpine();
    void PlayScriptOnce(sint32 sec);
    void AnimationOnce(sint32 sec_span);
    void ClearAllPathes(bool directly);
    void Render(ZayPanel& panel);
    void RenderItems(ZayPanel& panel, bool slot, uint64 msec);
    void RenderBreathArea(ZayPanel& panel) override;
    void ReadyForNextWave();
    sint32 GetCalcedBreathDamage();
    void SetDragonSchedule(const MapBreath* breath, bool retry);
    void SetBreathAttack(const MapBreath* breath);

public: // 게임상태
    bool mSpineInited;
    bool mShowDebug;
    bool mPaused;
    sint32 mClosing;
    sint32 mClosingOption;
    Context mWaveData;
    sint32 mWave; // -1: 게임전, 0~N: 게임중
    String mWaveTitle;
    sint32 mWaveSecCurrently;
    sint32 mWaveSecSettled;
    sint32 mWaveSecMax;
    uint64 mCurTickTimeSec;
    sint32 mCurParaTalk;
    MapMonsters mMonsters;

    // Item
    MapItemMap mItemMap;
    bool mSlotFlag[4];
    Point mSlotPos[4];
    String mCurItemSkin;
    String mCurItemSkinForDragon;
    uint64 mCurItemSkinEndTimeMsec;

    // MainTitle
    MapSpine mMainTitleSpine;

    // Effect
    static const sint32 mWallBoundMax = 64;
    sint32 mWallBoundFocus;
    MapSpine mWallBound[mWallBoundMax];
    Point mWallBoundPos[mWallBoundMax];

    // Dragon & Breath
    MapDragon mDragon;
    MapBreath mBreath[2];
    MapBreathAttack mBreathAttack;
    MapSpine mBreathReadySpine[2];
    MapSpine mBreathBarSpine;
    MapSpine mBreathAttackSpine;
    MapSpine mBreathEffectSpine;
    sint32 mBreathReadyCount;
    bool mBreathing;
    Point mBreathPos;
    uint64 mBreathMsec;
    sint32 mBreathGaugeTimeUsingCurrently;
    sint32 mBreathSizeRCurrently;
    sint32 mBreathPowerPermil;

    // UI
    MapSpine mWeather[2];
    MapSpine mGaugeHUD;
    MapSpine mSlotHUD;
    MapSpine mWaveHUD;
    MapSpine mStopButton;
    MapSpine mPausePopup;
    sint32 mBreathGaugeTime;
    float mBreathGaugeTimeLog;
};
