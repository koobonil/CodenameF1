#include <boss.hpp>
#include "classes_f1.hpp"

#include <resource.hpp>

////////////////////////////////////////////////////////////////////////////////
MapObject::MapObject() : MapSpine(ST_Object)
{
    mType = nullptr;
    mRID = 0;
    mEnable = true;
    mHPValue = 0;
    mHPTimeMsec = 0;
    mHPAni = 0;
    mParaView = nullptr;
}

MapObject::~MapObject()
{
    delete mParaView;
}

MapObject::MapObject(const MapObject& rhs)
{
    operator=(rhs);
}

MapObject& MapObject::operator=(const MapObject& rhs)
{
    MapSpine::operator=(rhs);
    mType = rhs.mType;
    mRID = rhs.mRID;
    mEnable = rhs.mEnable;
    mHPValue = rhs.mHPValue;
    mHPTimeMsec = rhs.mHPTimeMsec;
    mHPAni = rhs.mHPAni;
    mCurrentRect = rhs.mCurrentRect;
    return *this;
}

MapObject::MapObject(MapObject&& rhs)
{
    operator=(ToReference(rhs));
}

MapObject& MapObject::operator=(MapObject&& rhs)
{
    MapSpine::operator=(ToReference(rhs));
    mType = rhs.mType;
    mRID = rhs.mRID;
    mEnable = rhs.mEnable;
    mHPValue = rhs.mHPValue;
    mHPTimeMsec = rhs.mHPTimeMsec;
    mHPAni = rhs.mHPAni;
    mCurrentRect = rhs.mCurrentRect;

    delete mParaView;
    mParaView = rhs.mParaView;
    rhs.mParaView = nullptr;
    return *this;
}

void MapObject::ResetCB(FXState* state)
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::ResetCB(mSpineInstance,
            [this](chars motionname)
            {
                if(!String::Compare("dead", motionname))
                    mEnable = false;
            },
            [this, state](chars eventname)
            {
                if(!String::Compare("sound_", eventname, 6))
                    Platform::Sound::Play(state->GetSound(&eventname[6]));
            });
    }
}

bool MapObject::SetHP(sint32 hp, sint32 deleteTime)
{
    mHPValue = Math::Max(0, hp);
    mHPTimeMsec = Platform::Utility::CurrentTimeMsec() + deleteTime;
    // 파괴되는 모습
    if(mType->mType == ObjectType::TypeClass::Dynamic)
        SetSeekSec(1 - mHPValue / (float) mType->mHP);
    if(0 < mHPValue)
    {
        Hit();
        return false;
    }
    Dead();
    return true;
}

void MapObject::Hit() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "hit", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "hit", "idle", true);
    }
}

void MapObject::Dead() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "dead", false);
    }
}

void MapObject::Drop() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "drop", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "drop", "idle", true);
    }
}

void MapObject::Spot() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "spot", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "spot", "idle", true);
    }
}

////////////////////////////////////////////////////////////////////////////////
MapPolygon::MapPolygon()
{
    mType = nullptr;
    mRID = 0;
    mEnable = true;
    mIsCW = false;
}

MapPolygon::~MapPolygon()
{
}

MapPolygon::MapPolygon(const MapPolygon& rhs)
{
    operator=(rhs);
}

MapPolygon& MapPolygon::operator=(const MapPolygon& rhs)
{
    mType = rhs.mType;
    mRID = rhs.mRID;
    mEnable = rhs.mEnable;
    mDots = rhs.mDots;
    mIsCW = rhs.mIsCW;
    return *this;
}

MapPolygon::MapPolygon(MapPolygon&& rhs)
{
    operator=(ToReference(rhs));
}

MapPolygon& MapPolygon::operator=(MapPolygon&& rhs)
{
    mType = rhs.mType;
    mRID = rhs.mRID;
    mEnable = rhs.mEnable;
    mDots = ToReference(rhs.mDots);
    mIsCW = rhs.mIsCW;
    return *this;
}

void MapPolygon::UpdateCW()
{
    mIsCW = false;
    if(2 < mDots.Count())
    {
        Rect BoundRect(Point(mDots[0].x, mDots[0].y), Size(0, 0));
        for(sint32 i = 1, iend = mDots.Count(); i < iend; ++i)
        {
            const auto& CurPoint = mDots[i];
            BoundRect.l = Math::MinF(BoundRect.l, CurPoint.x);
            BoundRect.t = Math::MinF(BoundRect.t, CurPoint.y);
            BoundRect.r = Math::MaxF(BoundRect.r, CurPoint.x);
            BoundRect.b = Math::MaxF(BoundRect.b, CurPoint.y);
        }
        bool ResultCW = mIsCW;
        float ResultDistance = -1;
        const TryWorld::Dot TestLine[2] = {
            TryWorld::Dot(BoundRect.l - BoundRect.Width(), BoundRect.CenterY()),
            TryWorld::Dot(BoundRect.r + BoundRect.Width(), BoundRect.CenterY())};
        for(sint32 i = 0, iend = mDots.Count(); i < iend; ++i)
        {
            const auto& LineBegin = mDots[i];
            const auto& LineEnd = mDots[(i + 1) % iend];
            if(auto CurPos = TryWorld::Util::GetDotByLineCross(LineBegin, LineEnd, TestLine[0], TestLine[1]))
            {
                const float CurDistance = Math::Distance(TestLine[0].x, TestLine[0].y, CurPos->x, CurPos->y);
                if(ResultDistance < 0 || CurDistance < ResultDistance)
                {
                    ResultCW = (TryWorld::Util::GetClockwiseValue(LineBegin, LineEnd, TestLine[0]) < 0);
                    ResultDistance = CurDistance;
                }
            }
        }
        mIsCW = ResultCW;
    }
}

////////////////////////////////////////////////////////////////////////////////
MonsterTarget::MonsterTarget()
{
    mType = Null;
    mIndex = -1;
    mPos = Point(0, 0);
    mSizeR = 0;
    mPath = nullptr;
}

MonsterTarget::~MonsterTarget()
{
    TryWorld::Path::Release(mPath);
}

MonsterTarget::MonsterTarget(MonsterTarget&& rhs)
{
    operator=(ToReference(rhs));
}

MonsterTarget& MonsterTarget::operator=(MonsterTarget&& rhs)
{
    mType = rhs.mType;
    mIndex = rhs.mIndex;
    mPos = rhs.mPos;
    mSizeR = rhs.mSizeR;
    TryWorld::Path::Release(mPath);
    mPath = rhs.mPath;
    rhs.mPath = nullptr;
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
MapMonster::MapMonster() : MapSpine(ST_Monster), mToast(ST_MonsterToast)
{
    mType = nullptr;
    mRID = 0;
    mEntranced = false;
    mEntranceSec = 0;
    mHPValue = 0;
    mHPTimeMsec = 0;
    mHPAni = 0;
    mMode = Mode_Run;
    mDeathStep = 0;
    mDeathPos = Point(-1, -1);
    mAttackCount = 0;
    mAttackTimeMsec = 0;
    mFlipMode = false;
    mLastFlip = false;
    mLastFlipPos = Point(0, 0);
    mTargetTimeLimit = 0;
    mBounceObjectIndex = -1;
    mHasParaTalk = false;
    mIsBumpClock = false;
    mBumpObjectRID = -1;
    mBumpObjectRect = Rect(0, 0, 0, 0);

    mKnockBackMode = false;
    mKnockBackAccel = Point(0, 0);
    mKnockBackBoundCount = 0;
}

MapMonster::~MapMonster()
{
}

MapMonster::MapMonster(MapMonster&& rhs)
{
    operator=(ToReference(rhs));
}

MapMonster& MapMonster::operator=(MapMonster&& rhs)
{
    MapSpine::operator=(ToReference(rhs));
    mType = rhs.mType;
    mRID = rhs.mRID;
    mEntranced = rhs.mEntranced;
    mEntranceSec = rhs.mEntranceSec;
    mHPValue = rhs.mHPValue;
    mHPTimeMsec = rhs.mHPTimeMsec;
    mHPAni = rhs.mHPAni;
    mMode = rhs.mMode;
    mDeathStep = rhs.mDeathStep;
    mDeathPos = rhs.mDeathPos;
    mAttackCount = rhs.mAttackCount;
    mAttackTimeMsec = rhs.mAttackTimeMsec;
    mFlipMode = rhs.mFlipMode;
    mLastFlip = rhs.mLastFlip;
    mLastFlipPos = rhs.mLastFlipPos;
    mCurrentPos = rhs.mCurrentPos;
    mCurrentPosOld = rhs.mCurrentPosOld;
    mTargetPos = rhs.mTargetPos;
    mTargets = ToReference(rhs.mTargets);
    mTargetTimeLimit = rhs.mTargetTimeLimit;
    mBounceObjectIndex = rhs.mBounceObjectIndex;
    mToast = ToReference(rhs.mToast);
    mHasParaTalk = rhs.mHasParaTalk;
    mParaTalk = ToReference(rhs.mParaTalk);
    mIsBumpClock = ToReference(rhs.mIsBumpClock);
    mBumpObjectRID = ToReference(rhs.mBumpObjectRID);
    mBumpObjectRect = ToReference(rhs.mBumpObjectRect);

    mKnockBackMode = rhs.mKnockBackMode;
    mKnockBackAccel = rhs.mKnockBackAccel;
    mKnockBackBoundCount = rhs.mKnockBackBoundCount;
    return *this;
}

void MapMonster::Init(const MonsterType* type, sint32 rid, sint32 timesec, float x, float y,
    const SpineRenderer* renderer, const SpineRenderer* toast_renderer)
{
    mType = type;
    mRID = rid;
    mEntranced = true;
    mEntranceSec = timesec;
    mHPValue = type->mHP;
    mHPTimeMsec = 0;
    mHPAni = type->mHP;
    mMode = Mode_Run;
    mDeathStep = 0;
    mDeathPos = Point(-1, -1);
    mAttackCount = 0;
    mAttackTimeMsec = 0;
    mFlipMode = (x < 0);
    mLastFlip = mFlipMode;
    mCurrentPos = Point(x, y);
    mCurrentPosOld = Point(x, y);
    mTargetPos = Point(x, y);
    mHasParaTalk = false;
    mIsBumpClock = false;
    mBumpObjectRID = -1;
    mBumpObjectRect = Rect(0, 0, 0, 0);

    InitSpine(renderer, type->spineSkinName()).PlayMotion("run", true,
        (Platform::Utility::Random() % 500) * 0.001f);
    PlayMotionSeek("_style", false);
    SetSeekSec((Platform::Utility::Random() % 5000) * 0.001f);

    if(toast_renderer)
        mToast.InitSpine(toast_renderer);
}

void MapMonster::ResetCB(FXState* state)
{
    if(mSpineInstance)
        ZAY::SpineBuilder::ResetCB(mSpineInstance,
            [this](chars motionname)
            {
                if(!String::Compare("dead", motionname))
                    mDeathStep = 2;
                else if(!String::Compare("dead_hole", motionname))
                    mDeathStep = 2;
                else if(!String::Compare("touch", motionname))
                    mDeathStep = 2;
            },
            [this, state](chars eventname)
            {
                if(!String::Compare("sound_", eventname, 6))
                    Platform::Sound::Play(state->GetSound(&eventname[6]));
            });
}

bool MapMonster::IsEntranced()
{
    bool Result = mEntranced;
    mEntranced = false;
    return Result;
}

bool MapMonster::IsKnockBackMode()
{
    return mKnockBackMode;
}

void MapMonster::KnockBack(bool down, const Point& accel, chars skin)
{
    if(mSpineInstance)
    {
        mKnockBackMode = true;
        mKnockBackAccel = accel;
        mKnockBackBoundCount = 0;
        if(mHPValue == 0) mMode = Mode_Dying;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, (down)? "hit_down" : "hit_up", false);
        if(mToast.renderer())
        {
            mToast.mSpineMsecOld = 0;
            ZAY::SpineBuilder::SetMotionOnOnce(mToast.mSpineInstance,
                String::Format("breath_hit_%s", skin));
        }
    }
}

void MapMonster::KnockBackBound(sint32 damage)
{
    if(mSpineInstance)
    {
        mHPValue = Math::Max(0, mHPValue - damage);
        if(mHPValue == 0) mMode = Mode_Dying;
        mKnockBackBoundCount++;
    }
}

void MapMonster::KnockBackEnd()
{
    if(mSpineInstance)
    {
        mKnockBackMode = false;
        mKnockBackAccel = Point(0, 0);
        mKnockBackBoundCount = 0;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        if(mMode == Mode_Dying)
        {
            mDeathStep = 1;
            ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "dead", false);
        }
        else
        {
            mMode = Mode_Run;
            ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "run", true);
        }
    }
}

void MapMonster::KnockBackEndByHole(const Point& hole)
{
    if(mSpineInstance)
    {
        mMode = Mode_Dying;
        mDeathStep = 1;
        mDeathPos = hole;
        mKnockBackMode = false;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "dead_hole", false);
    }
}

void MapMonster::Turn() const
{
    if(mSpineInstance && mMode != Mode_Dying)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "flip", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "flip", (mMode == Mode_Attack)? "attack" : "run", true);
    }
}

sint32 MapMonster::TryAttack(const Point& target)
{
    if(!mSpineInstance || mMode == Mode_Dying)
        return 0;

    if(mMode != Mode_Attack)
    {
        mMode = Mode_Attack;
        mAttackCount = 0;
        mAttackTimeMsec = Platform::Utility::CurrentTimeMsec();
        mFlipMode = mLastFlip = (mCurrentPos.x < target.x);
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "attack", true);
    }

    const sint32 CurAttackCount = (Platform::Utility::CurrentTimeMsec() - mAttackTimeMsec) / mType->mAttackSpeed;
    const sint32 Result = CurAttackCount - mAttackCount;
    mAttackCount = CurAttackCount;
    return Result;
}

void MapMonster::CancelAttack()
{
    if(mSpineInstance && mMode == Mode_Attack)
    {
        mMode = Mode_Run;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "run", true);
    }
}

void MapMonster::ClearTargetOnce()
{
    if(1 < mTargets.Count())
        mTargets.SubtractionSection(0);
    else mTargets.Clear();
}

void MapMonster::ClearAllTargets()
{
    mTargets.Clear();
}

void MapMonster::TryDeathMove()
{
    if(mDeathPos != Point(-1, -1))
    {
        mCurrentPosOld = mCurrentPos;
        mCurrentPos.x = mDeathPos.x * 0.1f + (mCurrentPos.x + mKnockBackAccel.x) * 0.9f;
        mCurrentPos.y = mDeathPos.y * 0.1f + (mCurrentPos.y + mKnockBackAccel.y) * 0.9f;
        mKnockBackAccel.x *= 0.9f;
        mKnockBackAccel.y *= 0.9f;
    }
}

void MapMonster::TryParaTalk()
{
    if(!mHasParaTalk)
    {
        mHasParaTalk = true;
        if(0 < mParaTalk.Length())
            ZAY::SpineBuilder::SetMotionOnOnce(mToast.mSpineInstance, "paratalk");
    }
}

void MapMonster::Ally_Arrived()
{
    if(mSpineInstance)
    {
        mMode = Mode_Dying;
        mDeathStep = 2;
    }
}

void MapMonster::Ally_Touched()
{
    if(mSpineInstance)
    {
        mMode = Mode_Dying;
        mDeathStep = 1;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "touch", false);
    }
}

Point MapMonster::CalcBump(const MapObject* object)
{
    const float MonsterOX = mCurrentPos.x - object->mCurrentRect.CenterX();
    const float MonsterOY = mCurrentPos.y - object->mCurrentRect.CenterY();
    const bool IsXBig = (Math::AbsF(MonsterOY) - object->mCurrentRect.Height() / 2
        < Math::AbsF(MonsterOX) - object->mCurrentRect.Width() / 2);
    // 오브젝트 연속성과 방향성조사
    if(mBumpObjectRID != object->mRID)
    {
        if(mBumpObjectRID == -1 || !mBumpObjectRect.ContactTest(object->mCurrentRect))
        {
            if(IsXBig) mIsBumpClock = (mCurrentPos.y < mCurrentPosOld.y) ^ (0 < MonsterOX);
            else mIsBumpClock = (mCurrentPos.x > mCurrentPosOld.x) ^ (0 < MonsterOY);
        }
        mBumpObjectRID = object->mRID;
        mBumpObjectRect = object->mCurrentRect;
        const float SizeMin = 0.1f * Math::MinF(mBumpObjectRect.Width(), mBumpObjectRect.Height());
        mBumpObjectRect.Inflate(SizeMin, SizeMin);
    }    
    if(IsXBig)
        return Point(0, ((0 < MonsterOX) ^ mIsBumpClock)? -1 : 1);
    return Point(((0 < MonsterOY) ^ mIsBumpClock)? 1 : -1, 0);
}

////////////////////////////////////////////////////////////////////////////////
MapDragon::MapDragon() : MapSpine(ST_Dragon)
{
    mDragonFlipMode = false;
    mAttackDone = false;
    mAttackFinished = false;
    mDragonScale = 1;
    mDragonScaleMax = 1;
    mDragonEntryTimeMsec = 0;
    mDragonBreathBeginTimeMsec = 1;
    mDragonBreathEndTimeMsec = 2;
    mDragonExitTimeMsec = 3;
    mTween = nullptr;
}

MapDragon::~MapDragon()
{
    delete mTween;
}

MapDragon::MapDragon(MapDragon&& rhs)
{
    operator=(ToReference(rhs));
}

MapDragon& MapDragon::operator=(MapDragon&& rhs)
{
    MapSpine::operator=(ToReference(rhs));
    mDragonFlipMode = rhs.mDragonFlipMode;
    mAttackDone = rhs.mAttackDone;
    mAttackFinished = rhs.mAttackFinished;
    mDragonScale = rhs.mDragonScale;
    mDragonScaleMax = rhs.mDragonScaleMax;
    mBreathPosAdd = rhs.mBreathPosAdd;
    mDragonEntryTimeMsec = rhs.mDragonEntryTimeMsec;
    mDragonBreathBeginTimeMsec = rhs.mDragonBreathBeginTimeMsec;
    mDragonBreathEndTimeMsec = rhs.mDragonBreathEndTimeMsec;
    mDragonExitTimeMsec = rhs.mDragonExitTimeMsec;
    delete mTween;
    mTween = rhs.mTween;
    rhs.mTween = nullptr;
    mSpotPos[0] = rhs.mSpotPos[0];
    mSpotPos[1] = rhs.mSpotPos[1];
    mSpotPos[2] = rhs.mSpotPos[2];
    mNewPos = rhs.mNewPos;
    mOldPos = rhs.mOldPos;
    mLastPos = rhs.mLastPos;
    return *this;
}

void MapDragon::Init(const SpineRenderer* renderer, float scaleMax, Updater* updater,
    const Point& homepos, const Point& exitposL, const Point& exitposR)
{
    InitSpine(renderer, "normal").PlayMotion("run", true);
    mDragonScale = scaleMax;
    mDragonScaleMax = scaleMax;
    mTween = new Tween2D(updater);
    mTween->Reset(homepos.x, homepos.y);
    mSpotPos[0] = homepos;
    mSpotPos[1] = exitposL;
    mSpotPos[2] = exitposR;
    mNewPos = mOldPos = mLastPos = homepos;
}

void MapDragon::ResetCB(FXState* state)
{
    if(mSpineInstance)
        ZAY::SpineBuilder::ResetCB(mSpineInstance, nullptr,
            [this, state](chars eventname)
            {
                if(!String::Compare("shot", eventname))
                    mAttackFinished = true;
                else if(!String::Compare("sound_", eventname, 6))
                    Platform::Sound::Play(state->GetSound(&eventname[6]));
            });
}

void MapDragon::GoTarget(const Point& beginpos, const Point& pos, bool isExitRight,
    sint32 entryMsec, sint32 breathMsec, sint32 exitMsec)
{
    mAttackDone = false;
    mAttackFinished = false;
    mDragonEntryTimeMsec = Platform::Utility::CurrentTimeMsec();
    mDragonBreathBeginTimeMsec = mDragonEntryTimeMsec + entryMsec;
    mDragonBreathEndTimeMsec = mDragonBreathBeginTimeMsec + breathMsec;
    mDragonExitTimeMsec = mDragonBreathEndTimeMsec + exitMsec;

    mTween->ResetPathes();
    mTween->Reset(beginpos.x, beginpos.y);
    mTween->MoveTo(pos.x, pos.y, entryMsec / 1000.0f);
    mTween->MoveTo(pos.x, pos.y, breathMsec / 1000.0f);
    if(isExitRight) mTween->MoveTo(mSpotPos[2].x, mSpotPos[2].y, exitMsec / 1000.0f);
    else mTween->MoveTo(mSpotPos[1].x, mSpotPos[1].y, exitMsec / 1000.0f);
}

Point MapDragon::MoveOnce(float curve, Point mouthpos, sint32 breathdelayMsec, bool& attackflag)
{
    const uint64 CurTimeMsec = Platform::Utility::CurrentTimeMsec();

    // 공격애니의 시점
    const uint64 DragonBreathAniTimeMsec = mDragonBreathBeginTimeMsec - breathdelayMsec;
    if(!mAttackDone && DragonBreathAniTimeMsec <= CurTimeMsec)
    {
        mAttackDone = true;
        Attack();
    }

    // 공격애니의 끝시점
    if(mAttackFinished)
    {
        mAttackFinished = false;
        attackflag = true;
    }

    // 방향전환 가능구간
    if(CurTimeMsec < DragonBreathAniTimeMsec || mDragonBreathEndTimeMsec <= CurTimeMsec)
    {
        // X값으로 5픽셀이상 멀어지면 방향성 재판단
        if(5 <= Math::AbsF(mOldPos.x - mNewPos.x))
        {
            const bool FlipResult = (mOldPos.x < mNewPos.x);
            if(mDragonFlipMode != FlipResult)
                mDragonFlipMode = FlipResult;
            mOldPos = mNewPos;
        }
    }
    else mOldPos = mNewPos;

    // 줌과 입위치 결정
    const float EntryRate = CalcEntryRate(CurTimeMsec);
    const float ExitRate = CalcExitRate(CurTimeMsec);
    const float ScaleSeed = 1 - Math::Pow(1 - Math::MinF(EntryRate, 1 - ExitRate));
    const float CalcScale = 1 * ScaleSeed + mDragonScaleMax * (1 - ScaleSeed);
    mDragonScale = mDragonScale * 0.95 + CalcScale * 0.05;

    // 위아래로 곡선운동, 입위치로 다가섬
    const float CurveYSeedARadian = (1 - Math::AbsF(Math::AbsF(EntryRate * 4 - 2) - 1)) * Math::ToRadian(90);
    const float CurveYSeedBRadian = (1 - Math::AbsF(Math::AbsF(ExitRate * 4 - 2) - 1)) * Math::ToRadian(90);
    const float CurveYSeedA = Math::Sin(CurveYSeedARadian) * ((EntryRate < 0.5f)? 1 : -1);
    const float CurveYSeedB = Math::Sin(CurveYSeedBRadian) * ((ExitRate < 0.5f)? 1 : -1);
    const float CalcCurveY = (CurveYSeedA + CurveYSeedB) * curve * mDragonScale;
    mBreathPosAdd.x = mBreathPosAdd.x * 0.95 + ((mDragonFlipMode)? -mouthpos.x : mouthpos.x) * 0.05;
    mBreathPosAdd.y = mBreathPosAdd.y * 0.95 + (mouthpos.y + CalcCurveY) * 0.05;

    mNewPos = Point(mTween->x(), mTween->y());
    mLastPos = mNewPos + mBreathPosAdd * ScaleSeed;
    return mLastPos;
}

float MapDragon::CalcEntryRate(uint64 msec) const
{
    if(mDragonEntryTimeMsec < msec)
    if(sint32 TimeGap = mDragonBreathBeginTimeMsec - mDragonEntryTimeMsec)
        return Math::ClampF((msec - mDragonEntryTimeMsec) / (float) TimeGap, 0, 1);
    return 0;
}

float MapDragon::CalcBreathRate(uint64 msec) const
{
    if(mDragonBreathBeginTimeMsec < msec)
    if(sint32 TimeGap = mDragonBreathEndTimeMsec - mDragonBreathBeginTimeMsec)
        return Math::ClampF((msec - mDragonBreathBeginTimeMsec) / (float) TimeGap, 0, 1);
    return 0;
}

float MapDragon::CalcExitRate(uint64 msec) const
{
    if(mDragonBreathEndTimeMsec < msec)
    if(sint32 TimeGap = mDragonExitTimeMsec - mDragonBreathEndTimeMsec)
        return Math::ClampF((msec - mDragonBreathEndTimeMsec) / (float) TimeGap, 0, 1);
    return 0;
}

void MapDragon::Attack() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "attack", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "attack", "run", true);
    }
}

////////////////////////////////////////////////////////////////////////////////
MapItem::MapItem() : MapSpine(ST_Item)
{
    mSender = nullptr;
    mTween = nullptr;
    mFlyingBeginTimeMsec = 0;
    mFlyingEndTimeMsec = 0;
    mSlotBeginTimeMsec = 0;
    mSlotEndTimeMsec = 0;
    mSlotPos = nullptr;
    mMode = ItemMode::Wait;
}

MapItem::~MapItem()
{
    delete mTween;
}

MapItem::MapItem(MapItem&& rhs)
{
    operator=(ToReference(rhs));
}

MapItem& MapItem::operator=(MapItem&& rhs)
{
    MapSpine::operator=(ToReference(rhs));
    mSender = rhs.mSender;
    rhs.mSender = nullptr;
    delete mTween;
    mTween = rhs.mTween;
    rhs.mTween = nullptr;
    mFlyingBeginTimeMsec = rhs.mFlyingBeginTimeMsec;
    mFlyingEndTimeMsec = rhs.mFlyingEndTimeMsec;
    mSlotBeginTimeMsec = rhs.mSlotBeginTimeMsec;
    mSlotEndTimeMsec = rhs.mSlotEndTimeMsec;
    mSlotPos = rhs.mSlotPos;
    rhs.mSlotPos = nullptr;
    mMode = rhs.mMode;
    return *this;
}

void MapItem::Init(chars skin, const SpineRenderer* renderer, const MapObject* sender, Updater* updater,
    float ypos, sint32 entryMsec, sint32 flyingMsec)
{
    mSkin = skin;
    InitSpine(renderer, skin).PlayMotion("idle", true);
    mSender = sender;
    mTween = new Tween2D(updater);
    auto& Pos = sender->mCurrentRect.Center();
    mTween->Reset(Pos.x, Pos.y);
    mTween->MoveTo(Pos.x, Pos.y, entryMsec / 1000.0f);
    mTween->MoveTo(Pos.x, Pos.y + ypos, flyingMsec / 1000.0f);
    mFlyingBeginTimeMsec = Platform::Utility::CurrentTimeMsec() + entryMsec;
    mFlyingEndTimeMsec = mFlyingBeginTimeMsec + flyingMsec;

    if(mSpineInstance)
        ZAY::SpineBuilder::ResetCB(mSpineInstance,
            [this](chars motionname)
            {
                if(!String::Compare("use", motionname))
                    mMode = ItemMode::Destroy;
            }, nullptr);
}

bool MapItem::AnimationOnce()
{
    if(mMode == ItemMode::Wait)
    {
        if(mFlyingBeginTimeMsec < Platform::Utility::CurrentTimeMsec())
        {
            mMode = ItemMode::Show;
            mSender->Drop();
            Show();
        }
        return true;
    }
    return (mMode != ItemMode::Destroy);
}

void MapItem::MoveToSlot(const Point* pos, sint32 slotMsec)
{
    if(mSpineInstance && mMode == ItemMode::Show)
    {
        mMode = ItemMode::Slot;
        mTween->ResetPathes();
        mTween->MoveTo(pos->x, pos->y, slotMsec / 1000.0f);
        mSlotBeginTimeMsec = Platform::Utility::CurrentTimeMsec();
        mSlotEndTimeMsec = mSlotBeginTimeMsec + slotMsec;
        mSlotPos = pos;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "get", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "get", "slot", true);
    }
}

const Point* MapItem::Use()
{
    if(mSpineInstance && mMode == ItemMode::Slot)
    {
        mMode = ItemMode::Used;
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "use", false);
    }
    return mSlotPos;
}

Point MapItem::CalcPos(uint64 msec) const
{
    if(!mSlotPos || msec < mSlotEndTimeMsec)
        return Point(mTween->x(), mTween->y());
    return *mSlotPos;
}

float MapItem::CalcFlyingRate(uint64 msec) const
{
    if(mFlyingBeginTimeMsec < msec)
    if(sint32 TimeGap = mFlyingEndTimeMsec - mFlyingBeginTimeMsec)
        return Math::ClampF((msec - mFlyingBeginTimeMsec) / (float) TimeGap, 0, 1);
    return 0;
}

float MapItem::CalcSlotRate(uint64 msec) const
{
    if(mSlotBeginTimeMsec < msec)
    if(sint32 TimeGap = mSlotEndTimeMsec - mSlotBeginTimeMsec)
        return Math::ClampF((msec - mSlotBeginTimeMsec) / (float) TimeGap, 0, 1);
    return 0;
}

void MapItem::Show() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAllWithoutSeek(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "show", true);
    }
}

////////////////////////////////////////////////////////////////////////////////
TryWorldZone::TryWorldZone()
{
    mHurdle = nullptr;
    mMap = nullptr;
}

TryWorldZone::~TryWorldZone()
{
    TryWorld::Hurdle::Release(mHurdle);
    TryWorld::Map::Release(mMap);
}

////////////////////////////////////////////////////////////////////////////////
TargetZone::TargetZone()
{
    mObjectIndex = 0;
    mSizeR = 0;
}

TargetZone::~TargetZone()
{
}

TargetZone::TargetZone(TargetZone&& rhs)
{
    operator=(ToReference(rhs));
}

TargetZone& TargetZone::operator=(TargetZone&& rhs)
{
    mObjectIndex = rhs.mObjectIndex;
    mSizeR = rhs.mSizeR;
    return *this;
}

void TargetZone::Init(sint32 objectindex, float r)
{
    mObjectIndex = objectindex;
    mSizeR = r;
}

////////////////////////////////////////////////////////////////////////////////
F1State::F1State() : FXState("f1/"),
    mLandscape(Platform::Option::GetFlag("LandscapeMode")),
    mStage(Platform::Option::GetText("StageName"))
{
    Map<String> GlobalWeightMap;
    if(auto GlobalWeightTable = Context(ST_Json, SO_NeedCopy, String::FromFile("f1/table/globalweight_table.json")))
    {
        for(sint32 i = 0, iend = GlobalWeightTable.LengthOfIndexable(); i < iend; ++i)
        {
            chars CurID = GlobalWeightTable[i]("ID").GetString("noname");
            chars CurValue = GlobalWeightTable[i]("Value").GetString("0");
            GlobalWeightMap(CurID) = CurValue;
        }
    }
    else BOSS_ASSERT("globalweight_table.json의 로딩에 실패하였습니다", false);

    mUILeft.Link("F1", "UILeft", false);
    mUITop.Link("F1", "UITop", false);
    mUIRight.Link("F1", "UIRight", false);
    mUIBottom.Link("F1", "UIBottom", false);
    mUILeft.Parse(GlobalWeightMap("UILeft"));
    mUITop.Parse(GlobalWeightMap("UITop"));
    mUIRight.Parse(GlobalWeightMap("UIRight"));
    mUIBottom.Parse(GlobalWeightMap("UIBottom"));
    mUILeft.Execute();
    mUITop.Execute();
    mUIRight.Execute();
    mUIBottom.Execute();

    mDragonScale = Parser::GetInt(GlobalWeightMap("DragonScale")) / 1000.0f;
    mDragonScaleMax = Parser::GetInt(GlobalWeightMap("DragonScaleMax")) / 1000.0f;
    mDragonCurve = Parser::GetInt(GlobalWeightMap("DragonCurve")) / 1000.0f;
    mDragonMouthX = Parser::GetInt(GlobalWeightMap("DragonMouthX")) / 1000.0f;
    mDragonMouthY = Parser::GetInt(GlobalWeightMap("DragonMouthY")) / 1000.0f;
    mItemScale = Parser::GetInt(GlobalWeightMap("ItemScale")) / 1000.0f;
    mSlotScale = Parser::GetInt(GlobalWeightMap("SlotScale")) / 1000.0f;
    mHoleItemGetCount = Parser::GetInt(GlobalWeightMap("HoleItemGetCount"));
    mBreathScale = Parser::GetInt(GlobalWeightMap("BreathScale")) / 1000.0f;
    mBreathMinDamage = Parser::GetInt(GlobalWeightMap("BreathMinDamage"));
    mBreathMaxDamage = Parser::GetInt(GlobalWeightMap("BreathMaxDamage"));
    mBreathMaxGauge = Parser::GetInt(GlobalWeightMap("BreathMaxGauge"));
    mBreathGaugeChargingPerSec = Parser::GetInt(GlobalWeightMap("BreathGaugeChargingPerSec"));
    mHPbarDeleteTime = Parser::GetInt(GlobalWeightMap("HPbarDeleteTime")); // HP가 다시 투명이되는데 걸리는 시간
    mEggHPRegenValue = Parser::GetInt(GlobalWeightMap("EggHPRegenValue")); // HP의 초당 재생량
    m1StarHpRate = Parser::GetInt(GlobalWeightMap("1StarHpRate"));
    m2StarHpRate = Parser::GetInt(GlobalWeightMap("2StarHpRate"));
    m3StarHpRate = Parser::GetInt(GlobalWeightMap("3StarHpRate"));
    mMonsterScale = Parser::GetInt(GlobalWeightMap("MonsterScale")) / 1000.0f;
    mWallBoundScale = Parser::GetInt(GlobalWeightMap("WallBoundScale")) / 1000.0f;
    mKnockBackMinDistance = Parser::GetInt(GlobalWeightMap("KnockBackMinDistance")) / 1000.0f;
    mKnockBackMaxDistance = Parser::GetInt(GlobalWeightMap("KnockBackMaxDistance")) / 1000.0f;
    mToolGrid = Parser::GetInt(GlobalWeightMap("ToolGrid"));
    mDragonEntryTime = Parser::GetInt(GlobalWeightMap("DragonEntryTime"));
    mDragonRetryTime = Parser::GetInt(GlobalWeightMap("DragonRetryTime"));
    mDragonBreathTime = Parser::GetInt(GlobalWeightMap("DragonBreathTime"));
    mDragonExitTime = Parser::GetInt(GlobalWeightMap("DragonExitTime"));
    m1BoundDamageRate = Parser::GetInt(GlobalWeightMap("1BoundDamage")) / 1000.0f;
    m2BoundDamageRate = Parser::GetInt(GlobalWeightMap("2BoundDamage")) / 1000.0f;
    m3BoundDamageRate = Parser::GetInt(GlobalWeightMap("3BoundDamage")) / 1000.0f;

    auto ObjectTable = Context(ST_Json, SO_NeedCopy, String::FromFile("f1/table/object_table.json"));
    if(ObjectTable.IsValid())
    {
        for(sint32 i = 0, iend = ObjectTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewObjectType = mObjectTypes.AtAdding();
            NewObjectType.mID = ObjectTable[i]("ID").GetString("");
            NewObjectType.mType = ObjectTable[i]("Type").GetString("Null");
            NewObjectType.SetAsset(ObjectTable[i]("Asset").GetString(""));
            NewObjectType.SetSpine(ObjectTable[i]("Spine").GetString(""));
            NewObjectType.SetAssetShadow(ObjectTable[i]("AssetShadow").GetString(""));
            NewObjectType.mHP = ObjectTable[i]("HP").GetInt(0);
            NewObjectType.mGaugePosition = ObjectTable[i]("GaugePosition").GetInt(50);
        }
    }
    else BOSS_ASSERT("object_table.json의 로딩에 실패하였습니다", false);

    auto PolygonTable = Context(ST_Json, SO_NeedCopy, String::FromFile("f1/table/polygon_table.json"));
    if(PolygonTable.IsValid())
    {
        for(sint32 i = 0, iend = PolygonTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewPolygonType = mPolygonTypes.AtAdding();
            NewPolygonType.mID = PolygonTable[i]("ID").GetString("");
            NewPolygonType.mType = PolygonTable[i]("Type").GetString("Null");
            NewPolygonType.mColorR = PolygonTable[i]("Color_R").GetInt(0);
            NewPolygonType.mColorG = PolygonTable[i]("Color_G").GetInt(0);
            NewPolygonType.mColorB = PolygonTable[i]("Color_B").GetInt(0);
        }
    }
    else BOSS_ASSERT("polygon_table.json의 로딩에 실패하였습니다", false);

    auto MonsterTable = Context(ST_Json, SO_NeedCopy, String::FromFile("f1/table/monster_table.json"));
    if(MonsterTable.IsValid())
    {
        for(sint32 i = 0, iend = MonsterTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewMonsterType = mMonsterTypes.AtAdding();
            NewMonsterType.mID = MonsterTable[i]("ID").GetString("");
            NewMonsterType.mType = MonsterTable[i]("Type").GetString("Null");
            NewMonsterType.mHP = MonsterTable[i]("HP").GetInt(100);
            NewMonsterType.mMoveType = MonsterTable[i]("MoveType").GetString("Null");
            NewMonsterType.mMoveSpeed = MonsterTable[i]("MoveSpeed").GetInt(1000);
            NewMonsterType.mTurnDistance = MonsterTable[i]("TurnDistance").GetInt(10);
            NewMonsterType.mAttackPower = MonsterTable[i]("AttackPower").GetInt(100);
            NewMonsterType.mAttackSpeed = MonsterTable[i]("AttackSpeed").GetInt(1000);
            NewMonsterType.mAttackRange = MonsterTable[i]("AttackRange").GetInt(100);
            NewMonsterType.mWaistScaleWidth = MonsterTable[i]("WaistScaleWidth").GetInt(1000);
            NewMonsterType.mWaistScaleHeight = MonsterTable[i]("WaistScaleHeight").GetInt(1000);
            NewMonsterType.mGaugePosition = MonsterTable[i]("GaugePosition").GetInt(50);
            NewMonsterType.SetAsset(MonsterTable[i]("Asset").GetString(""));
            NewMonsterType.SetSpine(MonsterTable[i]("Spine").GetString(""));
            NewMonsterType.mPolygon = MonsterTable[i]("Polygon").GetString("");
            NewMonsterType.mWeight = MonsterTable[i]("Weight").GetInt(1000);
            NewMonsterType.mResistance = MonsterTable[i]("Resistance").GetInt(900);
            NewMonsterType.mWaveStop = !String::Compare("True", MonsterTable[i]("WaveStop").GetString("False"));
        }
    }
    else BOSS_ASSERT("monster_table.json의 로딩에 실패하였습니다", false);

    mUIL = 0;
    mUIT = 0;
    mUIR = 0;
    mUIB = 0;
    mScreenW = 0;
    mScreenH = 0;
    mInGameW = 0;
    mInGameH = 0;
    mInGameX = 0;
    mInGameY = 0;
    mInGameSize = 0;
    mDragonSizeR = 0;
    mBreathSizeR = 0;
    mItemSizeR = 0;
	mSlotSizeR = 0;
    mMonsterSizeR = 0;
    mKnockBackMinV = 0;
    mKnockBackMaxV = 0;

    mObjectLastRID = mObjectRIDBegin;
    mPolygonLastRID = mPolygonRIDBegin;
    mMonsterLastRID = mMonsterRIDBegin;
    mMissionLastRID = mMissionRIDBegin;

    mBGNameA = "";
    mBGNameB = "";
    mLayers.AtDumpingAdded(mLayerLength);
    mShadowSurface = nullptr;
}

F1State::~F1State()
{
    Platform::Graphics::RemoveSurface(mShadowSurface);
}

sint32 F1State::LoadMap(chars json, bool toolmode)
{
    mBGNameA = "";
    mBGNameB = "";
    mTargetsForEnemy.SubtractionAll();
    mTargetsForAlly.SubtractionAll();
    mLayers.SubtractionAll();
    mLayers.AtDumpingAdded(mLayerLength);
    mObjectRIDs.Reset();

    Context JsonLayer(ST_Json, SO_OnlyReference, json);
    const sint32 Result = JsonLayer("BuildNumber").GetInt(0);
    mBGNameA = JsonLayer("BGName").GetString();
    mBGNameB = mBGNameA;
    mBGNameB.Sub(2) += "_b";

    mObjectLastRID = JsonLayer("ObjectLastRID").GetInt(mObjectRIDBegin);
    mPolygonLastRID = JsonLayer("PolygonLastRID").GetInt(mPolygonRIDBegin);

    // Polygons
    for(sint32 layer = 0, layer_end = JsonLayer("Layers").LengthOfIndexable(); layer < layer_end; ++layer)
    {
        auto& CurJsonLayer = JsonLayer("Layers")[layer];
        const sint32 LayerID = CurJsonLayer("LayerID").GetInt();
        if(mLayerLength <= LayerID) continue;
        auto& CurLayer = mLayers.At(LayerID);
        for(sint32 plg = 0, plg_end = CurJsonLayer("Polygons").LengthOfIndexable(); plg < plg_end; ++plg)
        {
            auto& NewPolygon = CurLayer.mPolygons.AtAdding();
            auto& CurJsonPolygon = CurJsonLayer("Polygons")[plg];

            const String CurJsonPolygonID = CurJsonPolygon("ID").GetString();
            NewPolygon.mType = &mPolygonTypes[0];
            for(sint32 i = 0, iend = mPolygonTypes.Count(); i < iend; ++i)
                if(!CurJsonPolygonID.Compare(mPolygonTypes[i].mID))
                {
                    NewPolygon.mType = &mPolygonTypes[i];
                    break;
                }
            if(!(NewPolygon.mRID = CurJsonPolygon("RID").GetInt(0)))
                NewPolygon.mRID = ++mPolygonLastRID;
            NewPolygon.mEnable = (CurJsonPolygon("Visible").GetInt(1) != 0)? true : false;
            NewPolygon.mDots.AtDumpingAdded(CurJsonPolygon("Points").LengthOfIndexable());
            for(sint32 i = 0, iend = NewPolygon.mDots.Count(); i < iend; ++i)
            {
                auto& CurPoint = NewPolygon.mDots.At(i);
                if(mLandscape)
                {
                    CurPoint.x = CurJsonPolygon("Points")[i]("Y").GetFloat();
                    CurPoint.y = -CurJsonPolygon("Points")[i]("X").GetFloat();
                }
                else
                {
                    CurPoint.x = CurJsonPolygon("Points")[i]("X").GetFloat();
                    CurPoint.y = CurJsonPolygon("Points")[i]("Y").GetFloat();
                }
            }
            NewPolygon.UpdateCW();
        }
    }

    // Object생산자
    auto ObjectCreator = [this](MapObject& object, const Context& context, bool remake_spot_hole, bool toolmode)->bool
    {
        const String CurJsonObjectID = context("ID").GetString();
        object.mType = &mObjectTypes[0];
        for(sint32 i = 0, iend = mObjectTypes.Count(); i < iend; ++i)
            if(!CurJsonObjectID.Compare(mObjectTypes[i].mID))
            {
                object.mType = &mObjectTypes[i];
                break;
            }

        bool Remaked = false;
        if(remake_spot_hole)
        if(object.mType->mType == ObjectType::TypeClass::Spot || object.mType->mType == ObjectType::TypeClass::Hole)
            Remaked = true;

        if(Remaked && !toolmode) object.mRID = -1;
        else if(!(object.mRID = context("RID").GetInt(0)))
            object.mRID = ++mObjectLastRID;

        object.mEnable = (context("Visible").GetInt(1) != 0)? true : false;
        object.mHPValue = object.mType->mHP; // HP저장
        object.mHPAni = object.mType->mHP; // HP저장
        if(mLandscape)
        {
            object.mCurrentRect.l = context("RectT").GetFloat();
            object.mCurrentRect.t = -context("RectR").GetFloat();
            object.mCurrentRect.r = context("RectB").GetFloat();
            object.mCurrentRect.b = -context("RectL").GetFloat();
        }
        else
        {
            object.mCurrentRect.l = context("RectL").GetFloat();
            object.mCurrentRect.t = context("RectT").GetFloat();
            object.mCurrentRect.r = context("RectR").GetFloat();
            object.mCurrentRect.b = context("RectB").GetFloat();
        }

        if(!Remaked)
        if(auto CurSpine = GetSpine(object.mType->spineName()))
        {
            object.InitSpine(CurSpine, object.mType->spineSkinName()).PlayMotion("idle", true);
            if(object.mType->mType == ObjectType::TypeClass::Dynamic)
            {
                object.PlayMotionSeek("_state", false);
                object.SetSeekSec(0);
            }
        }
        return (Remaked && !toolmode);
    };

    // Objects
    sint32 CurParaView = 0;
    for(sint32 layer = 0, layer_end = JsonLayer("Layers").LengthOfIndexable(); layer < layer_end; ++layer)
    {
        auto& CurJsonLayer = JsonLayer("Layers")[layer];
        const sint32 LayerID = CurJsonLayer("LayerID").GetInt();
        if(mLayerLength <= LayerID) continue;
        auto& CurLayer = mLayers.At(LayerID);
        for(sint32 obj = 0, obj_end = CurJsonLayer("Objects").LengthOfIndexable(); obj < obj_end; ++obj)
        {
            auto& NewObject = CurLayer.mObjects.AtAdding();
            const sint32 NewObjectIndex = CurLayer.mObjects.Count() - 1;
            MapObject* NewObject1F = nullptr;
            sint32 NewObject1FIndex = 0;
            auto& CurJsonObject = CurJsonLayer("Objects")[obj];
            if(ObjectCreator(NewObject, CurJsonObject, true, toolmode))
            {
                // toolmode가 아닌 경우 Spot과 Hole은 1F에 다시 한번 복사함
                NewObject1F = &mLayers.At(2).mObjects.AtAdding();
                NewObject1FIndex = mLayers.At(2).mObjects.Count() - 1;
                ObjectCreator(*NewObject1F, CurJsonObject, false, false);
            }

            // 개별후처리
            if(toolmode) continue;
            else if(NewObject.mType->mType == ObjectType::TypeClass::Hole && NewObject1F)
            {
                const Rect HoleTestRect = NewObject1F->mCurrentRect.Inflate(0.001f, 0.001f);
                const sint32 ObjectPayload = ((2 & 0xFFFF) << 16) | (NewObject1FIndex & 0xFFFF);
                for(sint32 h = 0; h < mLayerLength; ++h)
                {
                    auto& CurLayerForHole = mLayers.At(h);
                    for(sint32 i = 0, iend = CurLayerForHole.mPolygons.Count(); i < iend; ++i)
                    {
                        auto& CurPolygon = CurLayerForHole.mPolygons.At(i);
                        for(sint32 j = 0, jend = CurPolygon.mDots.Count(); j < jend; ++j)
                        {
                            auto& CurDot = CurPolygon.mDots.At(j);
                            if(HoleTestRect.PtInRect(Point(CurDot.x, CurDot.y)))
                                CurDot.Payload = ObjectPayload;
                        }
                    }
                }
            }
            else
            {
                if(LayerID == 2 && NewObject.mType->mType == ObjectType::TypeClass::Target)
                {
                    auto& NewTarget = mTargetsForEnemy.AtAdding();
                    NewTarget.Init(NewObjectIndex, Math::Sqrt(Math::Pow(NewObject.mCurrentRect.Width()) * 2) / 2);
                }
                else if(LayerID == 2 && NewObject.mType->mType == ObjectType::TypeClass::AllyTarget)
                {
                    auto& NewTarget = mTargetsForAlly.AtAdding();
                    NewTarget.Init(NewObjectIndex, Math::Sqrt(Math::Pow(NewObject.mCurrentRect.Width()) * 2) / 2);
                }
                else if(NewObject.mType->mType == ObjectType::TypeClass::View)
                {
                    // 파라뷰 댓글매칭
                    if(CurParaView < Parser::GetInt(Platform::Option::GetText("ParaViewCount")))
                    {
                        chars CurParaViewText = Platform::Option::GetText(String::Format("ParaViewText_%d", CurParaView++));
                        if(!NewObject.mParaView)
                        {
                            NewObject.mParaView = new ParaSource::View();
                            NewObject.mParaView->Init(CurParaViewText);
                        }
                    }
                }
            }
        }
    }

    // Object전체후처리
    for(sint32 layer = 0, layer_end = mLayers.Count(); layer < layer_end; ++layer)
    {
        auto& CurLayer = mLayers.At(layer);
        for(sint32 obj = 0, obj_end = CurLayer.mObjects.Count(); obj < obj_end; ++obj)
        {
            auto& CurObject = CurLayer.mObjects.At(obj);
            if(CurObject.mRID == -1)
                CurObject.mRID = ++mObjectLastRID;
            CurObject.ResetCB(this);
            mObjectRIDs[CurObject.mRID] = &CurObject;
        }
    }
    return Result;
}

String F1State::SaveMap(const F1Tool* tool)
{
    Context JsonLayer;
    JsonLayer.At("ToolType").Set("maptool_f1");
    JsonLayer.At("ToolVersion").Set(String::FromInteger(tool->ToolVersion));
    JsonLayer.At("BuildNumber").Set(String::FromInteger(tool->mBuildNumber + 1));
    JsonLayer.At("BGName").Set(mBGNameA);
    JsonLayer.At("ObjectLastRID").Set(String::FromInteger(mObjectLastRID));
    JsonLayer.At("PolygonLastRID").Set(String::FromInteger(mPolygonLastRID));
    for(sint32 layer = 0; layer < mLayerLength; ++layer)
    {
        auto& CurLayer = mLayers.At(layer);
        if(CurLayer.mObjects.Count() == 0 && CurLayer.mPolygons.Count() == 0)
            continue;
        auto& NewJsonLayer = JsonLayer.At("Layers").At(layer);
        NewJsonLayer.At("LayerID").Set(String::FromInteger(layer));

        // Polygons
        for(sint32 plg = 0, plg_end = CurLayer.mPolygons.Count(); plg < plg_end; ++plg)
        {
            auto& CurPolygon = CurLayer.mPolygons[plg];
            if(CurPolygon.mDots.Count() < 2) continue;
            auto& NewJsonPolygon = NewJsonLayer.At("Polygons").At(plg);

            NewJsonPolygon.At("ID").Set(CurPolygon.mType->mID);
            NewJsonPolygon.At("RID").Set(String::FromInteger(CurPolygon.mRID));
            NewJsonPolygon.At("Visible").Set((CurPolygon.mEnable)? "1" : "0");
            for(sint32 i = 0, iend = CurPolygon.mDots.Count(); i < iend; ++i)
            {
                auto& NewJsonPoint = NewJsonPolygon.At("Points").At(i);
                if(mLandscape)
                {
                    NewJsonPoint.At("X").Set(String::FromFloat(CurPolygon.mDots[i].y));
                    NewJsonPoint.At("Y").Set(String::FromFloat(-CurPolygon.mDots[i].x));
                }
                else
                {
                    NewJsonPoint.At("X").Set(String::FromFloat(CurPolygon.mDots[i].x));
                    NewJsonPoint.At("Y").Set(String::FromFloat(CurPolygon.mDots[i].y));
                }
            }
        }

        // Objects
        for(sint32 obj = 0, obj_end = CurLayer.mObjects.Count(); obj < obj_end; ++obj)
        {
            auto& CurObject = CurLayer.mObjects[obj];
            auto& NewJsonObject = NewJsonLayer.At("Objects").At(obj);

            NewJsonObject.At("ID").Set(CurObject.mType->mID);
            NewJsonObject.At("RID").Set(String::FromInteger(CurObject.mRID));
            NewJsonObject.At("Visible").Set((CurObject.mEnable)? "1" : "0");
            if(mLandscape)
            {
                NewJsonObject.At("RectL").Set(String::FromFloat(CurObject.mCurrentRect.t));
                NewJsonObject.At("RectT").Set(String::FromFloat(-CurObject.mCurrentRect.r));
                NewJsonObject.At("RectR").Set(String::FromFloat(CurObject.mCurrentRect.b));
                NewJsonObject.At("RectB").Set(String::FromFloat(-CurObject.mCurrentRect.l));
            }
            else
            {
                NewJsonObject.At("RectL").Set(String::FromFloat(CurObject.mCurrentRect.l));
                NewJsonObject.At("RectT").Set(String::FromFloat(CurObject.mCurrentRect.t));
                NewJsonObject.At("RectR").Set(String::FromFloat(CurObject.mCurrentRect.r));
                NewJsonObject.At("RectB").Set(String::FromFloat(CurObject.mCurrentRect.b));
            }
        }
    }
    return JsonLayer.SaveJson();
}

void F1State::RebuildTryWorld()
{
    mAllTryWorldZones.Reset();
    for(sint32 i = 0, iend = mPolygonTypes.Count(); i < iend; ++i)
        mAllTryWorldZones(mPolygonTypes[i].mID).mHurdle = TryWorld::Hurdle::Create();

    for(sint32 i = 0; i < mLayerLength; ++i)
    {
        auto& CurPolygons = mLayers[i].mPolygons;
        for(sint32 j = 0, jend = CurPolygons.Count(); j < jend; ++j)
        {
            if(CurPolygons[j].mType->mType != PolygonType::TypeClass::Wall)
                continue;
            if(auto* CurTryWorldZone = mAllTryWorldZones.Access(CurPolygons[j].mType->mID))
            {
                auto& CurPoints = CurPolygons[j].mDots;
                TryWorld::DotList NewPoints;
                NewPoints.AtDumpingAdded(CurPoints.Count());
                for(sint32 k = 0, kend = CurPoints.Count(); k < kend; ++k)
                {
                    const float x = mInGameW * (CurPoints[k].x + 0.5f);
                    const float y = mInGameH * (CurPoints[k].y + 0.5f);
                    NewPoints.At(k) = TryWorld::Dot(x, y, CurPoints[k].Payload);
                }
                CurTryWorldZone->mHurdle->Add(NewPoints);
            }
        }
    }

    for(sint32 i = 0, iend = mAllTryWorldZones.Count(); i < iend; ++i)
        if(auto* CurTryWorldZone = mAllTryWorldZones.AccessByOrder(i))
            CurTryWorldZone->mMap = CurTryWorldZone->mHurdle->
                BuildMap(Rect(0, 0, mScreenW, mScreenH) - Point(mInGameX, mInGameY));
}

void F1State::SetSize(sint32 width, sint32 height)
{
    Solver ScreenWidth;
    Solver ScreenHeight;
    ScreenWidth.Link("F1", "ScreenWidth", false);
    ScreenHeight.Link("F1", "ScreenHeight", false);
    ScreenWidth.Parse(String::FromInteger(width));
    ScreenHeight.Parse(String::FromInteger(height));
    ScreenWidth.Execute();
    ScreenHeight.Execute();

    // 리사이징
    mUIL = (sint32) mUILeft.result();
    mUIT = (sint32) mUITop.result();
    mUIR = (sint32) mUIRight.result();
    mUIB = (sint32) mUIBottom.result();
    const sint32 ViewWidth = Math::Max(0, width - (mUIL + mUIR));
    const sint32 ViewHeight = Math::Max(0, height - (mUIT + mUIB));
    const float CurRate = ViewWidth / (float) ViewHeight;
    const float ViewRate = (mLandscape)? 6000.0f / 5000.0f : 5000.0f / 6000.0f;
    mScreenW = width;
    mScreenH = height;
    mInGameW = (sint32) (ViewWidth * ((CurRate < ViewRate)? 1 : ViewRate / CurRate));
    mInGameH = (sint32) (ViewHeight * ((CurRate < ViewRate)? CurRate / ViewRate : 1));
    mInGameX = mUIL + (ViewWidth - mInGameW) / 2;
    mInGameY = mUIT + (ViewHeight - mInGameH) / 2;
    mInGameSize = (mLandscape)? mInGameH : mInGameW;
    mDragonHome = Point(mInGameX + mInGameW / 2, mInGameY + mInGameH * 2);
    mDragonExitL = Point(mInGameX - mInGameW * 3 / 2, mInGameY - mInGameH);
    mDragonExitR = Point(mInGameX + mInGameW * 5 / 2, mInGameY - mInGameH);
    mDragonSizeR = mInGameSize * mDragonScale / 2;
    mBreathSizeR = mInGameSize * mBreathScale / 2;
    mItemSizeR = mInGameSize * mItemScale / 2;
    mSlotSizeR = mInGameSize * mSlotScale / 2;
    mMonsterSizeR = mInGameSize * mMonsterScale / 2;
    mWallBoundSizeR = mInGameSize * mWallBoundScale / 2;
    mKnockBackMinV = mInGameSize * mKnockBackMinDistance;
    mKnockBackMaxV = mInGameSize * mKnockBackMaxDistance;
}

void F1State::RenderImage(bool editmode, ZayPanel& panel, const Image& image)
{
    const sint32 Width = image.GetWidth();
    const sint32 Height = image.GetHeight();
    const float RateR = Math::MinF(panel.w() / Width, panel.h() / Height) / 2;
    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, Width * RateR, Height * RateR)
    {
        panel.stretch(image, true);
        if(editmode)
        ZAY_RGBA(panel, 255, 0, 0, 64)
            panel.fill();
    }

    if(editmode)
    ZAY_RGBA(panel, 255, 0, 0, 128)
        panel.rect(1);
}

void F1State::RenderLayer(bool editmode, ZayPanel& panel, const MapLayer& layer, const MapMonsters* monsters, const sint32 wavesec)
{
    class OrderNode : public OrderUpdater
    {
    public:
        OrderNode() {mIsMonster = false; mData = nullptr;}
        OrderNode(const OrderNode& rhs) {operator=(rhs);}
        ~OrderNode() override {}
        OrderNode& operator=(const OrderNode& rhs)
        {
            Updater::operator=(rhs);
            mIsMonster = rhs.mIsMonster;
            mData = rhs.mData;
            mRect = rhs.mRect;
            return *this;
        }

    public:
        static bool Sorter(Updater* a, Updater* b)
        {
            if(((OrderNode*) a)->mRect.t < ((OrderNode*) b)->mRect.t)
                return false;
            if(((OrderNode*) a)->mRect.t == ((OrderNode*) b)->mRect.t)
            if(((OrderNode*) a)->mRect.l < ((OrderNode*) b)->mRect.l)
                return false;
            return true;
        }

    public:
        bool mIsMonster;
        const void* mData;
        Rect mRect;
    };

    // 순서정렬
    OrderUpdater Head;
    static Array<OrderNode> Nodes;
    Nodes.SubtractionAll();

    // 몬스터 수집
    if(monsters)
    for(sint32 i = 0, iend = monsters->Count(); i < iend; ++i)
    {
        if(wavesec < (*monsters)[i].mEntranceSec) continue;
        if((*monsters)[i].mDeathStep < 2)
        {
            if((*monsters)[i].renderer())
            {
                // 선행 업데이트
                if((*monsters)[i].IsSeekUpdated())
                    (*monsters)[i].Seek();
                (*monsters)[i].Update();

                auto& NewNode = Nodes.AtAdding();
                NewNode.mIsMonster = true;
                NewNode.mData = &(*monsters)[i];
                NewNode.mRect.l = mInGameW * ((*monsters)[i].mCurrentPos.x + 0.5f) - mMonsterSizeR;
                NewNode.mRect.t = mInGameH * ((*monsters)[i].mCurrentPos.y + 0.5f) - mMonsterSizeR;
                NewNode.mRect.r = NewNode.mRect.l + mMonsterSizeR * 2;
                NewNode.mRect.b = NewNode.mRect.t + mMonsterSizeR * 2;
                NewNode.BindUpdater(&Head);
                Head.Sort(&NewNode, OrderNode::Sorter);
            }
        }
    }

    // 오브젝트 수집
    auto& CurObjects = layer.mObjects;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        auto& NewNode = Nodes.AtAdding();
        NewNode.mIsMonster = false;
        NewNode.mData = &CurObjects[i];
        NewNode.mRect.l = mInGameW * (CurObjects[i].mCurrentRect.l + 0.5f);
        NewNode.mRect.t = mInGameH * (CurObjects[i].mCurrentRect.t + 0.5f);
        NewNode.mRect.r = mInGameW * (CurObjects[i].mCurrentRect.r + 0.5f);
        NewNode.mRect.b = mInGameH * (CurObjects[i].mCurrentRect.b + 0.5f);
        NewNode.BindUpdater(&Head);
        Head.Sort(&NewNode, OrderNode::Sorter);
    }

    // 몬스터가 존재하는 레이어일 경우 그림자랜더링
    if(monsters)
    {
        if(!mShadowSurface
            || Platform::Graphics::GetSurfaceWidth(mShadowSurface) != mScreenW
            || Platform::Graphics::GetSurfaceHeight(mShadowSurface) != mScreenH)
        {
            Platform::Graphics::RemoveSurface(mShadowSurface);
            mShadowSurface = Platform::Graphics::CreateSurface(mScreenW, mScreenH);
        }
        const Point XY = panel.toview(0, 0);
		const float SX = XY.x * panel.zoom();
        const float SY = XY.y * panel.zoom();
        const float SW = panel.w() * panel.zoom();
        const float SH = panel.h() * panel.zoom();
        ZAY_MAKE_SUB(panel, mShadowSurface)
        {
            #if BOSS_WINDOWS || BOSS_LINUX //bx: 임시방편, 차후수정요망
                panel.erase();
            #else
                ZAY_RGBA(panel, 160, 160, 160, 255)
                    panel.fill();
            #endif
            ZAY_XYWH(panel, SX, SY, SW, SH)
            {
                OrderUpdater* CurNode = &Head;
                while((CurNode = CurNode->Next()) != &Head)
                {
                    OrderNode* CurOrderNode = (OrderNode*) CurNode;
                    ZAY_RECT(panel, CurOrderNode->mRect)
                    {
                        if(CurOrderNode->mIsMonster)
                        {
                            const MapMonster* CurMonster = (const MapMonster*) CurOrderNode->mData;
                            if(CurMonster->renderer())
                                CurMonster->RenderObjectShadow(panel, CurMonster->mFlipMode);
                        }
                        else
                        {
                            const MapObject* CurObject = (const MapObject*) CurOrderNode->mData;
                            if(CurObject->mEnable && 0 < CurObject->mType->mAssetShadow.Length())
                                RenderImage(editmode, panel, R(CurObject->mType->mAssetShadow));
                        }
                    }
                }
            }
        }
        ZAY_XYWH(panel, -XY.x, -XY.y, Platform::Graphics::GetSurfaceWidth(mShadowSurface), Platform::Graphics::GetSurfaceHeight(mShadowSurface))
        ZAY_RGBA(panel, 128, 128, 128, 32)
            panel.sub("shadow", mShadowSurface);
    }

    // 몬스터와 오브젝트
    OrderUpdater* CurNode = &Head;
    while((CurNode = CurNode->Next()) != &Head)
    {
        OrderNode* CurOrderNode = (OrderNode*) CurNode;
        ZAY_RECT(panel, CurOrderNode->mRect)
        {
            if(CurOrderNode->mIsMonster)
            {
                const MapMonster* CurMonster = (const MapMonster*) CurOrderNode->mData;
                if(CurMonster->renderer())
                {
                    CurMonster->RenderObject(false, editmode, panel, CurMonster->mFlipMode);
                    // 몬스터 토스트
                    if(CurMonster->mToast.renderer() && CurMonster->mToast.enabled())
                    if(const Rect* AreaRect = CurMonster->GetBoundRect("area"))
                    if(const Rect* ToastRect = CurMonster->GetBoundRect("toast"))
                    {
                        const float Width = AreaRect->Width();
                        const float Height = AreaRect->Height();
                        const float Rate = Math::MinF(panel.w() / Width, panel.h() / Height);
                        const float CX = (CurMonster->mFlipMode)? -AreaRect->CenterX() : AreaRect->CenterX();
                        const float CY = AreaRect->CenterY();
                        BOSS::Rect NewRect;
                        if(CurMonster->mFlipMode)
                        {
                            NewRect.l = panel.w() / 2 + (-ToastRect->r - CX) * Rate;
                            NewRect.t = panel.h() / 2 + (ToastRect->t - CY) * Rate;
                            NewRect.r = panel.w() / 2 + (-ToastRect->l - CX) * Rate;
                            NewRect.b = panel.h() / 2 + (ToastRect->b - CY) * Rate;
                        }
                        else
                        {
                            NewRect.l = panel.w() / 2 + (ToastRect->l - CX) * Rate;
                            NewRect.t = panel.h() / 2 + (ToastRect->t - CY) * Rate;
                            NewRect.r = panel.w() / 2 + (ToastRect->r - CX) * Rate;
                            NewRect.b = panel.h() / 2 + (ToastRect->b - CY) * Rate;
                        }
                        ZAY_RECT(panel, NewRect)
                            CurMonster->mToast.RenderObject(true, false, panel, false, nullptr, nullptr,
                                ZAY_RENDER_PN(p, n, CurMonster)
                                {
                                    if(!String::Compare(n, "paratalk_area"))
                                    {
                                        ZAY_RGB(p, 0, 0, 0)
                                        ZAY_FONT(p, p.h() / 20)
                                            p.text(CurMonster->mParaTalk, UIFA_CenterMiddle, UIFE_Right);
                                    }
                                });
                    }
                }
                else RenderImage(editmode, panel, R(CurMonster->mType->imageName()));
            }
            else
            {
                const MapObject* CurObject = (const MapObject*) CurOrderNode->mData;
                if(CurObject->renderer())
                {
                    if(CurObject->mType->mType == ObjectType::TypeClass::View)
                        CurObject->RenderObject(true, editmode, panel, false, nullptr, nullptr,
                            ZAY_RENDER_PN(p, n, CurObject)
                            {
                                if(CurObject->mParaView)
                                if(!String::Compare(n, "paraboard_area"))
                                {
                                    ZAY_COLOR_CLEAR(p)
                                        CurObject->mParaView->GetRenderer()(p, nullptr);
                                }
                            });
                    else CurObject->RenderObject(true, editmode, panel, false);
                }
                else if(CurObject->mEnable)
                    RenderImage(editmode, panel, R(CurObject->mType->imageName()));
            }
        }
    }

    // 몬스터와 오브젝트의 게이지
    CurNode = &Head;
    while((CurNode = CurNode->Next()) != &Head)
    {
        OrderNode* CurOrderNode = (OrderNode*) CurNode;
        ZAY_RECT(panel, CurOrderNode->mRect)
        {
            if(CurOrderNode->mIsMonster)
            {
                const MapMonster* CurMonster = (const MapMonster*) CurOrderNode->mData;
                // 몬스터 게이지
                if(CurMonster->mDeathStep < 2 && Platform::Utility::CurrentTimeMsec() < CurMonster->mHPTimeMsec)
                {
                    const sint32 GaugePos = mInGameSize * CurMonster->mType->mGaugePosition / 1000;
                    const sint32 GaugeSizeWR = Math::Max(10, mInGameSize * (30 + CurMonster->mType->mHP / 15) / 1000);
                    const sint32 GaugeSizeHR = Math::Max(4, mInGameSize * 12 / 1000);
                    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2 - GaugePos, GaugeSizeWR, GaugeSizeHR)
                    {
                        if(panel.ninepatch(R("mob_gauge_body")) == haschild_ok)
                        ZAY_CHILD(panel)
                        {
                            ZAY_XYWH(panel, 0, 0, panel.w() * CurMonster->mHPAni / CurMonster->mType->mHP, panel.h())
                                panel.ninepatch(R("mob_gauge_log"));
                            ZAY_XYWH(panel, 0, 0, panel.w() * CurMonster->mHPValue / CurMonster->mType->mHP, panel.h())
                                panel.ninepatch(R("mob_gauge_phy"));
                        }
                    }
                    // 게이지 애니메이션
                    CurMonster->mHPAni = (CurMonster->mHPAni * 9 + CurMonster->mHPValue * 1) / 10;
                }
                else CurMonster->mHPAni = CurMonster->mHPValue;
            }
            else
            {
                const MapObject* CurObject = (const MapObject*) CurOrderNode->mData;
                if(CurObject->mEnable)
                {
                    // 오브젝트 게이지
                    if((CurObject->mType->mType == ObjectType::TypeClass::Dynamic
                        || (CurObject->mType->mType == ObjectType::TypeClass::Target && 0 < CurObject->mHPValue))
                        && Platform::Utility::CurrentTimeMsec() < CurObject->mHPTimeMsec)
                    {
                        const sint32 GaugePos = mInGameSize * CurObject->mType->mGaugePosition / 1000;
                        const sint32 GaugeSizeWR = Math::Max(10, mInGameSize * (20 + CurObject->mType->mHP / 50) / 1000);
                        const sint32 GaugeSizeHR = Math::Max(4, mInGameSize * 12 / 1000);
                        ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2 - GaugePos, GaugeSizeWR, GaugeSizeHR)
                        {
                            if(panel.ninepatch(R("object_gauge_body")) == haschild_ok)
                            ZAY_CHILD(panel)
                            {
                                ZAY_XYWH(panel, 0, 0, panel.w() * CurObject->mHPAni / CurObject->mType->mHP, panel.h())
                                    panel.ninepatch(R("object_gauge_log"));
                                ZAY_XYWH(panel, 0, 0, panel.w() * CurObject->mHPValue / CurObject->mType->mHP, panel.h())
                                    panel.ninepatch(R("object_gauge_phy"));
                            }
                        }
                        // 게이지 애니메이션
                        CurObject->mHPAni = (CurObject->mHPAni * 9 + CurObject->mHPValue * 1) / 10;
                    }
                    else CurObject->mHPAni = CurObject->mHPValue;
                }
            }
        }
    }

    // 폴리곤
    if(editmode)
    {
        auto& CurPolygons = layer.mPolygons;
        for(sint32 i = 0, iend = CurPolygons.Count(); i < iend; ++i)
        {
            auto& CurPoints = CurPolygons[i].mDots;
            Points NewPoints;
            sint32s NewPayloads;
            NewPoints.AtDumpingAdded(CurPoints.Count() + (!CurPolygons[i].mIsCW));
            NewPayloads.AtDumpingAdded(CurPoints.Count());
            for(sint32 j = 0, jend = CurPoints.Count(); j < jend; ++j)
            {
                const float x = mInGameW * (CurPoints[j].x + 0.5f);
                const float y = mInGameH * (CurPoints[j].y + 0.5f);
                NewPoints.At(j) = Point(x, y);
                NewPayloads.At(j) = CurPoints[j].Payload;
            }
            if(!CurPolygons[i].mIsCW)
                NewPoints.At(-1) = NewPoints.At(0);

            if(CurPolygons[i].mEnable)
            {
                const sint32 ColorR = CurPolygons[i].mType->mColorR;
                const sint32 ColorG = CurPolygons[i].mType->mColorG;
                const sint32 ColorB = CurPolygons[i].mType->mColorB;
                ZAY_RGBA(panel, ColorR, ColorG, ColorB, 128)
                {
                    if(CurPolygons[i].mIsCW)
                        panel.polygon(NewPoints);
                    else ZAY_RGBA_IF(panel, 64, 192, 64, 128, CurPolygons[i].mDots.Count() < 3)
                        panel.polyline(NewPoints, 4);
                    // 오브젝트가 링크된 선분
                    ZAY_RGBA(panel, 64, 64, 192, 128)
                    for(sint32 j = 0, jend = CurPoints.Count(); j < jend; ++j)
                    {
                        const sint32 jNext = (j + 1) % jend;
                        if(NewPayloads[j] != -1 && NewPayloads[j] == NewPayloads[jNext])
                            panel.line(NewPoints[j], NewPoints[jNext], 4);
                    }
                }
            }
            else
            {
                ZAY_RGBA(panel, 64, 64, 64, 64)
                {
                    if(CurPolygons[i].mIsCW)
                        panel.polygon(NewPoints);
                    else panel.polyline(NewPoints, 4);
                }
            }
        }
    }

    // 몬스터와 오브젝트의 정보
    if(editmode && false)
    {
        CurNode = &Head;
        ZAY_FONT(panel, 0.8)
        while((CurNode = CurNode->Next()) != &Head)
        {
            OrderNode* CurOrderNode = (OrderNode*) CurNode;
            ZAY_RECT(panel, CurOrderNode->mRect)
            {
                if(CurOrderNode->mIsMonster)
                {
                    const MapMonster* CurMonster = (const MapMonster*) CurOrderNode->mData;
                    // 몬스터HP
                    const String HP = String::Format("HP-%d", CurMonster->mHPValue);
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, HP, UIFA_CenterTop);
                    ZAY_RGB(panel, 255, 255, 255)
                        panel.text(panel.w() / 2, panel.h() / 2, HP, UIFA_CenterTop);
                }
                else
                {
                    const MapObject* CurObject = (const MapObject*) CurOrderNode->mData;
                    if(CurObject->mEnable)
                    {
                        if(CurObject->mType->mType == ObjectType::TypeClass::Dynamic)
                        {
                            // 오브젝트HP
                            const String HP = String::Format("HP-%d", CurObject->mHPValue);
                            ZAY_RGB(panel, 0, 0, 0)
                                panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, HP, UIFA_CenterTop);
                            ZAY_RGB(panel, 255, 255, 255)
                                panel.text(panel.w() / 2, panel.h() / 2, HP, UIFA_CenterTop);
                        }
                        else // 타입ID
                            ZAY_RGBA(panel, 255, 255, 0, 64)
                                panel.text(panel.w() / 2, panel.h() / 2, CurObject->mType->mID, UIFA_CenterMiddle);
                    }
                    else
                    {
                        ZAY_RGBA(panel, 64, 64, 64, 64)
                            panel.rect(2);
                        ZAY_RGBA(panel, 64, 64, 64, 128)
                            panel.text(panel.w() / 2, panel.h() / 2, CurObject->mType->mID, UIFA_CenterMiddle);
                    }
                }
            }
        }
    }
}

Rect F1State::RenderMap(bool editmode, ZayPanel& panel, const MapMonsters* monsters, sint32 wavesec)
{
    // 배경
    const Image& BGImage = R((mLandscape)? (chars) (mBGNameA + "w") : (chars) mBGNameA);
    panel.stretch(BGImage, true);

    // 레이어
    ZAY_FONT(panel, 1.1, "Arial Black")
    for(sint32 i = 0; i < mLayerLength; ++i)
    {
        if(!mLayers[i].mShowFlag) continue;
        if(i == 2) RenderBreathArea(panel);
        RenderLayer(editmode, panel, mLayers[i], (i == 2)? monsters : nullptr, wavesec);
    }

    // 타겟정보
    if(editmode)
    for(sint32 i = 0, iend = mTargetsForEnemy.Count(); i < iend; ++i)
    {
        auto& CurObject = mLayers[2].mObjects[mTargetsForEnemy[i].mObjectIndex];
        const float x = mInGameW * (CurObject.mCurrentRect.CenterX() + 0.5f);
        const float y = mInGameH * (CurObject.mCurrentRect.CenterY() + 0.5f);
        ZAY_XYRR(panel, x, y, 0, 0)
        ZAY_FONT(panel, 1.5, "Arial Black")
        {
            const String Text = String::Format("%d", CurObject.mHPValue);
            ZAY_RGBA(panel, 0, 0, 0, -128)
                panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, Text, UIFA_CenterMiddle);
            ZAY_RGB(panel, 64, 255, 0)
                panel.text(panel.w() / 2, panel.h() / 2, Text, UIFA_CenterMiddle);
        }
    }

    // 배경(뚜껑)
    const R BGResource((mLandscape)? (chars) (mBGNameB + "w") : (chars) mBGNameB);
    if(BGResource.exist()) panel.stretch((const Image&) BGResource, true);

    const float XRate = panel.w() / BGImage.GetWidth();
    const float YRate = panel.h() / BGImage.GetHeight();
    return Rect(Point(-BGImage.L() * XRate, -BGImage.T() * YRate),
        Size(BGImage.GetImageWidth() * XRate, BGImage.GetImageHeight() * YRate));
}

void F1State::RenderCap(ZayPanel& panel, const Rect outline)
{
    // 배경의 아웃라인
    if(0 < outline.l || 0 < outline.t || outline.r < mScreenW || outline.b < mScreenH)
    {
        const Point XY = panel.toview(0, 0);
        const float SX = XY.x * panel.zoom();
        const float SY = XY.y * panel.zoom();
        const sint32 OutlineL = Math::Max(0, outline.l) - SX;
        const sint32 OutlineT = Math::Max(0, outline.t) - SY;
        const sint32 OutlineR = Math::Min(outline.r, mScreenW) - SX;
        const sint32 OutlineB = Math::Min(outline.b, mScreenH) - SY;
        ZAY_RGB(panel, 0, 0, 0)
        {
            if(-SY < OutlineT) // Top
            ZAY_LTRB(panel, -SX, -SY, mScreenW - SX, OutlineT) panel.fill();
            if(OutlineB < mScreenH - SY) // Bottom
            ZAY_LTRB(panel, -SX, OutlineB, mScreenW - SX, mScreenH - SY) panel.fill();
            if(-SX < OutlineL) // Left
            ZAY_LTRB(panel, -SX, OutlineT, OutlineL, OutlineB) panel.fill();
            if(OutlineR < mScreenW - SX) // Right
            ZAY_LTRB(panel, OutlineR, OutlineT, mScreenW - SX, OutlineB) panel.fill();
        }
        // 테두리
        ZAY_LTRB(panel, OutlineL, OutlineT, OutlineR, OutlineB)
            panel.stretch(R("black_aera"), true);
    }
}

void F1State::RenderDebug(ZayPanel& panel, const MapMonsters& monsters, sint32 wavesec)
{
    // 길찾기 정보
    static sint32 AniValue = 0;
    static uint64 OldValue = Platform::Utility::CurrentTimeMsec() / 3000;
    const uint64 CurValue = Platform::Utility::CurrentTimeMsec() / 3000;
    AniValue += CurValue - OldValue;
    OldValue = CurValue;

    if(const sint32 MonsterCount = monsters.Count())
    {
        // 디버깅포커스
        const sint32 iSelectBegin = AniValue % MonsterCount;
        sint32 iSelect = iSelectBegin;
        while(wavesec < monsters[iSelect].mEntranceSec)
        {
            AniValue++;
            iSelect = AniValue % MonsterCount;
            if(iSelect == iSelectBegin)
            {
                iSelect = -1;
                break;
            }
        }

        if(iSelect != -1)
        {
            const auto& CurMonster = monsters[iSelect];
            // 삼각분할결과
            if(auto* CurTryWorldZone = mAllTryWorldZones.Access(CurMonster.mType->mPolygon))
            {
                ZAY_RGBA(panel, 0, 0, 255, 64)
                for(sint32 i = 0, iend = CurTryWorldZone->mMap->Lines.Count(); i < iend; ++i)
                {
                    auto& DotA = CurTryWorldZone->mMap->Dots[CurTryWorldZone->mMap->Lines[i].DotA];
                    auto& DotB = CurTryWorldZone->mMap->Dots[CurTryWorldZone->mMap->Lines[i].DotB];
                    panel.line(Point(DotA.x, DotA.y), Point(DotB.x, DotB.y), 1);
                }
            }
            // 길정보
            if(CurMonster.mDeathStep < 2 && 0 < CurMonster.mTargets.Count())
            {
                if(auto CurPath = CurMonster.mTargets[0].mPath)
                if(0 < CurPath->Dots.Count())
                {
                    auto OldPoint = CurPath->Dots[0];
                    for(sint32 j = 1, jend = CurPath->Dots.Count(); j < jend; ++j)
                    {
                        auto& NewPoint = CurPath->Dots[j];
                        ZAY_RGBA(panel, 255, 0, 255, 128)
                        {
                            if(j & 1) panel.line(Point(OldPoint.x, OldPoint.y), Point(NewPoint.x, NewPoint.y), 2);
                            if(jend - 1 - j == CurPath->DotFocus)
                            ZAY_RGB(panel, 255, 0, 0)
                            ZAY_XYRR(panel, NewPoint.x, NewPoint.y, 4, 4)
                                panel.circle();
                        }
                        OldPoint = NewPoint;
                    }
                }
            }
        }
    }

    // 게임영역 표시
    ZAY_RGB(panel, 255, 255, 128)
        panel.rect(1);
}

////////////////////////////////////////////////////////////////////////////////
F1Tool::F1Tool() : mUITween(updater())
{
    mBuildNumber = 0;
    mCursorInWindow = false;
    mLockedUI = true;
    mGridMode = true;
    mSelectMode = false;
    mUITween.Reset(100);
}

F1Tool::~F1Tool()
{
}

void F1Tool::Command(CommandType type, id_share in)
{
    if(type == CT_Tick)
    {
        bool PosInRect = mLockedUI;
        if(!PosInRect)
        {
            point64 CursorPos;
            Platform::Utility::GetCursorPos(CursorPos);
            rect128 WindowRect;
            Platform::GetWindowRect(WindowRect);
            const bool XInRect = (WindowRect.l <= CursorPos.x && CursorPos.x < WindowRect.r);
            const bool YInRect = (WindowRect.t <= CursorPos.y && CursorPos.y < WindowRect.b);
            PosInRect = (XInRect & YInRect);
        }
        if(mCursorInWindow != PosInRect)
        {
            mCursorInWindow = PosInRect;
            mUITween.MoveTo((PosInRect)? 0 : 100, 0.5);
        }
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        mState.SetSize(Width, Height);
    }
}

Point F1Tool::GestureToPos(sint32 x, sint32 y)
{
    float fx = ((x - mMapPos.x) - mState.mInGameX) / mState.mInGameW - 0.5f;
    float fy = ((y - mMapPos.y) - mState.mInGameY) / mState.mInGameH - 0.5f;
    if(mGridMode)
        return AttachGrid(fx, fy);
    return Point(fx, fy);
}

Point F1Tool::AttachGrid(float fx, float fy)
{
    const float fxmax = (mState.mLandscape)? 1000 * mState.mInGameW / mState.mInGameH : 1000;
    const float fymax = (mState.mLandscape)? 1000 : 1000 * mState.mInGameH / mState.mInGameW;
    sint32 nx = 0, ny = 0;
    if(0 <= fx) nx = (((sint32) (fx * fxmax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid;
    else nx = -((((sint32) (-fx * fxmax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid);
    if(0 <= fy) ny = (((sint32) (fy * fymax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid;
    else ny = -((((sint32) (-fy * fymax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid);
    return Point(nx / fxmax, ny / fymax);
}

void F1Tool::RenderGrid(ZayPanel& panel)
{
    const sint32 WidthSize = (mState.mLandscape)? 600 : 500;
    const sint32 HeightSize = (mState.mLandscape)? 500 : 600;
    const float GridSize = (mState.mLandscape)? panel.h() / 1000 : panel.w() / 1000;
    // 중간 -> 좌측
    for(sint32 x = WidthSize - mState.mToolGrid, ix = 0; 0 < (ix = x * GridSize); x -= mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, ix - 1, 0, 1, panel.h())
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_XYWH(panel, ix, 0, 1, panel.h())
            panel.fill();
    }
    // 중간 -> 우측
    for(sint32 x = WidthSize, ix = 0; (ix = x * GridSize) < panel.w(); x += mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, ix - 1, 0, 1, panel.h())
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_RGBA_IF(panel, 128, 128, 192, 192, x == WidthSize)
        ZAY_XYWH(panel, ix, 0, 1, panel.h())
            panel.fill();
    }
    // 중간 -> 상측
    for(sint32 y = HeightSize - mState.mToolGrid, iy = 0; 0 < (iy = y * GridSize); y -= mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, 0, iy - 1, panel.w(), 1)
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_XYWH(panel, 0, iy, panel.w(), 1)
            panel.fill();
    }
    // 중간 -> 하측
    for(sint32 y = HeightSize, iy = 0; (iy = y * GridSize) < panel.h(); y += mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, 0, iy - 1, panel.w(), 1)
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_RGBA_IF(panel, 128, 128, 192, 192, y == HeightSize)
        ZAY_XYWH(panel, 0, iy, panel.w(), 1)
            panel.fill();
    }
}

void F1Tool::RenderLockToggle(ZayPanel& panel)
{
    ZAY_INNER_UI(panel, InnerGap, "lock",
        ZAY_GESTURE_T(t, this)
        {
            if(t == GT_InReleased)
            {
                mLockedUI = !mLockedUI;
            }
        })
    {
        ZAY_RGBA_IF(panel, 255, 128, 255, 192, mLockedUI)
        ZAY_RGBA_IF(panel, 128, 255, 255, 192, !mLockedUI)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            panel.rect(2);
            panel.text((mLockedUI)? "Lock" : "Unlock", UIFA_CenterMiddle);
        }
    }
}

void F1Tool::RenderGridToggle(ZayPanel& panel)
{
    ZAY_INNER_UI(panel, InnerGap, "grid",
        ZAY_GESTURE_T(t, this)
        {
            if(t == GT_InReleased)
            {
                mGridMode = !mGridMode;
            }
        })
    {
        ZAY_RGBA_IF(panel, 255, 128, 255, 192, mGridMode)
        ZAY_RGBA_IF(panel, 128, 255, 255, 192, !mGridMode)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            panel.rect(2);
            panel.text((mGridMode)? "Grid" : "NoGrid", UIFA_CenterMiddle);
        }
    }
}

void F1Tool::RenderSelectToggle(ZayPanel& panel)
{
    ZAY_INNER_UI(panel, InnerGap, "select",
        ZAY_GESTURE_T(t, this)
        {
            if(t == GT_InReleased)
            {
                mSelectMode = !mSelectMode;
                OnModeChanged();
            }
        })
    {
        ZAY_RGBA_IF(panel, 255, 128, 255, 192, mSelectMode)
        ZAY_RGBA_IF(panel, 128, 255, 255, 192, !mSelectMode)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            panel.rect(2);
            panel.text((mSelectMode)? "Select" : "Edit", UIFA_CenterMiddle);
        }
    }
}

void F1Tool::RenderSelect_SubButton(ZayPanel& panel, chars name)
{
    const String UIName = name;
    ZAY_INNER_UI(panel, InnerGap, String::Format("select_%s", name),
        ZAY_GESTURE_T(t, this, UIName)
        {
            if(t == GT_InReleased)
            {
                OnSelectSub(UIName);
            }
        })
    {
        panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
        {
            panel.rect(2);
            panel.text(name, UIFA_CenterMiddle);
        }
    }
}

void F1Tool::RenderDragButton(ZayPanel& panel)
{
    ZAY_INNER_UI(panel, InnerGap, "drag",
        ZAY_GESTURE_NTXY(n, t, x, y, this)
        {
            static bool HasDrag = false;
            if(t == GT_Pressed) HasDrag = false;
            else if(t == GT_InDragging || t == GT_OutDragging)
            {
                HasDrag = true;
                const point64& CurOldXY = oldxy(n);
                mMapPos += Point(x - CurOldXY.x, y - CurOldXY.y);
                invalidate();
            }
            else if(t == GT_InReleased && !HasDrag)
                mMapPos = Point(0, 0);
        })
    {
        ZAY_INNER(panel, -3)
        ZAY_RGB(panel, 0, 0, 0)
            panel.circle();
        ZAY_RGB(panel, 255, 64, 192)
            panel.circle();
        ZAY_RGB(panel, 0, 0, 0)
            panel.text("Drag", UIFA_CenterMiddle);
    }
}

void F1Tool::RenderHomeButton(ZayPanel& panel)
{
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
}

void F1Tool::RenderSelectBox(ZayPanel& panel, sint32 i, chars name)
{
    const sint32 Border = 2;
    const sint32 TitleHeight = 20;
    const sint32 ButtonWidth = 15;

    // 본체
    panel.rect(Border);

    // 타이틀
    const String TitleName = String::Format("selbox_title_%d", i);
    ZAY_XYWH_UI(panel, 0, -(TitleHeight + Border), panel.w(), TitleHeight, TitleName,
        ZAY_GESTURE_VNTXY(v, n, t, x, y, this, i)
        {
            if(t == GT_InDragging || t == GT_OutDragging)
            {
                OnSelectBoxMoving(i, x - v->oldxy(n).x, y - v->oldxy(n).y);
                v->invalidate();
            }
            else if(t == GT_InReleased || t == GT_OutReleased)
            {
                OnSelectBoxMoved(i);
            }
        })
    {
        const bool IsHold = !!(panel.state(TitleName) & (PS_Focused | PS_Dragging));
        ZAY_RGBA_IF(panel, 128, 128, 128, 192, IsHold)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
            panel.text(name, UIFA_CenterMiddle, UIFE_Right);
        ZAY_RGBA(panel, 128, 128, 128, 255)
            panel.rect(Border);
    }

    // 복사버튼
    const String CopyName = String::Format("selbox_copy_%d", i);
    ZAY_XYWH_UI(panel, -(ButtonWidth + Border), -(TitleHeight + Border), ButtonWidth, TitleHeight, CopyName,
        ZAY_GESTURE_VNTXY(v, n, t, x, y, this, i)
        {
            if(t == GT_Pressed)
            {
                OnSelectBoxClone(i);
            }
            else if(t == GT_InDragging || t == GT_OutDragging)
            {
                OnSelectBoxMoving(i, x - v->oldxy(n).x, y - v->oldxy(n).y);
                v->invalidate();
            }
            else if(t == GT_InReleased || t == GT_OutReleased)
            {
                OnSelectBoxMoved(i);
            }
        })
    {
        const bool IsHold = !!(panel.state(CopyName) & (PS_Focused | PS_Dragging));
        ZAY_RGBA(panel, 64, 64, 192, 128)
        ZAY_RGBA_IF(panel, 128, 128, 128, 192, IsHold)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
            panel.text("＋", UIFA_CenterMiddle, UIFE_Right);
        ZAY_RGBA(panel, 128, 128, 128, 255)
            panel.rect(Border);
    }

    // 사이징버튼
    const String SizeName = String::Format("selbox_size_%d", i);
    ZAY_XYWH_UI(panel, panel.w() + Border, panel.h() - TitleHeight, ButtonWidth, TitleHeight, SizeName,
        ZAY_GESTURE_VNTXY(v, n, t, x, y, this, i)
        {
            if(t == GT_InDragging || t == GT_OutDragging)
            {
                OnSelectBoxSizing(i, x - v->oldxy(n).x, y - v->oldxy(n).y);
                v->invalidate();
            }
            else if(t == GT_InReleased || t == GT_OutReleased)
            {
                OnSelectBoxSized(i);
            }
        })
    {
        const bool IsHold = !!(panel.state(SizeName) & (PS_Focused | PS_Dragging));
        ZAY_RGBA(panel, 64, 192, 64, 128)
        ZAY_RGBA_IF(panel, 128, 128, 128, 192, IsHold)
            panel.fill();
        ZAY_RGB(panel, 0, 0, 0)
            panel.text("↘", UIFA_CenterMiddle, UIFE_Right);
        ZAY_RGBA(panel, 128, 128, 128, 255)
            panel.rect(Border);
    }
}
