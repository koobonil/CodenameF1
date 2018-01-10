#pragma once
#include "classes.hpp"
#include <service/boss_tryworld.hpp>
#include <element/boss_solver.hpp>
#include <element/boss_tween.hpp>

////////////////////////////////////////////////////////////////////////////////
class ObjectType : public SpineAsset
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(ObjectType)
public:
    ObjectType()
    {
        mHP = 0;
        mGaugePosition = 0;
    }
    ~ObjectType()
    {
    }
    ObjectType(ObjectType&& rhs) {operator=(ToReference(rhs));}
    ObjectType& operator=(ObjectType&& rhs)
    {
        SpineAsset::operator=(ToReference(rhs));
        mID = ToReference(rhs.mID);
        mType = rhs.mType;
        mAssetShadow = ToReference(rhs.mAssetShadow);
        mHP = rhs.mHP;
        mGaugePosition = rhs.mGaugePosition;
        return *this;
    }

public:
    void SetAssetShadow(const String& asset) {mAssetShadow = (!asset.Compare("None"))? "" : asset;}

public:
    class TypeClass
    {
    public:
        enum Type {Static, Dynamic, Trigger, Target, AllyTarget, View, Ground, Spot, Hole, Max, Null = -1};
    public:
        TypeClass() {mValue = Null;}
        TypeClass(const TypeClass& rhs) {operator=(rhs);}
        TypeClass& operator=(const TypeClass& rhs) {mValue = rhs.mValue; return *this;}
        TypeClass& operator=(Type rhs) {mValue = rhs; return *this;}
        TypeClass& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Static"))
                mValue = Static;
            else if(!String::Compare(rhs, "Dynamic"))
                mValue = Dynamic;
            else if(!String::Compare(rhs, "Trigger"))
                mValue = Trigger;
            else if(!String::Compare(rhs, "Target"))
                mValue = Target;
            else if(!String::Compare(rhs, "AllyTarget"))
                mValue = AllyTarget;
            else if(!String::Compare(rhs, "View"))
                mValue = View;
            else if(!String::Compare(rhs, "Ground"))
                mValue = Ground;
            else if(!String::Compare(rhs, "Spot"))
                mValue = Spot;
            else if(!String::Compare(rhs, "Hole"))
                mValue = Hole;
            else
            {
                mValue = Null;
                if(!String::Compare(rhs, "Null"))
                    BOSS_ASSERT("키워드가 없습니다", false);
                else BOSS_ASSERT("알 수 없는 키워드입니다", false);
            }
            return *this;
        }
        bool operator==(Type rhs) const
        {return (mValue == rhs);}
        bool operator!=(Type rhs) const
        {return (mValue != rhs);}
    public:
        inline bool isWall() const
        {return (mValue < Ground);}
        inline bool canBroken() const
        {return (mValue == Dynamic);}
    private:
        Type mValue;
    };

public:
    String mID;
    TypeClass mType;
    String mAssetShadow;
    sint32 mHP;
    sint32 mGaugePosition;
};
typedef Array<ObjectType> ObjectTypes;

////////////////////////////////////////////////////////////////////////////////
class PolygonType
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(PolygonType)
public:
    PolygonType()
    {
    }
    ~PolygonType()
    {
    }
    PolygonType(PolygonType&& rhs) {operator=(ToReference(rhs));}
    PolygonType& operator=(PolygonType&& rhs)
    {
        mID = ToReference(rhs.mID);
        mType = rhs.mType;
        mColorR = rhs.mColorR;
        mColorG = rhs.mColorG;
        mColorB = rhs.mColorB;
        return *this;
    }

public:
    class TypeClass
    {
    public:
        enum Type {Wall, Water, Max, Null = -1};
    public:
        TypeClass() {mValue = Null;}
        TypeClass(const TypeClass& rhs) {operator=(rhs);}
        TypeClass& operator=(const TypeClass& rhs) {mValue = rhs.mValue; return *this;}
        TypeClass& operator=(Type rhs) {mValue = rhs; return *this;}
        TypeClass& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Wall"))
                mValue = Wall;
            else if(!String::Compare(rhs, "Water"))
                mValue = Water;
            else
            {
                mValue = Null;
                if(!String::Compare(rhs, "Null"))
                    BOSS_ASSERT("키워드가 없습니다", false);
                else BOSS_ASSERT("알 수 없는 키워드입니다", false);
            }
            return *this;
        }
        bool operator==(Type rhs) const
        {return (mValue == rhs);}
        bool operator!=(Type rhs) const
        {return (mValue != rhs);}
    private:
        Type mValue;
    };

public:
    String mID;
    TypeClass mType;
    sint32 mColorR;
    sint32 mColorG;
    sint32 mColorB;
};
typedef Array<PolygonType> PolygonTypes;

////////////////////////////////////////////////////////////////////////////////
class MonsterType : public SpineAsset
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MonsterType)
public:
    MonsterType()
    {
        mType = TypeClass::Null;
        mHP = 0;
        mMoveType = MoveType::Null;
        mMoveSpeed = 0;
        mTurnDistance = 0;
        mAttackPower = 0;
        mAttackSpeed = 0;
        mAttackRange = 0;
        mWaistScaleWidth = 0;
        mWaistScaleHeight = 0;
        mGaugePosition = 0;
        mPolygon = "";
        mWeight = 0;
        mResistance = 0;
        mWaveStop = false;
    }
    ~MonsterType() {}
    MonsterType(MonsterType&& rhs) {operator=(ToReference(rhs));}
    MonsterType& operator=(MonsterType&& rhs)
    {
        SpineAsset::operator=(ToReference(rhs));
        mID = rhs.mID;
        mType = rhs.mType;
        mHP = rhs.mHP;
        mMoveType = rhs.mMoveType;
        mMoveSpeed = rhs.mMoveSpeed;
        mTurnDistance = rhs.mTurnDistance;
        mAttackPower = rhs.mAttackPower;
        mAttackSpeed = rhs.mAttackSpeed;
        mAttackRange = rhs.mAttackRange;
        mWaistScaleWidth = rhs.mWaistScaleWidth;
        mWaistScaleHeight = rhs.mWaistScaleHeight;
        mGaugePosition = rhs.mGaugePosition;
        mPolygon = ToReference(rhs.mPolygon);
        mWeight = rhs.mWeight;
        mResistance = rhs.mResistance;
        mWaveStop = rhs.mWaveStop;
        return *this;
    }

public:
    class TypeClass
    {
    public:
        enum Type {Enemy, Ally, Max, Null = -1};
    public:
        TypeClass() {mValue = Null;}
        TypeClass(const TypeClass& rhs) {operator=(rhs);}
        TypeClass& operator=(const TypeClass& rhs) {mValue = rhs.mValue; return *this;}
        TypeClass& operator=(Type rhs) {mValue = rhs; return *this;}
        TypeClass& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Enemy"))
                mValue = Enemy;
            else if(!String::Compare(rhs, "Ally"))
                mValue = Ally;
            else
            {
                mValue = Null;
                if(!String::Compare(rhs, "Null"))
                    BOSS_ASSERT("키워드가 없습니다", false);
                else BOSS_ASSERT("알 수 없는 키워드입니다", false);
            }
            return *this;
        }
        bool operator==(Type rhs) const
        {return (mValue == rhs);}
        bool operator!=(Type rhs) const
        {return (mValue != rhs);}
    private:
        Type mValue;
    };

public:
    class MoveType
    {
    public:
        enum Type {StraightMoving, Max, Null = -1};
    public:
        MoveType() {mValue = Null;}
        MoveType(const MoveType& rhs) {operator=(rhs);}
        MoveType& operator=(const MoveType& rhs) {mValue = rhs.mValue; return *this;}
        MoveType& operator=(Type rhs) {mValue = rhs; return *this;}
        MoveType& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Straight"))
                mValue = StraightMoving;
            else
            {
                mValue = Null;
                if(!String::Compare(rhs, "Null"))
                    BOSS_ASSERT("키워드가 없습니다", false);
                else BOSS_ASSERT("알 수 없는 키워드입니다", false);
            }
            return *this;
        }
    private:
        Type mValue;
    };

public:
    String mID;
    TypeClass mType;
    sint32 mHP;
    MoveType mMoveType;
    sint32 mMoveSpeed;
    sint32 mTurnDistance;
    sint32 mAttackPower;
    sint32 mAttackSpeed;
    sint32 mAttackRange;
    sint32 mWaistScaleWidth;
    sint32 mWaistScaleHeight;
    sint32 mGaugePosition;
    String mPolygon;
    sint32 mWeight;
    sint32 mResistance;
    bool mWaveStop;
};
typedef Array<MonsterType> MonsterTypes;

////////////////////////////////////////////////////////////////////////////////
class MapObject : public MapSpine
{
public:
    MapObject();
    ~MapObject();
    MapObject(const MapObject& rhs);
    MapObject& operator=(const MapObject& rhs);
    MapObject(MapObject&& rhs);
    MapObject& operator=(MapObject&& rhs);

public:
    void ResetCB();
    bool SetHP(sint32 hp, sint32 deleteTime);
    void Hit() const;
    void Dead() const;
    void Drop() const;
    void Spot() const;

public:
    inline void AddExtraInfo(chars text) {mExtraInfo.AtAdding() = text;}
    inline void ResetExtraInfo() {mExtraInfo.Clear();}
    inline sint32 GetExtraInfoCount() const {return mExtraInfo.Count();}
    inline chars GetExtraInfo(sint32 i) const {return mExtraInfo[i];}

public:
    const ObjectType* mType;
    sint32 mRID;
    bool mEnable;
    sint32 mHPValue;
    uint64 mHPTimeMsec;
    mutable float mHPAni;
    Rect mCurrentRect;
    Strings mExtraInfo;
    ParaSource::View* mParaView;
};
typedef Array<MapObject> MapObjects;

////////////////////////////////////////////////////////////////////////////////
class MapPolygon
{
public:
    MapPolygon();
    ~MapPolygon();
    MapPolygon(const MapPolygon& rhs);
    MapPolygon& operator=(const MapPolygon& rhs);
    MapPolygon(MapPolygon&& rhs);
    MapPolygon& operator=(MapPolygon&& rhs);

public:
    void UpdateCW();

public:
    const PolygonType* mType;
    sint32 mRID;
    bool mEnable;
    TryWorld::DotList mDots;
    bool mIsCW;
};
typedef Array<MapPolygon> MapPolygons;

////////////////////////////////////////////////////////////////////////////////
class MapLayer
{
public:
    MapLayer() {mShowFlag = true;}
    ~MapLayer() {}
public:
    bool mShowFlag;
    MapObjects mObjects;
    MapPolygons mPolygons;
};
typedef Array<MapLayer> MapLayers;

////////////////////////////////////////////////////////////////////////////////
class MonsterTarget
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MonsterTarget)
public:
    MonsterTarget();
    ~MonsterTarget();
    MonsterTarget(MonsterTarget&& rhs);
    MonsterTarget& operator=(MonsterTarget&& rhs);

public:
    enum Type {Target, Wall, Mission, Null = -1};
    Type mType;
    sint32 mIndex;
    Point mPos;
    float mSizeR;
    TryWorld::Path* mPath;
};
typedef Array<MonsterTarget> MonsterTargets;

////////////////////////////////////////////////////////////////////////////////
class MapMonster : public MapSpine
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MapMonster)
public:
    MapMonster();
    ~MapMonster();
    MapMonster(MapMonster&& rhs);
    MapMonster& operator=(MapMonster&& rhs);

public:
    void Init(const MonsterType* type, sint32 rid, sint32 timesec, float x, float y,
        const SpineRenderer* renderer, const SpineRenderer* toast_renderer = nullptr);
    void ResetCB();
    bool IsEntranced();
    bool IsKnockBackMode();
    void KnockBack(bool down, const Point& accel, chars skin);
    void KnockBackBound(sint32 damage);
    void KnockBackEnd();
    void KnockBackEndByHole(const Point& hole);
    void Turn() const;
    sint32 TryAttack(const Point& target);
    void CancelAttack();
    void ClearTargetOnce();
    void ClearAllTargets();
    void TryDeathMove();
    void TryParaTalk();
    void Ally_Arrived();
    void Ally_Touched();
    Point CalcBump(const MapObject* object);

public:
    const float mKnockBackAccelMin = 0.001f; // 화면크기비율상수
    const sint32 mKnockBackNearMin = 3; // 3px
    enum Mode {Mode_Run, Mode_Attack, Mode_Dying};

public:
    const MonsterType* mType;
    sint32 mRID;
    bool mEntranced;
    sint32 mEntranceSec;
    sint32 mHPValue;
    uint64 mHPTimeMsec;
    mutable float mHPAni;
    Mode mMode;
    sint32 mDeathStep;
    Point mDeathPos;
    sint32 mAttackCount;
    uint64 mAttackTimeMsec;
    bool mFlipMode;
    bool mLastFlip;
    Point mLastFlipPos;
    Point mCurrentPos;
    Point mCurrentPosOld;
    Point mTargetPos;
    MonsterTargets mTargets;
    uint64 mTargetTimeLimit;
    sint32 mBounceObjectIndex;
    MapSpine mToast;
    bool mHasParaTalk;
    String mParaTalk;
    // 오브젝트충돌처리의 경험기록
    bool mIsBumpClock;
    sint32 mBumpObjectRID;
    Rect mBumpObjectRect;

private:
    bool mKnockBackMode;
    Point mKnockBackAccel;
    sint32 mKnockBackBoundCount;

public:
    inline const Point& knockbackaccel() const {return mKnockBackAccel;}
    inline void SetKnockBackAccel(const Point& accel) {mKnockBackAccel = accel;}
    inline sint32 knockbackboundcount() const {return mKnockBackBoundCount;}
};
typedef Array<MapMonster> MapMonsters;

////////////////////////////////////////////////////////////////////////////////
class MapDragon : public MapSpine
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MapDragon)
public:
    MapDragon();
    ~MapDragon();
    MapDragon(MapDragon&& rhs);
    MapDragon& operator=(MapDragon&& rhs);

public:
    void Init(const SpineRenderer* renderer, float scaleMax, Updater* updater,
        const Point& homepos, const Point& exitposL, const Point& exitposR);
    void ResetCB();
    void GoTarget(const Point& beginpos, const Point& pos, bool isExitRight,
        sint32 entryMsec, sint32 breathMsec, sint32 exitMsec);
    Point MoveOnce(float curve, Point mouthpos, sint32 breathdelayMsec, bool& attackflag);
    float CalcEntryRate(uint64 msec) const;
    float CalcBreathRate(uint64 msec) const;
    float CalcExitRate(uint64 msec) const;

public:
    inline bool flip() const {return mDragonFlipMode;}
    inline float scale() const {return mDragonScale;}
    inline uint64 endtime() const {return mDragonBreathEndTimeMsec;}
    inline Point pos() const {return mLastPos;}

private:
    void Attack() const;

private:
    bool mDragonFlipMode;
    bool mAttackDone;
    bool mAttackFinished;
    float mDragonScale;
    float mDragonScaleMax;
    Point mBreathPosAdd;
    uint64 mDragonEntryTimeMsec;
    uint64 mDragonBreathBeginTimeMsec;
    uint64 mDragonBreathEndTimeMsec;
    uint64 mDragonExitTimeMsec;
    Tween2D* mTween;
    Point mSpotPos[3];
    Point mNewPos;
    Point mOldPos;
    Point mLastPos;
};

////////////////////////////////////////////////////////////////////////////////
class MapItem : public MapSpine
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MapItem)
public:
    MapItem();
    ~MapItem();
    MapItem(MapItem&& rhs);
    MapItem& operator=(MapItem&& rhs);

public:
    void Init(chars skin, const SpineRenderer* renderer, const MapObject* sender, Updater* updater,
        float ypos, sint32 entryMsec, sint32 flyingMsec);
    bool AnimationOnce();
    void MoveToSlot(const Point* pos, sint32 slotMsec);
    const Point* Use();
    Point CalcPos(uint64 msec) const;
    float CalcFlyingRate(uint64 msec) const;
    float CalcSlotRate(uint64 msec) const;

public:
    enum class ItemMode {Wait, Show, Slot, Used, Destroy};

public:
    inline chars skin() const {return mSkin;}
    inline bool slot() const {return (mMode == ItemMode::Slot || mMode == ItemMode::Used || mMode == ItemMode::Destroy);}
    static String MakeId() {static sint32 _ = 0; return String::Format("%d", _++);}

private:
    void Show() const;

private:
    String mSkin;
    const MapObject* mSender;
    Tween2D* mTween;
    uint64 mFlyingBeginTimeMsec;
    uint64 mFlyingEndTimeMsec;
    uint64 mSlotBeginTimeMsec;
    uint64 mSlotEndTimeMsec;
    const Point* mSlotPos;
    ItemMode mMode;
};
typedef Map<MapItem> MapItemMap;

////////////////////////////////////////////////////////////////////////////////
class TryWorldZone
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(TryWorldZone)
public:
    TryWorldZone();
    ~TryWorldZone();

public:
    TryWorld::Hurdle* mHurdle;
    TryWorld::Map* mMap;
};
typedef Map<TryWorldZone> TryWorldZoneMap;

////////////////////////////////////////////////////////////////////////////////
class TargetZone
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(TargetZone)
public:
    TargetZone();
    ~TargetZone();
    TargetZone(TargetZone&& rhs);
    TargetZone& operator=(TargetZone&& rhs);

public:
    void Init(sint32 objectindex, float r);

public:
    sint32 mObjectIndex;
    float mSizeR;
};
typedef Array<TargetZone> TargetZones;

class F1Tool;
////////////////////////////////////////////////////////////////////////////////
class F1State : public FXState
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(F1State, FXState(""), mLandscape(false), mStage(""))
public:
    F1State();
    ~F1State();

public:
    sint32 LoadMap(chars json, bool toolmode);
    String SaveMap(const F1Tool* tool);
    void RebuildTryWorld();
    void SetSize(sint32 width, sint32 height);
    void RenderImage(bool editmode, ZayPanel& panel, const Image& image);
    void RenderLayer(bool editmode, ZayPanel& panel, const MapLayer& layer, const MapMonsters* monsters = nullptr, const sint32 wavesec = 0);
    Rect RenderMap(bool editmode, ZayPanel& panel, const MapMonsters* monsters = nullptr, sint32 wavesec = 0);
    void RenderCap(ZayPanel& panel, const Rect outline);
    void RenderDebug(ZayPanel& panel, const MapMonsters& monsters, sint32 wavesec);

public:
    virtual void RenderBreathArea(ZayPanel& panel) {}

public: // 상수요소
    const bool mLandscape;
    const String mStage;

public: // 기획요소
    Solver mUILeft;
    Solver mUITop;
    Solver mUIRight;
    Solver mUIBottom;
    float mViewRate; // 뷰비율 = 가로길이 / 세로길이
    float mDragonScale;
    float mDragonScaleMax;
    float mDragonCurve;
    float mDragonMouthX;
    float mDragonMouthY;
    float mItemScale;
    float mSlotScale;
    sint32 mHoleItemGetCount;
    float mBreathScale;
    sint32 mBreathMinDamage;
    sint32 mBreathMaxDamage;
    sint32 mBreathMaxGauge;
    sint32 mBreathGaugeChargingPerSec;
    sint32 mHPbarDeleteTime; // 투명해지는데 걸리는 시간
    sint32 mEggHPRegenValue; // 초당 재생량
    sint32 m1StarHpRate;
    sint32 m2StarHpRate;
    sint32 m3StarHpRate;
    float mMonsterScale;
    float mWallBoundScale;
    float mKnockBackMinDistance;
    float mKnockBackMaxDistance;
    sint32 mToolGrid;
    sint32 mDragonEntryTime;
    sint32 mDragonRetryTime;
    sint32 mDragonBreathTime;
    sint32 mDragonExitTime;
    float m1BoundDamageRate;
    float m2BoundDamageRate;
    float m3BoundDamageRate;
    ObjectTypes mObjectTypes;
    PolygonTypes mPolygonTypes;
    MonsterTypes mMonsterTypes;

public: // UI요소
    sint32 mUIL;
    sint32 mUIT;
    sint32 mUIR;
    sint32 mUIB;
    sint32 mScreenW;
    sint32 mScreenH;
    sint32 mInGameW;
    sint32 mInGameH;
    sint32 mInGameX;
    sint32 mInGameY;
    sint32 mInGameSize;
    Point mDragonHome;
    Point mDragonExitL;
    Point mDragonExitR;
    sint32 mDragonSizeR;
    sint32 mBreathSizeR;
    sint32 mItemSizeR;
    sint32 mSlotSizeR;
    sint32 mMonsterSizeR;
    sint32 mWallBoundSizeR;
    sint32 mKnockBackMinV;
    sint32 mKnockBackMaxV;
    const sint32 mTimelineLength = 60;
    const sint32 mLayerLength = 6;

public: // 인스턴스ID
    const sint32 mObjectRIDBegin = 81000000 - 1;
    const sint32 mPolygonRIDBegin = 82000000 - 1;
    const sint32 mMonsterRIDBegin = 83000000 - 1;
    const sint32 mMissionRIDBegin = 84000000 - 1;
    sint32 mObjectLastRID;
    sint32 mPolygonLastRID;
    sint32 mMonsterLastRID;
    sint32 mMissionLastRID;

public: // 맵요소
    String mBGNameA;
    String mBGNameB;
    TargetZones mTargetsForEnemy;
    TargetZones mTargetsForAlly;
    MapLayers mLayers;
    Map<MapObject*> mObjectRIDs;
    id_surface mShadowSurface;
    TryWorldZoneMap mAllTryWorldZones;
};

////////////////////////////////////////////////////////////////////////////////
class F1Tool : public ZayObject
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(F1Tool)
public:
    F1Tool();
    ~F1Tool();

public:
    void Command(CommandType type, id_share in);
    Point GestureToPos(sint32 x, sint32 y);
    Point AttachGrid(float fx, float fy);
    void RenderGrid(ZayPanel& panel);
    void RenderLockToggle(ZayPanel& panel);
    void RenderGridToggle(ZayPanel& panel);
    void RenderSelectToggle(ZayPanel& panel);
    void RenderSelect_SubButton(ZayPanel& panel, chars name);
    void RenderDragButton(ZayPanel& panel);
    void RenderHomeButton(ZayPanel& panel);
    void RenderSelectBox(ZayPanel& panel, sint32 i, chars name);

public:
    virtual void OnModeChanged() = 0;
    virtual void OnSelectSub(chars name) = 0;
    virtual void InitSelectBox(sint32 index) = 0;
    virtual void QuitSelectBox(sint32 index) = 0;
    virtual void ChangeSelectBox(sint32 type, sint32 index) = 0;
    virtual void OnSelectBoxMoving(sint32 index, float addx, float addy) = 0;
    virtual void OnSelectBoxMoved(sint32 index) = 0;
    virtual void OnSelectBoxSizing(sint32 index, float addx, float addy) = 0;
    virtual void OnSelectBoxSized(sint32 index) = 0;
    virtual void OnSelectBoxClone(sint32 index) = 0;

public:
    // 1: 스크립트 추가
    // 2: 툴버전/빌드넘버 추가
    const sint32 ToolVersion = 2;
    sint32 mBuildNumber;

public:
    const sint32 InnerGap = 10;
    const sint32 ButtonSize = 80;
    const sint32 ButtonSizeSmall = 50;
    const sint32 IconSize = 50;

public:
    F1State mState;
    Point mMapPos;
    bool mCursorInWindow;
    bool mLockedUI;
    bool mGridMode;
    bool mSelectMode;
    Tween1D mUITween;
};
