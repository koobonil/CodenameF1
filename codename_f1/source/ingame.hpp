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
            if(!String::Compare(rhs, "StraightMoving"))
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
class MapMonster
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MapMonster)
public:
    MapMonster()
    {
        mEntranceSec = 0;
        mType = nullptr;
        mHP = 0;
        mDeathCount = 0;
        mPath = nullptr;
        mSpineInstance = nullptr;
        mSpineMsecOld = 0;
    }
    ~MapMonster()
    {
        TryWorld::Path::Release(mPath);
        ZAY::SpineBuilder::Release(mSpineInstance);
    }
    MapMonster(MapMonster&& rhs) {operator=(ToReference(rhs));}
    MapMonster& operator=(MapMonster&& rhs)
    {
        mEntranceSec = rhs.mEntranceSec;
        mType = rhs.mType;
        mHP = rhs.mHP;
        mDeathCount = rhs.mDeathCount;
        mPos = rhs.mPos;
        mPath = rhs.mPath;
        rhs.mPath = nullptr;
        mSpineInstance = rhs.mSpineInstance;
        rhs.mSpineInstance = nullptr;
        mSpineMsecOld = rhs.mSpineMsecOld;
        return *this;
    }

public:
    void Init(sint32 timesec, const MonsterType* type, float x, float y, ZAY::id_spine spine)
    {
        mEntranceSec = timesec;
        mType = type;
        mHP = type->mHP;
        mDeathCount = 0;
        mPos.x = x;
        mPos.y = y;
        if(spine)
        {
            mSpineInstance = ZAY::SpineBuilder::Create(spine, "default",
                [this](chars motionname)
                {
                },
                [this](chars eventname)
                {
                });
            ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "idle", true);
        }
    }
    float delta_sec() const
    {
        const uint64 SpineMsecOld = mSpineMsecOld;
        const uint64 SpineMsecNew = Platform::Utility::CurrentTimeMsec();
        mSpineMsecOld = SpineMsecNew;
        if(SpineMsecOld == 0) return 0;
        return (SpineMsecNew - SpineMsecOld) * 0.001f;
    }

public:
    sint32 mEntranceSec;
    const MonsterType* mType;
    sint32 mHP;
    sint32 mDeathCount;
    Point mPos;
    TryWorld::Path* mPath;
    ZAY::id_spine_instance mSpineInstance;
    mutable uint64 mSpineMsecOld;
};
typedef Array<MapMonster> MapMonsters;

////////////////////////////////////////////////////////////////////////////////
class SpineRenderer
{
public:
    SpineRenderer()
    {
        mSpine = nullptr;
    }
    ~SpineRenderer()
    {
        Release();
    }

public:
    void Create(chars spinepath, chars imagepath)
    {
        Release();
        // 스파인 불러오기
        Context Pathes(ST_Json, SO_NeedCopy, String::FromFile(imagepath));
        id_assetpath NewPath = Asset::CreatePath();
        for(sint32 i = 0, iend = Pathes.LengthOfIndexable(); i < iend; ++i)
            Asset::AddByPath(NewPath, Pathes[i].GetString());
        ZAY::SpineBuilder::SetAssetPath(NewPath);
        mSpine = ZAY::SpineBuilder::LoadSpine(spinepath, "images");
        ZAY::SpineBuilder::SetAssetPath(nullptr);
        Asset::ReleasePath(NewPath);
    }
    void Release()
    {
        ZAY::SpineBuilder::FreeSpine(mSpine);
        ZAY::SpineBuilder::ClearCaches();
        mSpine = nullptr;
    }
    void Render(ZayPanel& panel, const MapMonster& monster, bool guideline, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h)
    {
        if(!monster.mSpineInstance) return;
        const Point XY = panel.toview(0, 0);
        const sint32 X = (sint32) (XY.x * panel.zoom());
        const sint32 Y = (sint32) ((h - (XY.y + panel.h())) * panel.zoom());
        const sint32 W = (sint32) (panel.w() * panel.zoom());
        const sint32 H = (sint32) (panel.h() * panel.zoom());
        const float cx = ((X + W / 2.0f) - sx) / sw;
        const float cy = ((Y + H / 2.0f) - sy) / sh;
        const float scale = sh / 3000.0f;
        ZAY::SpineBuilder::Update(monster.mSpineInstance, monster.delta_sec());
        ZAY::SpineBuilder::Render(panel, monster.mSpineInstance, cx, cy, false, scale, sx, sy, sw, sh);
        ZAY::SpineBuilder::RenderBound(panel, monster.mSpineInstance, cx, cy, guideline,
            [this](ZayObject* view, chars name, GestureType type, sint32 x, sint32 y)
            {
                if(type == GT_InReleased)
                {
                }
            });
    }
    ZAY::id_spine spine() const
    {
        return mSpine;
    }

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
    void RenderObject(bool editmode, ZayPanel& panel, const MapMonster& monster, sint32 sx, sint32 sy, sint32 sw, sint32 sh);
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
    sint32 mTreeHPbarDeleteTime; // 투명해지는데 걸리는 시간
    sint32 mTreeHPRegenValue; // 초당 재생량
    sint32 mTreeHPValue; // 초기값
    sint32 mTreeHP;
    sint32 m1StarHpRate;
    sint32 m2StarHpRate;
    sint32 m3StarHpRate;
    ObjectTypes mObjectTypes;
    PolygonTypes mPolygonTypes;
    MonsterTypes mMonsterTypes;
    SpineRenderer mSpine;

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

////////////////////////////////////////////////////////////////////////////////
class ingameData : public ZayObject, public F1State
{
public:
    ingameData();
    ~ingameData();

public:
    void AnimationOnce(sint32 timespan);
    void ClearPath();
    void Render(ZayPanel& panel);
    void ReadyForNextWave();
    void SetTouchSizeR(float size);
    void TouchAttack();

public: // 게임상태
    Context mWaveData;
    sint32 mWave; // -1: 게임전, 0~N: 게임중
    String mWaveTitle;
    sint32 mWaveSec;
    MapMonsters mMonsters;
    Point mTouchPos;
    float mTouchSizeR;
    sint32 mTouchDamage;
};
