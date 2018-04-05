#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

////////////////////////////////////////////////////////////////////////////////
class TimelineMonster
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(TimelineMonster)
public:
    TimelineMonster() {mType = nullptr; mRID = 0;}
    ~TimelineMonster() {}
    TimelineMonster(TimelineMonster&& rhs) {operator=(ToReference(rhs));}
    TimelineMonster& operator=(TimelineMonster&& rhs)
    {
        mType = rhs.mType;
        mRID = rhs.mRID;
        mPos = rhs.mPos;
        return *this;
    }

public:
    const MonsterType* mType;
    sint32 mRID;
    Point mPos;
};
typedef Array<TimelineMonster> TimelineMonsters;

////////////////////////////////////////////////////////////////////////////////
class TimelineMission
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(TimelineMission)
public:
    TimelineMission() {mTargetRID = 0; mRID = 0; mLocked = false;}
    ~TimelineMission() {}
    TimelineMission(TimelineMission&& rhs) {operator=(ToReference(rhs));}
    TimelineMission& operator=(TimelineMission&& rhs)
    {
        mTargetRID = rhs.mTargetRID;
        mRID = rhs.mRID;
        mPos = rhs.mPos;
        mLocked = rhs.mLocked;
        return *this;
    }

public:
    sint32 mTargetRID;
    sint32 mRID;
    Point mPos;
    bool mLocked;
};
typedef Map<TimelineMission> TimelineMissions;

////////////////////////////////////////////////////////////////////////////////
class TimelineScript
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(TimelineScript)
public:
    TimelineScript() {mTargetRID = 0; mLocked = false;}
    ~TimelineScript() {}
    TimelineScript(TimelineScript&& rhs) {operator=(ToReference(rhs));}
    TimelineScript& operator=(TimelineScript&& rhs)
    {
        mTargetRID = rhs.mTargetRID;
        mPos = rhs.mPos;
        mText = ToReference(rhs.mText);
        mLocked = rhs.mLocked;
        return *this;
    }

public:
    sint32 mTargetRID;
    Point mPos;
    String mText;
    bool mLocked;
};
typedef Map<TimelineScript> TimelineScripts;

////////////////////////////////////////////////////////////////////////////////
class TimelineWave
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(TimelineWave)
public:
    TimelineWave() {mTitle = "Untitled";}
    ~TimelineWave() {}
    TimelineWave(TimelineWave&& rhs) {operator=(ToReference(rhs));}
    TimelineWave& operator=(TimelineWave&& rhs)
    {
        mTitle = rhs.mTitle;
        mEventMonsters = ToReference(rhs.mEventMonsters);
        mEventMissions = ToReference(rhs.mEventMissions);
        mEventScripts = ToReference(rhs.mEventScripts);
        return *this;
    }

public:
    String mTitle;
    Array<TimelineMonsters> mEventMonsters;
    Array<TimelineMissions> mEventMissions;
    Array<TimelineScripts> mEventScripts;
};
typedef Array<TimelineWave> TimelineWaves;

////////////////////////////////////////////////////////////////////////////////
class MonsterGroup
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MonsterGroup)
public:
    MonsterGroup()
    {
    }
    ~MonsterGroup() {}
    MonsterGroup(MonsterGroup&& rhs) {operator=(ToReference(rhs));}
    MonsterGroup& operator=(MonsterGroup&& rhs)
    {
        mName = rhs.mName;
        mIDs = ToReference(rhs.mIDs);
        mDelaySecs = ToReference(rhs.mDelaySecs);
        return *this;
    }

public:
    String mName;
    sint32s mIDs;
    sint32s mDelaySecs;
};
typedef Array<MonsterGroup> MonsterGroups;

////////////////////////////////////////////////////////////////////////////////
class stagetoolData : public F1Tool
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(stagetoolData, mMonsterScrollMax(0), mMonsterGroupScrollMax(0))
public:
    stagetoolData();
    ~stagetoolData();

public:
    void Load(chars filename);
    void Save(chars filename);
    void Render(ZayPanel& panel);
    void RenderModeToggle(ZayPanel& panel);
    void RenderMission(ZayPanel& panel, sint32 rid);
    void RenderScript(ZayPanel& panel, sint32 eid, sint32 rid, bool circle = true, bool ui = true);

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
    enum class Mode {Mob, Mission, Script};
    Mode mMode;
    sint32 mCurWave;
    sint32 mCurEvent;
    sint32 mCurMonster;
    sint32 mCurMonsterGroup;
    Tween1D mMonsterScroll;
    const sint32 mMonsterScrollMax;
    Tween1D mMonsterGroupScroll;
    const sint32 mMonsterGroupScrollMax;
    bool mShowScript[3];
    bool mShowRemover;
    String mFastSaveFileName;
    Tween1D mFastSaveEffect;
    String mMapName;
    TimelineWaves mWaves;
    MonsterGroups mMonsterGroups;
};
