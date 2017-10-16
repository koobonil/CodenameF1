#pragma once
#include <service/boss_zay.hpp>
#include "classes.hpp"

////////////////////////////////////////////////////////////////////////////////
class TimelineMonster
{
public:
    TimelineMonster() {mType = nullptr;}
    ~TimelineMonster() {}
public:
    const MonsterType* mType;
    Point mPos;
};
typedef Array<TimelineMonster> TimelineEvent;

////////////////////////////////////////////////////////////////////////////////
class TimelineWave
{
public:
    TimelineWave() {mTitle = "Untitled";}
    ~TimelineWave() {}
public:
    String mTitle;
    Array<TimelineEvent> mEvents;
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

public:
    sint32 mCurWave;
    sint32 mCurEvent;
    sint32 mCurMonster;
    String mMapName;
    TimelineWaves mWaves;
};
