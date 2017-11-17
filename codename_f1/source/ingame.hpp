#pragma once
#include <service/boss_zay.hpp>
#include "classes.hpp"

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

public:
    uint64 mAniTimeMsec;
    uint64 mEndTimeMsec;
    Point mPos;
    sint32 mSizeR;
    sint32 mGaugeTime;
    sint32 mDamage;
};
typedef Array<MapBreath, datatype_pod_canmemcpy> MapBreathes;

class ingameData : public ZayObject, public F1State
{
public:
    ingameData();
    ~ingameData();

private:
    sint32 GetValidNextForDynamic(const MapMonster& monster, const Point& curPos, Point& nextPos, Point* reflectPos = nullptr);

public:
    void InitForSpine();
    void AnimationOnce(sint32 timespan);
    void ClearPath();
    void Render(ZayPanel& panel);
    void ReadyForNextWave();
    sint32 GetCalcedBreathDamage();
    void BreathAttack(const MapBreath* breath);

public: // 게임상태
    bool mIsSpineInited;
    bool mShowDebug;
    Context mWaveData;
    sint32 mWave; // -1: 게임전, 0~N: 게임중
    String mWaveTitle;
    sint32 mWaveSec;
    MapMonsters mMonsters;

    // Breath
    MapBreathes mBreathes;
    MapSpine mBreathReadySpine;
    MapSpine mBreathAttackSpine;
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
