#include <boss.hpp>
#include "ingame.hpp"

#include <r.hpp>

ZAY_DECLARE_VIEW_CLASS("ingameView", ingameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        // 시간진행
        static uint64 OldTimeSec = Platform::Utility::CurrentTimeMsec() / 1000;
        const uint64 CurTimeSec = Platform::Utility::CurrentTimeMsec() / 1000;
        sint32 CurTimeSecSpan = (sint32) (CurTimeSec - OldTimeSec);
        if(0 < CurTimeSecSpan)
        {
            m->mWaveSec += CurTimeSecSpan;
            OldTimeSec = CurTimeSec;
        }
        // 애니메이션 진행
        if(m->mWave != -1)
            m->AnimationOnce(CurTimeSecSpan);
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->SetSize(Width, Height);
        m->RebuildTryWorld();
        m->ClearPath();
    }
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    if(m->mWave == -1)
    {
        if(type == GT_Pressed)
            m->ReadyForNextWave();
    }
    else
    {
        if(type == GT_Pressed)
        {
            if(m->mInGameX <= x && x < m->mInGameX + m->mInGameW &&
                m->mInGameY <= y && y < m->mInGameY + m->mInGameH)
            {
                m->mTouchPos = Point(x, y);
                m->SetTouchSizeR(m->mMeteoSizeMinR);
            }
        }
        else if(type == GT_InReleased)
        {
            if(0 < m->mTouchSizeR)
            {
                m->TouchAttack();
                m->mTouchPos = Point(0, 0);
                m->SetTouchSizeR(0);
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

ingameData::ingameData()
{
    mShowDebug = false;
    mWaveData = Context(ST_Json, SO_NeedCopy, String::FromFile("table/stage_0.json"));
    mWave = -1;
    mWaveTitle = "";
    mWaveSec = 0;
    mTouchPos = Point(0, 0);
    mTouchSizeR = 0;
    mTouchDamage = 0;

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
        const float ox = mInGameW * mMonsters[i].mPos.x;
        const float oy = mInGameH * mMonsters[i].mPos.y;
        // 트리공격
        if(Math::Distance(ox, oy, 0, 0) < mTreeSizeR)
        {
            mMonsters.At(i).Attacking();
            mEggHP = Math::Max(0, mEggHP - mMonsters[i].mType->mAttackPower * timespan * mMonsters[i].mType->mAttackSpeed / 1000);
        }
        // 전진
        else if(mHurdle && mMap)
        {
            const Point MonsterPos
                = Point(mInGameW * (mMonsters[i].mPos.x + 0.5f), mInGameH * (mMonsters[i].mPos.y + 0.5f));
            if(!mMonsters[i].mPath)
                mMonsters.At(i).mPath = mMap->BuildPath(MonsterPos, Point(mInGameW / 2, mInGameH / 2), 4);
            const Point TargetPos = TryWorld::GetPosition::SubTarget(mHurdle, mMonsters[i].mPath, MonsterPos);
            const float SpeedDelay = 10;
            const float MoveDistance = mMonsterSizeR * mMonsters[i].mType->mMoveSpeed / (1000 * SpeedDelay);
            const float TargetDistance = Math::Distance(MonsterPos.x, MonsterPos.y, TargetPos.x, TargetPos.y);
            const float TryNextPosX = MonsterPos.x + (TargetPos.x - MonsterPos.x) * MoveDistance / TargetDistance * mMonsters[i].mType->mMoveSight;
            const float TryNextPosY = MonsterPos.y + (TargetPos.y - MonsterPos.y) * MoveDistance / TargetDistance * mMonsters[i].mType->mMoveSight;
            const Point ResultNextPos = TryWorld::GetPosition::ValidNext(mHurdle, MonsterPos, Point(TryNextPosX, TryNextPosY));
            const float NextPosX = MonsterPos.x + (ResultNextPos.x - MonsterPos.x) / mMonsters[i].mType->mMoveSight;
            const float NextPosY = MonsterPos.y + (ResultNextPos.y - MonsterPos.y) / mMonsters[i].mType->mMoveSight;
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

    // 트리회복
    if(0 < mEggHP)
        mEggHP = Math::Min(mEggHP + mEggHPRegenValue * timespan, mEggHPValue);

    // 웨이브클리어
    if(0 < mMonsters.Count() && mMonsters.Count() == DeadMonsterCount)
        ReadyForNextWave();

    // 터치영역확장
    if(mTouchSizeR != 0)
        SetTouchSizeR(mTouchSizeR + (mTouchSizeR - mMeteoSizeMinR + 1) * 0.1f);

    // 자동화면갱신
    invalidate();
}

void ingameData::ClearPath()
{
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
        TryWorld::Path::Release(mMonsters.At(i).mPath);
}

void ingameData::Render(ZayPanel& panel)
{
    // 인게임
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
        F1State::Render(mShowDebug, panel, &mMonsters, mWaveSec);

    const sint32 InnerGap = 10, ButtonSize = 80;
    ZAY_FONT(panel, 1.2, "Arial Black")
    {
        // 홈버튼
        ZAY_XYWH(panel, 0, 0, ButtonSize, ButtonSize)
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
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
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

    // 시간
    if(-1 < mWave)
    ZAY_RGBA(panel, 0, 0, 0, 128)
    ZAY_FONT(panel, 1.2, "Arial Black")
        panel.text(String::Format("Wave%d(%dsec) : %s", mWave + 1, mWaveSec, (chars) mWaveTitle),
            UIFA_CenterTop, UIFE_Right);

    // 터치영역
    if(mTouchSizeR != 0)
    {
        ZAY_RGBA(panel, 255, 255, 0, 64)
        ZAY_XYRR(panel, mTouchPos.x, mTouchPos.y, mTouchSizeR, mTouchSizeR)
        {
            panel.circle();
            // 데미지스코어
            ZAY_FONT(panel, 1.5, "Arial Black")
            {
                const String Text = String::Format("%d", mTouchDamage);
                ZAY_RGBA(panel, 0, 0, 0, -128)
                    panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, Text, UIFA_CenterMiddle);
                ZAY_RGB(panel, 255, 64, 0)
                    panel.text(panel.w() / 2, panel.h() / 2, Text, UIFA_CenterMiddle);
            }
        }
    }

    // 게임전 메시지
    if(mWave == -1)
    ZAY_RGBA(panel, 0, 0, 0, 128)
    ZAY_FONT(panel, 2.0, "Arial Black")
        panel.text("TOUCH THE SCREEN", UIFA_CenterMiddle, UIFE_Right);

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
        sint32 MonsterCount = 0;
        mWaveTitle = mWaveData("Waves")[mWave]("Title").GetString();
        for(sint32 i = 0, iend = mWaveData("Waves")[mWave]("Events").LengthOfIndexable(); i < iend; ++i)
        {
            const sint32 TimeSec = mWaveData("Waves")[mWave]("Events")[i]("TimeSec").GetInt(0);
            auto& JsonMonsters = mWaveData("Waves")[mWave]("Events")[i]("Monsters");
            auto AllMonsters = mMonsters.AtDumpingAdded(JsonMonsters.LengthOfIndexable());
            for(sint32 j = 0, jend = JsonMonsters.LengthOfIndexable(); j < jend; ++j)
            {
                const String CurJsonMonsterID = JsonMonsters[j]("ID").GetString("");
                for(sint32 k = 0, kend = mMonsterTypes.Count(); k < kend; ++k)
                    if(!CurJsonMonsterID.Compare(mMonsterTypes[k].mID))
                    {
                        chars CurAssetName = mMonsterTypes[k].mAsset;
                        AllMonsters[j].Init(String::Format("M%d:", MonsterCount++), TimeSec, &mMonsterTypes[k],
                            JsonMonsters[j]("PosX").GetFloat(0),
                            JsonMonsters[j]("PosY").GetFloat(0), mSpines.Access(CurAssetName));
                        break;
                    }
            }
        }
    }
    else mWaveTitle = "Stage Over";
}

void ingameData::SetTouchSizeR(float size)
{
    if(size != 0)
    {
        mTouchSizeR = Math::ClampF(size, mMeteoSizeMinR, mMeteoSizeMaxR);
        mTouchDamage = mMeteoMinDamage + (mMeteoMaxDamage - mMeteoMinDamage)
            * (mTouchSizeR - mMeteoSizeMinR) / (mMeteoSizeMaxR - mMeteoSizeMinR);
    }
    else
    {
        mTouchSizeR = 0;
        mTouchDamage = 0;
    }
}

void ingameData::TouchAttack()
{
    // 몬스터피해
    for(sint32 i = 0, iend = mMonsters.Count(); i < iend; ++i)
    {
        if(mWaveSec < mMonsters[i].mEntranceSec || mMonsters[i].mHP == 0)
            continue;
        const float x = mInGameX + mInGameW * (mMonsters[i].mPos.x + 0.5f);
        const float y = mInGameY + mInGameH * (mMonsters[i].mPos.y + 0.5f);
        if(Math::Distance(x, y, mTouchPos.x, mTouchPos.y) < mMonsterSizeR + mTouchSizeR)
        {
            mMonsters.At(i).mHP = Math::Max(0, mMonsters[i].mHP - mTouchDamage);
            if(mMonsters[i].mHP == 0) // 방금 죽어서 데스애니 시작
            {
                mMonsters.At(i).Kill();
                mMonsters.At(i).mDeathCount = 10;
            }
            else mMonsters.At(i).Hit();
        }
    }

    // 트리피해
    const float ox = mInGameX + mInGameW / 2;
    const float oy = mInGameY + mInGameH / 2;
    if(Math::Distance(ox, oy, mTouchPos.x, mTouchPos.y) < mTreeSizeR + mTouchSizeR)
    {
        mEggHP = Math::Max(0, mEggHP - mTouchDamage);
    }
}
