#pragma once
#include <service/boss_zay.hpp>
#include "classes.hpp"

////////////////////////////////////////////////////////////////////////////////
class TimelineMonster
{
public:
    TimelineMonster() {mType = nullptr; mRID = 0;}
    ~TimelineMonster() {}
public:
    const MonsterType* mType;
    sint32 mRID;
    Point mPos;
};
typedef Array<TimelineMonster> TimelineMonsters;

////////////////////////////////////////////////////////////////////////////////
class TimelineMission
{
public:
    TimelineMission() {mTargetRID = 0; mRID = 0; mLocked = false;}
    ~TimelineMission() {}
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
public:
    TimelineScript() {mTargetRID = 0; mLocked = false;}
    ~TimelineScript() {}
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
public:
    TimelineWave() {mTitle = "Untitled";}
    ~TimelineWave() {}
public:
    String mTitle;
    Array<TimelineMonsters> mEventMonsters;
    Array<TimelineMissions> mEventMissions;
    Array<TimelineScripts> mEventScripts;
};
typedef Array<TimelineWave> TimelineWaves;

////////////////////////////////////////////////////////////////////////////////
class stagetoolData : public F1Tool
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(stagetoolData)
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
    bool mShowScript[3];
    String mMapName;
    TimelineWaves mWaves;
};
