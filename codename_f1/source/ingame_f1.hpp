#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

class MapBreath
{
public:
    MapBreath()
    {
        mEnable = false;
        mAniTimeMsec = 0;
        mEndTimeMsec = 0;
        mSizeR = 0;
        mGaugeTime = 0;
        mDamage = 0;
    }
    ~MapBreath() {}

public:
    bool mEnable;
    uint64 mAniTimeMsec;
    uint64 mEndTimeMsec;
    Point mPos;
    sint32 mSizeR;
    sint32 mGaugeTime;
    sint32 mDamage;
};

class ingameData : public ZayObject, public F1State
{
public:
    ingameData();
    ~ingameData();

private:
    sint32 GetValidNextForDynamic(const MapMonster& monster, const Point& curPos, Point& nextPos, Point* reflectPos = nullptr);

public:
    void InitForSpine();
    void PlayScriptOnce(sint32 sec);
    void AnimationOnce(sint32 sec_span);
    void ClearAllPathes();
    void Render(ZayPanel& panel);
    void RenderItems(ZayPanel& panel, bool slot, uint64 msec);
    void RenderBreathArea(ZayPanel& panel) override;
    void ReadyForNextWave();
    sint32 GetCalcedBreathDamage();
    void BreathAttack(const MapBreath* breath);

public: // 게임상태
    bool mIsSpineInited;
    bool mShowDebug;
    sint32 mClosing;
    Context mWaveData;
    sint32 mWave; // -1: 게임전, 0~N: 게임중
    String mWaveTitle;
    sint32 mWaveSecCurrently;
    sint32 mWaveSecSettled;
    MapMonsters mMonsters;

    // Item
    MapItemMap mItemMap;
    bool mSlotFlag[4];
    Point mSlotPos[4];
    String mCurItemSkin;
    String mCurItemSkinForDragon;
    uint64 mCurItemSkinEndTimeMsec;

    // Dragon & Breath
    MapDragon mDragon;
    MapBreath mBreath;
    MapSpine mBreathReadySpine;
    MapSpine mBreathAttackSpine;
    MapSpine mBreathEffectSpine;
    bool mBreathing;
    Point mBreathPos;
    uint64 mBreathMsec;
    sint32 mBreathGaugeTimeUsingCurrently;
    sint32 mBreathSizeRCurrently;
    sint32 mBreathPowerPermil;

    // MainTitle
    MapSpine mMainTitleSpine;
    uint64 mMainTitleStaffTime;
    Point mMainTitleStaffBegin[3];
    Point mMainTitleStaffTarget[3];

    // UI
    MapSpine mWeather[2];
    MapSpine mGaugeHUD;
    MapSpine mSlotHUD;
    MapSpine mWaveHUD;
    MapSpine mStopButton;
    sint32 mBreathGaugeTime;
    float mBreathGaugeTimeLog;
};
