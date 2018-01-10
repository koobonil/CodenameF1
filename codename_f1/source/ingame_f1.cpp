#include <boss.hpp>
#include "ingame_f1.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("ingameView", ingameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        if(m->mSpineInited)
        {
            if(!m->mPaused)
            {
                // 시간진행
                const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
                static uint64 OldTimeSec = CurTimeMsec / 1000;
                const uint64 CurTimeSec = CurTimeMsec / 1000;
                sint32 CurTimeSecSpan = Math::Min((sint32) (CurTimeSec - OldTimeSec), 1);
                if(0 < CurTimeSecSpan)
                {
                    m->mWaveSecCurrently += CurTimeSecSpan;
                    OldTimeSec = CurTimeSec;
                }
                // 애니메이션 진행
                if(m->mWave != -1)
                {
                    while(m->mWaveSecSettled < m->mWaveSecCurrently)
                        m->PlayScriptOnce(m->mWaveSecSettled++);
                    m->AnimationOnce(CurTimeSecSpan);
                }
            }

            // 인게임종료
            if(0 < m->mClosing && --m->mClosing == 0)
            {
                Platform::Option::SetFlag("LandscapeMode", false);
                m->next("outgameView");
            }
            // 자동화면갱신
            m->invalidate();
        }
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->SetSize(Width, Height);
        if(!m->mSpineInited)
        {
            m->mSpineInited = true;
            m->InitForSpine();
        }
        else m->RebuildTryWorld();
        m->ClearAllPathes(true);
        // 윈도우 타이틀
        Platform::SetWindowName(String::Format("Codename F1 [%dx%d:%.03f]", Width, Height, Height / (float) Width));
    }
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    static bool IsPressEnabled = true;

    if(m->mWave == -1 || m->mPaused || m->mClosing != -1)
        IsPressEnabled = true;
    else
    {
        if(type == GT_Pressed)
        {
            // 동맹군 터치여부
            for(sint32 i = 0, iend = m->mMonsters.Count(); i < iend; ++i)
            {
                auto& CurMonster = m->mMonsters.At(i);
                if(CurMonster.mType->mType != MonsterType::TypeClass::Ally) continue;
                if(m->mWaveSecCurrently < CurMonster.mEntranceSec || 0 < CurMonster.mDeathStep) continue;

                float AllyX = m->mInGameX + m->mInGameW * (CurMonster.mCurrentPos.x + 0.5f);
                float AllyY = m->mInGameY + m->mInGameH * (CurMonster.mCurrentPos.y + 0.5f);
                if(Math::Distance(AllyX, AllyY, x, y) < m->mMonsterSizeR * 2)
                {
                    CurMonster.Ally_Touched();
                    IsPressEnabled = false;
                    return;
                }
            }

            // 2번째 브레스가 이미 존재한다면 제거후 게이지반환
            if(m->mBreathReadyCount == 2)
                m->mBreathReadyCount = 1;

            // 브레스준비
            m->mBreathing = true;
            m->mBreathPos = Point(x, y);
            m->mBreathMsec = Platform::Utility::CurrentTimeMsec();
            m->mBreathGaugeTimeUsingCurrently = 0;
            m->mBreathSizeRCurrently = m->mBreathSizeR;
            m->mBreathPowerPermil = 0;
            IsPressEnabled = true;
        }
        else if(IsPressEnabled)
        {
            if(type == GT_InDragging || type == GT_OutDragging)
            {
                if(m->mBreathing)
                    m->mBreathPos = Point(x, y);
            }
            else if(type == GT_InReleased)
            {
                if(m->mBreathing)
                {
                    m->mBreathing = false;
                    if(0 < m->mBreathGaugeTime && 0 < m->mBreathGaugeTimeUsingCurrently)
                    {
                        auto& CurBreath = m->mBreath[m->mBreathReadyCount++];
                        CurBreath.mAniTimeMsec = Platform::Utility::CurrentTimeMsec() + m->mDragonEntryTime;
                        CurBreath.mEndTimeMsec = CurBreath.mAniTimeMsec + m->mDragonBreathTime;
                        CurBreath.mPos = m->mBreathPos;
                        CurBreath.mSizeR = m->mBreathSizeRCurrently;
                        CurBreath.mGaugeTime = m->mBreathGaugeTimeUsingCurrently;
                        CurBreath.mDamage = m->GetCalcedBreathDamage();
                        if(m->mBreathReadyCount == 1)
                        {
                            m->mBreathGaugeTime = Math::Max(0, m->mBreathGaugeTime - CurBreath.mGaugeTime);
                            m->SetDragonSchedule(&CurBreath, false);
                        }
                    }
                }
            }
        }
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    #if BOSS_WINDOWS | BOSS_LINUX | BOSS_MAC_OSX
        // 출력미비영역 표현용
        Color TestColor(Platform::Utility::Random() | 0xFF000000);
        ZAY_COLOR(panel, TestColor)
            panel.fill();
    #endif
    m->Render(panel);
}

ingameData::ingameData()
{
    mSpineInited = false;
    mShowDebug = false;
    mPaused = false;
    mClosing = -1;
    mWaveData = Context(ST_Json, SO_NeedCopy, String::FromFile(mStage));
    mWave = -1;
    mWaveTitle = "";
    mWaveSecCurrently = 0;
    mWaveSecSettled = -1;
    mWaveSecMax = 0;
    mCurParaTalk = 0;
    mSlotFlag[0] = mSlotFlag[1] = mSlotFlag[2] = mSlotFlag[3] = false;
    mCurItemSkin = "normal";
    mCurItemSkinForDragon = "normal";
    mCurItemSkinEndTimeMsec = 0;
    mWallBoundFocus = 0;
    mBreathReadyCount = 0;
    mBreathing = false;
    mBreathPos = Point(0, 0);
    mBreathMsec = 0;
    mBreathGaugeTimeUsingCurrently = 0;
    mBreathSizeRCurrently = 0;
    mBreathPowerPermil = 0;

    // 브레스게이지
    mBreathGaugeTime = mBreathMaxGauge;
    mBreathGaugeTimeLog = mBreathMaxGauge;
}

ingameData::~ingameData()
{
}

#define PATHFIND_STEP (5)
void ingameData::Targeting(MapMonster& monster, const TryWorldZone& tryworld)
{
    // 타겟중 최적의 타겟을 선정
    TryWorld::Path* ResultPath = nullptr;
    sint32 ResultPathScore = 0, ResultIndex = -1;
    auto& CurTargets = (monster.mType->mType == MonsterType::TypeClass::Ally)? mTargetsForAlly : mTargetsForEnemy;
    const Point CurMonsterPos(mInGameW * (monster.mCurrentPos.x + 0.5f), mInGameH * (monster.mCurrentPos.y + 0.5f));
    for(sint32 j = 0, jend = CurTargets.Count(); j < jend; ++j)
    {
        auto& CurObject = mLayers[2].mObjects[CurTargets[j].mObjectIndex];
        if(CurObject.mHPValue == 0) continue;
        const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);

        sint32 GetScore = 0;
        auto NewPath = tryworld.mMap->BuildPath(CurMonsterPos, Point(x, y), PATHFIND_STEP, &GetScore,
            [this](const TryWorld::Dot& a, const TryWorld::Dot& b, const TryWorld::Dot& c)->int
            {
                // 루트선정시 오브젝트점수를 고려하여 판단
                sint32 SumScore = 0;
                auto& CurObjects = mLayers[2].mObjects;
                for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
                {
                    if(!CurObjects[i].mEnable || !CurObjects[i].mType->mType.canBroken())
                        continue;
                    const Point ObjectPos(
                        mInGameW * (CurObjects[i].mCurrentRect.CenterX() + 0.5f),
                        mInGameH * (CurObjects[i].mCurrentRect.CenterY() + 0.5f));
                    auto SideTest = [](const TryWorld::Dot& a, const TryWorld::Dot& b, const Point& c)->float
                    {return ((a.x - b.x) * (c.y - b.y) - (a.y - b.y) * (c.x - b.x));};
                    const bool Side1 = (SideTest(a, b, ObjectPos) < 0);
                    const bool Side2 = (SideTest(b, c, ObjectPos) < 0);
                    const bool Side3 = (SideTest(c, a, ObjectPos) < 0);
                    if(Side1 == Side2 && Side1 == Side3)
                    {
                        if(CurObjects[i].mType->mType.canBroken())
                            SumScore += CurObjects[i].mHPValue * mInGameSize / 1000; // 1000점이 인게임사이즈로 환산
                        else if(CurObjects[i].mType->mType.isWall())
                            SumScore += 1000000 * mInGameSize / 1000; // 인게임사이즈의 1000배
                    }
                }
                return SumScore;
            });
        sint32 NewPathScore = GetScore * 1000 / mInGameSize; // 인게임사이즈가 1000점으로 환산
        NewPathScore += CurObject.mHPValue; // 점수가 낮은 타겟이 우선
        NewPathScore += Platform::Utility::Random() % 500; // 랜덤요소 추가

        if(!ResultPath || (NewPath && NewPathScore < ResultPathScore))
        {
            TryWorld::Path::Release(ResultPath);
            ResultPath = NewPath;
            ResultPathScore = NewPathScore;
            ResultIndex = CurTargets[j].mObjectIndex;
        }
        else TryWorld::Path::Release(NewPath);
    }

    // 중간타겟으로 선정
    if(ResultPath)
    for(sint32 p = ResultPath->Dots.Count() - 2; 1 <= p && ResultPath; --p)
    {
        auto& CurPos = ResultPath->Dots[p];
        auto& CurObjects = mLayers[2].mObjects;
        for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
        {
            if(!CurObjects[i].mEnable || !CurObjects[i].mType->mType.canBroken())
                continue;
            const Rect Area(
                mInGameW * (CurObjects[i].mCurrentRect.l + 0.5f),
                mInGameH * (CurObjects[i].mCurrentRect.t + 0.5f),
                mInGameW * (CurObjects[i].mCurrentRect.r + 0.5f),
                mInGameH * (CurObjects[i].mCurrentRect.b + 0.5f));
            if(Area.PtInRect(CurPos.x, CurPos.y))
            {
                auto& NewTarget = monster.mTargets.AtAdding();
                NewTarget.mType = MonsterTarget::Wall;
                NewTarget.mIndex = i;
                NewTarget.mPos = CurObjects[i].mCurrentRect.Center();
                NewTarget.mPath = tryworld.mMap->CreatePath(PATHFIND_STEP);
                NewTarget.mPath->Dots.AtAdding() = Area.Center();
                for(sint32 j = p, jend = ResultPath->Dots.Count(); j < jend; ++j)
                    NewTarget.mPath->Dots.AtAdding() = ResultPath->Dots[j];
                TryWorld::Path::Release(ResultPath);
                ResultPath = nullptr;
                break;
            }
        }
    }

    // 최종타겟으로 선정
    if(ResultPath)
    {
        auto& NewTarget = monster.mTargets.AtAdding();
        NewTarget.mType = MonsterTarget::Target;
        NewTarget.mIndex = ResultIndex;
        NewTarget.mPos = mLayers[2].mObjects[ResultIndex].mCurrentRect.Center();
        NewTarget.mPath = ResultPath;
    }
}

sint32 ingameData::GetContactObject(const MapMonster& monster, const Point& curPos, const Point& nextPos)
{
    float ResultDistance = -1;
    sint32 ResultIndex = -1;
    auto& CurObjects = mLayers[2].mObjects;
    const float MonsterWaistSizeRW = mMonsterSizeR * monster.mType->mWaistScaleWidth / 1000;
    const float MonsterWaistSizeRH = mMonsterSizeR * monster.mType->mWaistScaleHeight / 1000;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        if(!CurObjects[i].mEnable || !CurObjects[i].mType->mType.isWall())
            continue;
        const Rect Area(
            mInGameW * (CurObjects[i].mCurrentRect.l + 0.5f) - MonsterWaistSizeRW,
            mInGameH * (CurObjects[i].mCurrentRect.t + 0.5f) - MonsterWaistSizeRH,
            mInGameW * (CurObjects[i].mCurrentRect.r + 0.5f) + MonsterWaistSizeRW,
            mInGameH * (CurObjects[i].mCurrentRect.b + 0.5f) + MonsterWaistSizeRH);
        if(!Area.PtInRect(nextPos))
            continue;

        // 오브젝트를 향해 다가서는 것이 맞는지의 여부
        bool IsContact = false;
        const float DistanceL = Area.l - curPos.x;
        const float DistanceR = curPos.x - Area.r;
        const float DistanceLR = Math::MaxF(DistanceL, DistanceR);
        const float DistanceT = Area.t - curPos.y;
        const float DistanceB = curPos.y - Area.b;
        const float DistanceTB = Math::MaxF(DistanceT, DistanceB);
        const float CurDistance = Math::MaxF(DistanceLR, DistanceTB);
        if(DistanceLR > DistanceTB)
        {
            if(DistanceL > DistanceR) IsContact = (Area.l - nextPos.x < CurDistance);
            else IsContact = (nextPos.x - Area.r < CurDistance);
        }
        else
        {
            if(DistanceT > DistanceB) IsContact = (Area.t - nextPos.y < CurDistance);
            else IsContact = (nextPos.y - Area.b < CurDistance);
        }
        if(!IsContact)
            continue;

        // 최소거리의 오브젝트를 찾음
        const float NextDistance = Math::Distance(nextPos.x, nextPos.y, Area.CenterX(), Area.CenterY());
        if(ResultDistance == -1 || NextDistance < ResultDistance)
        {
            ResultDistance = NextDistance;
            ResultIndex = i;
        }
    }
    return ResultIndex;
}

sint32 ingameData::GetValidNextObject(const MapMonster& monster, const Point& curPos, const Point& nextPos, Point& resultPos, Point& reflectPos)
{
    Point ResultPos = nextPos;
    float ResultDistance = -1;
    sint32 ResultIndex = -1;
    auto& CurObjects = mLayers[2].mObjects;
    const float MonsterWaistSizeRW = mMonsterSizeR * monster.mType->mWaistScaleWidth / 1000;
    const float MonsterWaistSizeRH = mMonsterSizeR * monster.mType->mWaistScaleHeight / 1000;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        if(!CurObjects[i].mEnable || !CurObjects[i].mType->mType.isWall())
            continue;
        const Rect Area(
            mInGameW * (CurObjects[i].mCurrentRect.l + 0.5f) - MonsterWaistSizeRW,
            mInGameH * (CurObjects[i].mCurrentRect.t + 0.5f) - MonsterWaistSizeRH,
            mInGameW * (CurObjects[i].mCurrentRect.r + 0.5f) + MonsterWaistSizeRW,
            mInGameH * (CurObjects[i].mCurrentRect.b + 0.5f) + MonsterWaistSizeRH);
        if(Math::MaxF(curPos.x, nextPos.x) < Area.l || Math::MinF(curPos.x, nextPos.x) > Area.r
		|| Math::MaxF(curPos.y, nextPos.y) < Area.t || Math::MinF(curPos.y, nextPos.y) > Area.b)
            continue;

        const TryWorld::Dot LineDot[4] = {
            TryWorld::Dot(Area.l, Area.t), TryWorld::Dot(Area.l, Area.b),
            TryWorld::Dot(Area.r, Area.b), TryWorld::Dot(Area.r, Area.t)};
        for(int j = 0, jend = 4; j < jend; ++j)
		{
            auto& LineBegin = LineDot[j];
            auto& LineEnd = LineDot[(j + 1) % jend];
            if(TryWorld::Util::GetClockwiseValue(LineBegin, LineEnd, curPos) < 0)
			if(auto CrossPos = TryWorld::Util::GetDotByLineCross(LineBegin, LineEnd, curPos, nextPos))
            {
                const float CurDistance = Math::Distance(curPos.x, curPos.y, CrossPos->x, CrossPos->y);
                if(ResultDistance == -1 || CurDistance < ResultDistance)
                {
                    ResultPos = Point(CrossPos->x, CrossPos->y);
                    ResultDistance = CurDistance;
                    ResultIndex = i;

                    const float LineDx = LineEnd.x - LineBegin.x;
                    const float LineDy = LineEnd.y - LineBegin.y;
                    const float TValue = ((curPos.x - LineBegin.x) * LineDx + (curPos.y - LineBegin.y) * LineDy)
                        / (LineDx * LineDx + LineDy * LineDy);
                    const float NearX = LineBegin.x + TValue * LineDx;
                    const float NearY = LineBegin.y + TValue * LineDy;
                    reflectPos.x = curPos.x + (ResultPos.x - NearX) * 2;
                    reflectPos.y = curPos.y + (ResultPos.y - NearY) * 2;
                }
            }
		}
    }
    resultPos = ResultPos;
    return ResultIndex;
}

void ingameData::InitForSpine()
{
    // 맵로드
    const String MapName = mWaveData("MapName").GetString();
    id_asset_read TextAsset = Asset::OpenForRead("f1/table/" + MapName + ".json");
    if(TextAsset)
    {
        const sint32 TextSize = Asset::Size(TextAsset);
        buffer TextBuffer = Buffer::Alloc(BOSS_DBG TextSize + 1);
        Asset::Read(TextAsset, (uint08*) TextBuffer, TextSize);
        Asset::Close(TextAsset);
        ((char*) TextBuffer)[TextSize] = '\0';
        LoadMap((chars) TextBuffer, false);
        Buffer::Free(TextBuffer);
    }
    // 맵로드후 길찾기맵 첫빌드
    RebuildTryWorld();

    if(Platform::Option::GetFlag("DirectPlay"))
        ReadyForNextWave();
    else
    {
        // 타이틀화면
        mMainTitleSpine.InitSpine(GetSpine("ui_main_title"), "default",
            [this](chars motionname)
            {
                if(!String::Compare("start", motionname))
                    ReadyForNextWave();
            }).PlayMotionAttached("loding", "idle", true);
    }

    // Effect
    for(sint32 i = 0; i < mWallBoundMax; ++i)
        mWallBound[i].InitSpine(GetSpine("wall_bound")).PlayMotion("idle", true);

    // InGame
    mDragon.Init(GetSpine("dragon"), mDragonScaleMax / mDragonScale, updater(),
        mDragonHome, mDragonExitL, mDragonExitR);
    mDragon.ResetCB();
    mBreathReadySpine[0].InitSpine(GetSpine("breath_ready"), "first").PlayMotionSeek("meteo_ready", false);
    mBreathReadySpine[1].InitSpine(GetSpine("breath_ready"), "second").PlayMotionSeek("meteo_ready", false);
    mBreathAttackSpine.InitSpine(GetSpine("breath_attack")).PlayMotion("idle", true);
    mBreathEffectSpine.InitSpine(GetSpine("breath_effect")).PlayMotion("idle", true);

    // UI
    mWeather[0].InitSpine(GetSpine("a_weather_toast")).PlayMotion("forest_weather", true);
    mWeather[1].InitSpine(GetSpine("b_weather_toast")).PlayMotion("forest_weather", true);
    mGaugeHUD.InitSpine(GetSpine("ui_ingame_gauge"), "normal").PlayMotionSeek("charge", false);
    mGaugeHUD.PlayMotionOnce("loading");
    mSlotHUD.InitSpine(GetSpine("ui_ingame_slot")).PlayMotionAttached("show", "idle", true);
    mWaveHUD.InitSpine(GetSpine("ui_ingame_wave")).PlayMotionAttached("show", "idle", true);
    mStopButton.InitSpine(GetSpine("ui_ingame_stop_butten")).PlayMotionAttached("show", "idle", true);
    mPausePopup.InitSpine(GetSpine("ui_pause"), "default",
        [this](chars motionname)
        {
            if(!String::Compare("click_play", motionname))
            {
                mPausePopup.StopMotion("idle");
                mPausePopup.PlayMotion("hide", false);
            }
            else if(!String::Compare("click_lobby", motionname))
            {
                mClosing = 50;
                Platform::Option::SetText("StartMode", "Lobby");
                mPausePopup.StopMotion("idle");
                mPausePopup.PlayMotion("hide", false);
            }
            else if(!String::Compare("hide", motionname))
                mPaused = false;
        }).PlayMotion("idle", true);
}

void ingameData::PlayScriptOnce(sint32 sec)
{
    for(sint32 i = 0, iend = mWaveData("Waves")[mWave]("Events").LengthOfIndexable(); i < iend; ++i)
    {
        const sint32 TimeSec = mWaveData("Waves")[mWave]("Events")[i]("TimeSec").GetInt(0);
        if(TimeSec == sec)
        {
            auto& JsonScripts = mWaveData("Waves")[mWave]("Events")[i]("Scripts");
            for(sint32 j = 0, jend = JsonScripts.LengthOfIndexable(); j < jend; ++j)
            {
                const sint32 ObjectRID = JsonScripts[j]("TargetRID").GetInt(0);
                const String ScriptText = JsonScripts[j]("Text").GetString();
                if(auto CurObject = mObjectRIDs.Access(ObjectRID))
                {
                    if(!String::Compare(ScriptText, "animate ", 8))
                        (*CurObject)->PlayMotionScript(((chars) ScriptText) + 8);
                }
            }
            break;
        }
    }
}

void ingameData::AnimationOnce(sint32 sec_span)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    // 스킨풀림
    if(mCurItemSkinEndTimeMsec != 0 && mCurItemSkinEndTimeMsec < CurTimeMsec)
    {
        mCurItemSkin = "normal";
        mCurItemSkinEndTimeMsec = 0;
        mGaugeHUD.SetSkin(mCurItemSkin);
        mGaugeHUD.PlayMotionOnce("change");
    }

    // 몬스터애니
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(mWaveSecCurrently < CurMonster.mEntranceSec || 0 < CurMonster.mDeathStep) // 등장전 또는 죽을때
        {
            if(CurMonster.mDeathStep == 1)
                CurMonster.TryDeathMove();
            continue;
        }
        const Point CurMonsterPos(mInGameW * (CurMonster.mCurrentPos.x + 0.5f), mInGameH * (CurMonster.mCurrentPos.y + 0.5f));

        // 파라토크
        if(CurMonster.mEntranceSec + 2 < mWaveSecCurrently)
            CurMonster.TryParaTalk();

        // 등장연출
        if(CurMonster.IsEntranced())
        {
            auto& CurObjects = mLayers.At(2).mObjects;
            for(sint32 j = 0, jend = CurObjects.Count(); j < jend; ++j)
            {
                auto& CurObject = CurObjects.At(j);
                if(!CurObject.mEnable || CurObject.mType->mType != ObjectType::TypeClass::Spot)
                    continue;
                const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
                const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
                const float r = mInGameSize * CurObject.mCurrentRect.Width();
                if(Math::Distance(x, y, CurMonsterPos.x, CurMonsterPos.y) < r)
                    CurObject.Spot();
            }
        }

        // 타겟유효시간 종료처리
        if(0 < CurMonster.mTargetTimeLimit && CurMonster.mTargetTimeLimit < CurTimeMsec)
        {
            CurMonster.mTargetTimeLimit = 0;
            CurMonster.mTargets.Clear();
            CurMonster.CancelAttack();
        }

        // 공격
        bool HasAction = false;
        if(!CurMonster.IsKnockBackMode())
        {
            // 타겟공격 또는 도착
            if(!HasAction)
            if(0 < CurMonster.mTargets.Count())
            {
                bool LossAttack = false;
                bool LossTarget = false;
                auto& CurTarget = CurMonster.mTargets[0];
                if(CurTarget.mType == MonsterTarget::Target)
                {
                    auto& CurObject = mLayers.At(2).mObjects.At(CurTarget.mIndex);
                    if(0 < CurObject.mHPValue)
                    {
                        if(CurTarget.mIndex == CurMonster.mBounceObjectIndex)
                        {
                            HasAction = true;
                            if(CurMonster.mType->mType == MonsterType::TypeClass::Ally) // 동맹군도착
                                CurMonster.Ally_Arrived();
                            else if(auto AttackCount = CurMonster.TryAttack(CurTarget.mPos)) // 타겟공격
                                if(CurObject.SetHP(CurObject.mHPValue - CurMonster.mType->mAttackPower * AttackCount, mHPbarDeleteTime))
                                    ClearAllPathes(false);
                        }
                    }
                    else LossAttack = LossTarget = true;
                }
                else if(CurTarget.mType == MonsterTarget::Wall)
                {
                    auto& CurObject = mLayers.At(2).mObjects.At(CurTarget.mIndex);
                    if(0 < CurObject.mHPValue)
                    {
                        if(CurTarget.mIndex == CurMonster.mBounceObjectIndex)
                        {
                            HasAction = true;
                            if(auto AttackCount = CurMonster.TryAttack(CurTarget.mPos)) // 오브젝트공격
                                if(CurObject.SetHP(CurObject.mHPValue - CurMonster.mType->mAttackPower * AttackCount, mHPbarDeleteTime))
                                    ClearAllPathes(false);
                        }
                    }
                    else
                    {
                        LossAttack = true;
                        const float x = mInGameW * (CurTarget.mPos.x + 0.5f);
                        const float y = mInGameH * (CurTarget.mPos.y + 0.5f);
                        if(Math::Distance(CurMonsterPos.x, CurMonsterPos.y, x, y) < 1) // 1px안쪽으로 들어오면 타겟종료
                            LossTarget = true;
                    }
                }
                else if(CurTarget.mType == MonsterTarget::Mission)
                {
                    const float x = mInGameW * (CurTarget.mPos.x + 0.5f);
                    const float y = mInGameH * (CurTarget.mPos.y + 0.5f);
                    const float r = mInGameSize * CurTarget.mSizeR;
                    if(Math::Distance(CurMonsterPos.x, CurMonsterPos.y, x, y) < r)
                        LossTarget = true;
                }
                if(LossAttack) CurMonster.CancelAttack();
                if(LossTarget) CurMonster.ClearTargetOnce();
            }
        }

        // 이동
        if(!HasAction)
        if(auto CurTryWorldZone = mAllTryWorldZones.Access(CurMonster.mType->mPolygon))
        {
            // 다음위치 계산
            Point NextPos = CurMonster.mCurrentPos;
            const float WallDistanceCheck = mInGameSize * 5 / 1000; // 벽에 닿아버린 상황판단
            const float WallDistanceMin = mInGameSize * 20 / 1000; // 계산상 오류를 방지하는 거리
            // 넉백중
            if(CurMonster.IsKnockBackMode())
            {
                // 오브젝트와 폴리곤에 의해 반동되어야 할 경우를 판단
                const float TryNextPosX = CurMonsterPos.x + CurMonster.knockbackaccel().x * mInGameW;
                const float TryNextPosY = CurMonsterPos.y + CurMonster.knockbackaccel().y * mInGameH;
                const Point TryResultPos(TryNextPosX, TryNextPosY);
                Point ResultPos = TryResultPos, ReflectPos;
                bool IsBounce = false, IsHole = false;
                float DistanceMin = -1;
                if(GetValidNextObject(CurMonster, CurMonsterPos, TryResultPos, ResultPos, ReflectPos) != -1)
                {
                    IsBounce = true;
                    DistanceMin = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, ResultPos.x, ResultPos.y);
                }
                if(auto Polygon = TryWorld::GetPosition::GetValidNext(CurTryWorldZone->mHurdle, CurMonsterPos, TryResultPos, ResultPos, ReflectPos, DistanceMin))
                {
                    IsBounce = true;
                    if(Polygon[0]->Payload != -1 && Polygon[0]->Payload == Polygon[1]->Payload)
                    {
                        IsHole = true;
                        const sint32 LayerIndex = (Polygon[0]->Payload >> 16) & 0xFFFF;
                        const sint32 ObjectIndex = Polygon[0]->Payload & 0xFFFF;
                        auto& HoleObject = mLayers.At(LayerIndex).mObjects.At(ObjectIndex);
                        HoleObject.Hit();
                        if(0 < CurMonster.mHPValue)
                        {
                            CurMonster.mHPValue = 0;
                            CurMonster.mHPTimeMsec = CurTimeMsec + mHPbarDeleteTime;
                        }
                        CurMonster.KnockBackEndByHole(HoleObject.mCurrentRect.Center());
                        // 홀실적추가
                        const String CurHoleSkin = HoleObject.mType->spineSkinName();
                        if(!!CurHoleSkin.Compare("normal"))
                        {
                            HoleObject.AddExtraInfo(CurMonster.mType->mID);
                            if(mHoleItemGetCount <= HoleObject.GetExtraInfoCount())
                            {
                                // 아이템추가
                                auto& NewItem = mItemMap(MapItem::MakeId());
                                NewItem.Init(CurHoleSkin, GetSpine("item"), &HoleObject, updater(), -0.15f, 2000, 1000);
                                // 홀실적초기화
                                HoleObject.ResetExtraInfo();
                            }
                        }
                    }
                }

                if(!IsHole)
                {
                    if(IsBounce)
                    {
                        // 반동의 충격
                        mWallBound[mWallBoundFocus].StopMotionAll();
                        sint32 BoundDamage = 0;
                        switch(CurMonster.knockbackboundcount())
                        {
                        case 0:
                            mWallBound[mWallBoundFocus].PlayMotionAttached("wall_bound_one", "idle", true);
                            BoundDamage = CurMonster.mType->mHP * m1BoundDamageRate;
                            break;
                        case 1:
                            mWallBound[mWallBoundFocus].PlayMotionAttached("wall_bound_two", "idle", true);
                            BoundDamage = CurMonster.mType->mHP * m2BoundDamageRate;
                            break;
                        case 2: default:
                            mWallBound[mWallBoundFocus].PlayMotionAttached("wall_bound_three", "idle", true);
                            BoundDamage = CurMonster.mType->mHP * m3BoundDamageRate;
                            break;
                        }
                        mWallBoundPos[mWallBoundFocus] = ResultPos;
                        mWallBoundFocus = (mWallBoundFocus + 1) % mWallBoundMax;
                        CurMonster.KnockBackBound(BoundDamage);

                        // 진행방향을 결정하고
                        CurMonster.SetKnockBackAccel(Point((ReflectPos.x - ResultPos.x) / mInGameW, (ReflectPos.y - ResultPos.y) / mInGameH));
                        // 벽에 너무 닿은 경우에는 살짝 뒤로 물러섬
                        const float ReflectOX = (CurMonsterPos.x + ReflectPos.x) / 2;
                        const float ReflectOY = (CurMonsterPos.y + ReflectPos.y) / 2;
                        const float ReflectDistance = Math::Distance(ResultPos.x, ResultPos.y, ReflectOX, ReflectOY);
                        if(ReflectDistance < WallDistanceCheck)
                        {
                            BOSS_ASSERT("게임의 시나리오가 잘못되었습니다", 0 < ReflectDistance);
                            ResultPos.x += (ReflectOX - ResultPos.x) * WallDistanceMin / ReflectDistance;
                            ResultPos.y += (ReflectOY - ResultPos.y) * WallDistanceMin / ReflectDistance;
                        }
                    }
                    CurMonster.SetKnockBackAccel(CurMonster.knockbackaccel() * CurMonster.mType->mResistance / 1000); // 가속도감소
                    const float NextKnockBackAccelSize = Math::Sqrt(Math::Pow(CurMonster.knockbackaccel().x) + Math::Pow(CurMonster.knockbackaccel().y));
                    if(NextKnockBackAccelSize < CurMonster.mKnockBackAccelMin) // 정신차리기
                        CurMonster.KnockBackEnd();
                }
                NextPos = Point(ResultPos.x / mInGameW - 0.5f, ResultPos.y / mInGameH - 0.5f);
            }
            else // 걷기진행
            {
                // 타겟설정
                if(CurMonster.mTargets.Count() == 0)
                    Targeting(CurMonster, *CurTryWorldZone);

                // 서브타겟위치 업데이트
                Point TempPos;
                if(CurMonster.mTargets.Count() == 0)
                    TempPos = Point(mInGameW * (CurMonster.mTargetPos.x + 0.5f), mInGameH * (CurMonster.mTargetPos.y + 0.5f));
                else
                {
                    if(TryWorld::GetPosition::SubTarget(CurTryWorldZone->mHurdle, CurMonster.mTargets[0].mPath, CurMonsterPos, TempPos))
                        CurMonster.mTargetPos = Point(TempPos.x / mInGameW - 0.5f, TempPos.y / mInGameH - 0.5f);
                    else CurMonster.ClearTargetOnce();
                }

                const float TargetDistance = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, TempPos.x, TempPos.y);
                if(0 < TargetDistance)
                {
                    const float SpeedDelay = 10;
                    const float MoveDistance = mMonsterSizeR * CurMonster.mType->mMoveSpeed / (1000 * SpeedDelay);
                    if(MoveDistance < TargetDistance)
                    {
                        // 오브젝트와 폴리곤에 의해 멈추어야 할 경우를 판단
                        const float TryNextPosX = CurMonsterPos.x + (TempPos.x - CurMonsterPos.x) * MoveDistance / TargetDistance;
                        const float TryNextPosY = CurMonsterPos.y + (TempPos.y - CurMonsterPos.y) * MoveDistance / TargetDistance;
                        const Point TryResultPos(TryNextPosX, TryNextPosY);
                        Point ResultPos = TryResultPos, ReflectPos;
                        if(TryWorld::GetPosition::GetValidNext(CurTryWorldZone->mHurdle, CurMonsterPos, TryResultPos, ResultPos, ReflectPos))
                        {
                            // 벽에 너무 닿은 경우에는 살짝 뒤로 물러섬
                            const float ReflectOX = (CurMonsterPos.x + ReflectPos.x) / 2;
                            const float ReflectOY = (CurMonsterPos.y + ReflectPos.y) / 2;
                            const float ResultToReflect = Math::Distance(ResultPos.x, ResultPos.y, ReflectOX, ReflectOY);
                            if(ResultToReflect < WallDistanceCheck)
                            {
                                BOSS_ASSERT("게임의 시나리오가 잘못되었습니다", 0 < ResultToReflect);
                                ResultPos.x += (ReflectOX - ResultPos.x) * WallDistanceMin / ResultToReflect;
                                ResultPos.y += (ReflectOY - ResultPos.y) * WallDistanceMin / ResultToReflect;
                            }
                            // 폴리곤식 추가이동
                            else
                            {
                                const float MonsterToReflect = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, ReflectPos.x, ReflectPos.y);
                                if(0 < MonsterToReflect)
                                {
                                    ResultPos.x = CurMonsterPos.x + (ReflectPos.x - CurMonsterPos.x) * MoveDistance / MonsterToReflect;
                                    ResultPos.y = CurMonsterPos.y + (ReflectPos.y - CurMonsterPos.y) * MoveDistance / MonsterToReflect;
                                }
                                else
                                {
                                    const float ResultToMonster = ResultToReflect; // Monster와 Reflect가 같은 위치라서
                                    ReflectPos.x = CurMonsterPos.x + (ResultPos.y - CurMonsterPos.y); // 90도 꺾음
                                    ReflectPos.y = CurMonsterPos.y - (ResultPos.x - CurMonsterPos.x);
                                    ResultPos.x = CurMonsterPos.x + (ReflectPos.x - CurMonsterPos.x) * MoveDistance / ResultToMonster;
                                    ResultPos.y = CurMonsterPos.y + (ReflectPos.y - CurMonsterPos.y) * MoveDistance / ResultToMonster;
                                }
                            }
                        }
                        else
                        {
                            // 닿은 오브젝트가 있다면 경험에 따른 추가이동
                            const sint32 ObjectIndex = GetContactObject(CurMonster, CurMonsterPos, TryResultPos);
                            if(ObjectIndex != -1)
                            {
                                CurMonster.mBounceObjectIndex = ObjectIndex; // 닿은 오브젝트의 기록
                                const Point MoveFlag = CurMonster.CalcBump(&mLayers.At(2).mObjects.At(ObjectIndex));
                                ResultPos.x = CurMonsterPos.x + MoveDistance * MoveFlag.x;
                                ResultPos.y = CurMonsterPos.y + MoveDistance * MoveFlag.y;
                            }
                        }
                        NextPos = Point(ResultPos.x / mInGameW - 0.5f, ResultPos.y / mInGameH - 0.5f);
                    }
                    else NextPos = CurMonster.mTargetPos;

                    // 방향전환된 꼭지점 기록
                    const bool CurFlip = (CurMonster.mCurrentPos.x < NextPos.x);
                    if(CurMonster.mLastFlip != CurFlip)
                    {
                        CurMonster.mLastFlip = CurFlip;
                        CurMonster.mLastFlipPos = CurMonster.mCurrentPos;
                    }
                    // 방향전환
                    if(CurMonster.mFlipMode != CurMonster.mLastFlip)
                    {
                        const float FlipDist = Math::Distance(CurMonster.mLastFlipPos.x, CurMonster.mLastFlipPos.y, NextPos.x, NextPos.y);
                        if(CurMonster.mType->mTurnDistance < FlipDist * 1000)
                        {
                            CurMonster.mFlipMode = CurMonster.mLastFlip;
                            CurMonster.Turn();
                        }
                    }
                }
            }
            CurMonster.mCurrentPosOld = CurMonster.mCurrentPos;
            CurMonster.mCurrentPos = NextPos;
        }
    }

    // 아이템 애니메이션처리
    for(sint32 i = mItemMap.Count() - 1; 0 <= i; --i)
    {
        chararray GetPath;
        auto CurItem = mItemMap.AccessByOrder(i, &GetPath);
        if(!CurItem->AnimationOnce())
            mItemMap.Remove(&GetPath[0]);
    }

    // 타겟회복
    sint32 LiveTargetCount = 0;
    for(sint32 i = 0, iend = mTargetsForEnemy.Count(); i < iend; ++i)
    {
        auto& CurObject = mLayers.At(2).mObjects.At(mTargetsForEnemy[i].mObjectIndex);
        if(0 < CurObject.mHPValue)
        {
            LiveTargetCount++;
            CurObject.mHPValue = Math::Min(CurObject.mHPValue + mEggHPRegenValue * sec_span, CurObject.mType->mHP);
        }
    }
    if(mClosing == -1 && LiveTargetCount == 0)
    {
        mClosing = 200;
        Platform::Option::SetText("StartMode", "Result");
        Platform::Option::SetText("LastResult", "LOSE");
        SetPanel("result",
            [](const FXState& state, FXPanel::Data& data)->void
            {
                data.mSpines("win_lose").InitSpine(state.GetSpine("ui_win_lose")).PlayMotionAttached("lose_loading", "lose_no_egg", false);
                data.mSpines("dragon").InitSpine(state.GetSpine("dragon"), "normal").PlayMotion("lose", true);
            },
            [](ZayPanel& panel, const FXPanel::Data& data)->void
            {
                if(auto WinLose = data.mSpines.Access("win_lose"))
                    WinLose->RenderObject(true, false, panel, false, nullptr, nullptr,
                        ZAY_RENDER_PN(p, n, data)
                        {
                            if(!String::Compare(n, "dragon_lose_area"))
                            if(auto Dragon = data.mSpines.Access("dragon"))
                                Dragon->RenderObject(true, false, p, false);
                        });
            });
    }

    // 웨이브클리어
    const sint32 WaveCount = mWaveData("Waves").LengthOfIndexable();
    if(mWave < WaveCount)
    {
        bool AnyMonsterAlived = false;
        bool AnyMonsterWaiting = false;
        bool WaveStop = false;
        for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
        {
            auto& CurMonster = mMonsters.At(i);
            if(CurMonster.mDeathStep == 2) continue;
            AnyMonsterAlived = true;
            if(AnyMonsterWaiting = (mWaveSecCurrently < CurMonster.mEntranceSec)) break;
            if(WaveStop = CurMonster.mType->mWaveStop) break;
        }
        // 중간 웨이브
        if(mWave < WaveCount - 1)
        {
            if(!AnyMonsterWaiting && !WaveStop && mWaveSecMax < mWaveSecSettled)
                ReadyForNextWave();
        }
        // 마지막 웨이브
        else if(!AnyMonsterAlived)
        {
            ReadyForNextWave();
            mClosing = 100;
            Platform::Option::SetText("StartMode", "Result");
            Platform::Option::SetText("LastResult", String::Format("WIN-%d", LiveTargetCount));
            SetPanel("result",
                [](const FXState& state, FXPanel::Data& data)->void
                {
                    chars EggResult = Platform::Option::GetText("LastResult");
                    if(!String::Compare(EggResult, "WIN-1"))
                        data.mSpines("win_lose").InitSpine(state.GetSpine("ui_win_lose")).PlayMotionAttached("win_loading", "win_one_egg", false);
                    else if(!String::Compare(EggResult, "WIN-2"))
                        data.mSpines("win_lose").InitSpine(state.GetSpine("ui_win_lose")).PlayMotionAttached("win_loading", "win_two_egg", false);
                    else data.mSpines("win_lose").InitSpine(state.GetSpine("ui_win_lose")).PlayMotionAttached("win_loading", "win_three_egg", false);
                    data.mSpines("dragon").InitSpine(state.GetSpine("dragon"), "normal").PlayMotion("run", true);
                },
                [](ZayPanel& panel, const FXPanel::Data& data)->void
                {
                    if(auto WinLose = data.mSpines.Access("win_lose"))
                        WinLose->RenderObject(true, false, panel, false, nullptr, nullptr,
                            ZAY_RENDER_PN(p, n, data)
                            {
                                if(!String::Compare(n, "dragon_win_area"))
                                if(auto Dragon = data.mSpines.Access("dragon"))
                                    Dragon->RenderObject(true, false, p, false);
                            });
                });
        }
    }

    // 브레스폭발
    if(0 < mBreathReadyCount && mBreath[0].mEndTimeMsec < CurTimeMsec)
    {
        SetBreathAttack(&mBreath[0]);
        // 브레스 애니
        mBreathAttackSpine.StopMotionAll();
        mBreathAttackSpine.PlayMotionAttached(String::Format("breath_%s_attack_b", (chars) mCurItemSkinForDragon), "idle", true);
        // 브레스이펙트 애니
        mBreathEffectSpine.StopMotionAll();
        mBreathEffectSpine.PlayMotionAttached(String::Format("breath_%s_ground", (chars) mCurItemSkinForDragon), "idle", true);
        // 예약된 브레스를 드래곤에 적용
        if(--mBreathReadyCount == 1)
        {
            mBreath[0] = mBreath[1];
            mBreath[0].mAniTimeMsec = CurTimeMsec + mDragonRetryTime;
            mBreath[0].mEndTimeMsec = mBreath[0].mAniTimeMsec + mDragonBreathTime;
            mBreathGaugeTime = Math::Max(0, mBreathGaugeTime - mBreath[0].mGaugeTime);
            SetDragonSchedule(&mBreath[0], true);
        }
    }

    // 브레스게이지연출
    mBreathGaugeTime = Math::Min(mBreathGaugeTime + sec_span * mBreathGaugeChargingPerSec, mBreathMaxGauge);
    mBreathGaugeTimeLog = (mBreathGaugeTimeLog * 9 + mBreathGaugeTime * 1) / 10;
}

void ingameData::ClearAllPathes(bool directly)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(directly)
        {
            CurMonster.mTargets.Clear();
            CurMonster.CancelAttack();
        }
        else CurMonster.mTargetTimeLimit = CurTimeMsec + 200 + (Platform::Utility::Random() % 1800);
    }
}

void ingameData::Render(ZayPanel& panel)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    // 인게임
    Rect OutlineRect;
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
    {
        OutlineRect = F1State::RenderMap(mShowDebug, panel, &mMonsters, mWaveSecCurrently);
        OutlineRect += Point(mInGameX, mInGameY);
        // 이펙트
        for(sint32 i = 0; i < mWallBoundMax; ++i)
        {
            const sint32 Index = (mWallBoundFocus + i) % mWallBoundMax;
            ZAY_XYRR(panel, mWallBoundPos[Index].x, mWallBoundPos[Index].y, mWallBoundSizeR, mWallBoundSizeR)
                mWallBound[Index].RenderObject(true, false, panel, false);
        }
        // 공중아이템
        RenderItems(panel, false, CurTimeMsec);
    }

    const sint32 OutlineL = Math::Max(0, OutlineRect.l);
    const sint32 OutlineT = Math::Max(0, OutlineRect.t);
    const sint32 OutlineR = Math::Min(OutlineRect.r, mScreenW);
    const sint32 OutlineB = Math::Min(OutlineRect.b, mScreenH);
    ZAY_LTRB(panel, OutlineL, OutlineT, OutlineR, OutlineB)
    {
        // 구름
        mWeather[1].RenderObject(true, false, panel, false);
        // 햇살
        if(const Rect* Area = mWeather[0].GetBoundRect("area"))
        {
            const float Rate = Math::MinF(panel.w() / Area->Width(), panel.h() / Area->Height());
            const float Width = Area->Width() * Rate;
            const float Height = Area->Height() * Rate;
            ZAY_XYWH(panel, 0, 0, Width, Height)
                mWeather[0].RenderObject(true, false, panel, false);
        }
    }

    // 브레스연출
    if(mWave != -1)
    if(const Rect* Area = mDragon.GetBoundRect("area"))
    if(const Rect* MouthArea = mDragon.GetBoundRect("mouth_area"))
    {
        // 드래곤위치 및 어택순간포착
        bool AttackFlag = false;
        const Point DragonPos = mDragon.MoveOnce(mInGameSize * mDragonCurve,
            Point(mInGameSize * mDragonMouthX, mInGameSize * mDragonMouthY), 300, AttackFlag);
        const float DragonSizeR = mDragonSizeR * mDragon.scale();
        if(AttackFlag)
        {
            mBreathAttack.mBreathPos = mBreath[0].mPos;
            mBreathAttack.mBreathSizeR = mBreath[0].mSizeR;
            mBreathAttack.mBreathAniTimeMsec = mBreath[0].mAniTimeMsec;
            mBreathAttack.mDragonFlip = mDragon.flip();
            mBreathAttack.mDragonPos = DragonPos;
            mBreathAttack.mDragonSizeR = DragonSizeR;
            mBreathAttack.mDragonBreathBeginTimeMsec = CurTimeMsec;
            mBreathAttack.mDragonBreathEndTimeMsec = mDragon.endtime();
            // 브레스 애니
            mBreathAttackSpine.StopMotionAll();
            mBreathAttackSpine.PlayMotion(String::Format("breath_%s_attack_a", (chars) mCurItemSkinForDragon), true);
        }
        // 브레스어택
        float BreathRate = 0;
        if(mBreathAttack.mDragonBreathBeginTimeMsec < CurTimeMsec)
        if(sint32 TimeGap = mBreathAttack.mDragonBreathEndTimeMsec - mBreathAttack.mDragonBreathBeginTimeMsec)
            BreathRate = Math::ClampF((CurTimeMsec - mBreathAttack.mDragonBreathBeginTimeMsec) / (float) TimeGap, 0, 1);
        if(0 < BreathRate)
        {
            const float DragonRate = mBreathAttack.mDragonSizeR * 2 / Math::MaxF(Area->Width(), Area->Height());
            const Point MouthPos = (MouthArea->Center() - Area->Center()) * DragonRate;
            const Point DragonMouthPos(mBreathAttack.mDragonPos.x + MouthPos.x * ((mBreathAttack.mDragonFlip)? -1 : 1),
                mBreathAttack.mDragonPos.y + MouthPos.y);
            const float RR = BreathRate * BreathRate;
            const float X = mBreathAttack.mBreathPos.x * RR + DragonMouthPos.x * (1 - RR);
            const float Y = mBreathAttack.mBreathPos.y * RR + DragonMouthPos.y * (1 - RR);
            const float R = mBreathAttack.mBreathSizeR * RR;
            // 브레스연출
            ZAY_XYRR(panel, X, Y, R, R)
            {
                if(mBreathReadyCount == 0 || mBreathAttack.mBreathAniTimeMsec < CurTimeMsec)
                    mBreathAttackSpine.RenderObject(true, false, panel, mBreathAttack.mDragonFlip);
                mBreathEffectSpine.RenderObject(true, false, panel, mBreathAttack.mDragonFlip);
            }
        }
        // 드래곤
        if(mDragon.CalcExitRate(CurTimeMsec) < 1)
            ZAY_XYRR(panel, DragonPos.x, DragonPos.y, DragonSizeR, DragonSizeR)
                mDragon.RenderObject(true, mShowDebug, panel, mDragon.flip());
    }

    // 게임전 UI
    if(mWave == -1)
    {
        // 메인타이틀
        mMainTitleSpine.RenderObject(true, false, panel, false, "Title_",
            ZAY_GESTURE_NT(n, t, this)
            {
                if(t == GT_Pressed)
                {
                    if(!String::Compare(n, "Title_butten_start_area") || !String::Compare(n, "Title_str_30"))
                        mMainTitleSpine.Staff_Start();
                    else if(!String::Compare(n, "Title_butten_start_area2") || !String::Compare(n, "Title_str_29"))
                    {
                        mClosing = 50;
                        Platform::Option::SetText("StartMode", "Lobby");
                    }
                    else if(!String::Compare(n, "Title_butten_staff_area2") || !String::Compare(n, "Title_str_10"))
                    {
                        mClosing = 50;
                        Platform::Option::SetText("StartMode", "StaffRoll");
                    }
                }
            }, mSubRenderer);
    }

    // 인게임캡
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
        F1State::RenderCap(panel, OutlineRect);

    // 인게임 UI
    if(mWave != -1)
    {
        // 스테이터스
        ZAY_LTRB(panel, 0, 0, panel.w(), mUIT)
        {
            // 웨이브표현
            if(const Rect* Area = mWaveHUD.GetBoundRect("area"))
            {
                const sint32 WaveWidth = Area->Width() * panel.h() / Area->Height();
                ZAY_LTRB(panel, 0, 0, WaveWidth, panel.h())
                    mWaveHUD.RenderObject(true, mShowDebug, panel, false, nullptr, nullptr,
                        ZAY_RENDER_PN(p, n, this)
                        {
                            if(!String::Compare(n, "pont_wave_b_area"))
                            {
                                ZAY_FONT(p, p.h() / 12)
                                {
                                    const sint32 WaveCount = mWaveData("Waves").LengthOfIndexable();
                                    if(mWave == WaveCount)
                                        p.text("END", UIFA_CenterMiddle, UIFE_Right);
                                    else p.text(String::Format("%d / %d", mWave + 1, WaveCount), UIFA_CenterMiddle, UIFE_Right);
                                }
                            }
                        });
            }
            // 중지버튼
            if(const Rect* Area = mStopButton.GetBoundRect("area"))
            {
                const sint32 StopWidth = Area->Width() * panel.h() / Area->Height();
                ZAY_LTRB(panel, panel.w() - StopWidth, 0, panel.w(), panel.h())
                    mStopButton.RenderObject(true, mShowDebug, panel, false, "Stop_",
                        ZAY_GESTURE_NT(n, t, this)
                        {
                            if(t == GT_Pressed)
                            {
                                if(!String::Compare(n, "Stop_stop_area"))
                                {
                                    mStopButton.PlayMotionOnce("click");
                                    mPausePopup.StopMotionAll();
                                    mPausePopup.PlayMotionAttached("show", "idle", true);
                                    mPaused = true;
                                }
                            }
                        });
            }
        }

        // 게이지
        mGaugeHUD.SetSeekSec(5 - Math::ClampF(5 * mBreathGaugeTimeLog / mBreathMaxGauge, 0, 5));
        sint32 GaugeWidth = 0, GaugeHeight = 0;
        if(const Rect* Area = mGaugeHUD.GetBoundRect("area"))
        {
            GaugeWidth = Math::Min(panel.w() / 2, mUIB * Area->Width() / Area->Height());
            GaugeHeight = GaugeWidth * Area->Height() / Area->Width();
            ZAY_LTRB(panel, 0, panel.h() - GaugeHeight, GaugeWidth, panel.h())
                mGaugeHUD.RenderObject(true, mShowDebug, panel, false);
        }
        // 슬롯
        if(const Rect* Area = mSlotHUD.GetBoundRect("area"))
        {
            const Rect* ButtonArea[4];
            if(ButtonArea[0] = mSlotHUD.GetBoundRect("item_butten_a_area"))
            if(ButtonArea[1] = mSlotHUD.GetBoundRect("item_butten_b_area"))
            if(ButtonArea[2] = mSlotHUD.GetBoundRect("item_butten_c_area"))
            if(ButtonArea[3] = mSlotHUD.GetBoundRect("item_butten_d_area"))
            {
                const sint32 SlotMaxWidth = panel.w() - GaugeWidth;
                const sint32 SlotWidth = Math::Min(SlotMaxWidth, mUIB * Area->Width() / Area->Height());
                const sint32 SlotHeight = SlotWidth * Area->Height() / Area->Width();
                const Rect GaugeRect(Point(GaugeWidth + (SlotMaxWidth - SlotWidth) / 2, panel.h() - (GaugeHeight + SlotHeight) / 2), Size(SlotWidth, SlotHeight));
                // 슬롯위치 저장
                for(sint32 i = 0; i < 4; ++i)
                {
                    const float L = (ButtonArea[i]->l - Area->l) / (Area->r - Area->l) * (GaugeRect.r - GaugeRect.l) + GaugeRect.l;
                    const float T = (ButtonArea[i]->t - Area->t) / (Area->b - Area->t) * (GaugeRect.b - GaugeRect.t) + GaugeRect.t;
                    const float R = (ButtonArea[i]->r - Area->l) / (Area->r - Area->l) * (GaugeRect.r - GaugeRect.l) + GaugeRect.l;
                    const float B = (ButtonArea[i]->b - Area->t) / (Area->b - Area->t) * (GaugeRect.b - GaugeRect.t) + GaugeRect.t;
                    mSlotPos[i].x = ((L + R) / 2 - mInGameX) / mInGameW - 0.5f;
                    mSlotPos[i].y = ((T + B) / 2 - mInGameY) / mInGameH - 0.5f;
                }
                ZAY_RECT(panel, GaugeRect)
                    mSlotHUD.RenderObject(true, mShowDebug, panel, false);
            }
        }
        // 슬롯아이템
        ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
            RenderItems(panel, true, CurTimeMsec);

        // 중지팝업
        if(mPaused)
        {
            ZAY_INNER_UI(panel, 0, "PausePopup")
            ZAY_RGBA(panel, 0, 0, 0, 128)
                panel.fill();
            mPausePopup.RenderObject(true, false, panel, false, "Pause_",
                ZAY_GESTURE_NT(n, t, this)
                {
                    if(t == GT_Pressed)
                    {
                        if(!String::Compare(n, "Pause_play_area"))
                            mPausePopup.PlayMotionOnce("click_play");
                        else if(!String::Compare(n, "Pause_loby_area"))
                            mPausePopup.PlayMotionOnce("click_lobby");
                    }
                });
        }
    }

    // 인게임클로징
    if(0 <= mClosing && mClosing < 50)
        ZAY_RGBA(panel, 0, 0, 0, 255 * (50 - mClosing) / 50)
            panel.fill();

    // 인게임 디버그정보
    if(mShowDebug)
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
        F1State::RenderDebug(panel, mMonsters, mWaveSecCurrently);

    #if BOSS_WINDOWS | BOSS_LINUX | BOSS_MAC_OSX
        const sint32 InnerGap = 10, ButtonSize = 80, ButtonSizeSmall = 50;
        ZAY_FONT(panel, 1.2, "Arial Black")
        {
            // 홈버튼
            ZAY_XYWH(panel, ButtonSize * 3, 0, ButtonSize, ButtonSizeSmall)
            ZAY_INNER_UI(panel, InnerGap, "home",
                ZAY_GESTURE_T(t, this)
                {
                    if(t == GT_InReleased)
                        next("codename_f1View");
                })
            {
                #if !BOSS_WINDOWS
                    ZAY_RGBA(panel, 255, 255, 128, 192)
                        panel.fill();
                    ZAY_RGB(panel, 0, 0, 0)
                    {
                        panel.rect(2);
                        panel.text("Home", UIFA_CenterMiddle);
                    }
                #endif
            }

            // 디버그버튼
            ZAY_XYWH(panel, ButtonSize * 4, 0, ButtonSize, ButtonSizeSmall)
            ZAY_INNER_UI(panel, InnerGap, "debug",
                ZAY_GESTURE_T(t, this)
                {
                    if(t == GT_InReleased)
                    {
                        mShowDebug = !mShowDebug;
                    }
                })
            {
                #if !BOSS_WINDOWS
                    ZAY_RGBA_IF(panel, 255, 128, 255, 192, mShowDebug)
                    ZAY_RGBA_IF(panel, 128, 255, 255, 192, !mShowDebug)
                        panel.fill();
                    ZAY_RGB(panel, 0, 0, 0)
                    {
                        panel.rect(2);
                        panel.text("Debug", UIFA_CenterMiddle);
                    }
                #endif
            }
        }
    #endif
}

void ingameData::RenderItems(ZayPanel& panel, bool slot, uint64 msec)
{
    for(sint32 i = 0, iend = mItemMap.Count(); i < iend; ++i)
    {
        auto CurItem = mItemMap.AccessByOrder(i);
        if(CurItem->slot() != slot) continue;
        auto CurPos = CurItem->CalcPos(msec);
        const float SlotRate = CurItem->CalcSlotRate(msec);
        const float X = mInGameW * (CurPos.x + 0.5f);
        const float Y = mInGameH * (CurPos.y + 0.5f);
        const float R = mItemSizeR * CurItem->CalcFlyingRate(msec) * (1 - SlotRate) + mSlotSizeR * SlotRate;
        ZAY_XYRR(panel, X, Y, R, R)
            CurItem->RenderObject(true, mShowDebug, panel, false, String::Format("Item%03d_", i),
                ZAY_GESTURE_NT(n, t, this, i)
                {
                    if(t == GT_Pressed && mClosing == -1 && !String::Compare(n + 8, "ston_touch_area"))
                    {
                        auto CurItem = mItemMap.AccessByOrder(i);
                        if(CurItem->slot())
                        {
                            auto* OldPos = CurItem->Use();
                            for(sint32 j = 0; j < 4; ++j)
                            {
                                if(&mSlotPos[j] == OldPos)
                                {
                                    mSlotFlag[j] = false;
                                    break;
                                }
                            }
                            // 스킨적용
                            mCurItemSkin = CurItem->skin();
                            mCurItemSkinEndTimeMsec = Platform::Utility::CurrentTimeMsec() + 30000;
                            mGaugeHUD.SetSkin(mCurItemSkin);
                            mGaugeHUD.PlayMotionOnce("change");
                        }
                        else for(sint32 j = 0; j < 4; ++j)
                        {
                            if(!mSlotFlag[j])
                            {
                                mSlotFlag[j] = true;
                                CurItem->MoveToSlot(&mSlotPos[j], 1000);
                                break;
                            }
                        }
                    }
                });
    }
}

void ingameData::RenderBreathArea(ZayPanel& panel)
{
    // 결정된 브레스영역
    for(sint32 i = 0; i < mBreathReadyCount; ++i)
    {
        ZAY_XYRR(panel, mBreath[i].mPos.x - mInGameX, mBreath[i].mPos.y - mInGameY, mBreathSizeR, mBreathSizeR)
        {
            mBreathReadySpine[i].SetSeekSec(mBreath[i].mGaugeTime * 0.001f);
            mBreathReadySpine[i].RenderObject(true, mShowDebug, panel, false);
        }
    }
    // 브레스영역 성장중
    if(mBreathing)
    {
        ZAY_XYRR(panel, mBreathPos.x - mInGameX, mBreathPos.y - mInGameY, mBreathSizeR, mBreathSizeR)
        {
            // 게이지파워 사용량 계산
            const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
            sint32 AniTime = Math::Min(3000, CurTimeMsec - mBreathMsec);
            mBreathGaugeTimeUsingCurrently = Math::Min(mBreathGaugeTimeLog, Math::Min(mBreathGaugeTime, AniTime));
            auto& CurBreathReadySpine = mBreathReadySpine[mBreathReadyCount];
            CurBreathReadySpine.SetSeekSec(mBreathGaugeTimeUsingCurrently * 0.001f);
            CurBreathReadySpine.RenderObject(true, mShowDebug, panel, false);

            if(const Rect* Area = CurBreathReadySpine.GetBoundRect("area"))
            if(const Rect* RedArea = CurBreathReadySpine.GetBoundRect("background_red_area"))
            if(const Rect* MinArea = CurBreathReadySpine.GetBoundRect("background_min_area"))
            if(const Rect* MaxArea = CurBreathReadySpine.GetBoundRect("background_max_area"))
            {
                // 데미지영역 계산
                const float AreaWidth = Area->Width();
                const float RedWidth = RedArea->Width();
                mBreathSizeRCurrently = Math::Max(mBreathSizeR, mBreathSizeR * RedWidth / AreaWidth);

                // 데미지파워 계산
                const float MinWidth = MinArea->Width();
                const float MaxWidth = MaxArea->Width();
                mBreathPowerPermil = Math::Clamp(1000 * (RedWidth - MinWidth) / (MaxWidth - MinWidth), 0, 1000);
            }

            // 데미지스코어
            if(mShowDebug)
            ZAY_FONT(panel, 1.5, "Arial Black")
            {
                const String Text = String::Format("%d(%dpermil)", GetCalcedBreathDamage(), mBreathPowerPermil);
                ZAY_RGBA(panel, 0, 0, 0, -128)
                    panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, Text, UIFA_CenterMiddle);
                ZAY_RGB(panel, 255, 64, 0)
                    panel.text(panel.w() / 2, panel.h() / 2, Text, UIFA_CenterMiddle);
            }
        }
    }
}

void ingameData::ReadyForNextWave()
{
    mWave++;
    mWaveTitle = "Load Failure";
    mWaveSecCurrently = 0;
    mWaveSecSettled = -1;
    mWaveSecMax = 0;

    if(mWave == 0)
        mMonsters.SubtractionAll();
    // 생존몬스터 보전
    else for(sint32 i = mMonsters.Count() - 1; 0 <= i; --i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(CurMonster.mDeathStep == 2)
            mMonsters.SubtractionSection(i, 1);
        else CurMonster.mEntranceSec = 0;
    }

    if(mWave < mWaveData("Waves").LengthOfIndexable())
    {
        mWaveTitle = mWaveData("Waves")[mWave]("Title").GetString();
        for(sint32 i = 0, iend = mWaveData("Waves")[mWave]("Events").LengthOfIndexable(); i < iend; ++i)
        {
            const sint32 TimeSec = mWaveData("Waves")[mWave]("Events")[i]("TimeSec").GetInt(0);
            mWaveSecMax = Math::Max(mWaveSecMax, TimeSec);
            auto& JsonMonsters = mWaveData("Waves")[mWave]("Events")[i]("Monsters");
            auto& JsonMissions = mWaveData("Waves")[mWave]("Events")[i]("Missions");
            if(const sint32 MonsterLength = JsonMonsters.LengthOfIndexable())
            {
                auto PtrMonsters = mMonsters.AtDumpingAdded(MonsterLength);
                for(sint32 j = 0; j < MonsterLength; ++j)
                {
                    const String CurJsonMonsterID = JsonMonsters[j]("ID").GetString("");
                    sint32 CurJsonMonsterRID = JsonMonsters[j]("RID").GetInt(0);
                    if(CurJsonMonsterRID == 0) CurJsonMonsterRID = ++mMonsterLastRID;
                    for(sint32 k = 0, kend = mMonsterTypes.Count(); k < kend; ++k)
                    {
                        const auto& CurMonsterType = mMonsterTypes[k];
                        if(!CurJsonMonsterID.Compare(CurMonsterType.mID))
                        {
                            auto& CurMonster = PtrMonsters[j];
                            CurMonster.Init(&CurMonsterType, CurJsonMonsterRID, TimeSec,
                                (mLandscape)? JsonMonsters[j]("PosY").GetFloat(0) : JsonMonsters[j]("PosX").GetFloat(0),
                                (mLandscape)? -JsonMonsters[j]("PosX").GetFloat(0) : JsonMonsters[j]("PosY").GetFloat(0),
                                GetSpine(CurMonsterType.spineName()), GetSpine("monster_toast"));
                            // 타겟로드
                            sint32 CurRID = CurMonster.mRID;
                            float CurX = mInGameW * (CurMonster.mCurrentPos.x + 0.5f);
                            float CurY = mInGameH * (CurMonster.mCurrentPos.y + 0.5f);
                            while(CurRID != -1)
                            {
                                sint32 NextRID = -1;
                                for(sint32 m = 0, mend = JsonMissions.LengthOfIndexable(); m < mend; ++m)
                                {
                                    if(CurRID == JsonMissions[m]("TargetRID").GetInt(-1))
                                    {
                                        NextRID = JsonMissions[m]("RID").GetInt(-1);
                                        auto& NewTarget = CurMonster.mTargets.AtAdding();
                                        NewTarget.mType = MonsterTarget::Mission;
                                        const Point OldMonsterPos(CurX, CurY);
                                        CurX += mInGameSize * JsonMissions[m]("PosX").GetFloat(0);
                                        CurY += mInGameSize * JsonMissions[m]("PosY").GetFloat(0);
                                        NewTarget.mPos.x = CurX / mInGameW - 0.5f;
                                        NewTarget.mPos.y = CurY / mInGameH - 0.5f;
                                        NewTarget.mSizeR = 0.01;
                                        if(auto* CurTryWorldZone = mAllTryWorldZones.Access(CurMonsterType.mPolygon))
                                        {
                                            sint32 GetScore = 0;
                                            const Point NewMonsterPos(mInGameW * (NewTarget.mPos.x + 0.5f), mInGameH * (NewTarget.mPos.y + 0.5f));
                                            NewTarget.mPath = CurTryWorldZone->mMap->BuildPath(OldMonsterPos, NewMonsterPos, 20, &GetScore);
                                        }
                                        break;
                                    }
                                }
                                CurRID = NextRID;
                            }
                            // 파라토크 댓글매칭
                            if(mCurParaTalk < Parser::GetInt(Platform::Option::GetText("ParaTalkCount")))
                            {
                                chars CurParaTalk = Platform::Option::GetText(String::Format("ParaTalkText_%d", mCurParaTalk++));
                                CurMonster.mParaTalk = CurParaTalk;
                            }
                            break;
                        }
                    }
                }
            }
        }
        for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
            mMonsters.At(i).ResetCB();
    }
    else mWaveTitle = "Stage Over";
}

void ingameData::SetDragonSchedule(const MapBreath* breath, bool retry)
{
    // 스킨적용
    mCurItemSkinForDragon = mCurItemSkin;
    // 드래곤 일정기록
    mDragon.SetSkin(mCurItemSkinForDragon);
    const Point LastPos = mDragon.pos();
    mDragon.GoTarget(LastPos, breath->mPos, mInGameX + mInGameW / 2 < breath->mPos.x,
        (retry)? mDragonRetryTime : mDragonEntryTime, mDragonBreathTime, mDragonExitTime);
}

sint32 ingameData::GetCalcedBreathDamage()
{
    return mBreathMinDamage + (mBreathMaxDamage - mBreathMinDamage) * mBreathPowerPermil / 1000;
}

void ingameData::SetBreathAttack(const MapBreath* breath)
{
    // 몬스터피해
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(CurMonster.mType->mType != MonsterType::TypeClass::Enemy) continue;
        if(mWaveSecCurrently < CurMonster.mEntranceSec || CurMonster.mHPValue == 0)
            continue;

        float x = mInGameX + mInGameW * (CurMonster.mCurrentPos.x + 0.5f);
        float y = mInGameY + mInGameH * (CurMonster.mCurrentPos.y + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mMonsterSizeR + breath->mSizeR)
        {
            CurMonster.mHPValue = Math::Max(0, CurMonster.mHPValue - breath->mDamage);
            CurMonster.mHPTimeMsec = Platform::Utility::CurrentTimeMsec() + mHPbarDeleteTime;

            // 넉백처리
            sint32 Distance = 0;
            while((Distance = Math::Distance(x, y, breath->mPos.x, breath->mPos.y)) < CurMonster.mKnockBackNearMin) // 극인접처리
            {
                x = breath->mPos.x + (Platform::Utility::Random() % 1000) - 500;
                y = breath->mPos.y + (Platform::Utility::Random() % 1000) - 500;
            }
            const float PowerRate = Math::ClampF((breath->mDamage - mBreathMinDamage) / (float) (mBreathMaxDamage - mBreathMinDamage), 0, 1);
            const float AccelRate = ((mKnockBackMaxV - mKnockBackMinV) * PowerRate + mKnockBackMinV) / Distance;
            const Point NewAccel = Point((x - breath->mPos.x) * AccelRate, (y - breath->mPos.y) * AccelRate) * 1000 / CurMonster.mType->mWeight;

            // 애니처리
            CurMonster.mFlipMode = (x < breath->mPos.x) ^ (y > breath->mPos.y);
            CurMonster.KnockBack(y > breath->mPos.y, Point(NewAccel.x / mInGameW, NewAccel.y / mInGameH), mCurItemSkinForDragon);
            CurMonster.ClearAllTargets();
        }
    }

    // 오브젝트피해
    auto& CurObjects = mLayers.At(2).mObjects;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        auto& CurObject = CurObjects.At(i);
        if(!CurObject.mEnable || CurObject.mType->mType != ObjectType::TypeClass::Dynamic)
            continue;
        const float x = mInGameX + mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameY + mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        const float r = mInGameSize * CurObject.mCurrentRect.Width();
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < r + breath->mSizeR)
            if(CurObject.SetHP(CurObject.mHPValue - breath->mDamage, mHPbarDeleteTime))
                ClearAllPathes(false);
    }

    // 타겟피해(유저의 조작실수)
    for(sint32 i = 0, iend = mTargetsForEnemy.Count(); i < iend; ++i)
    {
        auto& CurObject = mLayers.At(2).mObjects.At(mTargetsForEnemy[i].mObjectIndex);
        if(CurObject.mHPValue == 0) continue;
        const float x = mInGameX + mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameY + mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mInGameSize * mTargetsForEnemy[i].mSizeR + breath->mSizeR)
            if(CurObject.SetHP(CurObject.mHPValue - breath->mDamage, mHPbarDeleteTime))
                ClearAllPathes(false);
    }
}
