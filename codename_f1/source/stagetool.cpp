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
    static bool IsDraggingEnabled = true;

    if(type == GT_Pressed)
    {
        auto& CurMonsters = m->mWaves.At(m->mCurWave).mEvents.At(m->mCurEvent);
        // 삭제인지 판단
        for(sint32 i = 0, iend = CurMonsters.Count(); i < iend; ++i)
        {
            const float MonsterX = m->mState.mInGameX + m->mState.mInGameW * (CurMonsters[i].mPos.x + 0.5f);
            const float MonsterY = m->mState.mInGameY + m->mState.mInGameH * (CurMonsters[i].mPos.y + 0.5f);
            if(Math::Distance(x - m->mMapPos.x, y - m->mMapPos.y, MonsterX, MonsterY) < m->mState.mMonsterSizeR)
            {
                CurMonsters.SubtractionSection(i);
                IsDraggingEnabled = false;
                return;
            }
        }

        // 새로 생성
        auto& NewMonster = CurMonsters.AtAdding();
        NewMonster.mType = &m->mState.mMonsterTypes[m->mCurMonster];
        NewMonster.mPos.x = Pos.x;
        NewMonster.mPos.y = Pos.y;
        IsDraggingEnabled = true;
    }
    else if((type == GT_InDragging || type == GT_OutDragging) && IsDraggingEnabled)
    {
        auto& CurMonsters = m->mWaves.At(m->mCurWave).mEvents.At(m->mCurEvent);
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

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    // 출력미비영역 표현용
    ZAY_RGB(panel, 128, 128, 128)
        panel.fill();
    m->Render(panel);
}

stagetoolData::stagetoolData()
{
    mCurWave = 0;
    mCurEvent = 0;
    mCurMonster = 0;
    mMapName = "";
    mWaves.AtAdding().mEvents.AtDumpingAdded(mState.mTimelineLength);
}

stagetoolData::~stagetoolData()
{
}

void stagetoolData::Load(chars filename)
{
    mCurWave = 0;
    mCurEvent = 0;
    mCurMonster = 0;
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
        for(sint32 wave = 0, wave_end = JsonWave("Waves").LengthOfIndexable(); wave < wave_end; ++wave)
        {
            auto& NewWave = mWaves.AtAdding();
            NewWave.mEvents.AtDumpingAdded(mState.mTimelineLength);
            auto& CurJsonWave = JsonWave("Waves")[wave];
            NewWave.mTitle = CurJsonWave("Title").GetString();
            for(sint32 evt = 0, evt_end = CurJsonWave("Events").LengthOfIndexable(); evt < evt_end; ++evt)
            {
                auto& CurJsonEvent = CurJsonWave("Events")[evt];
                const sint32 TimeSec = Math::Clamp(CurJsonEvent("TimeSec").GetInt(), 0, mState.mTimelineLength - 1);
                auto& CurEvent = NewWave.mEvents.At(TimeSec);
                for(sint32 mon = 0, mon_end = CurJsonEvent("Monsters").LengthOfIndexable(); mon < mon_end; ++mon)
                {
                    auto& CurJsonMonster = CurJsonEvent("Monsters")[mon];
                    auto& NewTimelineMonster = CurEvent.AtAdding();
                    const String CurJsonMonsterID = CurJsonMonster("ID").GetString();
                    NewTimelineMonster.mType = &mState.mMonsterTypes[0];
                    for(sint32 i = 0, iend = mState.mMonsterTypes.Count(); i < iend; ++i)
                        if(!CurJsonMonsterID.Compare(mState.mMonsterTypes[i].mID))
                        {
                            NewTimelineMonster.mType = &mState.mMonsterTypes[i];
                            break;
                        }
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
            }
        }
    }

    if(mWaves.Count() == 0)
        mWaves.AtAdding().mEvents.AtDumpingAdded(mState.mTimelineLength);

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
        for(sint32 wave = 0, wave_end = mWaves.Count(); wave < wave_end; ++wave)
        {
            auto& CurWave = mWaves[wave];
            Context* NewJsonWave = nullptr;
            for(sint32 timesec = 0; timesec < mState.mTimelineLength; ++timesec)
            {
                auto& CurEvent = CurWave.mEvents[timesec];
                if(CurEvent.Count() == 0) continue;
                if(!NewJsonWave)
                {
                    const sint32 NewWaveIndex = JsonWave.At("Waves").LengthOfIndexable();
                    NewJsonWave = &JsonWave.At("Waves").At(NewWaveIndex);
                    NewJsonWave->At("Title").Set(CurWave.mTitle);
                }
                auto& NewJsonEvent = NewJsonWave->At("Events").At(NewJsonWave->At("Events").LengthOfIndexable());
                NewJsonEvent.At("TimeSec").Set(String::FromInteger(timesec));
                for(sint32 mon = 0, mon_end = CurEvent.Count(); mon < mon_end; ++mon)
                {
                    auto& NewJsonMonster = NewJsonEvent.At("Monsters").At(mon);
                    NewJsonMonster.At("ID").Set(CurEvent[mon].mType->mID);
                    if(mState.mLandscape)
                    {
                        NewJsonMonster.At("PosX").Set(String::FromFloat(CurEvent[mon].mPos.y));
                        NewJsonMonster.At("PosY").Set(String::FromFloat(-CurEvent[mon].mPos.x));
                    }
                    else
                    {
                        NewJsonMonster.At("PosX").Set(String::FromFloat(CurEvent[mon].mPos.x));
                        NewJsonMonster.At("PosY").Set(String::FromFloat(CurEvent[mon].mPos.y));
                    }
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
        mState.Render(true, panel);

        // 현재 타임라인의 몬스터들
        auto& CurMonsters = mWaves[mCurWave].mEvents[mCurEvent];
        ZAY_FONT(panel, 0.9, "Arial Black")
        for(sint32 i = 0, iend = CurMonsters.Count(); i < iend; ++i)
        {
            const float x = mState.mInGameW * (CurMonsters[i].mPos.x + 0.5f);
            const float y = mState.mInGameH * (CurMonsters[i].mPos.y + 0.5f);
            ZAY_XYRR(panel, x, y, mState.mMonsterSizeR, mState.mMonsterSizeR)
            {
                // 위치
                ZAY_RGB(panel, 255, 0, 0)
                    panel.circle();
                // 타입ID-음영
                ZAY_RGB(panel, 255, 0, 0)
                    panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, CurMonsters[i].mType->mID, UIFA_CenterMiddle);
                // 타입ID
                ZAY_RGB(panel, 255, 255, 0)
                    panel.text(panel.w() / 2, panel.h() / 2, CurMonsters[i].mType->mID, UIFA_CenterMiddle);
            }
        }

        // 그리드
        if(mGridMode)
            RenderGrid(panel);
    }

    // 툴UI
    const float OuterSize = ButtonSize * 1.5 * mUITween.value() / 100;
    ZAY_LTRB(panel, -OuterSize, -OuterSize, panel.w() + OuterSize, panel.h() + OuterSize)
    ZAY_FONT(panel, 1.0, "Arial Black")
    {
        // 고정버튼
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
            RenderLockToggle(panel);

        // 그리드모드
        ZAY_XYWH(panel, ButtonSize * 2, 0, ButtonSize, ButtonSize)
            RenderGridToggle(panel);

        // 선택모드
        //ZAY_XYWH(panel, ButtonSize * 3, 0, ButtonSize, ButtonSize)
        //    RenderSelectToggle(panel);

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
        const sint32 MonsterCount = mState.mMonsterTypes.Count();
        ZAY_XYWH(panel, InnerGap, ButtonSize * 2 + InnerGap, IconSize, IconSize * MonsterCount)
        {
            // 몬스터통계
            sint32s SumMonsters;
            Memory::Set(SumMonsters.AtDumpingAdded(MonsterCount), 0, sizeof(sint32) * MonsterCount);
            for(sint32 i = 0; i < mState.mTimelineLength; ++i)
            {
                auto& CurMonsters = mWaves[mCurWave].mEvents[i];
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
            for(sint32 i = 0; i < MonsterCount; ++i)
            {
                ZAY_XYWH_UI(panel, 0, IconSize * i, IconSize, IconSize, String::Format("monster-%d", i),
                    ZAY_GESTURE_NT(n, t, this)
                    {
                        if(t == GT_InReleased)
                            mCurMonster = Parser::GetInt(&n[8]);
                    })
                {
                    ZAY_RGBA_IF(panel, 255, 128, 64, 192, i == mCurMonster)
                    ZAY_RGBA_IF(panel, 128, 128, 128, 192, i != mCurMonster)
                        panel.fill();
                    ZAY_RGB(panel, 0, 0, 0)
                    {
                        panel.rect(2);
                        panel.text(String::Format("M-%d", i + 1), UIFA_CenterMiddle);
                    }
                    if(0 < SumMonsters[i])
                    ZAY_RGBA(panel, 0, 0, 255, 128)
                        panel.text(panel.w(), panel.h() / 2, String::Format(" (%d)", SumMonsters[i]), UIFA_LeftMiddle);
                }
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
                                    mWaves.AtAdding().mEvents.AtDumpingAdded(mState.mTimelineLength);
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
                    for(sint32 i = 0; i < mState.mTimelineLength; ++i)
                    {
                        const bool IsOdd = i & 1;
                        ZAY_LTRB(panel, panel.w() * i / mState.mTimelineLength, 0, panel.w() * (i + 1) / mState.mTimelineLength, panel.h())
                        ZAY_RGB_IF(panel, 240, 240, 240, IsOdd)
                        ZAY_RGB_IF(panel, 255, 255, 255, !IsOdd)
                        ZAY_RGB_IF(panel, 150, 150, 100, i == mCurEvent)
                        ZAY_RGB_IF(panel, 200, 50, 50, 0 < mWaves[mCurWave].mEvents[i].Count())
                            panel.fill();
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
