#include <boss.hpp>
#include "stagetool.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("stagetoolView", stagetoolData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Create)
    {
        // 윈도우 타이틀
        Platform::SetWindowName("Codename F1 [StageTool]");
    }
    else m->Command(type, in);
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    const Point Pos = m->GestureToPos(x, y);
    static bool IsPressEnabled = true;

    if(m->mMode != stagetoolData::Mode::Mob)
        IsPressEnabled = true;
    else
    {
        if(type == GT_Pressed)
        {
            auto& CurMonsters = m->mWaves.At(m->mCurWave).mEventMonsters.At(m->mCurEvent);
            // 삭제인지 판단
            for(sint32 i = 0, iend = CurMonsters.Count(); i < iend; ++i)
            {
                const float MonsterX = m->mState.mInGameX + m->mState.mInGameW * (CurMonsters[i].mPos.x + 0.5f);
                const float MonsterY = m->mState.mInGameY + m->mState.mInGameH * (CurMonsters[i].mPos.y + 0.5f);
                if(Math::Distance(x - m->mMapPos.x, y - m->mMapPos.y, MonsterX, MonsterY) < m->mState.mMonsterSizeR)
                {
                    sint32 CurRID = CurMonsters[i].mRID;
                    // 몬스터Script 선행삭제
                    m->mWaves.At(m->mCurWave).mEventScripts.At(m->mCurEvent).Remove(CurRID);
                    while(TimelineMission* CurMission = m->mWaves.At(m->mCurWave).mEventMissions.At(m->mCurEvent).Access(CurRID))
                    {
                        CurRID = CurMission->mRID;
                        // 몬스터미션Script 선행삭제
                        m->mWaves.At(m->mCurWave).mEventScripts.At(m->mCurEvent).Remove(CurRID);
                        // 몬스터미션 선행삭제
                        m->mWaves.At(m->mCurWave).mEventMissions.At(m->mCurEvent).Remove(CurMission->mTargetRID);
                    }
                    // 몬스터삭제
                    CurMonsters.SubtractionSection(i);
                    IsPressEnabled = false;
                    return;
                }
            }

            // 새로 생성
            auto& NewMonster = CurMonsters.AtAdding();
            NewMonster.mType = &m->mState.mMonsterTypes[m->mCurMonster];
            NewMonster.mRID = ++m->mState.mMonsterLastRID;
            NewMonster.mPos.x = Pos.x;
            NewMonster.mPos.y = Pos.y;
            IsPressEnabled = true;
        }
        else if(IsPressEnabled)
        {
            if(type == GT_InDragging || type == GT_OutDragging)
            {
                auto& CurMonsters = m->mWaves.At(m->mCurWave).mEventMonsters.At(m->mCurEvent);
                // 마지막 점을 이동
                if(0 < CurMonsters.Count())
                {
                    auto& LastMonster = CurMonsters.At(-1);
                    LastMonster.mPos.x = Pos.x;
                    LastMonster.mPos.y = Pos.y;
                    m->invalidate();
                }
            }
        }
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    // 출력미비영역 표현용
    ZAY_RGB(panel, 128, 128, 128)
        panel.fill();
    m->Render(panel);
}

stagetoolData::stagetoolData() : mMonsterScroll(updater()), mMonsterScrollMax(11)
{
    mMode = Mode::Mob;
    mCurWave = 0;
    mCurEvent = 0;
    mCurMonster = 0;
    mMonsterScroll.Reset(0);
    mShowScript[0] = true;
    mShowScript[1] = true;
    mShowScript[2] = true;
    mMapName = "";
    auto& NewWave = mWaves.AtAdding();
    NewWave.mEventMonsters.AtDumpingAdded(mState.mTimelineLength);
    NewWave.mEventMissions.AtDumpingAdded(mState.mTimelineLength);
    NewWave.mEventScripts.AtDumpingAdded(mState.mTimelineLength);
}

stagetoolData::~stagetoolData()
{
}

void stagetoolData::Load(chars filename)
{
    mMode = Mode::Mob;
    mCurWave = 0;
    mCurEvent = 0;
    mCurMonster = 0;
    mShowScript[0] = true;
    mShowScript[1] = true;
    mShowScript[2] = true;
    mMapName = "";
    mWaves.SubtractionAll();

    id_file_read TextFile = Platform::File::OpenForRead(filename);
    if(TextFile)
    {
        const sint32 TextSize = Platform::File::Size(TextFile);
        buffer TextBuffer = Buffer::Alloc(BOSS_DBG TextSize + 1);
        Platform::File::Read(TextFile, (uint08*) TextBuffer, TextSize);
        Platform::File::Close(TextFile);
        ((char*) TextBuffer)[TextSize] = '\0';

        Context JsonWave(ST_Json, TextBuffer);
        mMapName = JsonWave("MapName").GetString();
        mState.mMonsterLastRID = JsonWave("MonsterLastRID").GetInt(mState.mMonsterRIDBegin);
        mState.mMissionLastRID = JsonWave("MissionLastRID").GetInt(mState.mMissionRIDBegin);
        for(sint32 wave = 0, wave_end = JsonWave("Waves").LengthOfIndexable(); wave < wave_end; ++wave)
        {
            auto& NewWave = mWaves.AtAdding();
            NewWave.mEventMonsters.AtDumpingAdded(mState.mTimelineLength);
            NewWave.mEventMissions.AtDumpingAdded(mState.mTimelineLength);
            NewWave.mEventScripts.AtDumpingAdded(mState.mTimelineLength);
            auto& CurJsonWave = JsonWave("Waves")[wave];
            NewWave.mTitle = CurJsonWave("Title").GetString();
            for(sint32 evt = 0, evt_end = CurJsonWave("Events").LengthOfIndexable(); evt < evt_end; ++evt)
            {
                auto& CurJsonEvent = CurJsonWave("Events")[evt];
                const sint32 TimeSec = Math::Clamp(CurJsonEvent("TimeSec").GetInt(), 0, mState.mTimelineLength - 1);

                auto& CurMonsterEvent = NewWave.mEventMonsters.At(TimeSec);
                for(sint32 mon = 0, mon_end = CurJsonEvent("Monsters").LengthOfIndexable(); mon < mon_end; ++mon)
                {
                    auto& CurJsonMonster = CurJsonEvent("Monsters")[mon];
                    auto& NewTimelineMonster = CurMonsterEvent.AtAdding();
                    const String CurJsonMonsterID = CurJsonMonster("ID").GetString();
                    NewTimelineMonster.mType = &mState.mMonsterTypes[0];
                    for(sint32 i = 0, iend = mState.mMonsterTypes.Count(); i < iend; ++i)
                        if(!CurJsonMonsterID.Compare(mState.mMonsterTypes[i].mID))
                        {
                            NewTimelineMonster.mType = &mState.mMonsterTypes[i];
                            break;
                        }
                    if((NewTimelineMonster.mRID = CurJsonMonster("RID").GetInt(0)) == 0)
                        NewTimelineMonster.mRID = ++mState.mMonsterLastRID;
                    if(mState.mLandscape)
                    {
                        NewTimelineMonster.mPos.x = CurJsonMonster("PosY").GetFloat();
                        NewTimelineMonster.mPos.y = -CurJsonMonster("PosX").GetFloat();
                    }
                    else
                    {
                        NewTimelineMonster.mPos.x = CurJsonMonster("PosX").GetFloat();
                        NewTimelineMonster.mPos.y = CurJsonMonster("PosY").GetFloat();
                    }
                }

                auto& CurMissionEvent = NewWave.mEventMissions.At(TimeSec);
                for(sint32 mss = 0, mss_end = CurJsonEvent("Missions").LengthOfIndexable(); mss < mss_end; ++mss)
                {
                    auto& CurJsonMission = CurJsonEvent("Missions")[mss];
                    const sint32 NewTargetRID = CurJsonMission("TargetRID").GetInt(0);
                    auto& NewTimelineMission = CurMissionEvent[NewTargetRID];
                    NewTimelineMission.mTargetRID = NewTargetRID;
                    NewTimelineMission.mRID = CurJsonMission("RID").GetInt(0);
                    if(mState.mLandscape)
                    {
                        NewTimelineMission.mPos.x = CurJsonMission("PosY").GetFloat();
                        NewTimelineMission.mPos.y = -CurJsonMission("PosX").GetFloat();
                    }
                    else
                    {
                        NewTimelineMission.mPos.x = CurJsonMission("PosX").GetFloat();
                        NewTimelineMission.mPos.y = CurJsonMission("PosY").GetFloat();
                    }
                }

                auto& CurScriptEvent = NewWave.mEventScripts.At(TimeSec);
                for(sint32 scr = 0, scr_end = CurJsonEvent("Scripts").LengthOfIndexable(); scr < scr_end; ++scr)
                {
                    auto& CurJsonScript = CurJsonEvent("Scripts")[scr];
                    sint32 NewTargetRID = CurJsonScript("TargetRID").GetInt(-1);
                    if(NewTargetRID == -1) // 하위버전 json호환
                        NewTargetRID = CurJsonScript("RID").GetInt(0);
                    auto& NewTimelineScript = CurScriptEvent[NewTargetRID];
                    NewTimelineScript.mTargetRID = NewTargetRID;
                    if(mState.mLandscape)
                    {
                        NewTimelineScript.mPos.x = CurJsonScript("PosY").GetFloat();
                        NewTimelineScript.mPos.y = -CurJsonScript("PosX").GetFloat();
                    }
                    else
                    {
                        NewTimelineScript.mPos.x = CurJsonScript("PosX").GetFloat();
                        NewTimelineScript.mPos.y = CurJsonScript("PosY").GetFloat();
                    }
                    NewTimelineScript.mText = CurJsonScript("Text").GetString("");
                }
            }
        }
    }

    if(mWaves.Count() == 0)
    {
        auto& NewWave = mWaves.AtAdding();
        NewWave.mEventMonsters.AtDumpingAdded(mState.mTimelineLength);
        NewWave.mEventMissions.AtDumpingAdded(mState.mTimelineLength);
        NewWave.mEventScripts.AtDumpingAdded(mState.mTimelineLength);
    }

    id_asset_read TextAsset = Asset::OpenForRead("table/" + mMapName + ".json");
    if(TextAsset)
    {
        const sint32 TextSize = Asset::Size(TextAsset);
        buffer TextBuffer = Buffer::Alloc(BOSS_DBG TextSize + 1);
        Asset::Read(TextAsset, (uint08*) TextBuffer, TextSize);
        Asset::Close(TextAsset);
        ((char*) TextBuffer)[TextSize] = '\0';
        mState.LoadMap((chars) TextBuffer);
        Buffer::Free(TextBuffer);
    }
    else mState.LoadMap("");
}

void stagetoolData::Save(chars filename)
{
    id_file TextFile = Platform::File::OpenForWrite(filename);
    if(TextFile)
    {
        Context JsonWave;
        JsonWave.At("MapName").Set(mMapName);
        JsonWave.At("MonsterLastRID").Set(String::FromInteger(mState.mMonsterLastRID));
        JsonWave.At("MissionLastRID").Set(String::FromInteger(mState.mMissionLastRID));
        for(sint32 wave = 0, wave_end = mWaves.Count(); wave < wave_end; ++wave)
        {
            auto& CurWave = mWaves[wave];
            Context* NewJsonWave = nullptr;
            for(sint32 timesec = 0; timesec < mState.mTimelineLength; ++timesec)
            {
                auto& CurMonsterEvent = CurWave.mEventMonsters[timesec];
                auto& CurMissionEvent = CurWave.mEventMissions[timesec];
                auto& CurScriptEvent = CurWave.mEventScripts[timesec];
                if(CurMonsterEvent.Count() + CurMissionEvent.Count() + CurScriptEvent.Count() == 0)
                    continue;
                if(!NewJsonWave)
                {
                    const sint32 NewWaveIndex = JsonWave.At("Waves").LengthOfIndexable();
                    NewJsonWave = &JsonWave.At("Waves").At(NewWaveIndex);
                    NewJsonWave->At("Title").Set(CurWave.mTitle);
                }
                auto& NewJsonEvent = NewJsonWave->At("Events").At(NewJsonWave->At("Events").LengthOfIndexable());
                NewJsonEvent.At("TimeSec").Set(String::FromInteger(timesec));

                for(sint32 mon = 0, mon_end = CurMonsterEvent.Count(); mon < mon_end; ++mon)
                {
                    auto& NewJsonMonster = NewJsonEvent.At("Monsters").At(mon);
                    NewJsonMonster.At("ID").Set(CurMonsterEvent[mon].mType->mID);
                    NewJsonMonster.At("RID").Set(String::FromInteger(CurMonsterEvent[mon].mRID));
                    if(mState.mLandscape)
                    {
                        NewJsonMonster.At("PosX").Set(String::FromFloat(CurMonsterEvent[mon].mPos.y));
                        NewJsonMonster.At("PosY").Set(String::FromFloat(-CurMonsterEvent[mon].mPos.x));
                    }
                    else
                    {
                        NewJsonMonster.At("PosX").Set(String::FromFloat(CurMonsterEvent[mon].mPos.x));
                        NewJsonMonster.At("PosY").Set(String::FromFloat(CurMonsterEvent[mon].mPos.y));
                    }
                }

                for(sint32 mss = 0, mss_end = CurMissionEvent.Count(); mss < mss_end; ++mss)
                {
                    auto& NewJsonMission = NewJsonEvent.At("Missions").At(mss);
                    const auto* CurMission = CurMissionEvent.AccessByOrder(mss);
                    NewJsonMission.At("TargetRID").Set(String::FromInteger(CurMission->mTargetRID));
                    NewJsonMission.At("RID").Set(String::FromInteger(CurMission->mRID));
                    if(mState.mLandscape)
                    {
                        NewJsonMission.At("PosX").Set(String::FromFloat(CurMission->mPos.y));
                        NewJsonMission.At("PosY").Set(String::FromFloat(-CurMission->mPos.x));
                    }
                    else
                    {
                        NewJsonMission.At("PosX").Set(String::FromFloat(CurMission->mPos.x));
                        NewJsonMission.At("PosY").Set(String::FromFloat(CurMission->mPos.y));
                    }
                }

                for(sint32 scr = 0, scr_end = CurScriptEvent.Count(); scr < scr_end; ++scr)
                {
                    auto& NewJsonScript = NewJsonEvent.At("Scripts").At(scr);
                    const auto* CurScript = CurScriptEvent.AccessByOrder(scr);
                    NewJsonScript.At("TargetRID").Set(String::FromInteger(CurScript->mTargetRID));
                    if(mState.mLandscape)
                    {
                        NewJsonScript.At("PosX").Set(String::FromFloat(CurScript->mPos.y));
                        NewJsonScript.At("PosY").Set(String::FromFloat(-CurScript->mPos.x));
                    }
                    else
                    {
                        NewJsonScript.At("PosX").Set(String::FromFloat(CurScript->mPos.x));
                        NewJsonScript.At("PosY").Set(String::FromFloat(CurScript->mPos.y));
                    }
                    NewJsonScript.At("Text").Set(CurScript->mText);
                }
            }
        }

        String TextString = JsonWave.SaveJson();
        Platform::File::Write(TextFile, (bytes)(chars) TextString, TextString.Length());
        Platform::File::Close(TextFile);
    }
}

void stagetoolData::Render(ZayPanel& panel)
{
    // 인게임
    ZAY_XYWH(panel, mMapPos.x + mState.mInGameX, mMapPos.y + mState.mInGameY, mState.mInGameW, mState.mInGameH)
    {
        Rect OutlineRect = mState.RenderMap(true, panel);
        OutlineRect += Point(mMapPos.x + mState.mInGameX, mMapPos.y + mState.mInGameY);
        // 게임영역 표시
        ZAY_RGB(panel, 255, 255, 128)
            panel.rect(1);

        // 현재 타임라인의 몬스터들
        auto& CurMonsters = mWaves[mCurWave].mEventMonsters[mCurEvent];
        ZAY_FONT(panel, 0.9, "Arial Black")
        for(sint32 i = 0, iend = CurMonsters.Count(); i < iend; ++i)
        {
            const float x = mState.mInGameW * (CurMonsters[i].mPos.x + 0.5f);
            const float y = mState.mInGameH * (CurMonsters[i].mPos.y + 0.5f);
            ZAY_XYRR(panel, x, y, mState.mMonsterSizeR, mState.mMonsterSizeR)
            {
                if(mMode != Mode::Mob)
                    mState.RenderImage(false, panel, R(CurMonsters[i].mType->imageName()));
                else
                {
                    // 위치
                    ZAY_RGB(panel, 255, 0, 0)
                        panel.circle();
                    mState.RenderImage(false, panel, R(CurMonsters[i].mType->imageName()));
                    // 타입ID-음영
                    ZAY_RGB(panel, 255, 0, 0)
                        panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, CurMonsters[i].mType->mID, UIFA_CenterMiddle);
                    // 타입ID
                    ZAY_RGB(panel, 255, 255, 0)
                        panel.text(panel.w() / 2, panel.h() / 2, CurMonsters[i].mType->mID, UIFA_CenterMiddle);
                }
            }
        }

        // 그리드
        if(mGridMode && mMode == Mode::Mob)
            RenderGrid(panel);

        const float SpotSizeR = 5;
        if(mMode == Mode::Mission)
        {
            for(sint32 i = 0, iend = CurMonsters.Count(); i < iend; ++i)
            {
                sint32 CurRID = CurMonsters[i].mRID;
                float AreaX = mState.mInGameW * (CurMonsters[i].mPos.x + 0.5f);
                float AreaY = mState.mInGameH * (CurMonsters[i].mPos.y + 0.5f);

                TimelineMission* NextMission = nullptr;
                do
                {
                    NextMission = mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Access(CurRID);
                    ZAY_XYRR(panel, AreaX, AreaY, SpotSizeR, SpotSizeR)
                    ZAY_RGBA(panel, 0, 0, 255, 128)
                        RenderMission(panel, CurRID);
                    if(NextMission)
                    {
                        CurRID = NextMission->mRID;
                        AreaX += mState.mInGameSize * NextMission->mPos.x;
                        AreaY += mState.mInGameSize * NextMission->mPos.y;
                    }
                }
                while(NextMission);
            }
        }
        else if(mMode == Mode::Script)
        {
            for(sint32 i = 0; i < mState.mLayerLength; ++i)
            {
                if(!mState.mLayers[i].mShowFlag) continue;
                // 오브젝트 스크립트
                if(mShowScript[0])
                {
                    auto& CurObjects = mState.mLayers[i].mObjects;
                    for(sint32 j = 0, jend = CurObjects.Count(); j < jend; ++j)
                    {
                        if(!CurObjects[j].mVisible) continue;
                        const float AreaL = mState.mInGameW * (CurObjects[j].mCurrentRect.l + 0.5f);
                        const float AreaT = mState.mInGameH * (CurObjects[j].mCurrentRect.t + 0.5f);
                        const float AreaR = mState.mInGameW * (CurObjects[j].mCurrentRect.r + 0.5f);
                        const float AreaB = mState.mInGameH * (CurObjects[j].mCurrentRect.b + 0.5f);
                        ZAY_XYRR(panel, (AreaL + AreaR) / 2, (AreaT + AreaB) / 2, SpotSizeR, SpotSizeR)
                        ZAY_RGBA(panel, 255, 255, 0, 128)
                            RenderScript(panel, mCurEvent, CurObjects[j].mRID);
                    }
                }
                // 폴리곤 스크립트
                if(mShowScript[1])
                {
                    auto& CurPolygons = mState.mLayers[i].mPolygons;
                    for(sint32 j = 0, jend = CurPolygons.Count(); j < jend; ++j)
                    {
                        if(!CurPolygons[j].mVisible) continue;
                        const float AreaX = mState.mInGameW * (CurPolygons[j].mDots[0].x + 0.5f);
                        const float AreaY = mState.mInGameH * (CurPolygons[j].mDots[0].y + 0.5f);
                        ZAY_XYRR(panel, AreaX, AreaY, SpotSizeR, SpotSizeR)
                        ZAY_RGBA(panel, 255, 0, 255, 128)
                            RenderScript(panel, mCurEvent, CurPolygons[j].mRID);
                    }
                }
            }
            // 몬스터 스크립트
            if(mShowScript[2])
            for(sint32 i = 0, iend = CurMonsters.Count(); i < iend; ++i)
            {
                float AreaX = mState.mInGameW * (CurMonsters[i].mPos.x + 0.5f);
                float AreaY = mState.mInGameH * (CurMonsters[i].mPos.y + 0.5f);
                ZAY_XYRR(panel, AreaX, AreaY, SpotSizeR, SpotSizeR)
                ZAY_RGBA(panel, 255, 0, 0, 128)
                    RenderScript(panel, mCurEvent, CurMonsters[i].mRID);
                // 몬스터미션 스크립트
                sint32 CurRID = CurMonsters[i].mRID;
                ZAY_RGBA(panel, 255, 128, 0, 128)
                while(TimelineMission* CurMission = mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Access(CurRID))
                {
                    CurRID = CurMission->mRID;
                    const float NewAreaX = AreaX + mState.mInGameSize * CurMission->mPos.x;
                    const float NewAreaY = AreaY + mState.mInGameSize * CurMission->mPos.y;
                    ZAY_RGBA(panel, 64, 64, 64, 192)
                        panel.line(Point(AreaX, AreaY), Point(NewAreaX, NewAreaY), 2);
                    ZAY_XYRR(panel, NewAreaX, NewAreaY, SpotSizeR, SpotSizeR)
                        RenderScript(panel, mCurEvent, CurRID);
                    AreaX = NewAreaX;
                    AreaY = NewAreaY;
                }
            }
        }
    }

    // 툴UI
    const float OuterSize = ButtonSize * 1.5 * mUITween.value() / 100;
    ZAY_LTRB(panel, -OuterSize, -OuterSize, panel.w() + OuterSize, panel.h() + OuterSize)
    ZAY_FONT(panel, 1.0, "Arial Black")
    {
        // 고정버튼
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
            RenderLockToggle(panel);

        // 작업모드
        ZAY_XYWH(panel, ButtonSize * 2, 0, ButtonSize, ButtonSize)
            RenderModeToggle(panel);

        // 그리드모드
        if(mMode == Mode::Mob)
        ZAY_XYWH(panel, ButtonSize * 3, 0, ButtonSize, ButtonSize)
            RenderGridToggle(panel);

        // 이동버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, 0, ButtonSize, ButtonSize)
            RenderDragButton(panel);

        // 로드버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, ButtonSize * 1, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "load",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String FileName;
                    if(Platform::Popup::FileDialog(FileName, nullptr, "Load Stage(json)"))
                        Load(FileName);
                }
            })
        {
            ZAY_RGBA(panel, 64, 128, 255, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Load\nStage", UIFA_CenterMiddle);
            }
        }

        // 세이브버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, ButtonSize * 2, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "save",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String FileName;
                    if(Platform::Popup::FileDialog(FileName, nullptr, "Save Stage(json)"))
                        Save(FileName);
                }
            })
        {
            ZAY_RGBA(panel, 64, 128, 255, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Save\nStage", UIFA_CenterMiddle);
            }
        }

        // 스크립트 보여짐여부
        if(mMode == Mode::Script)
        {
            const sint32 ShowCount = 3;
            const String ShowName[3] = {"Object", "Polygon", "Monster"};
            const Color ShowColor[3] = {Color(255, 255, 0, 192), Color(255, 0, 255, 192), Color(255, 0, 0, 192)};
            ZAY_XYWH(panel, panel.w() - InnerGap - IconSize * 2, ButtonSize * 3 + InnerGap, IconSize * 2, IconSize * ShowCount)
            {
                for(sint32 i = 0; i < ShowCount; ++i)
                {
                    ZAY_XYWH_UI(panel, 0, IconSize * i, panel.w(), IconSize, String::Format("show-%d", i),
                        ZAY_GESTURE_NT(n, t, this)
                        {
                            if(t == GT_InReleased)
                            {
                                const sint32 i = Parser::GetInt(&n[5]);
                                mShowScript[i] ^= true;
                            }
                        })
                    {
                        ZAY_COLOR_IF(panel, ShowColor[i], mShowScript[i])
                        ZAY_RGBA_IF(panel, 128, 128, 128, 192, !mShowScript[i])
                            panel.fill();
                        ZAY_RGB(panel, 0, 0, 0)
                        {
                            panel.rect(2);
                            panel.text(ShowName[i], UIFA_CenterMiddle, UIFE_Right);
                        }
                    }
                }
            }
        }

        // 홈버튼
        ZAY_XYWH(panel, 0, 0, ButtonSize, ButtonSize)
            RenderHomeButton(panel);

        // 맵로드버튼
        ZAY_XYWH(panel, 0, ButtonSize, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "load_map",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String MapName = mMapName;
                    if(Platform::Popup::TextDialog(MapName, "Load Map", "Typing the asset name"))
                    {
                        mMapName = MapName;
                        id_asset_read TextAsset = Asset::OpenForRead("table/" + mMapName + ".json");
                        if(TextAsset)
                        {
                            const sint32 TextSize = Asset::Size(TextAsset);
                            buffer TextBuffer = Buffer::Alloc(BOSS_DBG TextSize + 1);
                            Asset::Read(TextAsset, (uint08*) TextBuffer, TextSize);
                            Asset::Close(TextAsset);
                            ((char*) TextBuffer)[TextSize] = '\0';
                            mState.LoadMap((chars) TextBuffer);
                            Buffer::Free(TextBuffer);
                        }
                        else mState.LoadMap("");
                    }
                }
            })
        {
            ZAY_RGBA(panel, 64, 255, 128, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Load\nMap", UIFA_CenterMiddle);
            }
        }

        // 몬스터리스트
        if(mMode == Mode::Mob)
        {
            ZAY_XYWH_SCISSOR(panel, InnerGap, ButtonSize * 2 + InnerGap, IconSize * 2, IconSize * mMonsterScrollMax)
            {
                const sint32 MonsterCount = mState.mMonsterTypes.Count();
                // 몬스터통계
                sint32s SumMonsters;
                Memory::Set(SumMonsters.AtDumpingAdded(MonsterCount), 0, sizeof(sint32) * MonsterCount);
                for(sint32 i = 0; i < mState.mTimelineLength; ++i)
                {
                    auto& CurMonsters = mWaves[mCurWave].mEventMonsters[i];
                    for(sint32 j = 0, jend = CurMonsters.Count(); j < jend; ++j)
                    {
                        for(sint32 k = 0; k < MonsterCount; ++k)
                            if(CurMonsters[j].mType == &mState.mMonsterTypes[k])
                            {
                                SumMonsters.At(k)++;
                                break;
                            }
                    }
                }
                const float ScrollPos = mMonsterScroll.value();
                for(sint32 i = 0; i < MonsterCount; ++i)
                {
                    ZAY_XYWH_UI(panel, 0, IconSize * (i - ScrollPos), panel.w(), IconSize, String::Format("monster-%d", i),
                        ZAY_GESTURE_NT(n, t, this)
                        {
                            if(t == GT_InReleased)
                            {
                                mCurMonster = Parser::GetInt(&n[8]);
                                const sint32 ScrollLimit = Math::Max(0, mState.mMonsterTypes.Count() - mMonsterScrollMax);
                                mMonsterScroll.MoveTo(Math::Clamp(mCurMonster - mMonsterScrollMax / 2, 0, ScrollLimit), 0.5);
                            }
                        })
                    {
                        ZAY_RGBA_IF(panel, 255, 128, 64, 192, i == mCurMonster)
                        ZAY_RGBA_IF(panel, 128, 128, 128, 192, i != mCurMonster)
                            panel.fill();

                        ZAY_RGB(panel, 0, 0, 0)
                            panel.rect(2);
                        ZAY_INNER_SCISSOR(panel, 4)
                        ZAY_XYWH(panel, (panel.w() - IconSize / 2) / 2, panel.h() * 3 / 4, IconSize / 2, panel.h() / 4)
                        ZAY_RGBA(panel, 128, 128, 128, 64)
                            mState.RenderImage(false, panel, R(mState.mMonsterTypes[i].imageName()));
                        ZAY_RGB(panel, 0, 0, 0)
                            panel.text(mState.mMonsterTypes[i].mID, UIFA_CenterMiddle, UIFE_Right);

                        if(0 < SumMonsters[i])
                        ZAY_RGBA(panel, 255, 0, 255, 128)
                            panel.text(panel.w(), panel.h() / 2, String::Format(" (%d)", SumMonsters[i]), UIFA_LeftMiddle);
                    }
                }
                ZAY_INNER(panel, 2)
                ZAY_RGB(panel, 0, 0, 0)
                    panel.rect(2);
            }
        }

        // 타임라인
        ZAY_LTRB(panel, 0, panel.h() - ButtonSize, panel.w(), panel.h())
        ZAY_INNER_UI(panel, InnerGap, "timeline_board")
        {
            ZAY_RGBA(panel, 64, 255, 128, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                // 웨이브 이동버튼
                const sint32 SkipButtonSize = panel.h();
                ZAY_FONT(panel, 1.5)
                {
                    // 이전
                    ZAY_LTRB(panel, 0, 0, SkipButtonSize, panel.h())
                    ZAY_INNER_UI(panel, InnerGap, "prev_wave",
                        ZAY_GESTURE_T(t, this)
                        {
                            if(t == GT_InReleased)
                            {
                                mCurWave = Math::Max(0, mCurWave - 1);
                            }
                        })
                    {
                        ZAY_RGBA(panel, 64, 128, 255, 64)
                            panel.fill();
                        ZAY_RGB(panel, 0, 0, 0)
                        {
                            panel.rect(2);
                            panel.text("◀", UIFA_CenterMiddle);
                        }
                    }
                    // 다음
                    ZAY_LTRB(panel, panel.w() - SkipButtonSize, 0, panel.w(), panel.h())
                    ZAY_INNER_UI(panel, InnerGap, "next_wave",
                        ZAY_GESTURE_T(t, this)
                        {
                            if(t == GT_InReleased)
                            {
                                if(++mCurWave == mWaves.Count())
                                {
                                    auto& NewWave = mWaves.AtAdding();
                                    NewWave.mEventMonsters.AtDumpingAdded(mState.mTimelineLength);
                                    NewWave.mEventMissions.AtDumpingAdded(mState.mTimelineLength);
                                    NewWave.mEventScripts.AtDumpingAdded(mState.mTimelineLength);
                                }
                            }
                        })
                    {
                        ZAY_RGBA(panel, 64, 128, 255, 64)
                            panel.fill();
                        ZAY_RGB(panel, 0, 0, 0)
                        {
                            panel.rect(2);
                            panel.text("▶", UIFA_CenterMiddle);
                        }
                    }
                }
                // 프로그레스바
                const sint32 ProgressInnerSize = (panel.h() - InnerGap) / 2;
                ZAY_LTRB_UI(panel, SkipButtonSize, ProgressInnerSize, panel.w() - SkipButtonSize, panel.h() - ProgressInnerSize, "timeline",
                    ZAY_GESTURE_NTXY(n, t, x, y, this)
                    {
                        if(t == GT_Pressed || t == GT_InDragging || t == GT_OutDragging)
                        {
                            const rect128& CurRect = rect(n);
                            const sint32 CalcedEvent = (x - CurRect.l) * mState.mTimelineLength / (CurRect.r - CurRect.l);
                            mCurEvent = Math::Clamp(CalcedEvent, 0, mState.mTimelineLength - 1);
                            invalidate();
                        }
                    })
                {
                    // 타임라인 노드
                    for(sint32 i = 0; i < mState.mTimelineLength; ++i)
                    {
                        const bool IsOdd = i & 1;
                        ZAY_LTRB(panel, panel.w() * i / mState.mTimelineLength, 0, panel.w() * (i + 1) / mState.mTimelineLength, panel.h())
                        ZAY_RGB_IF(panel, 240, 240, 240, IsOdd)
                        ZAY_RGB_IF(panel, 255, 255, 255, !IsOdd)
                        ZAY_RGB_IF(panel, 150, 150, 100, i == mCurEvent)
                        ZAY_RGB_IF(panel, 200, 50, 50, mMode == Mode::Mob && 0 < mWaves[mCurWave].mEventMonsters[i].Count())
                        ZAY_RGB_IF(panel, 50, 50, 200, mMode == Mode::Mission && 0 < mWaves[mCurWave].mEventMissions[i].Count())
                        ZAY_RGB_IF(panel, 200, 50, 200, mMode == Mode::Script && 0 < mWaves[mCurWave].mEventScripts[i].Count())
                        {
                            panel.fill();
                            // 타임라인별 시스템 스크립트
                            if(mMode == Mode::Script)
                                RenderScript(panel, i, 0, false, i == mCurEvent);
                        }
                    }
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.rect(2);
                    ZAY_RGBA(panel, 255, 128, 64, 192)
                    {
                        // 커서위치
                        ZAY_XYRR(panel, panel.w() * (mCurEvent + 0.5) / mState.mTimelineLength, 0, 0, 0)
                            panel.text(0, 0, "▼", UIFA_CenterBottom);
                        // 커서시간
                        ZAY_FONT(panel, 0.8)
                        ZAY_XYRR(panel, panel.w() * (mCurEvent + 0.5) / mState.mTimelineLength, panel.h(), 0, 0)
                            panel.text(0, 3, String::Format("%ds", mCurEvent), UIFA_CenterTop);
                    }
                }
            }

            ZAY_RGB(panel, 0, 0, 0)
                panel.text(panel.w() / 2, -3,
                    String::Format("[Wave%d.%s]", mCurWave + 1, (chars) mWaves[mCurWave].mTitle), UIFA_CenterBottom);
        }
    }
}

void stagetoolData::RenderModeToggle(ZayPanel& panel)
{
    ZAY_INNER_UI(panel, InnerGap, "script",
        ZAY_GESTURE_T(t, this)
        {
            if(t == GT_InReleased)
            {
                switch(mMode)
                {
                case Mode::Mob: mMode = Mode::Mission; break;
                case Mode::Mission: mMode = Mode::Script; break;
                case Mode::Script: mMode = Mode::Mob; break;
                }
            }
        })
    {
        ZAY_RGBA_IF(panel, 255, 128, 128, 192, mMode == Mode::Mob)
        ZAY_RGBA_IF(panel, 128, 128, 255, 192, mMode == Mode::Mission)
        ZAY_RGBA_IF(panel, 255, 128, 255, 192, mMode == Mode::Script)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            panel.rect(2);
            switch(mMode)
            {
            case Mode::Mob: panel.text("Mob", UIFA_CenterMiddle); break;
            case Mode::Mission: panel.text("Mission", UIFA_CenterMiddle); break;
            case Mode::Script: panel.text("Script", UIFA_CenterMiddle); break;
            }
        }
    }
}

void stagetoolData::RenderMission(ZayPanel& panel, sint32 rid)
{
    ZayPanel::SubGestureCB UI_CB = ZAY_GESTURE_VNTXY(v, n, t, x, y, this, rid)
        {
            if(t == GT_Pressed)
            {
                BOSS_ASSERT("잘못된 시나리오입니다", !mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Access(rid));
                TimelineMission& NewMission = mWaves.At(mCurWave).mEventMissions.At(mCurEvent)[rid];
                NewMission.mTargetRID = rid;
                NewMission.mRID = ++m->mState.mMissionLastRID;
                NewMission.mPos = Point(0, 0);
                NewMission.mLocked = true;
            }
            else if(t == GT_ExtendPress)
            {
                auto& MissionMap = mWaves.At(mCurWave).mEventMissions.At(mCurEvent);
                for(sint32 i = 0, iend = MissionMap.Count(); i < iend; ++i)
                {
                    auto* CurMission = MissionMap.AccessByOrder(i);
                    if(CurMission->mRID == rid)
                    {
                        mWaves.At(mCurWave).mEventScripts.At(mCurEvent).Remove(CurMission->mRID); // Script선행삭제
                        mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Remove(CurMission->mTargetRID);
                        v->invalidate();
                        break;
                    }
                }
            }
            else if(t == GT_InDragging || t == GT_OutDragging)
            {
                if(TimelineMission* CurMission = mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Access(rid))
                {
                    const point64& OldXY = v->oldxy(n);
                    CurMission->mPos.x += (x - OldXY.x) / (float) mState.mInGameSize;
                    CurMission->mPos.y += (y - OldXY.y) / (float) mState.mInGameSize;
                    v->invalidate();
                }
            }
            else if(t == GT_InReleased || t == GT_OutReleased)
            {
                if(TimelineMission* CurMission = mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Access(rid))
                    CurMission->mLocked = false;
            }
        };
    ZayPanel::SubGestureCB NULL_CB = nullptr;

    TimelineMission* CurMission = mWaves.At(mCurWave).mEventMissions.At(mCurEvent).Access(rid);
    const bool DisableUI = (CurMission && !CurMission->mLocked);
    ZAY_INNER_UI(panel, -4, (DisableUI)? nullptr : (chars) String::Format("mission_spot_%d", rid), (DisableUI)? NULL_CB : UI_CB)
    {
        panel.fill();
        ZAY_INNER(panel, 4)
        {
            if(!CurMission)
            {
                ZAY_RGB(panel, 0, 0, 0)
                    panel.fill();
            }
            else
            {
                const Point BeginPos(panel.w() / 2, panel.h() / 2);
                const Point EndPos = BeginPos + CurMission->mPos * mState.mInGameSize;
                panel.line(BeginPos, EndPos, 2);
                ZAY_RGB(panel, 0, 0, 0)
                    panel.circle();

                if(DisableUI)
                ZAY_XYRR_UI(panel, EndPos.x, EndPos.y, panel.w() / 2 + 4, panel.h() / 2 + 4, String::Format("mission_spot_end_%d", rid),
                    ZAY_GESTURE_VNTXY(v, n, t, x, y, this, CurMission)
                    {
                        if(t == GT_InDragging || t == GT_OutDragging)
                        {
                            const point64& OldXY = v->oldxy(n);
                            CurMission->mPos.x += (x - OldXY.x) / (float) mState.mInGameSize;
                            CurMission->mPos.y += (y - OldXY.y) / (float) mState.mInGameSize;
                            v->invalidate();
                        }
                    });
            }
        }
    }
}

void stagetoolData::RenderScript(ZayPanel& panel, sint32 eid, sint32 rid, bool circle, bool ui)
{
    ZayPanel::SubGestureCB UI_CB = ZAY_GESTURE_VNTXY(v, n, t, x, y, this, eid, rid)
        {
            if(t == GT_Pressed)
            {
                TimelineScript& NewScript = mWaves.At(mCurWave).mEventScripts.At(eid)[rid];
                NewScript.mTargetRID = rid;
                NewScript.mPos = Point(0, 0);
                NewScript.mLocked = true;
            }
            else if(t == GT_InDragging || t == GT_OutDragging)
            {
                if(TimelineScript* CurScript = mWaves.At(mCurWave).mEventScripts.At(eid).Access(rid))
                {
                    const point64& OldXY = v->oldxy(n);
                    CurScript->mPos.x += (x - OldXY.x) / (float) mState.mInGameSize;
                    CurScript->mPos.y += (y - OldXY.y) / (float) mState.mInGameSize;
                    v->invalidate();
                }
            }
            else if(t == GT_InReleased || t == GT_OutReleased)
            {
                if(TimelineScript* CurScript = mWaves.At(mCurWave).mEventScripts.At(eid).Access(rid))
                {
                    CurScript->mLocked = false;
                    String GetText = CurScript->mText;
                    GetText.Replace("\\\"", "\"");
                    if(Platform::Popup::TextDialog(GetText, "Script", "Please enter a script."))
                    {
                        if(CurScript = mWaves.At(mCurWave).mEventScripts.At(eid).Access(rid))
                            CurScript->mText = GetText.Replace("\"", "\\\"");
                    }
                    else mWaves.At(mCurWave).mEventScripts.At(eid).Remove(rid);
                }
            }
        };
    ZayPanel::SubGestureCB NULL_CB = nullptr;

    TimelineScript* CurScript = mWaves.At(mCurWave).mEventScripts.At(eid).Access(rid);
    ZAY_INNER_UI(panel, -4, (!ui)? nullptr : (chars) String::Format("script_spot_%d_%d", eid, rid), (!ui)? NULL_CB : UI_CB)
    {
        if(circle)
            panel.circle();
        ZAY_INNER(panel, 4)
        {
            if(!CurScript)
            {
                if(circle)
                ZAY_RGB(panel, 64, 64, 64)
                    panel.circle();
            }
            else
            {
                const Point BeginPos(panel.w() / 2, panel.h() / 2);
                const Point EndPos = BeginPos + CurScript->mPos * mState.mInGameSize;
                panel.line(BeginPos, EndPos, 2);
                if(circle)
                ZAY_RGB(panel, 0, 0, 0)
                    panel.circle();

                if(!CurScript->mLocked)
                {
                    String SafeString = CurScript->mText;
                    SafeString.Replace("\\\"", "\"");
                    const sint32 TextSize = Math::Min(300, Platform::Graphics::GetStringWidth(SafeString) + 4);
                    ZAY_XYWH_UI(panel, EndPos.x, EndPos.y - 10, TextSize + 20, 20, String::Format("script_box_%d_%d", eid, rid),
                        ZAY_GESTURE_VNTXY(v, n, t, x, y, this, CurScript)
                        {
                            if(t == GT_InDragging || t == GT_OutDragging)
                            {
                                const point64& OldXY = v->oldxy(n);
                                CurScript->mPos.x += (x - OldXY.x) / (float) mState.mInGameSize;
                                CurScript->mPos.y += (y - OldXY.y) / (float) mState.mInGameSize;
                                v->invalidate();
                            }
                        })
                    {
                        ZAY_INNER(panel, -2)
                        ZAY_RGB(panel, 0, 0, 0)
                            panel.fill();
                        ZAY_LTRB(panel, 0, 0, TextSize, panel.h())
                        {
                            ZAY_RGB(panel, 224, 224, 224)
                                panel.fill();
                            ZAY_LTRB(panel, 2, 0, panel.w(), panel.h())
                            ZAY_RGBA(panel, 0, 0, 255, 192)
                                panel.text(SafeString, UIFA_LeftMiddle, UIFE_Right);
                        }
                        const String ExitName = String::Format("script_exit_%d_%d", eid, rid);
                        ZAY_LTRB_UI(panel, TextSize + 2, 0, panel.w(), panel.h(), ExitName,
                            ZAY_GESTURE_T(t, this, eid, rid)
                            {
                                if(t == GT_InReleased)
                                    mWaves.At(mCurWave).mEventScripts.At(eid).Remove(rid);
                            })
                        {
                            ZAY_RGB(panel, -255, -128, -128)
                                panel.fill();
                            ZAY_RGB(panel, 0, 0, 0)
                            ZAY_RGB_IF(panel, 255, 255, 255, panel.state(ExitName) & PS_Focused)
                                panel.text(panel.w() / 2, panel.h() / 2, "x", UIFA_CenterMiddle);
                        }
                    }
                }
                else
                {
                    ZAY_XYRR(panel, EndPos.x, EndPos.y, 4, 4)
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.fill();
                }
            }
        }
    }
}

void stagetoolData::OnModeChanged()
{
}

void stagetoolData::OnSelectSub(chars name)
{
}

void stagetoolData::InitSelectBox(sint32 index)
{
}

void stagetoolData::QuitSelectBox(sint32 index)
{
}

void stagetoolData::ChangeSelectBox(sint32 type, sint32 index)
{
}

void stagetoolData::OnSelectBoxMoving(sint32 index, float addx, float addy)
{
}

void stagetoolData::OnSelectBoxMoved(sint32 index)
{
}

void stagetoolData::OnSelectBoxSizing(sint32 index, float addx, float addy)
{
}

void stagetoolData::OnSelectBoxSized(sint32 index)
{
}

void stagetoolData::OnSelectBoxClone(sint32 index)
{
}
