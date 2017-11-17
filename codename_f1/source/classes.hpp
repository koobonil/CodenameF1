#pragma once
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
    void SetAsset(const String& asset) {mAsset = asset;}
    void SetSpine(const String& spine) {mSpine = spine;}
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
        return *this;
    }

public:
    class TypeClass
    {
    public:
        enum Type {Target, Static, Dynamic, Ground, Hole, Max, Null = -1};
    public:
        TypeClass() {mValue = Null;}
        TypeClass(const TypeClass& rhs) {operator=(rhs);}
        TypeClass& operator=(const TypeClass& rhs) {mValue = rhs.mValue; return *this;}
        TypeClass& operator=(Type rhs) {mValue = rhs; return *this;}
        TypeClass& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Target"))
                mValue = Target;
            else if(!String::Compare(rhs, "Static"))
                mValue = Static;
            else if(!String::Compare(rhs, "Dynamic"))
                mValue = Dynamic;
            else if(!String::Compare(rhs, "Ground"))
                mValue = Ground;
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
    private:
        Type mValue;
    };

public:
    String mID;
    TypeClass mType;
    String mAssetShadow;
    sint32 mHP;
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
        mHP = 0;
        mMoveType = MoveType::Null;
        mMoveSpeed = 0;
        mTurnDistance = 0;
        mAttackPower = 0;
        mAttackSpeed = 0;
        mAttackRange = 0;
        mWaistScaleWidth = 0;
        mWaistScaleHeight = 0;
        mPolygon = "";
        mWeight = 0;
        mResistance = 0;
    }
    ~MonsterType() {}
    MonsterType(MonsterType&& rhs) {operator=(ToReference(rhs));}
    MonsterType& operator=(MonsterType&& rhs)
    {
        SpineAsset::operator=(ToReference(rhs));
        mID = rhs.mID;
        mHP = rhs.mHP;
        mMoveType = rhs.mMoveType;
        mMoveSpeed = rhs.mMoveSpeed;
        mTurnDistance = rhs.mTurnDistance;
        mAttackPower = rhs.mAttackPower;
        mAttackSpeed = rhs.mAttackSpeed;
        mAttackRange = rhs.mAttackRange;
        mWaistScaleWidth = rhs.mWaistScaleWidth;
        mWaistScaleHeight = rhs.mWaistScaleHeight;
        mPolygon = ToReference(rhs.mPolygon);
        mWeight = rhs.mWeight;
        mResistance = rhs.mResistance;
        return *this;
    }

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
    sint32 mHP;
    MoveType mMoveType;
    sint32 mMoveSpeed;
    sint32 mTurnDistance;
    sint32 mAttackPower;
    sint32 mAttackSpeed;
    sint32 mAttackRange;
    sint32 mWaistScaleWidth;
    sint32 mWaistScaleHeight;
    String mPolygon;
    sint32 mWeight;
    sint32 mResistance;
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
    enum SpineType {ST_Unknown, ST_Object, ST_Monster, ST_MonsterToast};

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
    void PlayMotion(chars motion, bool repeat);
    void PlayMotionAttached(chars first_motion, chars second_motion, bool repeat);
    void PlayMotionSeek(chars seek_motion, bool repeat);
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
    void Hit() const;
    void Dead() const;

public:
    const ObjectType* mType;
    bool mVisible;
    sint32 mHP;
    Rect mCurrentRect;
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
    bool mVisible;
    Points mPoints;
    bool mIsCW;
    sint32 mNearHoleIndex;
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
    void Init(const MonsterType* type, sint32 timesec, float x, float y,
        const SpineRenderer& renderer, const SpineRenderer* toast_renderer = nullptr);
    void ResetCB();
    bool IsKnockBackMode();
    void KnockBack(bool down, const Point& accel);
    void KnockBackEnd();
    void KnockBackEndByHole(const Point& hole);
    void Turn() const;
    sint32 TryAttack();
    void CancelAttack();
    void ClearTarget();
    void TryDeathMove();

public:
    const float mKnockBackAccelMin = 0.001f; // 화면크기비율상수
    const sint32 mKnockBackNearMin = 3; // 3px
    enum Mode {Mode_Run, Mode_Attack, Mode_Dying};

public:
    const MonsterType* mType;
    sint32 mEntranceSec;
    sint32 mHP;
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
    void RenderObject(bool needupdate, bool editmode, ZayPanel& panel, const MapSpine& spine, sint32 sx, sint32 sy, sint32 sw, sint32 sh, bool flip,
        chars uiname = nullptr, ZayPanel::SubGestureCB cb = nullptr);
    void RenderObjectShadow(ZayPanel& panel, const MapSpine& spine, sint32 sx, sint32 sy, sint32 sw, sint32 sh, bool flip);
    void RenderLayer(bool editmode, ZayPanel& panel, const MapLayer& layer,
        const MapMonsters* monsters = nullptr, const sint32 wavesec = 0, const sint32 SX = 0, const sint32 SY = 0, const sint32 SW = 0, const sint32 SH = 0);
    void Render(bool editmode, ZayPanel& panel, const MapMonsters* monsters = nullptr, sint32 wavesec = 0);

public: // 기획요소
    Solver mUILeft;
    Solver mUITop;
    Solver mUIRight;
    Solver mUIBottom;
    float mViewRate; // 뷰비율 = 가로길이 / 세로길이
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

public: // 글로벌 요소
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
    sint32 mBreathSizeR;
    sint32 mMonsterSizeR;
    sint32 mKnockBackMinV;
    sint32 mKnockBackMaxV;
    const sint32 mTimelineLength = 60;
    const sint32 mLayerLength = 6;

public: // 맵요소
    String mBGNameA;
    String mBGNameB;
    TargetZones mTargets;
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
