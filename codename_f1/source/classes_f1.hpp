#pragma once
#include "classes.hpp"
#include <service/boss_tryworld.hpp>
#include <element/boss_solver.hpp>
#include <element/boss_tween.hpp>

#include <../source-gen/item_get_type.hpp>
#include <../source-gen/monster_move_type.hpp>
#include <../source-gen/monster_type.hpp>
#include <../source-gen/object_type.hpp>
#include <../source-gen/polygon_type.hpp>
#include <../source-gen/skill_id.hpp>

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
    class ObjectTypeClass : public object_type
    {
    public:
        ObjectTypeClass& operator=(chars rhs)
        {
            object_type::operator=(rhs);
            return *this;
        }
    public:
        inline bool isWall() const
        {return (mName == Wall || mName == Dynamic || mName == View);}
        inline bool isDynamic() const
        {return (mName == Dynamic);}
        inline bool isTarget() const
        {return (mName == Target);}
        inline bool isAllyTarget() const
        {return (mName == AllyTarget);}
        inline bool isTrigger() const
        {return (mName == Trigger);}
    };

public:
    String mID;
    ObjectTypeClass mType;
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
        mWaistScaleWidth = 0;
        mWaistScaleHeight = 0;
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
        mWaistScaleWidth = rhs.mWaistScaleWidth;
        mWaistScaleHeight = rhs.mWaistScaleHeight;
        return *this;
    }

public:
    String mID;
    polygon_type mType;
    sint32 mColorR;
    sint32 mColorG;
    sint32 mColorB;
    sint32 mWaistScaleWidth;
    sint32 mWaistScaleHeight;
};
typedef Array<PolygonType> PolygonTypes;

////////////////////////////////////////////////////////////////////////////////
class ItemType
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(ItemType)
public:
    ItemType()
    {
        mRemoveTime = 0;
        mSkillDuration = 0;
        mAddGauge = 0;
        mAddDamage = 0;
    }
    ~ItemType()
    {
    }
    ItemType(ItemType&& rhs) {operator=(ToReference(rhs));}
    ItemType& operator=(ItemType&& rhs)
    {
        mID = ToReference(rhs.mID);
        mGetType = rhs.mGetType;
        mRemoveTime = rhs.mRemoveTime;
        mSkillID = ToReference(rhs.mSkillID);
        mSkillDuration = rhs.mSkillDuration;
        mSkillParameter = rhs.mSkillParameter;
        mAddGauge = rhs.mAddGauge;
        mAddDamage = rhs.mAddDamage;
        mSkinName = ToReference(rhs.mSkinName);
        return *this;
    }

public:
    String mID;
    item_get_type mGetType;
    sint32 mRemoveTime;
    skill_id mSkillID;
    sint32 mSkillDuration;
    String mSkillParameter;
    sint32 mAddGauge;
    sint32 mAddDamage;
    String mSkinName;
};
typedef Array<ItemType> ItemTypes;

////////////////////////////////////////////////////////////////////////////////
class MonsterType : public SpineAsset
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MonsterType)
public:
    MonsterType()
    {
        mType = monster_type::Null;
        mHP = 0;
        mMoveType = monster_move_type::Null;
        mMoveSpeed = 0;
        mMoveVector = 0;
        mTurnDistance = 0;
        mAttackPower = 0;
        mAttackSpeed = 0;
        mAttackRange = 0;
        mGaugePosition = 0;
        mPolygon = "";
        mWeight = 0;
        mResistance = 0;
        mInfinityPoint = 0;
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
        mMoveVector = rhs.mMoveVector;
        mTurnDistance = rhs.mTurnDistance;
        mAttackPower = rhs.mAttackPower;
        mAttackSpeed = rhs.mAttackSpeed;
        mAttackRange = rhs.mAttackRange;
        mGaugePosition = rhs.mGaugePosition;
        mPolygon = ToReference(rhs.mPolygon);
        mWeight = rhs.mWeight;
        mResistance = rhs.mResistance;
        mInfinityPoint = rhs.mInfinityPoint;
        mWaveStop = rhs.mWaveStop;
        return *this;
    }

public:
    String mID;
    monster_type mType;
    sint32 mHP;
    monster_move_type mMoveType;
    sint32 mMoveSpeed;
    sint32 mMoveVector;
    sint32 mTurnDistance;
    sint32 mAttackPower;
    sint32 mAttackSpeed;
    sint32 mAttackRange;
    sint32 mGaugePosition;
    String mPolygon;
    sint32 mWeight;
    sint32 mResistance;
    sint32 mInfinityPoint;
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
    void ResetCB(FXState* state);
    bool SetHP(sint32 hp, sint32 deleteTime);
    void HealHP(sint32 hp, sint32 deleteTime);
    void Hit() const;
    void Dead() const;
    void Drop() const;
    void Heal() const;
    void Spot() const;
    void Target() const;

public:
    inline void AddExtraInfo(chars text) {mExtraInfo.AtAdding() = text;}
    inline void ResetExtraInfo() {mExtraInfo.Clear();}
    inline sint32 GetExtraInfoCount() const {return mExtraInfo.Count();}
    inline chars GetExtraInfo(sint32 i) const {return mExtraInfo[i];}

public:
    inline bool CanBroken(chars polygon_name) const
    {
        if(mType->mType.isDynamic())
        if(auto ChainID = mDynamicChainID.Access(polygon_name))
            return (*ChainID != -1);
        return false;
    }
    inline bool ValidTrigger() const
    {
        return (mType->mType.isTrigger() && !mTriggerOpened);
    }
    inline bool ValidTarget(const MonsterType& type) const
    {
        return ((mType->mType.isTarget() && type.mType == monster_type::Enemy)
            || (mType->mType.isAllyTarget() && type.mType == monster_type::Ally));
    }

public:
    const ObjectType* mType;
    sint32 mRID;
    bool mVisible;
    sint32 mHPValue;
    uint64 mHPTimeMsec;
    mutable float mHPAni;
    Rect mCurrentRect;
    Map<sint32> mDynamicChainID;
    bool mTriggerOpened;
    Strings mExtraInfo;
    ParaView* mParaView;
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
    void UpdateCCW();

public:
    const PolygonType* mType;
    sint32 mRID;
    bool mVisible;
    TryWorld::DotList mDots;
    bool mIsCCW;
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
    enum Type {Target, Wall, Mission, Holic, Null = -1};
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
    void Init(const FXState* state, const MonsterType* type, sint32 rid, sint32 timesec, float hprate, float x, float y,
        const SpineRenderer* renderer, const SpineRenderer* toast_renderer = nullptr);
    void ResetCB(FXState* state);
    bool IsEntranced();
    bool IsKnockBackMode();
    void KnockBack(bool down, const Point& accel, float knockbackup, chars skillskin, float skillspeed, sint32 skilltime, chars skin);
    void KnockBackBound(sint32 damage);
    bool KnockBackEnd();
    bool KnockBackEndByHole(const Point& hole);
    void HolicBegin(const Point& pos, TryWorld::Path* path);
    void HolicEnd();
    void Turn() const;
    sint32 TryAttack(const Point& target);
    void CancelAttack();
    void ClearTargetOnce();
    void ClearAllTargets();
    void TryDeathMove();
    void TryParaTalk();
    void Ally_Arrived();
    void Ally_Touched();
    float CalcedSpeed(uint64 msec) const;
    float CalcedVector() const;
    void ToastTest(uint64 msec);
    void PlayToast(chars motion, bool repeat);

public:
    const float mKnockBackAccelMin = 0.001f; // 화면크기비율상수
    const sint32 mKnockBackNearMin = 3; // 3px
    enum Mode {Mode_Run, Mode_Attack, Mode_Dying};

public:
    const MonsterType* mType;
    sint32 mRID;
    bool mEntranced;
    sint32 mEntranceSec;
    bool mImmortal;
    sint32 mHPValueMax;
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
    Point mCurrentVec;
    Point mTargetPos;
    MonsterTargets mTargets;
    uint64 mTargetTimeLimit;
    uint64 mShoveTimeMsec;
    sint32 mBounceObjectIndex;
    MapSpine mToast;
    bool mHasParaTalk;
    String mParaTalk;

private:
    bool mKnockBackMode;
    Point mKnockBackAccel;
    float mKnockResistance;
    sint32 mKnockBackBoundCount;
    String mSkillSkin;
    float mSkillSpeed;
    sint32 mSkillTime;
    uint64 mSkillTimeLimit;

public:
    inline const Point& knockBackAccel() const {return mKnockBackAccel;}
    inline void SetKnockBackAccel(const Point& accel) {mKnockBackAccel = accel;}
    inline float knockBackResistance() const {return mKnockResistance;}
    inline sint32 knockBackBoundCount() const {return mKnockBackBoundCount;}
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
    void Init(const FXState* state, const SpineRenderer* renderer, float scaleMax, Updater* updater,
        const Point& homepos, const Point& exitposL, const Point& exitposR);
    void ResetCB(FXState* state);
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
    void Init(const FXState* state, const ItemType* type, const SpineRenderer* renderer, const MapObject* sender, Updater* updater,
        float ypos, sint32 entryMsec, sint32 flyingMsec);
    void InitForPos(const FXState* state, const ItemType* type, const SpineRenderer* renderer, Updater* updater, const Point pos,
        float ypos, sint32 entryMsec, sint32 flyingMsec);
    void InitForSlot(const FXState* state, const ItemType* type, const SpineRenderer* renderer, Updater* updater, const Point& pos);
    bool AnimationOnce();
    void MoveToSlot(sint32 i, const Point* pos, sint32 slotMsec);
    void MoveToOut(Point pos, sint32 outMsec);
    bool UseOnce();
    Point CalcPos(uint64 msec) const;
    float CalcFlyingRate(uint64 msec) const;
    float CalcSlotRate(uint64 msec) const;

public:
    inline const ItemType* type() const {return mType;}
    inline chars skin() const {return mType->mSkinName;}
    inline bool slot() const {return (mMode == ItemMode::Slot || mMode == ItemMode::Used || mMode == ItemMode::Destroy);}
    inline sint32 slot_index() const {return mSlotIndex;}
    static String MakeId() {static sint32 _ = 0; return String::Format("%d", _++);}

private:
    void Show() const;

public:
    enum class ItemMode {Wait, Show, Slot, Out, Used, Destroy};

private:
    const ItemType* mType;
    const MapObject* mSender;
    Tween2D* mTween;
    uint64 mFlyingBeginTimeMsec;
    uint64 mFlyingEndTimeMsec;
    uint64 mSlotBeginTimeMsec;
    uint64 mSlotEndTimeMsec;
    sint32 mSlotIndex;
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
    const PolygonType* mType;
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
    F1State(bool needdoor = false);
    ~F1State();

public:
    sint32 LoadMap(chars json, bool toolmode);
    String SaveMap(const F1Tool* tool);
    void BuildTryWorld(bool error_test, chars polygon_name = nullptr);
    void RebuildTryWorld(sint32 dynamic_chain_id, chars polygon_name = nullptr);
    void SetSize(sint32 width, sint32 height);
    void RenderImage(DebugMode debug, ZayPanel& panel, const Image& image);
    void RenderLayer(DebugMode debug, ZayPanel& panel, const MapLayer& layer, const MapMonsters* monsters = nullptr, const sint32 wavesec = 0);
    Rect RenderMap(DebugMode debug, ZayPanel& panel, const MapMonsters* monsters = nullptr, sint32 wavesec = 0);
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
    float mDragonScale;
    float mDragonScaleMax;
    float mDragonCurve;
    float mDragonMouthX;
    float mDragonMouthY;
    float mFingerScale;
    float mItemScale;
    float mSlotScale;
    sint32 mHoleItemGetCount;
    sint32 mHeartHoleCreatCount; // 하트구멍에 몬스터를 넣었을 경우 하트가 생성될 최소 몬스터 마리수
    float mHeartHoleCreatRate; // 하트구멍에 몬스터를 넣었을 경우 하트가 생성될 확률 (마리수가 만족할때마다 체크)
    sint32 mInfinityPlusValue; // 무한모드가 1웨이브 지날때마다 몬스터에게 추가될 점수
    float mInfinityMoveSpeedRate; // 무한모드가 1웨이브 지날때마다 몬스터에게 증가될 이동속도 비율
    float mInfinityHPRate; // 무한모드가 1웨이브 지날때마다 몬스터에게 증가될 체력 비율
    sint32 mItemSlotStack; // 한 슬롯에 등록될 수 있는 아이템의 최대 개수
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
    sint32 mFireStoneDropRate;
    sint32 mIceStoneDropRate;
    sint32 mWindStoneDropRate;
    sint32 mLightningStoneDropRate;
    float mUnicornEggHealValue;
    float mUnicornGageUpValue;
    sint32 mShoveCoolTime;
    float mShoveCheckDistance;
    float mShovePower;

    ObjectTypes mObjectTypes;
    PolygonTypes mPolygonTypes;
    ItemTypes mItemTypes;
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
    sint32 mFingerSizeR;
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
    String mBGName;
    String mBGImageB;
    String mBGMusic;
    String mBGWeather;
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
    const sint32 IconSizeSmall = 40;

public:
    F1State mState;
    Point mMapPos;
    bool mCursorInWindow;
    bool mLockedUI;
    bool mGridMode;
    bool mSelectMode;
    Tween1D mUITween;
};
