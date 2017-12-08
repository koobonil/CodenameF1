#include <boss.hpp>
#include "ingame.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("ingameView", ingameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        if(m->mIsSpineInited)
        {
            // 시간진행
            const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
            static uint64 OldTimeSec = CurTimeMsec / 1000;
            const uint64 CurTimeSec = CurTimeMsec / 1000;
            sint32 CurTimeSecSpan = (sint32) (CurTimeSec - OldTimeSec);
            if(0 < CurTimeSecSpan)
            {
                m->mWaveSec += CurTimeSecSpan;
                OldTimeSec = CurTimeSec;
            }
            // 애니메이션 진행
            if(m->mWave == -1)
            {
                float StaffProgress = (CurTimeMsec - m->mMainTitleStaffTime) / 1200.0f;
                // 스태프 애니
                if(1 < StaffProgress)
                    for(sint32 i = 0; i < 3; ++i)
                        m->mMonsters.At(i).Staff_TryIdle();
                // 스태프 위치
                StaffProgress = Math::MinF(1.0f, StaffProgress);
                for(sint32 i = 0; i < 3; ++i)
                {
                    m->mMonsters.At(i).mCurrentPos = Point(
                        m->mMainTitleStaffBegin[i].x * (1 - StaffProgress) + m->mMainTitleStaffTarget[i].x * StaffProgress,
                        m->mMainTitleStaffBegin[i].y * (1 - StaffProgress) + m->mMainTitleStaffTarget[i].y * StaffProgress);
                    m->mMonsters.At(i).mTargetPos = m->mMonsters[i].mCurrentPos;
                }
            }
            else m->AnimationOnce(CurTimeSecSpan);
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
        if(!m->mIsSpineInited)
        {
            m->mIsSpineInited = true;
            m->InitForSpine();
        }
        m->RebuildTryWorld();
        m->ClearPath();
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

    if(m->mWave == -1)
        IsPressEnabled = true;
    else
    {
        const sint32 ClampX = Math::Clamp(x, m->mInGameX, m->mInGameX + m->mInGameW);
        const sint32 ClampY = Math::Clamp(y, m->mInGameY, m->mInGameY + m->mInGameH);
        if(type == GT_Pressed)
        {
            // 동맹군 터치여부
            for(sint32 i = 0, iend = m->mMonsters.Count(); i < iend; ++i)
            {
                auto& CurMonster = m->mMonsters.At(i);
                if(CurMonster.mType->mType != MonsterType::TypeClass::Ally) continue;
                if(m->mWaveSec < CurMonster.mEntranceSec || 0 < CurMonster.mDeathStep) continue;

                float AllyX = m->mInGameX + m->mInGameW * (CurMonster.mCurrentPos.x + 0.5f);
                float AllyY = m->mInGameY + m->mInGameH * (CurMonster.mCurrentPos.y + 0.5f);
                if(Math::Distance(AllyX, AllyY, x, y) < m->mMonsterSizeR * 2)
                {
                    CurMonster.Ally_Touched();
                    IsPressEnabled = false;
                    return;
                }
            }

            // 브레스준비
            if(!m->mBreath.mEnable)
            {
                m->mBreathing = true;
                m->mBreathPos = Point(ClampX, ClampY);
                m->mBreathMsec = Platform::Utility::CurrentTimeMsec();
                m->mBreathGaugeTimeUsingCurrently = 0;
                m->mBreathSizeRCurrently = m->mBreathSizeR;
                m->mBreathPowerPermil = 0;
            }
            IsPressEnabled = true;
        }
        else if(IsPressEnabled)
        {
            if(type == GT_InDragging || type == GT_OutDragging)
            {
                if(m->mBreathing)
                    m->mBreathPos = Point(ClampX, ClampY);
                else if(!m->mBreath.mEnable)
                {
                    m->mBreathing = true;
                    m->mBreathPos = Point(ClampX, ClampY);
                    m->mBreathMsec = Platform::Utility::CurrentTimeMsec();
                    m->mBreathGaugeTimeUsingCurrently = 0;
                    m->mBreathSizeRCurrently = m->mBreathSizeR;
                    m->mBreathPowerPermil = 0;
                }
            }
            else if(type == GT_InReleased)
            {
                if(m->mBreathing)
                {
                    m->mBreathing = false;
                    if(0 < m->mBreathGaugeTime && 0 < m->mBreathGaugeTimeUsingCurrently)
                    {
                        m->mBreath.mEnable = true;
                        m->mBreath.mAniTimeMsec = Platform::Utility::CurrentTimeMsec() + m->mDragonEntryTime;
                        m->mBreath.mEndTimeMsec = m->mBreath.mAniTimeMsec + m->mDragonBreathTime;
                        m->mBreath.mPos = m->mBreathPos;
                        m->mBreath.mSizeR = m->mBreathSizeRCurrently;
                        m->mBreath.mGaugeTime = m->mBreathGaugeTimeUsingCurrently;
                        m->mBreath.mDamage = m->GetCalcedBreathDamage();
                        m->mBreathGaugeTime = Math::Max(0, m->mBreathGaugeTime - m->mBreath.mGaugeTime);
                        // 스킨적용
                        m->mCurItemSkinForDragon = m->mCurItemSkin;
                        // 드래곤 일정기록
                        m->mDragon.SetSkin(m->mCurItemSkinForDragon);
                        m->mDragon.GoTarget(m->mBreathPos, m->mInGameX + m->mInGameW / 2 < m->mBreathPos.x,
                            m->mDragonEntryTime, m->mDragonBreathTime, m->mDragonExitTime);
                        // 브레스 애니
                        m->mBreathAttackSpine.StopMotionAll();
                        m->mBreathAttackSpine.PlayMotion(String::Format("breath_%s_attack_a", (chars) m->mCurItemSkinForDragon), true);
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
    mIsSpineInited = false;
    mShowDebug = false;
    mWaveData = Context(ST_Json, SO_NeedCopy, String::FromFile(String::Format("table/%s.json", (chars) mStage)));
    mWave = -1;
    mWaveTitle = "";
    mWaveSec = 0;
    mSlotFlag[0] = mSlotFlag[1] = mSlotFlag[2] = mSlotFlag[3] = false;
    mCurItemSkin = "normal";
    mCurItemSkinForDragon = "normal";
    mCurItemSkinEndTimeMsec = 0;
    mBreathing = false;
    mBreathPos = Point(0, 0);
    mBreathMsec = 0;
    mBreathGaugeTimeUsingCurrently = 0;
    mBreathSizeRCurrently = 0;
    mBreathPowerPermil = 0;

    // 타이틀화면
    mMainTitleStaffTime = Platform::Utility::CurrentTimeMsec();
    mMainTitleStaffBegin[0] = Point(-0.3f, 0.0f);
    mMainTitleStaffTarget[0] = Point(-0.2f, 0.0f);
    mMainTitleStaffBegin[1] = Point(0.1f, 0.3f);
    mMainTitleStaffTarget[1] = Point(0.0f, 0.2f);
    mMainTitleStaffBegin[2] = Point(0.3f, 0.0f);
    mMainTitleStaffTarget[2] = Point(0.2f, 0.0f);
    if(mLandscape)
    for(sint32 i = 0; i < 3; ++i)
    {
        mMainTitleStaffBegin[i] = Point(mMainTitleStaffBegin[i].y, -mMainTitleStaffBegin[i].x);
        mMainTitleStaffTarget[i] = Point(mMainTitleStaffTarget[i].y, -mMainTitleStaffTarget[i].x);
    }

    // 브레스게이지
    mBreathGaugeTime = mBreathMaxGauge;
    mBreathGaugeTimeLog = mBreathMaxGauge;
}

ingameData::~ingameData()
{
}

sint32 ingameData::GetValidNextForDynamic(const MapMonster& monster, const Point& curPos, Point& nextPos, Point* reflectPos)
{
    Point ResultPos = nextPos;
    float ResultDistance = -1;
    sint32 ResultIndex = -1;
    auto& CurObjects = mLayers[2].mObjects;
    const float MonsterWaistSizeRW = mMonsterSizeR * monster.mType->mWaistScaleWidth / 1000;
    const float MonsterWaistSizeRH = mMonsterSizeR * monster.mType->mWaistScaleHeight / 1000;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        if(!CurObjects[i].mVisible || CurObjects[i].mType->mType != ObjectType::TypeClass::Dynamic)
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
                if(ResultDistance < 0 || CurDistance < ResultDistance)
                {
                    ResultPos = Point(CrossPos->x, CrossPos->y);
                    ResultDistance = CurDistance;
                    ResultIndex = i;
                    if(reflectPos)
                    {
                        const float LineDx = LineEnd.x - LineBegin.x;
                        const float LineDy = LineEnd.y - LineBegin.y;
                        const float TValue = ((curPos.x - LineBegin.x) * LineDx + (curPos.y - LineBegin.y) * LineDy)
                            / (LineDx * LineDx + LineDy * LineDy);
                        const float NearX = LineBegin.x + TValue * LineDx;
                        const float NearY = LineBegin.y + TValue * LineDy;
                        reflectPos->x = curPos.x + (ResultPos.x - NearX) * 2;
                        reflectPos->y = curPos.y + (ResultPos.y - NearY) * 2;
                    }
                }
            }
		}
    }
    nextPos = ResultPos;
    return ResultIndex;
}

void ingameData::InitForSpine()
{
    // 맵로드
    const String MapName = mWaveData("MapName").GetString();
    id_asset_read TextAsset = Asset::OpenForRead("table/" + MapName + ".json");
    if(TextAsset)
    {
        const sint32 TextSize = Asset::Size(TextAsset);
        buffer TextBuffer = Buffer::Alloc(BOSS_DBG TextSize + 1);
        Asset::Read(TextAsset, (uint08*) TextBuffer, TextSize);
        Asset::Close(TextAsset);
        ((char*) TextBuffer)[TextSize] = '\0';
        LoadMap((chars) TextBuffer);
        Buffer::Free(TextBuffer);
    }

    mDragon.Init(mAllSpines("dragon"), mDragonScaleMax / mDragonScale, updater(),
        mDragonHome, mDragonExitL, mDragonExitR);
    mBreathReadySpine.InitSpine(&mAllSpines("breath_ready")).PlayMotionSeek("meteo_ready", false);
    mBreathAttackSpine.InitSpine(&mAllSpines("breath_attack")).PlayMotion("idle", true);
    mBreathEffectSpine.InitSpine(&mAllSpines("breath_effect")).PlayMotion("idle", true);

    // 타이틀화면
    mMainTitleSpine.InitSpine(&mAllSpines("ui_main_title"), "default",
        [this](chars motionname)
        {
            if(!String::Compare("start", motionname))
                ReadyForNextWave();
        }).PlayMotionAttached("loding", "idle", true);

    // 스태프 캐릭터
    auto PtrStaff = mMonsters.AtDumpingAdded(3);
    static const MonsterType StaffType;
    PtrStaff[0].Init(&StaffType, 1, 0, mMainTitleStaffBegin[0].x, mMainTitleStaffBegin[0].y,
        mAllSpines("ui_main_title_jjs"), nullptr);
    PtrStaff[1].Init(&StaffType, 2, 0, mMainTitleStaffBegin[1].x, mMainTitleStaffBegin[1].y,
        mAllSpines("ui_main_title_kbh"), nullptr);
    PtrStaff[2].Init(&StaffType, 3, 0, mMainTitleStaffBegin[2].x, mMainTitleStaffBegin[2].y,
        mAllSpines("ui_main_title_kbi"), nullptr);

    // UI
    mWeather[0].InitSpine(&mAllSpines("a_weather_toast")).PlayMotion("forest_weather", true);
    mWeather[1].InitSpine(&mAllSpines("b_weather_toast")).PlayMotion("forest_weather", true);
    mGaugeHUD.InitSpine(&mAllSpines("ui_ingame_gauge"), "normal").PlayMotionSeek("charge", false);
    mGaugeHUD.PlayMotionOnce("loading");
    mSlotHUD.InitSpine(&mAllSpines("ui_ingame_slot")).PlayMotion("loading", false);
    mWaveHUD.InitSpine(&mAllSpines("ui_ingame_wave_pont")).PlayMotion("loding", false);
    mStopButton.InitSpine(&mAllSpines("ui_ingame_stop_butten")).PlayMotion("loding", false);
}

void ingameData::AnimationOnce(sint32 timespan)
{
    // 스킨풀림
    if(mCurItemSkinEndTimeMsec != 0 && mCurItemSkinEndTimeMsec < Platform::Utility::CurrentTimeMsec())
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
        if(mWaveSec < CurMonster.mEntranceSec || 0 < CurMonster.mDeathStep) // 등장전 또는 죽을때
        {
            if(CurMonster.mDeathStep == 1)
                CurMonster.TryDeathMove();
            continue;
        }
        const Point CurMonsterPos = Point(mInGameW * (CurMonster.mCurrentPos.x + 0.5f), mInGameH * (CurMonster.mCurrentPos.y + 0.5f));

        // 공격
        bool HasAttack = false;
        if(!CurMonster.IsKnockBackMode())
        {
            // 오브젝트공격
            if(CurMonster.mType->mType == MonsterType::TypeClass::Enemy)
            if(CurMonster.mBounceObjectIndex != -1)
            {
                auto& CurObject = mLayers.At(2).mObjects.At(CurMonster.mBounceObjectIndex);
                if(0 < CurObject.mHPValue)
                {
                    const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
                    const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
                    const float r = mInGameSize * CurObject.mCurrentRect.Width() * 2; // 넉넉하게
                    if(Math::Distance(CurMonsterPos.x, CurMonsterPos.y, x, y) < r)
                    {
                        HasAttack = true;
                        if(auto AttackCount = CurMonster.TryAttack())
                            CurObject.SetHP(CurObject.mHPValue - CurMonster.mType->mAttackPower * AttackCount, mHPbarDeleteTime);
                    }
                    else CurMonster.mBounceObjectIndex = -1;
                }
                else
                {
                    CurMonster.CancelAttack();
                    CurMonster.mBounceObjectIndex = -1;
                }
            }

            // 타겟공격 또는 도착
            if(!HasAttack)
            if(CurMonster.mTargetIndex != -1)
            {
                auto& CurTarget = (CurMonster.mType->mType == MonsterType::TypeClass::Ally)?
                    mTargetsForAlly[CurMonster.mTargetIndex] : mTargetsForEnemy[CurMonster.mTargetIndex];
                auto& CurObject = mLayers.At(CurTarget.mLayerIndex).mObjects.At(CurTarget.mObjectIndex);
                if(0 < CurObject.mHPValue)
                {
                    const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
                    const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
                    const float r = mInGameSize * CurTarget.mSizeR;
                    if(Math::Distance(CurMonsterPos.x, CurMonsterPos.y, x, y) < r)
                    {
                        HasAttack = true;
                        if(CurMonster.mType->mType == MonsterType::TypeClass::Ally) // 동맹군도착
                            CurMonster.Ally_Arrived();
                        else if(auto AttackCount = CurMonster.TryAttack()) // 타겟공격
                            CurObject.SetHP(CurObject.mHPValue - CurMonster.mType->mAttackPower * AttackCount, mHPbarDeleteTime);
                    }
                }
                else
                {
                    CurMonster.CancelAttack();
                    CurMonster.ClearTarget();
                }
            }
        }

        // 이동
        if(!HasAttack)
        if(auto* CurTryWorldZone = mAllTryWorldZones.Access(CurMonster.mType->mPolygon))
        {
            // 다음위치 계산
            Point NextPos = CurMonster.mCurrentPos;
            const float WallDistanceMin = 2; // 벽에 닿아버려 생길 계산상 오류를 방지하는 거리
            if(CurMonster.IsKnockBackMode()) // 넉백중
            {
                // 오브젝트와 폴리곤에 의해 반동되어야 할 경우를 판단
                const float TryNextPosX = CurMonsterPos.x + CurMonster.knockbackaccel().x * mInGameW;
                const float TryNextPosY = CurMonsterPos.y + CurMonster.knockbackaccel().y * mInGameH;
                Point ResultPos(TryNextPosX, TryNextPosY);
                Point ReflectPos;
                bool IsBounce = false, IsHole = false;
                float DistanceMin = -1;
                if(GetValidNextForDynamic(CurMonster, CurMonsterPos, ResultPos, &ReflectPos) != -1)
                {
                    IsBounce = true;
                    DistanceMin = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, ResultPos.x, ResultPos.y);
                }
                if(auto Polygon = TryWorld::GetPosition::GetValidNext(CurTryWorldZone->mHurdle, CurMonsterPos, ResultPos, DistanceMin, &ReflectPos))
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
                            CurMonster.mHPTimeMsec = Platform::Utility::CurrentTimeMsec() + mHPbarDeleteTime;
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
                                NewItem.Init(CurHoleSkin, mAllSpines("item"), &HoleObject, updater(), -0.15f, 2000, 1000);
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
                        // 진행방향을 결정하고
                        CurMonster.SetKnockBackAccel(Point((ReflectPos.x - ResultPos.x) / mInGameW, (ReflectPos.y - ResultPos.y) / mInGameH));
                        // 벽에 닿은 경우에는 살짝 뒤로 물러섬
                        const float ResultDistance = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, ResultPos.x, ResultPos.y);
                        if(0 < ResultDistance)
                        {
                            const float StopRate = (ResultDistance - WallDistanceMin) / ResultDistance;
                            ResultPos.x = CurMonsterPos.x + (ResultPos.x - CurMonsterPos.x) * StopRate;
                            ResultPos.y = CurMonsterPos.y + (ResultPos.y - CurMonsterPos.y) * StopRate;
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
                if(CurMonster.mTargetIndex == -1)
                {
                    sint32 ResultIndex = -1;
                    TryWorld::Path* ResultPath = nullptr;
                    sint32 ResultPathScore = 0;
                    auto& CurTargets = (CurMonster.mType->mType == MonsterType::TypeClass::Ally)? mTargetsForAlly : mTargetsForEnemy;
                    for(sint32 j = 0, jend = CurTargets.Count(); j < jend; ++j)
                    {
                        auto& CurObject = mLayers[CurTargets[j].mLayerIndex].mObjects[CurTargets[j].mObjectIndex];
                        if(CurObject.mHPValue == 0) continue;
                        const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
                        const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
                        sint32 GetScore = 0;
                        auto NewPath = CurTryWorldZone->mMap->BuildPath(CurMonsterPos, Point(x, y), 20, &GetScore);
                        sint32 NewPathScore = GetScore * 1000 / mInGameW;
                        NewPathScore -= CurObject.mHPValue; // 타겟의 현재점수를 뺌
                        NewPathScore += Platform::Utility::Random() % 500; // 랜덤점수 추가
                        if(!ResultPath || (NewPath && NewPathScore < ResultPathScore))
                        {
                            TryWorld::Path::Release(ResultPath);
                            ResultIndex = j;
                            ResultPath = NewPath;
                            ResultPathScore = NewPathScore;
                        }
                        else TryWorld::Path::Release(NewPath);
                    }
                    CurMonster.mTargetIndex = ResultIndex;
                    CurMonster.mTargetPath = ResultPath;
                    CurMonster.mTargetPathScore = ResultPathScore;
                }

                // 서브타겟위치 업데이트
                Point TargetPos;
                if(CurMonster.mTargetIndex == -1)
                    TargetPos = Point(mInGameW * (CurMonster.mTargetPos.x + 0.5f), mInGameH * (CurMonster.mTargetPos.y + 0.5f));
                else
                {
                    TargetPos = TryWorld::GetPosition::SubTarget(CurTryWorldZone->mHurdle, CurMonster.mTargetPath, CurMonsterPos);
                    CurMonster.mTargetPos = Point(TargetPos.x / mInGameW - 0.5f, TargetPos.y / mInGameH - 0.5f);
                }

                const float TargetDistance = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, TargetPos.x, TargetPos.y);
                if(0 < TargetDistance)
                {
                    // 오브젝트와 폴리곤에 의해 멈추어야 할 경우를 판단
                    const float SpeedDelay = 10;
                    const float MoveDistance = mMonsterSizeR * CurMonster.mType->mMoveSpeed / (1000 * SpeedDelay);
                    const float TryNextPosX = CurMonsterPos.x + (TargetPos.x - CurMonsterPos.x) * MoveDistance / TargetDistance;
                    const float TryNextPosY = CurMonsterPos.y + (TargetPos.y - CurMonsterPos.y) * MoveDistance / TargetDistance;
                    Point ResultPos(TryNextPosX, TryNextPosY);
                    bool IsBounce = false;
                    float DistanceMin = -1;
                    sint32 ObjectIndex = GetValidNextForDynamic(CurMonster, CurMonsterPos, ResultPos);
                    if(ObjectIndex != -1)
                    {
                        IsBounce = true;
                        DistanceMin = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, ResultPos.x, ResultPos.y);
                    }
                    if(TryWorld::GetPosition::GetValidNext(CurTryWorldZone->mHurdle, CurMonsterPos, ResultPos, DistanceMin))
                    {
                        IsBounce = true;
                        ObjectIndex = -1;
                    }

                    // 벽에 닿은 경우에는 살짝 뒤로 물러섬
                    if(IsBounce)
                    {
                        const float ResultDistance = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, ResultPos.x, ResultPos.y);
                        if(0 < ResultDistance)
                        {
                            const float StopRate = (ResultDistance - WallDistanceMin) / ResultDistance;
                            const float ResultNextPosX = CurMonsterPos.x + (ResultPos.x - CurMonsterPos.x) * StopRate;
                            const float ResultNextPosY = CurMonsterPos.y + (ResultPos.y - CurMonsterPos.y) * StopRate;
                            NextPos = Point(ResultNextPosX / mInGameW - 0.5f, ResultNextPosY / mInGameH - 0.5f);
                        }
                        // 닿은 오브젝트가 있다면 기억
                        if(ObjectIndex != -1)
                            CurMonster.mBounceObjectIndex = ObjectIndex;
                    }
                    else
                    {
                        const float ResultNextPosX = CurMonsterPos.x + (ResultPos.x - CurMonsterPos.x);
                        const float ResultNextPosY = CurMonsterPos.y + (ResultPos.y - CurMonsterPos.y);
                        NextPos = Point(ResultNextPosX / mInGameW - 0.5f, ResultNextPosY / mInGameH - 0.5f);
                    }

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
    for(sint32 i = 0, iend = mTargetsForEnemy.Count(); i < iend; ++i)
    {
        auto& CurObject = mLayers.At(mTargetsForEnemy[i].mLayerIndex).mObjects.At(mTargetsForEnemy[i].mObjectIndex);
        if(0 < CurObject.mHPValue)
            CurObject.mHPValue = Math::Min(CurObject.mHPValue + mEggHPRegenValue * timespan, CurObject.mType->mHP);
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
            if(AnyMonsterWaiting = (mWaveSec < CurMonster.mEntranceSec)) break;
            if(WaveStop = CurMonster.mType->mWaveStop) break;
        }
        // 중간 웨이브
        if(mWave < WaveCount - 1)
        {
            if(!AnyMonsterWaiting && !WaveStop)
                ReadyForNextWave();
        }
        // 마지막 웨이브
        else if(!AnyMonsterAlived)
            ReadyForNextWave();
    }

    // 브레스폭발
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
    if(mBreath.mEnable && mBreath.mEndTimeMsec < CurTimeMsec)
    {
        mBreath.mEnable = false;
        BreathAttack(&mBreath);
        // 브레스 애니
        mBreathAttackSpine.StopMotionAll();
        mBreathAttackSpine.PlayMotion(String::Format("breath_%s_attack_b", (chars) mCurItemSkinForDragon), false);
        mBreathAttackSpine.PlayMotionAttached(String::Format("breath_%s_attack_b", (chars) mCurItemSkinForDragon), "idle", true);
        // 브레스이펙트 애니
        mBreathEffectSpine.StopMotionAll();
        mBreathEffectSpine.PlayMotion(String::Format("breath_%s_ground", (chars) mCurItemSkinForDragon), false);
        mBreathEffectSpine.PlayMotionAttached(String::Format("breath_%s_ground", (chars) mCurItemSkinForDragon), "idle", true);
    }

    // 브레스게이지연출
    mBreathGaugeTime = Math::Min(mBreathGaugeTime + timespan * mBreathGaugeChargingPerSec, mBreathMaxGauge);
    mBreathGaugeTimeLog = (mBreathGaugeTimeLog * 9 + mBreathGaugeTime * 1) / 10;
}

void ingameData::ClearPath()
{
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        mMonsters.At(i).mTargetIndex = -1;
        TryWorld::Path::Release(mMonsters.At(i).mTargetPath);
        mMonsters.At(i).mTargetPath = nullptr;
        mMonsters.At(i).mTargetPathScore = 0;
    }
}

void ingameData::Render(ZayPanel& panel)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    // 인게임
    Rect OutlineRect;
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
    {
        OutlineRect = F1State::RenderMap(mShowDebug, panel, &mMonsters, mWaveSec);
        OutlineRect += Point(mInGameX, mInGameY);
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
        RenderObject(true, false, panel, mWeather[1], false);
        // 햇살
        if(const Rect* Area = mWeather[0].GetBoundRect("area"))
        {
            const float Rate = Math::MinF(panel.w() / Area->Width(), panel.h() / Area->Height());
            const float Width = Area->Width() * Rate;
            const float Height = Area->Height() * Rate;
            ZAY_XYWH(panel, 0, 0, Width, Height)
                RenderObject(true, false, panel, mWeather[0], false);
        }
    }

    // 브레스연출
    if(mWave != -1)
    if(const Rect* Area = mDragon.GetBoundRect("area"))
    if(const Rect* MouthArea = mDragon.GetBoundRect("mouth_area"))
    {
        const Point DragonPos = mDragon.MoveOnce(mInGameSize * mDragonCurve,
            Point(mInGameSize * mDragonMouthX, mInGameSize * mDragonMouthY), 300);
        const float DragonSizeR = mDragonSizeR * mDragon.scale();
        const float BreathRate = mDragon.CalcBreathRate(CurTimeMsec);
        if(0 < BreathRate)
        {
            const float DragonRate = DragonSizeR * 2 / Math::MaxF(Area->Width(), Area->Height());
            const Point MouthPos = (MouthArea->Center() - Area->Center()) * DragonRate;
            const Point DragonMouthPos(DragonPos.x + MouthPos.x * ((mDragon.breath_flip())? -1 : 1), DragonPos.y + MouthPos.y);
            const float RR = BreathRate * BreathRate;
            const float X = mBreath.mPos.x * RR + DragonMouthPos.x * (1 - RR);
            const float Y = mBreath.mPos.y * RR + DragonMouthPos.y * (1 - RR);
            const float R = mBreath.mSizeR * RR;
            // 브레스연출
            ZAY_XYRR(panel, X, Y, R, R)
            {
                if(!mBreath.mEnable || mBreath.mAniTimeMsec < CurTimeMsec)
                    RenderObject(true, false, panel, mBreathAttackSpine, mDragon.breath_flip());
                RenderObject(true, false, panel, mBreathEffectSpine, mDragon.breath_flip());
            }
        }
        // 드래곤
        if(mDragon.CalcExitRate(CurTimeMsec) < 1)
            ZAY_XYRR(panel, DragonPos.x, DragonPos.y, DragonSizeR, DragonSizeR)
                RenderObject(true, mShowDebug, panel, mDragon, mDragon.flip());
    }

    // 게임전 UI
    if(mWave == -1)
    {
        // 메인타이틀
        RenderObject(true, false, panel, mMainTitleSpine, false, "Title_",
            ZAY_GESTURE_NT(n, t, this)
            {
                if(t == GT_Pressed && !String::Compare(n, "Title_butten_start_area"))
                {
                    mMainTitleSpine.Staff_Start();
                    for(sint32 i = 0; i < 3; ++i)
                        mMonsters.At(i).Staff_Start();
                }
            });
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
                    RenderObject(true, mShowDebug, panel, mWaveHUD, false);
            }
            // 중지버튼
            if(const Rect* Area = mStopButton.GetBoundRect("area"))
            {
                const sint32 StopWidth = Area->Width() * panel.h() / Area->Height();
                ZAY_LTRB(panel, panel.w() - StopWidth, 0, panel.w(), panel.h())
                    RenderObject(true, mShowDebug, panel, mStopButton, false);
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
                RenderObject(true, mShowDebug, panel, mGaugeHUD, false);
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
                    RenderObject(true, mShowDebug, panel, mSlotHUD, false);
            }
        }
        // 슬롯아이템
        ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
            RenderItems(panel, true, CurTimeMsec);
    }

    // 인게임 디버그정보
    if(mShowDebug)
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
        F1State::RenderDebug(panel, mMonsters, mWaveSec);

    #if BOSS_WINDOWS | BOSS_LINUX | BOSS_MAC_OSX
        const sint32 InnerGap = 10, ButtonSize = 80, ButtonSizeSmall = 50;
        ZAY_FONT(panel, 1.2, "Arial Black")
        {
            // 홈버튼
            ZAY_XYWH(panel, 0, 0, ButtonSize, ButtonSizeSmall)
            ZAY_INNER_UI(panel, InnerGap, "home",
                ZAY_GESTURE_T(t, this)
                {
                    if(t == GT_InReleased)
                    {
                        next("codename_f1View");
                    }
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
            ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSizeSmall)
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
            RenderObject(true, mShowDebug, panel, *CurItem, false, String::Format("Item%03d_", i),
                ZAY_GESTURE_NT(n, t, this, i)
                {
                    if(t == GT_Pressed && !String::Compare(n + 8, "ston_touch_area"))
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
    // 브레스영역 성장중
    if(mBreathing)
    {
        ZAY_XYRR(panel, mBreathPos.x - mInGameX, mBreathPos.y - mInGameY, mBreathSizeR, mBreathSizeR)
        {
            // 게이지파워 사용량 계산
            const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
            sint32 AniTime = Math::Min(3000, CurTimeMsec - mBreathMsec);
            mBreathGaugeTimeUsingCurrently = Math::Min(mBreathGaugeTimeLog, Math::Min(mBreathGaugeTime, AniTime));
            mBreathReadySpine.SetSeekSec(mBreathGaugeTimeUsingCurrently * 0.001f);
            RenderObject(true, mShowDebug, panel, mBreathReadySpine, false);

            if(const Rect* Area = mBreathReadySpine.GetBoundRect("area"))
            if(const Rect* RedArea = mBreathReadySpine.GetBoundRect("background_red_area"))
            if(const Rect* MinArea = mBreathReadySpine.GetBoundRect("background_min_area"))
            if(const Rect* MaxArea = mBreathReadySpine.GetBoundRect("background_max_area"))
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
    // 결정된 브레스영역
    else if(mBreath.mEnable)
    {
        ZAY_XYRR(panel, mBreath.mPos.x - mInGameX, mBreath.mPos.y - mInGameY, mBreathSizeR, mBreathSizeR)
        {
            mBreathReadySpine.SetSeekSec(mBreath.mGaugeTime * 0.001f);
            RenderObject(true, mShowDebug, panel, mBreathReadySpine, false);
        }
    }
}

void ingameData::ReadyForNextWave()
{
    mWave++;
    mWaveTitle = "Load Failure";
    mWaveSec = 0;

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
            auto& JsonMonsters = mWaveData("Waves")[mWave]("Events")[i]("Monsters");
            auto PtrMonsters = mMonsters.AtDumpingAdded(JsonMonsters.LengthOfIndexable());
            for(sint32 j = 0, jend = JsonMonsters.LengthOfIndexable(); j < jend; ++j)
            {
                const String CurJsonMonsterID = JsonMonsters[j]("ID").GetString("");
                sint32 CurJsonMonsterRID = JsonMonsters[j]("RID").GetInt(0);
                if(CurJsonMonsterRID == 0) CurJsonMonsterRID = ++mMonsterLastRID;
                for(sint32 k = 0, kend = mMonsterTypes.Count(); k < kend; ++k)
                {
                    const auto& CurMonster = mMonsterTypes[k];
                    if(!CurJsonMonsterID.Compare(CurMonster.mID))
                    {
                        chars CurSpineName = CurMonster.spineName();
                        PtrMonsters[j].Init(&CurMonster, CurJsonMonsterRID, TimeSec,
                            (mLandscape)? JsonMonsters[j]("PosY").GetFloat(0) : JsonMonsters[j]("PosX").GetFloat(0),
                            (mLandscape)? -JsonMonsters[j]("PosX").GetFloat(0) : JsonMonsters[j]("PosY").GetFloat(0),
                            mAllSpines(CurSpineName), mAllSpines.Access("monster_toast"));
                        break;
                    }
                }
            }
        }
        for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
            mMonsters.At(i).ResetCB();
    }
    else mWaveTitle = "Stage Over";
}

sint32 ingameData::GetCalcedBreathDamage()
{
    return mBreathMinDamage + (mBreathMaxDamage - mBreathMinDamage) * mBreathPowerPermil / 1000;
}

void ingameData::BreathAttack(const MapBreath* breath)
{
    // 몬스터피해
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        auto& CurMonster = mMonsters.At(i);
        if(CurMonster.mType->mType != MonsterType::TypeClass::Enemy) continue;
        if(mWaveSec < CurMonster.mEntranceSec || CurMonster.mHPValue == 0) continue;

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
            CurMonster.ClearTarget();
        }
    }

    // 오브젝트피해
    auto& CurObjects = mLayers.At(2).mObjects;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        auto& CurObject = CurObjects.At(i);
        if(!CurObject.mVisible || CurObject.mType->mType != ObjectType::TypeClass::Dynamic)
            continue;
        const float x = mInGameX + mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameY + mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        const float r = mInGameSize * CurObject.mCurrentRect.Width();
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < r + breath->mSizeR)
            CurObject.SetHP(CurObject.mHPValue - breath->mDamage, mHPbarDeleteTime);
    }

    // 타겟피해(유저의 조작실수)
    for(sint32 i = 0, iend = mTargetsForEnemy.Count(); i < iend; ++i)
    {
        auto& CurObject = mLayers.At(mTargetsForEnemy[i].mLayerIndex).mObjects.At(mTargetsForEnemy[i].mObjectIndex);
        if(CurObject.mHPValue == 0) continue;
        const float x = mInGameX + mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameY + mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mInGameSize * mTargetsForEnemy[i].mSizeR + breath->mSizeR)
            CurObject.SetHP(CurObject.mHPValue - breath->mDamage, mHPbarDeleteTime);
    }
}
