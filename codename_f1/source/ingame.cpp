#include <boss.hpp>
#include "ingame.hpp"

#include <r.hpp>

ZAY_DECLARE_VIEW_CLASS("ingameView", ingameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
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
                m->mMonsters.At(i).mPos = Point(
                    m->mMainTitleStaffBegin[i].x * (1 - StaffProgress) + m->mMainTitleStaffTarget[i].x * StaffProgress,
                    m->mMainTitleStaffBegin[i].y * (1 - StaffProgress) + m->mMainTitleStaffTarget[i].y * StaffProgress);
        }
        else m->AnimationOnce(CurTimeSecSpan);
        // 자동화면갱신
        m->invalidate();
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->SetSize(Width, Height);
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
    if(m->mWave != -1)
    {
        const sint32 ClampX = Math::Clamp(x, m->mInGameX, m->mInGameX + m->mInGameW);
        const sint32 ClampY = Math::Clamp(y, m->mInGameY, m->mInGameY + m->mInGameH);
        if(type == GT_Pressed)
        {
            m->mBreathing = true;
            m->mBreathPos = Point(ClampX, ClampY);
            m->mBreathMsec = Platform::Utility::CurrentTimeMsec();
            m->mBreathPowerPermil = 0;
            m->mBreathSizeRCurrently = m->mBreathSizeR;
        }
        else if(type == GT_InDragging || type == GT_OutDragging)
        {
            if(m->mBreathing)
                m->mBreathPos = Point(ClampX, ClampY);
        }
        else if(type == GT_InReleased)
        {
            if(m->mBreathing)
            {
                m->mBreathing = false;
                auto& NewBreath = m->mBreathes.AtAdding();
                NewBreath.mAniTimeMsec = Platform::Utility::CurrentTimeMsec() + m->mDragonEntryTime;
                NewBreath.mEndTimeMsec = NewBreath.mAniTimeMsec + m->mDragonBreathTime;
                NewBreath.mPos = m->mBreathPos;
                NewBreath.mSizeR = m->mBreathSizeRCurrently;
                NewBreath.mDamage = m->GetCalcedBreathDamage();
            }
        }
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    // 출력미비영역 표현용
    ZAY_RGB(panel, 0, 0, 0)
        panel.fill();
    m->Render(panel);
}

ingameData::ingameData()
{
    mShowDebug = false;
    mWaveData = Context(ST_Json, SO_NeedCopy, String::FromFile("table/stage_0.json"));
    mWave = -1;
    mWaveTitle = "";
    mWaveSec = 0;
    mBreathReadySpine.InitSpineForSeek(&mSpines("meteo_ready"), "meteo_ready", false);
    mBreathAttackSpine.InitSpineForSeek(&mSpines("meteo_attack"), "attack", false);
    mBreathing = false;
    mBreathPos = Point(0, 0);
    mBreathMsec = 0;
    mBreathPowerPermil = 0;
    mBreathSizeRCurrently = 0;

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

    // 타이틀화면
    mMainTitleSpine.InitSpine(&mSpines("ui_main_title"), "loding", "idle",
        [this](chars motionname)
        {
            if(!String::Compare("start", motionname))
                ReadyForNextWave();
        });
    mMainTitleStaffTime = Platform::Utility::CurrentTimeMsec();
    mMainTitleStaffBegin[0] = Point(-0.3f, 0.0f);
    mMainTitleStaffTarget[0] = Point(-0.2f, 0.0f);
    mMainTitleStaffBegin[1] = Point(0.1f, 0.3f);
    mMainTitleStaffTarget[1] = Point(0.0f, 0.2f);
    mMainTitleStaffBegin[2] = Point(0.3f, 0.0f);
    mMainTitleStaffTarget[2] = Point(0.2f, 0.0f);
    // 스태프 캐릭터
    auto PtrStaff = mMonsters.AtDumpingAdded(3);
    PtrStaff[0].InitSpine(&mSpines("ui_main_title_jjs"), "run");
    PtrStaff[0].Init(&mMonsterTypes[0], 0, mMainTitleStaffBegin[0].x, mMainTitleStaffBegin[0].y);
    PtrStaff[1].InitSpine(&mSpines("ui_main_title_kbh"), "run");
    PtrStaff[1].Init(&mMonsterTypes[0], 0, mMainTitleStaffBegin[1].x, mMainTitleStaffBegin[1].y);
    PtrStaff[2].InitSpine(&mSpines("ui_main_title_kbi"), "run");
    PtrStaff[2].Init(&mMonsterTypes[0], 0, mMainTitleStaffBegin[2].x, mMainTitleStaffBegin[2].y);
}

ingameData::~ingameData()
{
}

void ingameData::AnimationOnce(sint32 timespan)
{
    // 몬스터애니
    sint32 DeadMonsterCount = 0;
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        if(mMonsters[i].mHP == 0 && mMonsters[i].mDeathCount == 0)
            DeadMonsterCount++;
        if(mWaveSec < mMonsters[i].mEntranceSec || mMonsters[i].mHP == 0)
            continue;
        const Point CurMonsterPos = Point(mInGameW * (mMonsters[i].mPos.x + 0.5f), mInGameH * (mMonsters[i].mPos.y + 0.5f));
        // 트리공격
        bool HasAttack = false;
        if(mMonsters[i].mTargetId != -1)
        {
            const sint32 TargetId = mMonsters[i].mTargetId;
            if(0 < mTargets[TargetId].mHP)
            {
                const float x = mInGameW * (mTargets[TargetId].mPos.x + 0.5f);
                const float y = mInGameH * (mTargets[TargetId].mPos.y + 0.5f);
                if(Math::Distance(CurMonsterPos.x, CurMonsterPos.y, x, y) < mTargets[TargetId].mSizeR * mInGameW)
                {
                    HasAttack = true;
                    if(auto AttackCount = mMonsters.At(i).TryAttack())
                    {
                        mTargets.At(TargetId).mHP = Math::Max(0, mTargets[TargetId].mHP - mMonsters[i].mType->mAttackPower * AttackCount);
                        auto& CurObject = mLayers[mTargets[TargetId].mLayerId].mObjects[mTargets[TargetId].mObjectId];
                        if(0 < mTargets[TargetId].mHP)
                            CurObject.Hit();
                        else CurObject.Dead();
                    }
                }
            }
            else
            {
                mMonsters.At(i).mTargetId = -1;
                TryWorld::Path::Release(mMonsters.At(i).mTargetPath);
                mMonsters.At(i).mTargetPath = nullptr;
                mMonsters.At(i).mTargetPathScore = 0;
                mMonsters.At(i).CancelAttack();
            }
        }
        // 전진
        if(!HasAttack && mHurdle && mMap)
        {
            if(mMonsters[i].mTargetId == -1)
            {
                sint32 ResultId = -1;
                TryWorld::Path* ResultPath = nullptr;
                sint32 ResultPathScore = 0;
                for(sint32 j = 0, jend = mTargets.Count(); j < jend; ++j)
                {
                    if(mTargets[j].mHP == 0) continue;
                    const float x = mInGameW * (mTargets[j].mPos.x + 0.5f);
                    const float y = mInGameH * (mTargets[j].mPos.y + 0.5f);
                    sint32 GetScore = 0;
                    auto NewPath = mMap->BuildPath(CurMonsterPos, Point(x, y), 20, &GetScore);
                    sint32 NewPathScore = GetScore * 1000 / mInGameW;
                    NewPathScore -= mTargets[j].mHP; // 타겟의 현재점수를 뺌
                    NewPathScore += Platform::Utility::Random() % 500; // 랜덤점수 추가
                    if(!ResultPath || (NewPath && NewPathScore < ResultPathScore))
                    {
                        TryWorld::Path::Release(ResultPath);
                        ResultId = j;
                        ResultPath = NewPath;
                        ResultPathScore = NewPathScore;
                    }
                    else TryWorld::Path::Release(NewPath);
                }
                mMonsters.At(i).mTargetId = ResultId;
                mMonsters.At(i).mTargetPath = ResultPath;
                mMonsters.At(i).mTargetPathScore = ResultPathScore;
            }
            if(mMonsters[i].mTargetId == -1)
            {
                // 목표가 없는 경우
            }
            else
            {
                const Point TargetPos = TryWorld::GetPosition::SubTarget(mHurdle, mMonsters[i].mTargetPath, CurMonsterPos);
                const float SpeedDelay = 10;
                const float MoveDistance = mMonsterSizeR * mMonsters[i].mType->mMoveSpeed / (1000 * SpeedDelay);
                const float TargetDistance = Math::Distance(CurMonsterPos.x, CurMonsterPos.y, TargetPos.x, TargetPos.y);
                const float TryNextPosX = CurMonsterPos.x + (TargetPos.x - CurMonsterPos.x) * MoveDistance / TargetDistance * mMonsters[i].mType->mMoveSight;
                const float TryNextPosY = CurMonsterPos.y + (TargetPos.y - CurMonsterPos.y) * MoveDistance / TargetDistance * mMonsters[i].mType->mMoveSight;
                const Point ResultNextPos = TryWorld::GetPosition::ValidNext(mHurdle, CurMonsterPos, Point(TryNextPosX, TryNextPosY));
                const float NextPosX = CurMonsterPos.x + (ResultNextPos.x - CurMonsterPos.x) / mMonsters[i].mType->mMoveSight;
                const float NextPosY = CurMonsterPos.y + (ResultNextPos.y - CurMonsterPos.y) / mMonsters[i].mType->mMoveSight;
                const Point NextPos = Point(NextPosX / mInGameW - 0.5f, NextPosY / mInGameH - 0.5f);
                // 방향전환된 꼭지점 기록
                const bool CurFlip = (mMonsters[i].mPos.x < NextPos.x);
                if(mMonsters[i].mLastFlip != CurFlip)
                {
                    mMonsters.At(i).mLastFlip = CurFlip;
                    mMonsters.At(i).mLastFlipPos = mMonsters[i].mPos;
                }
                // 방향전환
                if(mMonsters[i].mFlipMode != mMonsters[i].mLastFlip)
                {
                    const float FlipDist = Math::Distance(mMonsters[i].mLastFlipPos.x, mMonsters[i].mLastFlipPos.y, NextPos.x, NextPos.y);
                    if(mMonsters[i].mType->mTurnDistance < FlipDist * 1000)
                    {
                        mMonsters.At(i).mFlipMode = mMonsters[i].mLastFlip;
                        mMonsters.At(i).Turn();
                    }
                }
                mMonsters.At(i).mPos = NextPos;
            }
        }
    }

    // 트리회복
    for(sint32 i = 0, iend = mTargets.Count(); i < iend; ++i)
    {
        if(0 < mTargets[i].mHP)
            mTargets.At(i).mHP = Math::Min(mTargets[i].mHP + mEggHPRegenValue * timespan, mEggHP);
    }

    // 웨이브클리어
    if(0 < mMonsters.Count() && mMonsters.Count() == DeadMonsterCount)
        ReadyForNextWave();

    // 브레스시전
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();
    for(sint32 i = mBreathes.Count() - 1; 0 <= i; --i)
    {
        auto& CurBreath = mBreathes[i];
        if(CurBreath.mEndTimeMsec < CurTimeMsec)
        {
            BreathAttack(&CurBreath);
            mBreathes.SubtractionSection(i);
        }
    }
}

void ingameData::ClearPath()
{
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        mMonsters.At(i).mTargetId = -1;
        TryWorld::Path::Release(mMonsters.At(i).mTargetPath);
        mMonsters.At(i).mTargetPath = nullptr;
        mMonsters.At(i).mTargetPathScore = 0;
    }
}

void ingameData::Render(ZayPanel& panel)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    // 인게임
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
        F1State::Render(mShowDebug, mWave == -1, panel, &mMonsters, mWaveSec);

    // 시간
    if(mWave != -1)
    ZAY_RGBA(panel, 0, 0, 0, 128)
    ZAY_FONT(panel, 1.2, "Arial Black")
        panel.text(String::Format("Wave%d(%dsec) : %s ", mWave + 1, mWaveSec, (chars) mWaveTitle),
            UIFA_RightTop, UIFE_Right);

    // 브레스시전
    for(sint32 i = mBreathes.Count() - 1; 0 <= i; --i)
    {
        auto& CurBreath = mBreathes[i];
        ZAY_XYRR(panel, CurBreath.mPos.x, CurBreath.mPos.y, CurBreath.mSizeR, CurBreath.mSizeR)
        {
            ZAY_RGBA(panel, 255, 0, 0, 64)
                panel.circle();
            if(CurBreath.mAniTimeMsec < CurTimeMsec)
            {
                mBreathAttackSpine.SetSeekSec((CurTimeMsec - CurBreath.mAniTimeMsec) * 0.001f);
                RenderObject(mShowDebug, panel, mBreathAttackSpine, mInGameX, mInGameY, mInGameW, mInGameH, false);
            }
        }
    }

    // 브레스영역
    if(mBreathing)
    ZAY_XYRR(panel, mBreathPos.x, mBreathPos.y, mBreathSizeR, mBreathSizeR)
    {
        mBreathReadySpine.SetSeekSec((CurTimeMsec - mBreathMsec) * 0.001f);
        RenderObject(mShowDebug, panel, mBreathReadySpine, mInGameX, mInGameY, mInGameW, mInGameH, false);

        // 데미지영역 계산
        const float AreaWidth = mBreathReadySpine.GetBoundRect("area").Width();
        const float RedWidth = mBreathReadySpine.GetBoundRect("background_red_area").Width();
        mBreathSizeRCurrently = Math::Max(mBreathSizeR, mBreathSizeR * RedWidth / AreaWidth);

        // 데미지파워 계산
        const float MinWidth = mBreathReadySpine.GetBoundRect("background_min_area").Width();
        const float MaxWidth = mBreathReadySpine.GetBoundRect("background_max_area").Width();
        mBreathPowerPermil = Math::Clamp(1000 * (RedWidth - MinWidth) / (MaxWidth - MinWidth), 0, 1000);

        // 데미지스코어
        ZAY_FONT(panel, 1.5, "Arial Black")
        {
            const String Text = String::Format("%d(%d‰)", GetCalcedBreathDamage(), mBreathPowerPermil);
            ZAY_RGBA(panel, 0, 0, 0, -128)
                panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, Text, UIFA_CenterMiddle);
            ZAY_RGB(panel, 255, 64, 0)
                panel.text(panel.w() / 2, panel.h() / 2, Text, UIFA_CenterMiddle);
        }
    }

    // 게임전 메인타이틀
    if(mWave == -1)
    {
        const Point XY = panel.toview(0, 0);
        const sint32 SX = (sint32) (XY.x * panel.zoom());
        const sint32 SY = (sint32) (XY.y * panel.zoom());
        const sint32 SW = (sint32) (panel.w() * panel.zoom());
        const sint32 SH = (sint32) (panel.h() * panel.zoom());
        RenderObject(mShowDebug, panel, mMainTitleSpine, SX, SY, SW, SH, false, "Title_",
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

    #if BOSS_WINDOWS | BOSS_MAC_OSX
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

    // 몬스터 데스카운트
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        if(0 < mMonsters[i].mDeathCount)
            mMonsters.At(i).mDeathCount--;
    }
}

void ingameData::ReadyForNextWave()
{
    mWave++;
    mWaveTitle = "Load Failure";
    mWaveSec = 0;
    mMonsters.SubtractionAll();

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
                for(sint32 k = 0, kend = mMonsterTypes.Count(); k < kend; ++k)
                    if(!CurJsonMonsterID.Compare(mMonsterTypes[k].mID))
                    {
                        chars CurAssetName = mMonsterTypes[k].mAsset;
                        PtrMonsters[j].InitSpine(mSpines.Access(CurAssetName), "run");
                        PtrMonsters[j].Init(&mMonsterTypes[k], TimeSec,
                            JsonMonsters[j]("PosX").GetFloat(0), JsonMonsters[j]("PosY").GetFloat(0));
                        break;
                    }
            }
        }
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
        if(mWaveSec < mMonsters[i].mEntranceSec || mMonsters[i].mHP == 0)
            continue;
        const float x = mInGameX + mInGameW * (mMonsters[i].mPos.x + 0.5f);
        const float y = mInGameY + mInGameH * (mMonsters[i].mPos.y + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mMonsterSizeR + breath->mSizeR)
        {
            mMonsters.At(i).mHP = Math::Max(0, mMonsters[i].mHP - breath->mDamage);
            if(mMonsters[i].mHP == 0) // 방금 죽어서 데스애니 시작
            {
                mMonsters.At(i).Kill();
                mMonsters.At(i).mDeathCount = 10;
            }
            else mMonsters.At(i).Hit();
        }
    }

    // 트리피해(유저의 조작실수)
    for(sint32 i = 0, iend = mTargets.Count(); i < iend; ++i)
    {
        if(mTargets.At(i).mHP == 0) continue;
        const float x = mInGameX + mInGameW * (mTargets[i].mPos.x + 0.5f);
        const float y = mInGameY + mInGameH * (mTargets[i].mPos.y + 0.5f);
        if(Math::Distance(x, y, breath->mPos.x, breath->mPos.y) < mTargets[i].mSizeR * mInGameW + breath->mSizeR)
        {
            mTargets.At(i).mHP = Math::Max(0, mTargets[i].mHP - breath->mDamage);
            auto& CurObject = mLayers[mTargets[i].mLayerId].mObjects[mTargets[i].mObjectId];
            if(0 < mTargets[i].mHP)
                CurObject.Hit();
            else CurObject.Dead();
        }
    }
}
