#include <boss.hpp>
#include "ingame_f1.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("ingameView", ingameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        // 세이브파일 업데이트
        FXSaver::Update();
        if(m->mSpineInited)
        {
            if(!m->mPaused)
            {
                // 시간진행
                const uint64 CurTimeSec = Platform::Utility::CurrentTimeMsec() / 1000;
                if(m->mCurTickTimeSec == 0) m->mCurTickTimeSec = CurTimeSec;
                sint32 CurTimeSecSpan = (sint32) (CurTimeSec - m->mCurTickTimeSec);
                if(0 < CurTimeSecSpan)
                {
                    CurTimeSecSpan = 1; // 1초초과는 인정하지 않음(Pause모드나 디버깅시)
                    m->mWaveSecCurrently += CurTimeSecSpan;
                    m->mCurTickTimeSec = CurTimeSec;
                }
                // 애니메이션 진행
                if(m->mWave != -1)
                {
                    while(m->mWaveSecSettled < m->mWaveSecCurrently)
                        m->PlayScriptOnce(m->mWaveSecSettled++);
                    m->AnimationOnce(CurTimeSecSpan);
                    m->ReserveToSlotOnce();
                }
            }

            // 인게임종료
            if(0 < m->mClosing && --m->mClosing == 0)
            {
                if(m->mClosingOption == 0)
                {
                    Platform::Option::SetFlag("LandscapeMode", false);
                    m->next("outgameView");
                }
                else if(m->mClosingOption == 1)
                {
                    Platform::Option::SetFlag("LandscapeMode", false);
                    Platform::Option::SetFlag("DirectPlay", true);
                    m->next("ingameView");
                }
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
            // 배경사운드시작
            if(FXSaver::Read("SoundFlag").GetInt() && FXSaver::Read("BGMFlag").GetInt())
                Platform::Sound::Play(m->GetSound(m->mBGMusic, true));
        }
        else m->BuildTryWorld(false);
        m->ClearAllPathes(true);
        // 윈도우 타이틀
        if(FXSaver::Read("DevMode").GetInt())
            Platform::SetWindowName(String::Format("Codename F1 [%dx%d:%.03f] - %s(%s)",
                Width, Height, Height / (float) Width, (chars) m->mStage, (chars) m->mWaveData("MapName").GetString()));
        else Platform::SetWindowName("DragonBreath - MonthlyKoobonil");
    }
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    static bool IsPressEnabled = true;

    if(m->mWave == -1 || m->mPaused || m->mClosing != -1)
    {
        IsPressEnabled = true;
        if(type == GT_Pressed)
        {
            // 디버그모드시 타이틀에서는 블럭을 부셔볼 수 있음
            if(m->mWave == -1 && m->mShowDebug)
            {
                auto& CurObjects = m->mLayers.At(2).mObjects;
                for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
                {
                    auto& CurObject = CurObjects.At(i);
                    if(!CurObject.mType->mType.isDynamic() || CurObject.mHPValue == 0)
                        continue;
                    const float l = m->mInGameX + m->mInGameW * (CurObject.mCurrentRect.l + 0.5f);
                    const float t = m->mInGameY + m->mInGameH * (CurObject.mCurrentRect.t + 0.5f);
                    const float r = m->mInGameX + m->mInGameW * (CurObject.mCurrentRect.r + 0.5f);
                    const float b = m->mInGameY + m->mInGameH * (CurObject.mCurrentRect.b + 0.5f);
                    if(l <= x && x < r && t <= y && y < b)
                    {
                        if(CurObject.SetHP(0, m->mHPbarDeleteTime))
                        {
                            if(m->mShowDebug)
                                CurObject.mVisible = false;
                            m->SetBrokenObject(CurObject);
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(type == GT_Pressed)
        {
            // 동맹군 터치여부
            for(sint32 i = 0, iend = m->mMonsters.Count(); i < iend; ++i)
            {
                auto& CurMonster = m->mMonsters.At(i);
                if(CurMonster.mType->mType != monster_type::Ally) continue;
                if(m->mWaveSecCurrently < CurMonster.mEntranceSec || 0 < CurMonster.mDeathStep)
                    continue;

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
            m->mBreathPos = Point(x, y - m->mFingerSizeR);
            m->mBreathMsec = Platform::Utility::CurrentTimeMsec();
            m->mBreathGaugeTimeUsingCurrently = 0;
            m->mBreathSizeRCurrently = m->mBreathSizeR;
            m->mBreathPowerPermil = 0;
            IsPressEnabled = true;
            // 브레스바 노출
            m->mBreathBarSpine.SetSkin((m->mBreathReadyCount == 0)? "first" : "second");
            m->mBreathBarSpine.StopMotionAll();
            m->mBreathBarSpine.PlayMotion("meteo_bar", false);
        }
        else if(IsPressEnabled)
        {
            if(type == GT_InDragging || type == GT_OutDragging)
            {
                if(m->mBreathing)
                    m->mBreathPos = Point(x, y - m->mFingerSizeR);
            }
            else if(type == GT_InReleased || type == GT_OutReleased)
            {
                if(m->mBreathing)
                {
                    // 브레스전달
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
                    // 브레스바 숨김
                    m->mBreathBarSpine.StopMotionAll();
                    m->mBreathBarSpine.PlayMotion("idle", true);
                }
            }
        }
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    #if BOSS_WINDOWS | BOSS_LINUX | BOSS_MAC_OSX
        ZAY_RGB(panel, 0, 0, 0)
            panel.fill();
    #endif

    // 인게임
    m->Render(panel);

    // 도어인증
    if(m->mWave == -1)
        m->door().Render(panel);
}

ingameData::ingameData() : F1State(true)
{
    mShowDebug = false;

    mSpineInited = false;
    mOptionItemInited = false;
    mPaused = false;
    mClosing = -1;
    mClosingOption = 0;
    mWaveData.LoadJson(SO_NeedCopy, String::FromAsset(mStage));
    mWave = -1;
    mWaveTitle = "";
    mWaveSecCurrently = 0;
    mWaveSecSettled = -1;
    mWaveSecMax = 0;
    mCurTickTimeSec = 0;
    mCurParaTalk = 0;
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

    // 옵션
    mGameMode = GameMode::Normal;
    mGameBeginMsec = 0;
    mGameStopMsec = 0;
    mGameSumStopMsec = 0;
    mGameSumWave = 0;
    mGameScore = 0;
    mGameScoreLog = 0;
    // 로컬실행을 위한 옵션셋팅
    if(!String::Compare(mStage, "f1/table_etc/", 13))
        mGameOption.LoadJson(SO_NeedCopy, "{\"FireStone\":10,\"IceStone\":10,\"WindStone\":10,\"LightningStone\":10}");
    // 옵션셋팅
    else if(!mStage.Right(5).CompareNoCase(".json"))
    {
        const String OptionAssetName = mStage.Left(mStage.Length() - 5) + "_option.json";
        const String Option = String::FromAsset(OptionAssetName);
        if(0 < Option.Length())
            mGameOption.LoadJson(SO_NeedCopy, Option);
    }
    else BOSS_ASSERT("mStage의 값은 .json으로 끝나야 합니다", false);
    mGameMode = mGameOption("GameMode").GetString("Normal");
}

ingameData::~ingameData()
{
    // 배경사운드끝
    Platform::Sound::Stop(GetSound(mBGMusic, true));
}

#define PATHFIND_STEP (5)
void ingameData::Targeting(MapMonster& monster, const TryWorldZone& tryworld)
{
    // 타겟중 최적의 타겟을 선정
    TryWorld::Path* ResultPath = nullptr;
    sint32 ResultPathScore = 0, ResultIndex = -1;
    auto& CurTargets = (monster.mType->mType == monster_type::Ally)? mTargetsForAlly : mTargetsForEnemy;
    const Point CurMonsterPos(mInGameW * (monster.mCurrentPos.x + 0.5f), mInGameH * (monster.mCurrentPos.y + 0.5f));
    for(sint32 j = 0, jend = CurTargets.Count(); j < jend; ++j)
    {
        auto& CurObject = mLayers[2].mObjects[CurTargets[j].mObjectIndex];
        if(CurObject.mHPValue == 0) continue;
        const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);

        sint32 GetScore = 0;
        auto NewPath = tryworld.mMap->BuildPath(CurMonsterPos, Point(x, y), PATHFIND_STEP, &GetScore);
        sint32 NewPathScore = GetScore * 1000 / mInGameSize; // 인게임사이즈당 1000점으로 환산
        NewPathScore += CurObject.mHPValue; // 점수가 낮은 타겟이 우선
        NewPathScore += Platform::Utility::Random() % 1000; // 랜덤요소 추가

        if(!ResultPath || (NewPath && NewPathScore < ResultPathScore))
        {
            TryWorld::Path::Release(ResultPath);
            ResultPath = NewPath;
            ResultPathScore = NewPathScore;
            ResultIndex = CurTargets[j].mObjectIndex;
        }
        else TryWorld::Path::Release(NewPath);
    }

    if(ResultPath)
    {
        auto& NewTarget = monster.mTargets.AtAdding();
        NewTarget.mType = MonsterTarget::Target;
        NewTarget.mIndex = ResultIndex;
        NewTarget.mPos = mLayers[2].mObjects[ResultIndex].mCurrentRect.Center();
        NewTarget.mPath = ResultPath;
    }
}

sint32 ingameData::GetContactObject(const MapMonster& monster, const Point& nextPos)
{
    float ResultDistance = -1;
    sint32 ResultIndex = -1;
    if(auto CurTryWorldZone = mAllTryWorldZones.Access(monster.mType->mPolygon))
    {
        const auto& CurObjects = mLayers[2].mObjects;
        const float MonsterWaistSizeRW = mMonsterSizeR * CurTryWorldZone->mType->mWaistScaleWidth / 1000 * 1.1f; // 110%
        const float MonsterWaistSizeRH = mMonsterSizeR * CurTryWorldZone->mType->mWaistScaleHeight / 1000 * 1.1f; // 110%
        for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
        {
            const auto& CurObject = CurObjects[i];
            if(!CurObject.mVisible || !(CurObject.CanBroken(monster.mType->mPolygon) || CurObject.ValidTarget(*monster.mType) || CurObject.ValidTrigger()))
                continue;
            const Rect Area(
                mInGameW * (CurObject.mCurrentRect.l + 0.5f) - MonsterWaistSizeRW,
                mInGameH * (CurObject.mCurrentRect.t + 0.5f) - MonsterWaistSizeRH,
                mInGameW * (CurObject.mCurrentRect.r + 0.5f) + MonsterWaistSizeRW,
                mInGameH * (CurObject.mCurrentRect.b + 0.5f) + MonsterWaistSizeRH);
            if(!Area.PtInRect(nextPos))
                continue;
            // 최소거리의 오브젝트를 찾음
            const float NextDistance = Math::Distance(nextPos.x, nextPos.y, Area.CenterX(), Area.CenterY());
            if(ResultDistance == -1 || NextDistance < ResultDistance)
            {
                ResultDistance = NextDistance;
                ResultIndex = i;
            }
        }
    }
    return ResultIndex;
}

sint32 ingameData::GetValidNextObject(const MapMonster& monster, const Point& curPos, const Point& nextPos, Point& resultPos, Point& reflectPos)
{
    Point ResultPos = nextPos;
    float ResultDistance = -1;
    sint32 ResultIndex = -1;
    if(auto CurTryWorldZone = mAllTryWorldZones.Access(monster.mType->mPolygon))
    {
        auto& CurObjects = mLayers[2].mObjects;
        const float MonsterWaistSizeRW = mMonsterSizeR * CurTryWorldZone->mType->mWaistScaleWidth / 1000 * 1.1f; // 110%
        const float MonsterWaistSizeRH = mMonsterSizeR * CurTryWorldZone->mType->mWaistScaleHeight / 1000 * 1.1f; // 110%
        for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
        {
            const auto& CurObject = CurObjects[i];
            if(!CurObject.mVisible || !(CurObject.CanBroken(monster.mType->mPolygon) || CurObject.ValidTrigger()))
                continue;
            const Rect Area(
                mInGameW * (CurObject.mCurrentRect.l + 0.5f) - MonsterWaistSizeRW,
                mInGameH * (CurObject.mCurrentRect.t + 0.5f) - MonsterWaistSizeRH,
                mInGameW * (CurObject.mCurrentRect.r + 0.5f) + MonsterWaistSizeRW,
                mInGameH * (CurObject.mCurrentRect.b + 0.5f) + MonsterWaistSizeRH);
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
    }
    resultPos = ResultPos;
    return ResultIndex;
}

void ingameData::ItemToSlot(MapItem& item)
{
    sint32 SaveIndex = -1;
    for(sint32 i = 0; i < 5; ++i)
    {
        if(i == 4)
        {
            if(SaveIndex != -1)
            {
                mSlotStatus[SaveIndex].mItemType = item.type();
                mSlotStatus[SaveIndex].mCount++;
                item.MoveToSlot(SaveIndex, &mSlotStatus[SaveIndex].mPos, 1000);
            }
        }
        else if(mSlotStatus[i].mItemType)
        {
            if(mSlotStatus[i].mItemType == item.type())
            {
                mSlotStatus[i].mCount++;
                item.MoveToSlot(i, &mSlotStatus[i].mPos, 1000);
            }
        }
        else if(SaveIndex == -1)
            SaveIndex = i;
    }
}

void ingameData::ReserveItem(chars skin)
{
    for(sint32 i = 0; i < 4; ++i)
    {
        if(!mSlotStatus[i].mItemType) continue;
        if(!mSlotStatus[i].mItemType->mSkinName.Compare(skin))
        {
            mSlotStatus[i].mReserved++;
            break;
        }
    }
}

void ingameData::InitForSpine()
{
    // 맵로드
    const String MapName = mWaveData("MapName").GetString();
    // 로컬스테이지 플레이인지 체크
    const String MapPath = (!String::Compare(mStage, "f1/table_etc/", 13))? "f1/table_etc/" : "f1/table/";
    id_asset_read TextAsset = Asset::OpenForRead(MapPath + MapName + ".json");
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
    // 길찾기맵 첫빌드
    BuildTryWorld(false);

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
        if(door().IsLocked())
        {
            mMainTitleSpine.PlayMotion("start_unlock", false);
            mMainTitleSpine.PlayMotion("loby_unlock", false);
        }
        else if(FXSaver::Read("SumHeart").IsValid())
        {
            if(FXSaver::Read("SumHeart").GetInt() == 0)
                mMainTitleSpine.PlayMotion("start_unlock", false);
        }
    }

    // Effect
    for(sint32 i = 0; i < mWallBoundMax; ++i)
        mWallBound[i].InitSpine(GetSpine("wall_bound")).PlayMotion("idle", true);

    // InGame
    mDragon.Init(GetSpine("dragon"), mDragonScaleMax / mDragonScale, updater(), mDragonHome, mDragonExitL, mDragonExitR);
    mDragon.ResetCB(this);
    mBreathReadySpine[0].InitSpine(GetSpine("breath_ready"), "first").PlayMotionSeek("meteo_ready", false);
    mBreathReadySpine[1].InitSpine(GetSpine("breath_ready"), "second").PlayMotionSeek("meteo_ready", false);
    mBreathBarSpine.InitSpine(GetSpine("breath_bar"), "first").PlayMotion("idle", true);
    mBreathAttackSpine.InitSpine(GetSpine("breath_attack")).PlayMotion("idle", true);
    mBreathEffectSpine.InitSpine(GetSpine("breath_effect")).PlayMotion("idle", true);

    // UI
    if(mGameMode == GameMode::Infinity)
        mWeather[0].InitSpine(GetSpine("a_weather_toast")).PlayMotion("infinity", true);
    else mWeather[0].InitSpine(GetSpine("a_weather_toast")).PlayMotion(mBGWeather, true);
    mWeather[1].InitSpine(GetSpine("b_weather_toast")).PlayMotion(mBGWeather, true);
    mCampaign.InitSpine(GetSpine("ui_campaign")).PlayMotion("idle", true);
    mGaugeHUD.InitSpine(GetSpine("ui_ingame_gauge"), "normal").PlayMotionSeek("charge", false);
    mGaugeHUD.PlayMotionOnce("loading");
    mSlotHUD.InitSpine(GetSpine("ui_ingame_slot")).PlayMotionAttached("show", "idle", true);
    if(mGameMode == GameMode::Infinity)
        mWaveHUD.InitSpine(GetSpine("ui_ingame_wave")).PlayMotionAttached("show_infinity", "idle_infinity", true);
    else mWaveHUD.InitSpine(GetSpine("ui_ingame_wave")).PlayMotionAttached("show", "idle", true);
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
            {
                mPaused = false;
                if(mClosing == -1)
                {
                    mGameSumStopMsec += Platform::Utility::CurrentTimeMsec() - mGameStopMsec;
                    mGameStopMsec = 0;
                }
            }
        }).PlayMotion("idle", true);
}

void ingameData::PlayScriptOnce(sint32 sec)
{
    auto& CurEvents = mWaveData("Waves")[mWave]("Events");
    for(sint32 i = 0, iend = CurEvents.LengthOfIndexable(); i < iend; ++i)
    {
        const sint32 TimeSec = CurEvents[i]("TimeSec").GetInt(0);
        if(TimeSec == sec)
        {
            auto& CurScripts = CurEvents[i]("Scripts");
            for(sint32 j = 0, jend = CurScripts.LengthOfIndexable(); j < jend; ++j)
            {
                const sint32 ObjectRID = CurScripts[j]("TargetRID").GetInt(0);
                const String ScriptText = CurScripts[j]("Text").GetString();
                if(ObjectRID == 0)
                {
                    if(!String::Compare(ScriptText, "wavejump ", 9))
                    {
                        // 웨이브 선택과정
                        sint32s WaveIDCollector;
                        sint32 LastWaveID = 0;
                        chars CurWord = ((chars) ScriptText) + 9 - 1; // ++CurWord을 위해 한칸 앞으로 이동
                        while(*(++CurWord))
                        {
                            if(*CurWord == '/')
                            {
                                WaveIDCollector.AtAdding() = LastWaveID;
                                LastWaveID = 0;
                            }
                            else if('0' <= *CurWord && *CurWord <= '9')
                                LastWaveID = LastWaveID * 10 + (*CurWord - '0');
                        }
                        WaveIDCollector.AtAdding() = LastWaveID;
                        // 웨이브이동
                        const sint32 SelectedIndex = Platform::Utility::Random() % WaveIDCollector.Count();
                        const sint32 NewWaveIndex = WaveIDCollector[SelectedIndex] - 1; // 툴에서는 1부터 시작, 0은 내 웨이브잔존
                        if(0 <= NewWaveIndex)
                        {
                            // ReadyForNextWave에서 1증가 될테니 -1
                            mWave = Math::Min(NewWaveIndex, mWaveData("Waves").LengthOfIndexable() - 1) - 1;
                            ReadyForNextWave();
                        }
                        // 스크립트로그
                        mDebugScriptLogs.AtAdding() = String::Format("[wavejump] %s ▶ %d", (chars) ScriptText, NewWaveIndex + 1);
                    }
                }
                else if(auto CurObject = mObjectRIDs.Access(ObjectRID))
                {
                    if(!String::Compare(ScriptText, "animate ", 8))
                    {
                        const String LastMotion = (*CurObject)->PlayMotionScript(((chars) ScriptText) + 8);
                        // 트리거처리
                        if((*CurObject)->mType->mType.isTrigger())
                        {
                            if(!LastMotion.Compare("idle"))
                                (*CurObject)->mTriggerOpened = true;
                            else if(!LastMotion.Compare("show"))
                                (*CurObject)->mTriggerOpened = false;
                        }
                        // 스크립트로그
                        mDebugScriptLogs.AtAdding() = String::Format("[animate] %s ▶ [%s] %d",
                            (chars) ScriptText, (chars) (*CurObject)->mType->mID, ObjectRID);
                    }
                }
            }
            break;
        }
    }
    // 스크립트로그 수량제한
    if(20 < mDebugScriptLogs.Count())
        mDebugScriptLogs.SubtractionSection(0, mDebugScriptLogs.Count() - 20);
}

void ingameData::AnimationOnce(sint32 sec_span)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    // 옵션로드
    if(!mOptionItemInited && 2 < mWaveSecCurrently)
    {
        mOptionItemInited = true;
        // 시작캠페인
        mCampaign.StopMotionAll();
        mCampaign.PlayMotionAttached("start_show", "idle", true);
        // 아이템추가
        chars OptionNames[4] = {"FireStone", "IceStone", "WindStone", "LightningStone"};
        chars SkinNames[4] = {"fire", "ice", "wind", "lightning"};
        for(sint32 i = 0; i < 4; ++i)
        for(sint32 j = 0, jend = mGameOption(OptionNames[i]).GetInt(0); j < jend; ++j)
        {
            if(j == 0)
            {
                for(sint32 k = 0, kend = mItemTypes.Count(); k < kend; ++k)
                {
                    if(!mItemTypes[k].mSkinName.Compare(SkinNames[i]))
                    {
                        auto& NewItem = mItemMap(MapItem::MakeId());
                        NewItem.InitForSlot(&mItemTypes[k], GetSpine("item"), updater(), Point(0, -1));
                        ItemToSlot(NewItem);
                        break;
                    }
                }
            }
            else ReserveItem(SkinNames[i]);
        }
    }

    // 스킬의 종료시점 확인
    if(mCurSkill.IsFinished(CurTimeMsec))
    {
        // 스킬풀림
        mCurSkill.Clear();
        // 스킨풀림
        mGaugeHUD.SetSkin(mCurSkill.skin());
        mGaugeHUD.PlayMotionOnce("change");
    }

    // 몬스터
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(mWaveSecCurrently < CurMonster.mEntranceSec || 0 < CurMonster.mDeathStep) // 등장전 또는 죽을때
        {
            if(CurMonster.mDeathStep == 1)
                CurMonster.TryDeathMove();
            continue;
        }

        // 인접몬스터로부터 밀려나기
        bool HasShove = false;
        if(CurMonster.mShoveTimeMsec + mShoveCoolTime <= CurTimeMsec) // ShoveCoolTime마다 테스트실시
        {
            CurMonster.mShoveTimeMsec = CurTimeMsec;
            for(sint32 j = 0, jend = mMonsters.Count(); j < jend; ++j)
            {
                if(i == j) continue;
                auto& OtherMonster = mMonsters.At(j);
                if(mWaveSecCurrently < OtherMonster.mEntranceSec || 0 < OtherMonster.mDeathStep)
                    continue;
                const float OtherDistance = Math::Distance(CurMonster.mCurrentPos.x, CurMonster.mCurrentPos.y,
                    OtherMonster.mCurrentPos.x, OtherMonster.mCurrentPos.y);
                if(OtherDistance < mShoveCheckDistance)
                {
                    HasShove = true;
                    if(OtherDistance < 0.001f)
                    {
                        const float NewRad = Math::ToRadian(Platform::Utility::Random() % 360);
                        CurMonster.mCurrentVec.x = mShovePower * Math::Cos(NewRad);
                        CurMonster.mCurrentVec.y = mShovePower * Math::Sin(NewRad);
                    }
                    else
                    {
                        CurMonster.mCurrentVec.x = mShovePower * (CurMonster.mCurrentPos.x - OtherMonster.mCurrentPos.x) / OtherDistance;
                        CurMonster.mCurrentVec.y = mShovePower * (CurMonster.mCurrentPos.y - OtherMonster.mCurrentPos.y) / OtherDistance;
                    }
                    break;
                }
            }
        }

        // 진행방향 가속벡터 업데이트
        if(!HasShove)
        {
            const float VecDistanceMax = mMonsterScale * CurMonster.CalcedVector();
            const sint32 VecWeight = 1000 - CurMonster.mType->mWeight;
            CurMonster.mCurrentVec.x = (CurMonster.mCurrentVec.x * VecWeight + (CurMonster.mCurrentPos.x - CurMonster.mCurrentPosOld.x)) / (VecWeight + 1);
            CurMonster.mCurrentVec.y = (CurMonster.mCurrentVec.y * VecWeight + (CurMonster.mCurrentPos.y - CurMonster.mCurrentPosOld.y)) / (VecWeight + 1);
            const float VecDistance = Math::Sqrt(CurMonster.mCurrentVec.x * CurMonster.mCurrentVec.x + CurMonster.mCurrentVec.y * CurMonster.mCurrentVec.y);
            if(VecDistanceMax < VecDistance) CurMonster.mCurrentVec *= VecDistanceMax / VecDistance;
        }

        // 위치와 벡터
        const Point CurPos(mInGameW * (CurMonster.mCurrentPos.x + 0.5f), mInGameH * (CurMonster.mCurrentPos.y + 0.5f));
        const Point CurVec(mInGameW * CurMonster.mCurrentVec.x, mInGameH * CurMonster.mCurrentVec.y);

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
                if(!CurObject.mVisible || CurObject.mType->mType != object_type::Spot)
                    continue;
                const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
                const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
                const float r = mInGameSize * CurObject.mCurrentRect.Width();
                if(Math::Distance(x, y, CurPos.x, CurPos.y) < r)
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

        // 토스트 잔존테스트
        CurMonster.ToastTest(CurTimeMsec);

        // 몬스터AI
        if(!MonsterActionOnce(CurMonster, CurPos))
        {
            if(auto CurTryWorldZone = mAllTryWorldZones.Access(CurMonster.mType->mPolygon))
            {
                // 닿은 오브젝트 초기화
                CurMonster.mBounceObjectIndex = -1;
                // 다음위치 계산
                CurMonster.mCurrentPosOld = CurMonster.mCurrentPos;
                CurMonster.mCurrentPos = (CurMonster.IsKnockBackMode())?
                    MonsterKnockBackOnce(CurMonster, *CurTryWorldZone, CurPos, CurTimeMsec) :
                    MonsterMoveOnce(CurMonster, *CurTryWorldZone, CurPos, CurVec, CurTimeMsec);
            }
        }
        else CurMonster.mCurrentVec = Point(0, 0);
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

    // 죽어서 게임종료
    if(mClosing == -1 && LiveTargetCount == 0)
    {
        mClosing = 200;
        if(mGameMode == GameMode::Infinity)
        {
            Platform::Option::SetText("StartMode", "Lobby");
            const sint32 OldScore = FXSaver::Read("InfinityScore").GetInt(0);
            FXSaver::Write("InfinityScore").Set(String::Format("%d", Math::Max(mGameScoreLog, OldScore)));
        }
        else
        {
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
                        WinLose->RenderObject(DebugMode::None, true, panel, false, nullptr, nullptr,
                            ZAY_RENDER_PN(p, n, data)
                            {
                                if(!String::Compare(n, "dragon_lose_area"))
                                if(auto Dragon = data.mSpines.Access("dragon"))
                                    Dragon->RenderObject(DebugMode::None, true, p, false);
                            });
                });
        }
    }

    // 웨이브전환(초단위로 판단)
    if(0 < sec_span)
    {
        const sint32 WaveCount = mWaveData("Waves").LengthOfIndexable();
        if(mWave < WaveCount)
        {
            // 몬스터 상황
            bool AnyMonsterAlived = false; // 등장된 몬스터 잔존여부
            bool WaveStop = false; // 웨이브조건인 대장몬스터의 생존여부
            for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
            {
                auto& CurMonster = mMonsters.At(i);
                if(CurMonster.mDeathStep == 2) continue;
                AnyMonsterAlived = true;
                if(WaveStop = CurMonster.mType->mWaveStop) break;
            }
            // 미실행 이벤트 존재여부(몬스터, 스크립트등)
            bool AnyEventWaiting = false;
            auto& CurEvents = mWaveData("Waves")[mWave]("Events");
            for(sint32 i = 0, iend = CurEvents.LengthOfIndexable(); i < iend; ++i)
            {
                const sint32 TimeSec = CurEvents[i]("TimeSec").GetInt(0);
                if(AnyEventWaiting = (mWaveSecSettled <= TimeSec)) break;
            }

            // 중간 웨이브
            if(mWave < WaveCount - 1)
            {
                if(!AnyEventWaiting && !WaveStop && mWaveSecMax < mWaveSecSettled)
                    ReadyForNextWave();
            }
            // 마지막 웨이브(살아서 게임종료)
            else if(mGameMode != GameMode::Infinity && !AnyMonsterAlived)
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
                            WinLose->RenderObject(DebugMode::None, true, panel, false, nullptr, nullptr,
                                ZAY_RENDER_PN(p, n, data)
                                {
                                    if(!String::Compare(n, "dragon_win_area"))
                                    if(auto Dragon = data.mSpines.Access("dragon"))
                                        Dragon->RenderObject(DebugMode::None, true, p, false);
                                });
                    });
            }
        }
    }

    // 브레스폭발
    if(0 < mBreathReadyCount && mBreath[0].mEndTimeMsec < CurTimeMsec)
    {
        SetBreathAttack(&mBreath[0]);
        // 브레스 애니
        mBreathAttackSpine.StopMotionAll();
        mBreathAttackSpine.PlayMotionAttached(String::Format("breath_%s_attack_b", mCurSkill.dragon_skin()), "idle", true);
        // 브레스이펙트 애니
        mBreathEffectSpine.StopMotionAll();
        mBreathEffectSpine.PlayMotionAttached(String::Format("breath_%s_ground", mCurSkill.dragon_skin()), "idle", true);
        // 현재 브레스바 스킨재조정
        mBreathBarSpine.SetSkin("first");
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

bool ingameData::MonsterActionOnce(MapMonster& monster, const Point& pos)
{
    // 공격
    bool HasAction = false;
    if(!monster.IsKnockBackMode())
    {
        // 타겟공격 또는 도착
        if(0 < monster.mTargets.Count())
        {
            bool LossAttack = false;
            bool LossTarget = false;
            auto& CurTarget = monster.mTargets[0];

            if(monster.mBounceObjectIndex == -1)
            {
                if(CurTarget.mType == MonsterTarget::Target)
                {
                    auto& CurObject = mLayers.At(2).mObjects.At(CurTarget.mIndex);
                    if(CurObject.mHPValue == 0)
                        LossAttack = LossTarget = true;
                }
                else if(CurTarget.mType == MonsterTarget::Mission)
                {
                    const float x = mInGameW * (CurTarget.mPos.x + 0.5f);
                    const float y = mInGameH * (CurTarget.mPos.y + 0.5f);
                    const float r = mInGameSize * CurTarget.mSizeR;
                    if(Math::Distance(pos.x, pos.y, x, y) < r)
                        LossTarget = true;
                }
            }
            else
            {
                auto& CurObject = mLayers.At(2).mObjects.At(monster.mBounceObjectIndex);
                // 타겟에 대한 공격
                if(CurObject.mType->mType.isTarget() && monster.mType->mType == monster_type::Enemy)
                {
                    if(0 < CurObject.mHPValue)
                    {
                        HasAction = true;
                        if(auto AttackCount = monster.TryAttack(CurObject.mCurrentRect.Center())) // 타겟공격
                            if(CurObject.SetHP(CurObject.mHPValue - monster.mType->mAttackPower * AttackCount, mHPbarDeleteTime))
                            {
                                if(mShowDebug)
                                    CurObject.mVisible = false;
                                ClearAllPathes(false);
                            }
                    }
                    else LossAttack = LossTarget = true;
                }
                else if(CurObject.mType->mType.isAllyTarget() && monster.mType->mType == monster_type::Ally) // 동맹군도착
                {
                    HasAction = true;
                    monster.Ally_Arrived();
                }
                // 다이나믹 오브젝트에 대한 공격
                else if(CurObject.CanBroken(monster.mType->mPolygon))
                {
                    if(0 < CurObject.mHPValue)
                    {
                        HasAction = true;
                        if(auto AttackCount = monster.TryAttack(CurObject.mCurrentRect.Center())) // 오브젝트공격
                        {
                            if(CurObject.SetHP(CurObject.mHPValue - monster.mType->mAttackPower * AttackCount, mHPbarDeleteTime))
                            {
                                if(mShowDebug)
                                    CurObject.mVisible = false;
                                SetBrokenObject(CurObject);
                            }
                        }
                    }
                    else LossAttack = true;
                }
            }
            if(LossAttack) monster.CancelAttack();
            if(LossTarget) monster.ClearTargetOnce();
        }
    }
    return HasAction;
}

Point ingameData::MonsterKnockBackOnce(MapMonster& monster, const TryWorldZone& zone, const Point& pos, const uint64 msec)
{
    const float WallDistanceCheck = mInGameSize * 5 / 1000; // 벽에 닿아버린 상황판단
    const float WallDistanceMin = mInGameSize * 20 / 1000; // 계산상 오류를 방지하는 거리
    const float TryNextPosX = pos.x + monster.knockBackAccel().x * mInGameW;
    const float TryNextPosY = pos.y + monster.knockBackAccel().y * mInGameH;
    const Point TryResultPos(TryNextPosX, TryNextPosY);
    Point ResultPos = TryResultPos, ReflectPos;
    bool IsBounce = false, IsHole = false;
    float DistanceMin = -1;

    if(GetValidNextObject(monster, pos, TryResultPos, ResultPos, ReflectPos) != -1)
    {
        IsBounce = true;
        DistanceMin = Math::Distance(pos.x, pos.y, ResultPos.x, ResultPos.y);
    }

    if(auto Polygon = TryWorld::GetPosition::GetValidNext(zone.mHurdle, pos, TryResultPos, ResultPos, ReflectPos, DistanceMin))
    {
        IsBounce = true;
        if(Polygon[0]->Payload != -1 && Polygon[0]->Payload == Polygon[1]->Payload)
        {
            IsHole = true;
            const sint32 LayerIndex = (Polygon[0]->Payload >> 16) & 0xFFFF;
            const sint32 ObjectIndex = Polygon[0]->Payload & 0xFFFF;
            auto& HoleObject = mLayers.At(LayerIndex).mObjects.At(ObjectIndex);
            HoleObject.Hit();
            if(0 < monster.mHPValue)
            {
                monster.mHPValue = 0;
                monster.mHPTimeMsec = msec;
            }
            if(monster.KnockBackEndByHole(HoleObject.mCurrentRect.Center()))
                mGameScoreLog += monster.mType->mInfinityPoint + CalcedPlusValue();

            // 홀실적추가
            const String CurHoleSkin = HoleObject.mType->spineSkinName();
            if(!!CurHoleSkin.Compare("normal"))
            {
                const bool IsHeart = !CurHoleSkin.Compare("heart");
                const sint32 CheckCount = (IsHeart)? mHeartHoleCreatCount : mHoleItemGetCount;
                HoleObject.AddExtraInfo(monster.mType->mID);
                if(CheckCount <= HoleObject.GetExtraInfoCount())
                {
                    // 아이템추가
                    if(IsHeart)
                    {
                        if(Platform::Utility::Random() % 1000 < mHeartHoleCreatRate) // 확율
                        {
                            auto& NewItem = mItemMap(MapItem::MakeId());
                            NewItem.Init(nullptr, GetSpine("item"), &HoleObject, updater(), -0.15f, 2000, 1000);
                        }
                    }
                    else for(sint32 k = 0, kend = mItemTypes.Count(); k < kend; ++k)
                    {
                        if(!mItemTypes[k].mSkinName.Compare(CurHoleSkin))
                        {
                            auto& NewItem = mItemMap(MapItem::MakeId());
                            NewItem.Init(&mItemTypes[k], GetSpine("item"), &HoleObject, updater(), -0.15f, 2000, 1000);
                            break;
                        }
                    }
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
            switch(monster.knockBackBoundCount())
            {
            case 0:
                mWallBound[mWallBoundFocus].PlayMotionAttached("wall_bound_one", "idle", true);
                BoundDamage = monster.mHPValueMax * m1BoundDamageRate;
                break;
            case 1:
                mWallBound[mWallBoundFocus].PlayMotionAttached("wall_bound_two", "idle", true);
                BoundDamage = monster.mHPValueMax * m2BoundDamageRate;
                break;
            case 2: default:
                mWallBound[mWallBoundFocus].PlayMotionAttached("wall_bound_three", "idle", true);
                BoundDamage = monster.mHPValueMax * m3BoundDamageRate;
                break;
            }
            mWallBoundPos[mWallBoundFocus] = ResultPos;
            mWallBoundFocus = (mWallBoundFocus + 1) % mWallBoundMax;
            monster.KnockBackBound(BoundDamage);

            // 진행방향을 결정하고
            monster.SetKnockBackAccel(Point((ReflectPos.x - ResultPos.x) / mInGameW, (ReflectPos.y - ResultPos.y) / mInGameH));
            // 벽에 너무 닿은 경우에는 살짝 뒤로 물러섬
            const float ReflectOX = (pos.x + ReflectPos.x) / 2;
            const float ReflectOY = (pos.y + ReflectPos.y) / 2;
            const float ReflectDistance = Math::Distance(ResultPos.x, ResultPos.y, ReflectOX, ReflectOY);
            if(ReflectDistance < WallDistanceCheck)
            {
                BOSS_ASSERT("게임의 시나리오가 잘못되었습니다", 0 < ReflectDistance);
                ResultPos.x += (ReflectOX - ResultPos.x) * WallDistanceMin / ReflectDistance;
                ResultPos.y += (ReflectOY - ResultPos.y) * WallDistanceMin / ReflectDistance;
            }
        }

        monster.SetKnockBackAccel(monster.knockBackAccel() * monster.knockBackResistance()); // 가속도감소
        const float NextKnockBackAccelSize = Math::Sqrt(Math::Pow(monster.knockBackAccel().x) + Math::Pow(monster.knockBackAccel().y));
        if(NextKnockBackAccelSize < monster.mKnockBackAccelMin) // 정신차리기
        {
            if(monster.KnockBackEnd())
                mGameScoreLog += monster.mType->mInfinityPoint + CalcedPlusValue();
        }
    }
    return Point(ResultPos.x / mInGameW - 0.5f, ResultPos.y / mInGameH - 0.5f);
}

Point ingameData::MonsterMoveOnce(MapMonster& monster, const TryWorldZone& zone, const Point& pos, const Point& vec, const uint64 msec)
{
    // 타겟설정
    if(monster.mTargets.Count() == 0)
        Targeting(monster, zone);

    // 서브타겟위치 업데이트
    Point TempPos;
    if(monster.mTargets.Count() == 0)
        TempPos = Point(mInGameW * (monster.mTargetPos.x + 0.5f), mInGameH * (monster.mTargetPos.y + 0.5f));
    else
    {
        if(TryWorld::GetPosition::SubTarget(zone.mHurdle, monster.mTargets[0].mPath, pos, TempPos))
            monster.mTargetPos = Point(TempPos.x / mInGameW - 0.5f, TempPos.y / mInGameH - 0.5f);
        else monster.ClearTargetOnce();
    }

    Point NextPos = monster.mCurrentPos;
    const float TargetDistance = Math::Distance(pos.x, pos.y, TempPos.x, TempPos.y);
    if(0 < TargetDistance)
    {
        const float MoveDistance = mMonsterSizeR * monster.CalcedSpeed(msec) * CalcedMoveSpeedRate();
        if(MoveDistance < TargetDistance)
        {
            // 폴리곤에 의해 멈추어야 할 경우를 판단
            const float TryNextPosX = pos.x + (TempPos.x - pos.x) * MoveDistance / TargetDistance + vec.x;
            const float TryNextPosY = pos.y + (TempPos.y - pos.y) * MoveDistance / TargetDistance + vec.y;
            const Point TryResultPos(TryNextPosX, TryNextPosY);
            Point ResultPos = TryResultPos, ReflectPos = TryResultPos;
            if(!TryWorld::GetPosition::GetValidNext(zone.mHurdle, pos, TryResultPos, ResultPos, ReflectPos))
            {
                // 닿은 오브젝트가 있다면 정지
                const sint32 ObjectIndex = GetContactObject(monster, TryResultPos);
                if(ObjectIndex != -1)
                {
                    monster.mBounceObjectIndex = ObjectIndex; // 닿은 오브젝트의 기록
                    ReflectPos.x = pos.x;
                    ReflectPos.y = pos.y;
                }
            }
            NextPos = Point(ReflectPos.x / mInGameW - 0.5f, ReflectPos.y / mInGameH - 0.5f);
        }
        else NextPos = monster.mTargetPos;

        // 방향전환된 꼭지점 기록
        const bool CurFlip = (monster.mCurrentPos.x < NextPos.x);
        if(monster.mLastFlip != CurFlip)
        {
            monster.mLastFlip = CurFlip;
            monster.mLastFlipPos = monster.mCurrentPos;
        }
        // 방향전환
        if(monster.mFlipMode != monster.mLastFlip)
        {
            const float FlipDist = Math::Distance(monster.mLastFlipPos.x, monster.mLastFlipPos.y, NextPos.x, NextPos.y);
            if(monster.mType->mTurnDistance < FlipDist * 1000)
            {
                monster.mFlipMode = monster.mLastFlip;
                monster.Turn();
            }
        }
    }
    return NextPos;
}

void ingameData::ReserveToSlotOnce()
{
    static uint64 LastTimeMsec = Platform::Utility::CurrentTimeMsec();
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
    if(CurTimeMsec < LastTimeMsec + 60) return;
    LastTimeMsec = (LastTimeMsec + 60 > CurTimeMsec - 20)?
        LastTimeMsec + 60 : CurTimeMsec - 20;

    for(sint32 i = 0; i < 4; ++i)
    {
        if(0 < mSlotStatus[i].mReserved)
        {
            if(mSlotStatus[i].mItemType)
            {
                mSlotStatus[i].mReserved--;
                auto& NewItem = mItemMap(MapItem::MakeId());
                NewItem.InitForSlot(mSlotStatus[i].mItemType, GetSpine("item"), updater(), Point(0, -1));
                // 즉시 슬롯으로 이동
                mSlotStatus[i].mCount++;
                NewItem.MoveToSlot(i, &mSlotStatus[i].mPos, 1000);
            }
            else BOSS_ASSERT("예약된 아이템을 생성할 수 없습니다", false);
        }
    }
}

void ingameData::ClearAllPathes(bool directly, chars polygon_name)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(polygon_name && CurMonster.mType->mPolygon.Compare(polygon_name))
            continue;
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
        OutlineRect = F1State::RenderMap((mShowDebug)? DebugMode::Strong : DebugMode::None, panel, &mMonsters, mWaveSecCurrently);
        OutlineRect += Point(mInGameX, mInGameY);
        // 이펙트
        for(sint32 i = 0; i < mWallBoundMax; ++i)
        {
            const sint32 Index = (mWallBoundFocus + i) % mWallBoundMax;
            ZAY_XYRR(panel, mWallBoundPos[Index].x, mWallBoundPos[Index].y, mWallBoundSizeR, mWallBoundSizeR)
                mWallBound[Index].RenderObject(DebugMode::None, true, panel, false);
        }
        // 공중아이템
        RenderItems(panel, false, CurTimeMsec);
    }

    if(!mShowDebug)
    {
        const sint32 OutlineL = Math::Max(0, OutlineRect.l);
        const sint32 OutlineT = Math::Max(0, OutlineRect.t);
        const sint32 OutlineR = Math::Min(OutlineRect.r, mScreenW);
        const sint32 OutlineB = Math::Min(OutlineRect.b, mScreenH);
        ZAY_LTRB(panel, OutlineL, OutlineT, OutlineR, OutlineB)
        {
            // 구름
            mWeather[1].RenderObject(DebugMode::None, true, panel, false);
            // 햇살
            if(const Rect* Area = mWeather[0].GetBoundRect("area"))
            {
                const float Rate = Math::MinF(panel.w() / Area->Width(), panel.h() / Area->Height());
                const float Width = Area->Width() * Rate;
                const float Height = Area->Height() * Rate;
                ZAY_XYWH(panel, 0, 0, Width, Height)
                    mWeather[0].RenderObject(DebugMode::None, true, panel, false);
            }
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
            mBreathAttackSpine.PlayMotion(String::Format("breath_%s_attack_a", mCurSkill.dragon_skin()), true);
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
                    mBreathAttackSpine.RenderObject(DebugMode::None, true, panel, mBreathAttack.mDragonFlip);
                mBreathEffectSpine.RenderObject(DebugMode::None, true, panel, mBreathAttack.mDragonFlip);
            }
        }
        // 드래곤
        if(mDragon.CalcExitRate(CurTimeMsec) < 1)
            ZAY_XYRR(panel, DragonPos.x, DragonPos.y, DragonSizeR, DragonSizeR)
                mDragon.RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, mDragon.flip());
    }

    // 게임전 UI
    if(mWave == -1 && !mShowDebug)
    {
        // 메인타이틀
        mMainTitleSpine.RenderObject(DebugMode::None, true, panel, false, "Title_",
            ZAY_GESTURE_NT(n, t, this)
            {
                if(t == GT_Pressed)
                {
                    if(!String::Compare(n, "Title_butten_start_area") || !String::Compare(n, "Title_str_30"))
                    {
                        if(!door().IsLocked())
                        {
                            bool GoStart = false;
                            if(FXSaver::Read("SumHeart").IsValid())
                            {
                                const sint32 SumHeart = FXSaver::Read("SumHeart").GetInt();
                                if(0 < SumHeart)
                                {
                                    FXSaver::Write("SumHeart").Set(String::FromInteger(SumHeart - 1));
                                    GoStart = true;
                                }
                            }
                            else GoStart = true;

                            if(GoStart)
                            {
                                mMainTitleSpine.PlayMotionOnce("start");
                                mMainTitleSpine.Staff_Start();
                            }
                        }
                    }
                    else if(!String::Compare(n, "Title_butten_start_area2") || !String::Compare(n, "Title_str_29"))
                    {
                        if(!door().IsLocked())
                        {
                            mMainTitleSpine.PlayMotionOnce("loby");
                            mClosing = 50;
                            Platform::Option::SetText("StartMode", "Lobby");
                        }
                    }
                    else if(!String::Compare(n, "Title_butten_facebook_area") || !String::Compare(n, "Title_str_11"))
                    {
                        mMainTitleSpine.PlayMotionOnce("facebook");
                        Platform::Popup::WebBrowserDialog("https://www.facebook.com/Monthlykoobonil/");
                    }
                    else if(!String::Compare(n, "Title_butten_staff_area2") || !String::Compare(n, "Title_str_10"))
                    {
                        mMainTitleSpine.PlayMotionOnce("staferoll");
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
                    mWaveHUD.RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false, nullptr, nullptr,
                        ZAY_RENDER_PN(p, n, this)
                        {
                            if(!String::Compare(n, "pont_wave_b_area"))
                            {
                                ZAY_INNER_SCISSOR(p, 0)
                                ZAY_COLOR_CLEAR(p)
                                {
                                    if(mGameMode == GameMode::Infinity && !mShowDebug)
                                        RenderNumbers(p, String::Format("%d", mGameSumWave), false);
                                    else
                                    {
                                        const sint32 WaveCount = mWaveData("Waves").LengthOfIndexable();
                                        RenderNumbers(p, String::Format("%d/%d", Math::Min(mWave + 1, WaveCount), WaveCount), false);
                                    }
                                }
                            }
                            else if(!String::Compare(n, "time_area"))
                            {
                                if(0 < mGameBeginMsec)
                                {
                                    const sint32 SumTime = ((0 < mGameStopMsec)? mGameStopMsec - mGameBeginMsec - mGameSumStopMsec
                                        : Platform::Utility::CurrentTimeMsec() - mGameBeginMsec - mGameSumStopMsec) / 100;
                                    ZAY_INNER_SCISSOR(p, 0)
                                    ZAY_COLOR_CLEAR(p)
                                    {
                                        if(SumTime < 600) RenderNumbers(p, String::Format("%d.%d", (SumTime / 10) % 60, SumTime % 10), true);
                                        else RenderNumbers(p, String::Format("%d:%02d.%d", SumTime / 600, (SumTime / 10) % 60, SumTime % 10), true);
                                    }
                                }
                            }
                            else if(!String::Compare(n, "score_area"))
                            {
                                ZAY_INNER_SCISSOR(p, 0)
                                ZAY_COLOR_CLEAR(p)
                                {
                                    mGameScore = (mGameScore * 9 + mGameScoreLog * 1) / 10;
                                    if(mGameScore < mGameScoreLog) mGameScore++;
                                    else if(mGameScore > mGameScoreLog) mGameScore--;
                                    RenderNumbers(p, String::Format("%d", mGameScore), true);
                                }
                            }
                        });
            }

            // 중지버튼
            if(const Rect* Area = mStopButton.GetBoundRect("area"))
            {
                const sint32 StopWidth = Area->Width() * panel.h() / Area->Height();
                ZAY_LTRB(panel, panel.w() - StopWidth, 0, panel.w(), panel.h())
                {
                    mStopButton.RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false, "Stop_",
                        ZAY_GESTURE_NT(n, t, this)
                        {
                            if(t == GT_Pressed)
                            {
                                if(!String::Compare(n, "Stop_stop_area"))
                                {
                                    mStopButton.PlayMotionOnce("click");
                                    mPausePopup.StopMotionAll();
                                    mPausePopup.PlayMotionAttached("show", "idle", true);
                                    if(FXSaver::Read("SoundFlag").GetInt())
                                        mPausePopup.PlayMotion("idle_sound", true);
                                    if(FXSaver::Read("BGMFlag").GetInt())
                                        mPausePopup.PlayMotion("idle_bgm", true);
                                    if(!FXSaver::Read("SumHeart").IsValid() || 0 < FXSaver::Read("SumHeart").GetInt())
                                        mPausePopup.PlayMotion("idle_replay", true);
                                    mPaused = true;
                                    mGameStopMsec = Platform::Utility::CurrentTimeMsec();
                                }
                            }
                        });
                }
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
                mGaugeHUD.RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false);
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
                const Rect GaugeRect(Point(panel.w() - SlotWidth, panel.h() - (GaugeHeight + SlotHeight) / 2), Size(SlotWidth, SlotHeight));
                // 슬롯위치 저장
                for(sint32 i = 0; i < 4; ++i)
                {
                    const float L = (ButtonArea[i]->l - Area->l) / (Area->r - Area->l) * (GaugeRect.r - GaugeRect.l) + GaugeRect.l;
                    const float T = (ButtonArea[i]->t - Area->t) / (Area->b - Area->t) * (GaugeRect.b - GaugeRect.t) + GaugeRect.t;
                    const float R = (ButtonArea[i]->r - Area->l) / (Area->r - Area->l) * (GaugeRect.r - GaugeRect.l) + GaugeRect.l;
                    const float B = (ButtonArea[i]->b - Area->t) / (Area->b - Area->t) * (GaugeRect.b - GaugeRect.t) + GaugeRect.t;
                    mSlotStatus[i].mPos.x = ((L + R) / 2 - mInGameX) / mInGameW - 0.5f;
                    mSlotStatus[i].mPos.y = ((T + B) / 2 - mInGameY) / mInGameH - 0.5f;
                }
                ZAY_RECT(panel, GaugeRect)
                    mSlotHUD.RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false);
            }
        }

        // 슬롯아이템
        ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
            RenderItems(panel, true, CurTimeMsec);

        // 캠페인
        ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
            mCampaign.RenderObject(DebugMode::None, true, panel, false);

        // 중지팝업
        if(mPaused)
        {
            ZAY_INNER_UI(panel, 0, "PausePopup")
            ZAY_RGBA(panel, 0, 0, 0, 128)
                panel.fill();
            mPausePopup.RenderObject(DebugMode::None, true, panel, false, "Pause_",
                ZAY_GESTURE_NT(n, t, this)
                {
                    if(t == GT_Pressed)
                    {
                        if(!String::Compare(n, "Pause_play_area"))
                            mPausePopup.PlayMotionOnce("click_play");
                        else if(!String::Compare(n, "Pause_loby_area"))
                            mPausePopup.PlayMotionOnce("click_lobby");
                        else if(!String::Compare(n, "Pause_replay_area"))
                        {
                            bool GoStart = false;
                            if(FXSaver::Read("SumHeart").IsValid())
                            {
                                const sint32 SumHeart = FXSaver::Read("SumHeart").GetInt();
                                if(0 < SumHeart)
                                {
                                    FXSaver::Write("SumHeart").Set(String::FromInteger(SumHeart - 1));
                                    GoStart = true;
                                }
                            }
                            else GoStart = true;

                            if(GoStart)
                            {
                                mPausePopup.PlayMotionOnce("click_replay");
                                mClosing = 50;
                                mClosingOption = 1;
                            }
                        }
                        else if(!String::Compare(n, "Pause_pause_sound_mid_area"))
                        {
                            if(FXSaver::Read("SoundFlag").GetInt())
                            {
                                FXSaver::Write("SoundFlag").Set("0");
                                mPausePopup.StopMotion("idle_sound");
                                mPausePopup.StopMotion("show_sound");
                                mPausePopup.PlayMotionOnce("hide_sound");
                                Platform::Sound::Stop(GetSound(mBGMusic, true));
                            }
                            else
                            {
                                FXSaver::Write("SoundFlag").Set("1");
                                mPausePopup.StopMotion("hide_sound");
                                mPausePopup.PlayMotionAttached("show_sound", "idle_sound", true);
                                if(FXSaver::Read("BGMFlag").GetInt())
                                    Platform::Sound::Play(GetSound(mBGMusic, true));
                            }
                        }
                        else if(!String::Compare(n, "Pause_pause_bgm_mid_area"))
                        {
                            if(FXSaver::Read("BGMFlag").GetInt())
                            {
                                FXSaver::Write("BGMFlag").Set("0");
                                mPausePopup.StopMotion("idle_bgm");
                                mPausePopup.StopMotion("show_bgm");
                                mPausePopup.PlayMotionOnce("hide_bgm");
                                Platform::Sound::Stop(GetSound(mBGMusic, true));
                            }
                            else
                            {
                                FXSaver::Write("BGMFlag").Set("1");
                                mPausePopup.StopMotion("hide_bgm");
                                mPausePopup.PlayMotionAttached("show_bgm", "idle_bgm", true);
                                Platform::Sound::Play(GetSound(mBGMusic, true));
                            }
                        }
                    }
                }, mSubRenderer);
        }
    }

    // 인게임 클로징
    if(0 <= mClosing && mClosing < 50)
        ZAY_RGBA(panel, 0, 0, 0, 255 * (50 - mClosing) / 50)
            panel.fill();

    // 인게임 디버그정보
    if(mShowDebug)
    {
        ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
            F1State::RenderDebug(panel, mMonsters, mWaveSecCurrently);
        // 스크립트 로그
        ZAY_LTRB(panel, 0, mInGameY, panel.w(), panel.h())
        ZAY_RGB(panel, 255, 255, 255)
        for(sint32 i = 0, iend = mDebugScriptLogs.Count(); i < iend; ++i)
            panel.text(10, 10 + i * 20, mDebugScriptLogs[i], UIFA_LeftTop);
    }

    if(FXSaver::Read("DevMode").GetInt())
    {
        const sint32 InnerGap = 10, ButtonSize = 80, ButtonSizeSmall = 50;
        ZAY_FONT(panel, 1.2, "Arial Black")
        {
            // 홈버튼
            ZAY_XYWH(panel, panel.w() - ButtonSize * 3, 0, ButtonSize, ButtonSizeSmall)
            ZAY_INNER_UI(panel, InnerGap, "home",
                ZAY_GESTURE_T(t, this)
                {
                    if(t == GT_InReleased)
                        next("f1View");
                })
            {
                ZAY_RGBA(panel, 255, 255, 128, 192)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.rect(2);
                    panel.text("Home", UIFA_CenterMiddle);
                }
            }

            // 디버그버튼
            ZAY_XYWH(panel, panel.w() - ButtonSize * 2, 0, ButtonSize, ButtonSizeSmall)
            ZAY_INNER_UI(panel, InnerGap, "debug",
                ZAY_GESTURE_T(t, this)
                {
                    if(t == GT_InReleased)
                    {
                        mShowDebug = !mShowDebug;
                    }
                })
            {
                ZAY_RGBA_IF(panel, 255, 128, 255, 192, mShowDebug)
                ZAY_RGBA_IF(panel, 128, 255, 255, 192, !mShowDebug)
                    panel.fill();
                ZAY_RGB(panel, 0, 0, 0)
                {
                    panel.rect(2);
                    panel.text("Debug", UIFA_CenterMiddle);
                }
            }
        }
    }
}

void ingameData::RenderNumbers(ZayPanel& panel, chars numbers, bool rside)
{
    static sint32s NumberIDs; // 0~9, 10('.'), 11(','), 12(':'), 13('/'), 14(' ')
    static sint32s Widths;
    NumberIDs.SubtractionAll();
    Widths.SubtractionAll();

    // 수집
    sint32 SumWidth = 0;
    for(; *numbers; ++numbers)
    {
        const Image* CurImage = nullptr;
        if('0' <= *numbers && *numbers <= '9')
        {
            NumberIDs.AtAdding() = *numbers - '0';
            CurImage = &((const Image&) R(String::Format("s_%c", 'a' + (*numbers - '0'))));
        }
        else if(*numbers == '.') {NumberIDs.AtAdding() = 10; CurImage = &((const Image&) R("s_dot"));}
        else if(*numbers == ',') {NumberIDs.AtAdding() = 11; CurImage = &((const Image&) R("s_comma"));}
        else if(*numbers == ':') {NumberIDs.AtAdding() = 12; CurImage = &((const Image&) R("s_colon"));}
        else if(*numbers == '/') {NumberIDs.AtAdding() = 13; CurImage = &((const Image&) R("s_slash"));}
        else if(*numbers == ' ') {NumberIDs.AtAdding() = 14; CurImage = &((const Image&) R("s_dot"));} // dot크기가 공백크기
        else
        {
            NumberIDs.AtAdding() = -1;
            Widths.AtAdding() = 0;
            continue;
        }
        const sint32 CurWidth = CurImage->GetWidth() * panel.h() / CurImage->GetHeight();
        Widths.AtAdding() = CurWidth;
        SumWidth += CurWidth;
    }

    // 출력
    sint32 XPos = (rside)? panel.w() - SumWidth : (panel.w() - SumWidth) / 2;
    for(sint32 i = 0, iend = NumberIDs.Count(); i < iend; ++i)
    {
        ZAY_XYWH(panel, XPos, 0, Widths[i], panel.h())
        {
            const sint32 CurID = NumberIDs[i];
            if(0 <= CurID && CurID <= 9)
                panel.stretch(R(String::Format("s_%c", 'a' + CurID)), true);
            else if(CurID == 10) panel.stretch(R("s_dot"), true);
            else if(CurID == 11) panel.stretch(R("s_comma"), true);
            else if(CurID == 12) panel.stretch(R("s_colon"), true);
            else if(CurID == 13) panel.stretch(R("s_slash"), true);
        }
        XPos += Widths[i];
    }
}

void ingameData::RenderItems(ZayPanel& panel, bool slot, uint64 msec)
{
    sint32 FlyingCount[4] = {0, 0, 0, 0};
    bool SlotArrived[4] = {false, false, false, false};
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
        {
            const bool Arrived = (0.99f < SlotRate);
            if(!slot || !Arrived || !SlotArrived[CurItem->slot_index()]) // 슬롯에 도착한 아이템들 겹치는 현상 방지
            {
                CurItem->RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false, String::Format("Item%d_", i),
                    ZAY_GESTURE_NT(n, t, this, i)
                    {
                        if(t == GT_Pressed && mClosing == -1 && !String(n).Right(15).Compare("ston_touch_area"))
                        {
                            auto CurItem = mItemMap.AccessByOrder(i);
                            if(!CurItem) return; // 이미 지워진 경우
                            if(!CurItem->type()) // 하트처리
                            {
                                CurItem->MoveToOut(Point(0, -1), 1000);
                                if(FXSaver::Read("SumHeart").IsValid())
                                {
                                    const sint32 SumHeart = FXSaver::Read("SumHeart").GetInt();
                                    FXSaver::Write("SumHeart").Set(String::FromInteger(SumHeart + 1));
                                }
                                else BOSS_ASSERT("잘못된 시나리오입니다", false);
                            }
                            else if(!CurItem->slot())
                                ItemToSlot(*CurItem);
                            else if(CurItem->UseOnce())
                            {
                                const sint32 SlotIndex = CurItem->slot_index();
                                if(0 < mSlotStatus[SlotIndex].mCount && --mSlotStatus[SlotIndex].mCount == 0 && mSlotStatus[SlotIndex].mReserved == 0)
                                    mSlotStatus[SlotIndex].mItemType = nullptr;
                                // 스킬적용
                                mCurSkill.Reset(CurItem);
                                // 스킨적용
                                mGaugeHUD.SetSkin(mCurSkill.skin());
                                mGaugeHUD.PlayMotionOnce("change");
                                // 브레스게이지 증가
                                mBreathGaugeTime = Math::Min(mBreathGaugeTime + mBreathMaxGauge * CurItem->type()->mAddGauge / 1000, mBreathMaxGauge);
                            }
                        }
                    });
            }
            if(slot)
            {
                if(Arrived)
                    SlotArrived[CurItem->slot_index()] = true;
                else FlyingCount[CurItem->slot_index()]++; // 현재 비행중인 수량은 카운트에서 뺌
            }
        }
    }
    // 슬롯카운트
    if(slot)
    for(sint32 i = 0; i < 4; ++i)
    {
        const sint32 CurCount = Math::Min(mSlotStatus[i].mCount - FlyingCount[i], 99);
        if(1 < CurCount)
        {
            const float X = mInGameW * (mSlotStatus[i].mPos.x + 0.5f) + mSlotSizeR * 0.5;
            const float Y = mInGameH * (mSlotStatus[i].mPos.y + 0.5f) + mSlotSizeR * 0.4;
            const float RSize = mSlotSizeR * 0.3;
            ZAY_XYRR(panel, X, Y, RSize, RSize)
            {
                RenderImage(DebugMode::None, panel, R(String::Format("s_%c", 'a' + (CurCount % 10))));
                if(9 < CurCount)
                ZAY_XYWH(panel, -mSlotSizeR * 0.5, 0, panel.w(), panel.h())
                    RenderImage(DebugMode::None, panel, R(String::Format("s_%c", 'a' + ((CurCount / 10) % 10))));
            }
        }
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
            mBreathReadySpine[i].RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false);
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
            // 브레스영역
            auto& CurBreathReadySpine = mBreathReadySpine[mBreathReadyCount];
            CurBreathReadySpine.SetSeekSec(mBreathGaugeTimeUsingCurrently * 0.001f);
            CurBreathReadySpine.RenderObject((mShowDebug)? DebugMode::Weak : DebugMode::None, true, panel, false);
            // 브레스바
            mBreathBarSpine.RenderObject(DebugMode::None, true, panel, false);

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
    if(0 <= ++mWave)
    if(0 == mGameSumWave++)
        mGameBeginMsec = Platform::Utility::CurrentTimeMsec();
    mWaveTitle = "Load Failure";
    mWaveSecCurrently = 0;
    mWaveSecSettled = -1;
    mWaveSecMax = 0;

    // 생존몬스터 보전
    for(sint32 i = mMonsters.Count() - 1; 0 <= i; --i)
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
                            CurMonster.Init(&CurMonsterType, CurJsonMonsterRID, TimeSec, CalcedHPRate(),
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
                                            const Point NewMonsterPos(mInGameW * (NewTarget.mPos.x + 0.5f), mInGameH * (NewTarget.mPos.y + 0.5f));
                                            NewTarget.mPath = CurTryWorldZone->mMap->BuildPath(OldMonsterPos, NewMonsterPos, PATHFIND_STEP);
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
            mMonsters.At(i).ResetCB(this);
    }
    else mWaveTitle = "Stage Over";
}

void ingameData::SetDragonSchedule(const MapBreath* breath, bool retry)
{
    // 드래곤에 스킬적용
    mCurSkill.CopyToDragon();
    // 드래곤 일정기록
    mDragon.SetSkin(mCurSkill.dragon_skin());
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
        if(CurMonster.mType->mType != monster_type::Enemy) continue;
        if(mWaveSecCurrently < CurMonster.mEntranceSec || (!CurMonster.mImmortal && CurMonster.mHPValue == 0))
            continue;

        float x = mInGameX + mInGameW * (CurMonster.mCurrentPos.x + 0.5f);
        float y = mInGameY + mInGameH * (CurMonster.mCurrentPos.y + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mMonsterSizeR + breath->mSizeR)
        {
            if(!CurMonster.mImmortal)
            {
                CurMonster.mHPValue = Math::Max(0, CurMonster.mHPValue - mCurSkill.CalcedDamage(breath->mDamage));
                CurMonster.mHPTimeMsec = Platform::Utility::CurrentTimeMsec() + mHPbarDeleteTime;
            }

            // 넉백처리
            sint32 Distance = 0;
            while((Distance = Math::Distance(x, y, breath->mPos.x, breath->mPos.y)) < CurMonster.mKnockBackNearMin) // 극인접처리
            {
                x = breath->mPos.x + (Platform::Utility::Random() % 1000) - 500;
                y = breath->mPos.y + (Platform::Utility::Random() % 1000) - 500;
            }
            const float PowerRate = Math::ClampF((breath->mDamage - mBreathMinDamage) / (float) (mBreathMaxDamage - mBreathMinDamage), 0, 1);
            const float AccelRate = ((mKnockBackMaxV - mKnockBackMinV) * PowerRate * mCurSkill.CalcedKnockBackUp() + mKnockBackMinV) / Distance;
            const Point NewAccel = Point((x - breath->mPos.x) * AccelRate, (y - breath->mPos.y) * AccelRate) * 1000 / CurMonster.mType->mWeight;

            // 애니처리
            CurMonster.mFlipMode = (x < breath->mPos.x) ^ (y > breath->mPos.y);
            CurMonster.mCurrentVec = Point(0, 0);
            CurMonster.KnockBack(y > breath->mPos.y, Point(NewAccel.x / mInGameW, NewAccel.y / mInGameH),
                mCurSkill.CalcedKnockBackUp(), mCurSkill.CalcedSkillSkin(), mCurSkill.CalcedSkillSpeed(), mCurSkill.CalcedSkillTime(),
                mCurSkill.dragon_skin());
            CurMonster.ClearAllTargets();
        }
    }

    // 오브젝트피해
    auto& CurObjects = mLayers.At(2).mObjects;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        auto& CurObject = CurObjects.At(i);
        if(!CurObject.mType->mType.isDynamic() || CurObject.mHPValue == 0)
            continue;
        const float x = mInGameX + mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameY + mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        const float r = mInGameSize * CurObject.mCurrentRect.Width();
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < r + breath->mSizeR)
        {
            if(CurObject.SetHP(CurObject.mHPValue - mCurSkill.CalcedDamage(breath->mDamage), mHPbarDeleteTime))
            {
                if(mShowDebug)
                    CurObject.mVisible = false;
                SetBrokenObject(CurObject);
            }
        }
    }

    // 타겟피해(유저의 조작실수)
    for(sint32 i = 0, iend = mTargetsForEnemy.Count(); i < iend; ++i)
    {
        auto& CurObject = mLayers.At(2).mObjects.At(mTargetsForEnemy[i].mObjectIndex);
        if(CurObject.mHPValue == 0) continue;
        const float x = mInGameX + mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameY + mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mInGameSize * mTargetsForEnemy[i].mSizeR + breath->mSizeR)
        {
            if(CurObject.SetHP(CurObject.mHPValue - mCurSkill.CalcedDamage(breath->mDamage), mHPbarDeleteTime))
            {
                if(mShowDebug)
                    CurObject.mVisible = false;
                ClearAllPathes(false);
            }
        }
    }
}

void ingameData::SetBrokenObject(const MapObject& object)
{
    for(sint32 i = 0, iend = mAllTryWorldZones.Count(); i < iend; ++i)
    {
        chararray PolygonName;
        if(auto CurTryWorldZone = mAllTryWorldZones.AccessByOrder(i, &PolygonName))
        if(auto CurChainID = object.mDynamicChainID.Access(&PolygonName[0]))
        {
            if(*CurChainID == -1)
            {
                BuildTryWorld(false, &PolygonName[0]);
                ClearAllPathes(false, &PolygonName[0]);
            }
            else
            {
                RebuildTryWorld(*CurChainID, &PolygonName[0]);
                ClearAllPathes(true, &PolygonName[0]);
            }
        }
    }
}
