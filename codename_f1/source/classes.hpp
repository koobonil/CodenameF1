﻿#pragma once
#include <service/boss_zay.hpp>
#include <service/boss_tryworld.hpp>
#include <element/boss_solver.hpp>
#include <element/boss_tween.hpp>

#include "spine_for_zay/zay_spine_builder.hpp"

////////////////////////////////////////////////////////////////////////////////
class SpineAsset
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(SpineAsset)
public:
    SpineAsset()
    {
        mAsset = "";
        mSpine = "";
    }
    ~SpineAsset()
    {
    }
    SpineAsset(SpineAsset&& rhs) {operator=(ToReference(rhs));}
    SpineAsset& operator=(SpineAsset&& rhs)
    {
        mAsset = ToReference(rhs.mAsset);
        mSpine = ToReference(rhs.mSpine);
        return *this;
    }

private:
    String mAsset;
    String mSpine;

public:
    void SetAsset(const String& asset) {mAsset = (!asset.Compare("None"))? "" : asset;}
    void SetSpine(const String& spine) {mSpine = (!spine.Compare("None"))? "" : spine;}
    chars imageName() const {return mAsset;}
    const String spineName() const
    {
        if(0 < mSpine.Length())
        {
            sint32 Pos = mSpine.Find(0, ':');
            if(Pos == -1) return mSpine;
            if(0 < Pos) return mSpine.Left(Pos);
        }
        return mAsset;
    }
    const String spineSkinName() const
    {
        if(0 < mSpine.Length())
        {
            sint32 Pos = mSpine.Find(0, ':');
            if(0 <= Pos)
            if(sint32 Length = mSpine.Length() - 1 - Pos)
                return mSpine.Right(Length);
        }
        return "default";
    }
};

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
        enum Type {Static, Dynamic, Ground, Hole, Trigger, Target, AllyTarget, Max, Null = -1};
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
            else if(!String::Compare(rhs, "Ground"))
                mValue = Ground;
            else if(!String::Compare(rhs, "Hole"))
                mValue = Hole;
            else if(!String::Compare(rhs, "Trigger"))
                mValue = Trigger;
            else if(!String::Compare(rhs, "Target"))
                mValue = Target;
            else if(!String::Compare(rhs, "AllyTarget"))
                mValue = AllyTarget;
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
class SpineRenderer
{
public:
    SpineRenderer();
    ~SpineRenderer();

public:
    void Create(chars spinepath, chars imagepath);
    void Release();
    void Render(ZAY::id_spine_instance instance, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip, bool outline) const;
    void RenderShadow(ZAY::id_spine_instance instance, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip) const;
    void RenderBound(ZAY::id_spine_instance instance, ZayPanel& panel, bool guideline, float ox, float oy, float scale, bool flip,
        chars uiname, ZayPanel::SubGestureCB cb) const;

public:
    inline ZAY::id_spine spine() const {return mSpine;}

private:
    ZAY::id_spine mSpine;
};
typedef Map<SpineRenderer> SpineRendererMap;

////////////////////////////////////////////////////////////////////////////////
class MapSpine
{
public:
    enum SpineType {ST_Unknown, ST_Object, ST_Monster, ST_MonsterToast, ST_Dragon, ST_Item};

public:
    MapSpine(SpineType type = ST_Unknown);
    ~MapSpine();
    MapSpine(const MapSpine& rhs);
    MapSpine& operator=(const MapSpine& rhs);
    MapSpine(MapSpine&& rhs);
    MapSpine& operator=(MapSpine&& rhs);

public:
    MapSpine& InitSpine(const SpineRenderer* renderer, chars skin = "default",
        ZAY::SpineBuilder::MotionFinishedCB fcb = nullptr, ZAY::SpineBuilder::UserEventCB ecb = nullptr);
    void SetSkin(chars skin);
    void PlayMotion(chars motion, bool repeat);
    void PlayMotionOnce(chars motion);
    void PlayMotionAttached(chars first_motion, chars second_motion, bool repeat);
    void PlayMotionSeek(chars seek_motion, bool repeat);
    void StopMotionAll();
    void Seek() const;
    void Update() const;
    const Rect* GetBoundRect(chars name) const;

public:
    void SetSeekSec(float sec);
    bool IsSeekUpdated() const;
    void Staff_TryIdle();
    void Staff_Start();

public:
    inline const SpineRenderer* renderer() const {return mSpineRenderer;}
    inline const bool enabled() const
    {return (mSpineInstance && ZAY::SpineBuilder::IsMotionEnabled(mSpineInstance));}

public:
    const SpineType mSpineType;
    const SpineRenderer* mSpineRenderer;
    ZAY::id_spine_instance mSpineInstance;
    mutable uint64 mSpineMsecOld;
    float mSeekSec;
    mutable float mSeekSecOld;
    bool mStaffIdleMode;
    bool mStaffStartMode;
};

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
    void SetHP(sint32 hp, sint32 deleteTime);
    void Hit() const;
    void Dead() const;
    void Drop() const;

public:
    inline void AddExtraInfo(chars text) {mExtraInfo.AtAdding() = text;}
    inline void ResetExtraInfo() {mExtraInfo.Clear();}
    inline sint32 GetExtraInfoCount() const {return mExtraInfo.Count();}
    inline chars GetExtraInfo(sint32 i) const {return mExtraInfo[i];}

public:
    const ObjectType* mType;
    sint32 mRID;
    bool mVisible;
    sint32 mHPValue;
    uint64 mHPTimeMsec;
    mutable float mHPAni;
    Rect mCurrentRect;
    Strings mExtraInfo;
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
    bool mVisible;
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
        const SpineRenderer& renderer, const SpineRenderer* toast_renderer = nullptr);
    void ResetCB();
    bool IsKnockBackMode();
    void KnockBack(bool down, const Point& accel, chars skin);
    void KnockBackEnd();
    void KnockBackEndByHole(const Point& hole);
    void Turn() const;
    sint32 TryAttack();
    void CancelAttack();
    void ClearTarget();
    void TryDeathMove();
    void Ally_Arrived();
    void Ally_Touched();

public:
    const float mKnockBackAccelMin = 0.001f; // 화면크기비율상수
    const sint32 mKnockBackNearMin = 3; // 3px
    enum Mode {Mode_Run, Mode_Attack, Mode_Dying};

public:
    const MonsterType* mType;
    sint32 mRID;
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
    Point mTargetPos;
    sint32 mTargetIndex;
    TryWorld::Path* mTargetPath;
    sint32 mTargetPathScore;
    sint32 mBounceObjectIndex;
    MapSpine mToast;

private:
    bool mKnockBackMode;
    Point mKnockBackAccel;

public:
    inline const Point& knockbackaccel() const {return mKnockBackAccel;}
    inline void SetKnockBackAccel(const Point& accel) {mKnockBackAccel = accel;}
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
    void Init(const SpineRenderer& renderer, float scaleMax, Updater* updater,
        const Point& homepos, const Point& exitposL, const Point& exitposR);
    void GoTarget(const Point& pos, bool isExitRight,
        sint32 entryMsec, sint32 breathMsec, sint32 exitMsec);
    Point MoveOnce(float curve, Point mouthpos, sint32 breathdelayMsec);
    float CalcEntryRate(uint64 msec) const;
    float CalcBreathRate(uint64 msec) const;
    float CalcExitRate(uint64 msec) const;

public:
    inline bool flip() const {return mDragonFlipMode;}
    inline bool breath_flip() const {return mBreathFlipMode;}
    inline float scale() const {return mDragonScale;}

private:
    void Turn() const;
    void Attack() const;

private:
    bool mDragonFlipMode;
    bool mBreathFlipMode;
    bool mAttackDone;
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
    void Init(chars skin, const SpineRenderer& renderer, const MapObject* sender, Updater* updater,
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
    void Init(sint32 layerindex, sint32 objectindex, float r);

public:
    sint32 mLayerIndex;
    sint32 mObjectIndex;
    float mSizeR;
};
typedef Array<TargetZone> TargetZones;

////////////////////////////////////////////////////////////////////////////////
class F1State
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(F1State, mLandscape(landscape()), mStage(stage()))
public:
    F1State();
    ~F1State();

public:
    void LoadMap(chars json);
    String SaveMap();
    void RebuildTryWorld();
    void SetSize(sint32 width, sint32 height);
    void RenderImage(bool editmode, ZayPanel& panel, const Image& image);
    void RenderObject(bool needupdate, bool editmode, ZayPanel& panel, const MapSpine& spine, bool flip, chars uiname = nullptr, ZayPanel::SubGestureCB cb = nullptr);
    void RenderObjectShadow(ZayPanel& panel, const MapSpine& spine, bool flip);
    void RenderLayer(bool editmode, ZayPanel& panel, const MapLayer& layer, const MapMonsters* monsters = nullptr, const sint32 wavesec = 0);
    Rect RenderMap(bool editmode, ZayPanel& panel, const MapMonsters* monsters = nullptr, sint32 wavesec = 0);
    void RenderCap(ZayPanel& panel, const Rect outline);
    void RenderDebug(ZayPanel& panel, const MapMonsters& monsters, sint32 wavesec);

public:
    virtual void RenderBreathArea(ZayPanel& panel) {}

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
    float mKnockBackMinDistance;
    float mKnockBackMaxDistance;
    sint32 mBreathMaxGauge;
    sint32 mBreathGaugeChargingPerSec;
    sint32 mHPbarDeleteTime; // 투명해지는데 걸리는 시간
    sint32 mEggHPRegenValue; // 초당 재생량
    sint32 m1StarHpRate;
    sint32 m2StarHpRate;
    sint32 m3StarHpRate;
    float mMonsterScale;
    sint32 mToolGrid;
    sint32 mDragonEntryTime;
    sint32 mDragonBreathTime;
    sint32 mDragonExitTime;
    ObjectTypes mObjectTypes;
    PolygonTypes mPolygonTypes;
    MonsterTypes mMonsterTypes;
    SpineRendererMap mAllSpines;

public: // 글로벌요소
    static bool& landscape() {static bool _ = false; return _;}
    const bool mLandscape;
    static String& stage() {static String _; return _;}
    const String mStage;

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
