#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

class MapBreathSkill
{
public:
    MapBreathSkill()
    {
        mDamage = 0;
        mKnockBackUp = 0;
        mSkillSkin = "";
        mSkillSpeed = 0;
        mSkillTime = 0;
        mDragonSkin = "";
    }
    ~MapBreathSkill() {}
    MapBreathSkill(const MapBreathSkill& rhs) {operator=(rhs);}
    MapBreathSkill& operator=(const MapBreathSkill& rhs)
    {
        mDamage = rhs.mDamage;
        mKnockBackUp = rhs.mKnockBackUp;
        mSkillSkin = rhs.mSkillSkin;
        mSkillSpeed = rhs.mSkillSpeed;
        mSkillTime = rhs.mSkillTime;
        mDragonSkin = rhs.mDragonSkin;
        return *this;
    }

public:
    sint32 mDamage;
    float mKnockBackUp;
    String mSkillSkin;
    float mSkillSpeed;
    sint32 mSkillTime;
    String mDragonSkin;
};

class MapBreath
{
public:
    MapBreath()
    {
        mAniTimeMsec = 0;
        mEndTimeMsec = 0;
        mSizeR = 0;
        mGaugeTime = 0;
        mPower = 0;
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
        mPower = rhs.mPower;
        mSkill = rhs.mSkill;
        return *this;
    }

public:
    uint64 mAniTimeMsec;
    uint64 mEndTimeMsec;
    Point mPos;
    sint32 mSizeR;
    sint32 mGaugeTime;
    float mPower;
    sint32 mDamage;
    MapBreathSkill mSkill;
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
    sint32 GetContactObject(const MapMonster& monster, const Point& nextPos);
    sint32 GetValidNextObject(const MapMonster& monster, const Point& curPos, const Point& nextPos, Point& resultPos, Point& reflectPos);
    void ItemToSlot(MapItem& item);
    void ReserveItem(chars skin);

private:
    inline sint32 CalcedPlusValue() const
    {return (mGameMode == GameMode::Infinity)? mInfinityPlusValue * (mGameSumWave - 1) : 0;}
    inline float CalcedMoveSpeedRate() const
    {return (mGameMode == GameMode::Infinity)? 1 + mInfinityMoveSpeedRate * (mGameSumWave - 1) : 1;}
    inline float CalcedHPRate() const
    {return (mGameMode == GameMode::Infinity)? 1 + mInfinityHPRate * (mGameSumWave - 1) : 1;}

public:
    void InitForSpine();
    void PlayScriptOnce(sint32 sec);
    void AnimationOnce(sint32 sec_span);
    bool MonsterActionOnce(MapMonster& monster, const Point& pos);
    Point MonsterKnockBackOnce(MapMonster& monster, const TryWorldZone& zone, const Point& pos, const uint64 msec);
    Point MonsterMoveOnce(MapMonster& monster, const TryWorldZone& zone, const Point& pos, const Point& vec, const uint64 msec);
    void ReserveToSlotOnce();
    void ClearAllPathes(chars polygon_name = nullptr);
    void Render(ZayPanel& panel);
    void RenderNumbers(ZayPanel& panel, chars numbers, bool rside);
    void RenderItems(ZayPanel& panel, bool slot, uint64 msec);
    void RenderBreathArea(ZayPanel& panel) override;
    void ReadyForNextWave(bool nocount);
    sint32 GetCalcedBreathDamage();
    void SetDragonSchedule(const MapBreath* breath, bool retry);
    void SetBreathAttack(const MapBreath* breath);
    void AddEventForMap(const MapObject& broken_object);
    void CheckAllMaps();

public:
    void OnEvent(chars name, const Rect& rect, const MapSpine* spine) override;

public:
    class GameMode
    {
    public:
        enum Type {Normal, Infinity};
    public:
        GameMode() {mValue = Normal;}
        GameMode(const GameMode& rhs) {operator=(rhs);}
        GameMode& operator=(const GameMode& rhs) {mValue = rhs.mValue; return *this;}
        GameMode& operator=(Type rhs) {mValue = rhs; return *this;}
        GameMode& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Normal"))
                mValue = Normal;
            else if(!String::Compare(rhs, "Infinity"))
                mValue = Infinity;
            else BOSS_ASSERT("알 수 없는 키워드입니다", false);
            return *this;
        }
        bool operator==(Type rhs) const
        {return (mValue == rhs);}
        bool operator!=(Type rhs) const
        {return (mValue != rhs);}
    private:
        Type mValue;
    };

public: // 디버깅정보
    bool mShowDebug;
    Strings mDebugScriptLogs;

public: // 게임상태
    bool mSpineInited;
    bool mOptionItemInited;
    sint32 mPauseType; // 0: 중지아님, 1: 다이얄로그, 2: 컷씬
    sint32 mClosing;
    sint32 mClosingOption;
    Context mGameOption;
    GameMode mGameMode;
    uint64 mGameBeginMsec;
    uint64 mGameStopMsec;
    sint32 mGameSumStopMsec;
    sint32 mGameSumWave;
    sint32 mGameScore;
    sint32 mGameScoreLog;
    Context mWaveData;
    sint32 mWave; // -1: 게임전, 0~N: 게임중
    String mWaveTitle;
    sint32 mWaveSecCurrently;
    sint32 mWaveSecSettled;
    sint32 mWaveSecMax;
    uint64 mCurTickSec;
	uint64 mCurRenderMsec;
    sint32 mCurParaTalk;
    MapSpine* mCurCutscene;
    MapSpine* mCurCutsceneClose;
    bool mHolicMode;
    MapMonsters mMonsters;

    // Item과 Slot
    class SlotStatus
    {
    public:
        SlotStatus() {mItemType = nullptr; mCount = 0; mReserved = 0;}
        ~SlotStatus() {}
    public:
        const ItemType* mItemType;
        sint32 mCount;
        Point mPos;
        sint32 mReserved;
    };
    MapItemMap mItemMap;
    SlotStatus mSlotStatus[4];

    // Skill
    class SkillStatus
    {
    public:
        SkillStatus()
        {
            Clear();
            mDragonSkinName = "normal";
            mBreathSkillID = skill_id::NoSkill;
            mBreathDamageUp = 1000;
        }
        ~SkillStatus() {}
    public:
        void Clear()
        {
            mDragonSkinName = "normal";
            mEndTimeMsec = 0;
            mBreathSkillID = skill_id::NoSkill;
            mBreathSkillOption.Clear();
            mBreathDamageUp = 1000;
        }
        void Reset(const MapItem* item)
        {
            mDragonSkinName = item->skin();
            mEndTimeMsec = Platform::Utility::CurrentTimeMsec() + item->type()->mSkillDuration;
            mBreathSkillID = item->type()->mSkillID;
            mBreathSkillOption.LoadPrm(item->type()->mSkillParameter);
            mBreathDamageUp = item->type()->mAddDamage;
        }
    public:
        bool IsFinished(uint64 msec) const
        {
            return (mEndTimeMsec != 0 && mEndTimeMsec < msec);
        }
        sint32 CalcedDamage(sint32 damage) const
        {
            return damage * mBreathDamageUp / 1000;
        }
        float CalcedKnockBackUp() const
        {
            if(mBreathSkillID == skill_id::BreathKnockBackUp)
                return mBreathSkillOption("KnockBackRange").GetInt(1000) / 1000.0f;
            return 1;
        }
        chars CalcedSkillSkin() const
        {
            branch;
            jump(mBreathSkillID == skill_id::BreathSlow)
                return "skill_ice";
            jump(mBreathSkillID == skill_id::BreathKnockBackUp)
                return "skill_wind";
            jump(mBreathSkillID == skill_id::BreathStun)
                return "skill_lightning";
            jump(mBreathSkillID == skill_id::BreathBurn)
                return "skill_fire";
            return "";
        }
        float CalcedSkillSpeed() const
        {
            return mBreathSkillOption("MoveSpeed").GetInt(1000) / 1000.0f;
        }
        sint32 CalcedSkillTime() const
        {
            return mBreathSkillOption("While").GetInt(0);
        }

    public:
        inline chars dragon_skin() const {return mDragonSkinName;}
        inline sint32 damage_up() const {return mBreathDamageUp;}
    private:
        uint64 mEndTimeMsec;
        String mDragonSkinName;
        skill_id mBreathSkillID;
        Context mBreathSkillOption;
        sint32 mBreathDamageUp;
    };
    SkillStatus mCurSkill;

    // MainTitle
    MapSpine mMainTitleSpine;

    // Effect
    EffectManager mEffectManager;
    ParicleManager mParticleManager;

    // Dragon & Breath
    MapDragon mDragon;
    MapSpine mDragonChangeSpine;
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
    Tween1D mCutsceneTween;

    // UI
    MapSpine mWeather[2];
    MapSpine mCampaign;
    MapSpine mGaugeHUD;
    MapSpine mGaugeChange;
    MapSpine mSlotHUD;
    MapSpine mWaveHUD;
    MapSpine mStopButton;
    MapSpine mPausePopup;
    sint32 mBreathGaugeTime;
    float mBreathGaugeTimeLog;
};
