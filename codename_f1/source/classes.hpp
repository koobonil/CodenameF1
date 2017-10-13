#pragma once
#include <service/boss_zay.hpp>
#include <service/boss_tryworld.hpp>
#include <element/boss_solver.hpp>

#include "spine_for_zay/zay_spine_builder.hpp"

////////////////////////////////////////////////////////////////////////////////
class ObjectType
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(ObjectType)
public:
    ObjectType()
    {
    }
    ~ObjectType()
    {
    }
    ObjectType(ObjectType&& rhs) {operator=(ToReference(rhs));}
    ObjectType& operator=(ObjectType&& rhs)
    {
        mID = rhs.mID;
        mAsset = rhs.mAsset;
        return *this;
    }

public:
    String mID;
    String mAsset;
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
        mID = rhs.mID;
        mType = rhs.mType;
        mColorR = rhs.mColorR;
        mColorG = rhs.mColorG;
        mColorB = rhs.mColorB;
        return *this;
    }

public:
    class ZoneType
    {
    public:
        enum Type {Spot, Wall, Mud, Water, Max, Null = -1};
    public:
        ZoneType() {mValue = Null;}
        ZoneType(const ZoneType& rhs) {operator=(rhs);}
        ZoneType& operator=(const ZoneType& rhs) {mValue = rhs.mValue; return *this;}
        ZoneType& operator=(Type rhs) {mValue = rhs; return *this;}
        ZoneType& operator=(chars rhs)
        {
            if(!String::Compare(rhs, "Spot"))
                mValue = Spot;
            else if(!String::Compare(rhs, "Wall"))
                mValue = Wall;
            else if(!String::Compare(rhs, "Mud"))
                mValue = Mud;
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
    ZoneType mType;
    sint32 mColorR;
    sint32 mColorG;
    sint32 mColorB;
};
typedef Array<PolygonType> PolygonTypes;

////////////////////////////////////////////////////////////////////////////////
class MonsterType
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MonsterType)
public:
    MonsterType()
    {
        mHP = 0;
        mMoveType = MoveType::Null;
        mMoveSpeed = 0;
        mMoveSight = 1;
        mTurnDistance = 0;
        mAttackPower = 0;
        mAttackSpeed = 0;
        mAttackRange = 0;
        mAsset = "noname";
        mScale = 0;
    }
    ~MonsterType() {}
    MonsterType(MonsterType&& rhs) {operator=(ToReference(rhs));}
    MonsterType& operator=(MonsterType&& rhs)
    {
        mID = rhs.mID;
        mHP = rhs.mHP;
        mMoveType = rhs.mMoveType;
        mMoveSpeed = rhs.mMoveSpeed;
        mMoveSight = rhs.mMoveSight;
        mTurnDistance = rhs.mTurnDistance;
        mAttackPower = rhs.mAttackPower;
        mAttackSpeed = rhs.mAttackSpeed;
        mAttackRange = rhs.mAttackRange;
        mAsset = ToReference(rhs.mAsset);
        mScale = rhs.mScale;
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
    sint32 mMoveSight;
    sint32 mTurnDistance;
    sint32 mAttackPower;
    sint32 mAttackSpeed;
    sint32 mAttackRange;
    String mAsset;
    sint32 mScale;
};
typedef Array<MonsterType> MonsterTypes;

////////////////////////////////////////////////////////////////////////////////
class MapObject
{
public:
    MapObject() {mType = nullptr;}
    ~MapObject() {}
public:
    const ObjectType* mType;
    Rect mRect;
};
typedef Array<MapObject> MapObjects;

////////////////////////////////////////////////////////////////////////////////
class MapPolygon
{
public:
    MapPolygon() {mType = nullptr;}
    ~MapPolygon() {}
public:
    const PolygonType* mType;
    Points mPolygon;
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
class SpineRenderer;
class MapMonster
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MapMonster)
public:
    MapMonster();
    ~MapMonster();
    MapMonster(MapMonster&& rhs);
    MapMonster& operator=(MapMonster&& rhs);

public:
    void Init(chars uiname, sint32 timesec, const MonsterType* type, float x, float y, const SpineRenderer* renderer);
    void Kill();
    void Turn();
    void Hit();
    void Attacking();
    float CalcDeltaSec() const;

public:
    inline const SpineRenderer* renderer() const {return mSpineRenderer;}

public:
    String mUIName;
    sint32 mEntranceSec;
    const MonsterType* mType;
    sint32 mHP;
    bool mAttackMode;
    bool mFlipMode;
    bool mLastFlip;
    Point mLastFlipPos;
    sint32 mDeathCount;
    Point mPos;
    TryWorld::Path* mPath;
    const SpineRenderer* mSpineRenderer;
    ZAY::id_spine_instance mSpineInstance;
    mutable uint64 mSpineMsecOld;
};
typedef Array<MapMonster> MapMonsters;

////////////////////////////////////////////////////////////////////////////////
class SpineRenderer
{
public:
    SpineRenderer();
    ~SpineRenderer();

public:
    void Create(chars spinepath, chars imagepath);
    void Release();
    Rect Update(const MapMonster& monster) const;
    void Render(const MapMonster& monster, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip) const;
    void RenderBound(const MapMonster& monster, ZayPanel& panel, bool guideline, float ox, float oy, float scale, bool flip) const;

public:
    inline ZAY::id_spine spine() const {return mSpine;}

private:
    ZAY::id_spine mSpine;
};

////////////////////////////////////////////////////////////////////////////////
class F1State
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(F1State)
public:
    F1State();
    ~F1State();

public:
    void LoadMap(chars json);
    String SaveMap();
    void RebuildTryWorld();
    void SetSize(sint32 width, sint32 height);
    void RenderImage(bool editmode, ZayPanel& panel, const Image& image);
    void RenderObject(bool editmode, ZayPanel& panel, const SpineRenderer* renderer, const MapMonster& monster, sint32 sx, sint32 sy, sint32 sw, sint32 sh, bool flip);
    void Render(bool editmode, ZayPanel& panel, const MapMonsters* monsters = nullptr, sint32 wavesec = 0);

public: // 기획요소
    Solver mUILeft;
    Solver mUITop;
    Solver mUIRight;
    Solver mUIBottom;
    float mViewRate; // 뷰비율 = 가로길이 / 세로길이
    float mMeteoMinScale;
    float mMeteoMaxScale;
    sint32 mMeteoMinDamage;
    sint32 mMeteoMaxDamage;
    sint32 mEggHPbarDeleteTime; // 투명해지는데 걸리는 시간
    sint32 mEggHPRegenValue; // 초당 재생량
    sint32 mEggHPValue; // 초기값
    sint32 mEggHP;
    sint32 m1StarHpRate;
    sint32 m2StarHpRate;
    sint32 m3StarHpRate;
    sint32 mToolGrid;
    ObjectTypes mObjectTypes;
    PolygonTypes mPolygonTypes;
    MonsterTypes mMonsterTypes;
    Map<SpineRenderer> mSpines;

public: // UI요소
    sint32 mScreenW;
    sint32 mScreenH;
    sint32 mInGameW;
    sint32 mInGameH;
    sint32 mInGameX;
    sint32 mInGameY;
    sint32 mTreeSizeR;
    sint32 mMonsterSizeR;
    sint32 mMeteoSizeMinR;
    sint32 mMeteoSizeMaxR;
    const sint32 mTimelineLength = 60;
    const sint32 mLayerLength = 6;

public: // 맵요소
    String mBGName;
    MapLayers mLayers;
    TryWorld::Hurdle* mHurdle;
    TryWorld::Map* mMap;
};
