#include <boss.hpp>
#include "classes.hpp"

#include <r.hpp>

////////////////////////////////////////////////////////////////////////////////
static sint32 gSpineRendererCount = 0;
SpineRenderer::SpineRenderer()
{
    gSpineRendererCount++;
    mSpine = nullptr;
}

SpineRenderer::~SpineRenderer()
{
    Release();
    if(--gSpineRendererCount == 0)
        ZAY::SpineBuilder::ClearCaches();
}

void SpineRenderer::Create(chars spinepath, chars imagepath)
{
    Release();
    // 스파인 불러오기
    Context Pathes(ST_Json, SO_NeedCopy, String::FromFile(imagepath));
    id_assetpath NewPath = Asset::CreatePath();
    for(sint32 i = 0, iend = Pathes.LengthOfIndexable(); i < iend; ++i)
        Asset::AddByPath(NewPath, Pathes[i].GetString());
    ZAY::SpineBuilder::SetAssetPath(NewPath);
    mSpine = ZAY::SpineBuilder::LoadSpine(spinepath, "image");
    ZAY::SpineBuilder::SetAssetPath(nullptr);
    Asset::ReleasePath(NewPath);
}

void SpineRenderer::Release()
{
    ZAY::SpineBuilder::FreeSpine(mSpine);
    mSpine = nullptr;
}

void SpineRenderer::Render(ZAY::id_spine_instance instance, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip) const
{
    if(!instance) return;
    const Point XY = panel.toview(0, 0);
    const sint32 X = (sint32) (XY.x * panel.zoom());
    const sint32 Y = (sint32) ((h - (XY.y + panel.h())) * panel.zoom());
    const sint32 W = (sint32) (panel.w() * panel.zoom());
    const sint32 H = (sint32) (panel.h() * panel.zoom());
    const float cx = ((X + W * 0.5f) - sx) / sw;
    const float cy = ((Y + H * 0.5f) - sy) / sh;
    ZAY::SpineBuilder::Render(panel, instance, flip, cx, cy, scale, sx, sy, sw, sh);
}

void SpineRenderer::RenderBound(ZAY::id_spine_instance instance, ZayPanel& panel, bool guideline, float ox, float oy, float scale, bool flip,
    chars uiname, ZayPanel::SubGestureCB cb) const
{
    if(!instance) return;
    ZAY::SpineBuilder::RenderBound(panel, instance, ox, oy, scale, flip, guideline, uiname, cb);
}

////////////////////////////////////////////////////////////////////////////////
MapSpine::MapSpine()
{
    mSpineRenderer = nullptr;
    mSpineInstance = nullptr;
    mSpineMsecOld = 0;
    mSeekMode = false;
    mSeekSec = 0;
    mSeekSecOld = -1;
    mStaffIdleMode = false;
    mStaffStartMode = false;
}

MapSpine::~MapSpine()
{
    ZAY::SpineBuilder::Release(mSpineInstance);
}

MapSpine::MapSpine(const MapSpine& rhs)
{
    mSpineInstance = nullptr;
    operator=(rhs);
}

MapSpine& MapSpine::operator=(const MapSpine& rhs)
{
    mSpineRenderer = rhs.mSpineRenderer;
    ZAY::SpineBuilder::Release(mSpineInstance);
    mSpineInstance = nullptr;
    if(rhs.mSpineInstance)
        mSpineInstance = ZAY::SpineBuilder::Clone(rhs.mSpineInstance);
    mSpineMsecOld = rhs.mSpineMsecOld;
    mSeekMode = rhs.mSeekMode;
    mSeekSec = rhs.mSeekSec;
    mSeekSecOld = rhs.mSeekSecOld;
    mStaffIdleMode = rhs.mStaffIdleMode;
    mStaffStartMode = rhs.mStaffStartMode;
    return *this;
}

MapSpine::MapSpine(MapSpine&& rhs)
{
    operator=(ToReference(rhs));
}

MapSpine& MapSpine::operator=(MapSpine&& rhs)
{
    mSpineRenderer = rhs.mSpineRenderer;
    mSpineInstance = rhs.mSpineInstance;
    rhs.mSpineInstance = nullptr;
    mSpineMsecOld = rhs.mSpineMsecOld;
    mSeekMode = rhs.mSeekMode;
    mSeekSec = rhs.mSeekSec;
    mSeekSecOld = rhs.mSeekSecOld;
    mStaffIdleMode = rhs.mStaffIdleMode;
    mStaffStartMode = rhs.mStaffStartMode;
    return *this;
}

void MapSpine::InitSpine(const SpineRenderer* renderer, chars first_motion, chars second_motion,
    ZAY::SpineBuilder::MotionFinishedCB fcb, ZAY::SpineBuilder::UserEventCB ecb)
{
    if(mSpineRenderer = renderer)
    {
        mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), "default", fcb, ecb);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, first_motion, second_motion == nullptr);
        if(second_motion)
            ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, first_motion, second_motion, true);
    }
}

void MapSpine::InitSpineForSeek(const SpineRenderer* renderer, chars motion, bool repeat)
{
    if(mSpineRenderer = renderer)
    {
        mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), "default", nullptr, nullptr);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, motion, repeat);
        mSeekMode = true;
    }
}

void MapSpine::Seek() const
{
    if(mSpineInstance)
    {
        mSeekSecOld = mSeekSec;
        ZAY::SpineBuilder::Seek(mSpineInstance, mSeekSec);
    }
}

void MapSpine::Update() const
{
    if(mSpineInstance)
    {
        const uint64 SpineMsecOld = mSpineMsecOld;
        const uint64 SpineMsecNew = Platform::Utility::CurrentTimeMsec();
        mSpineMsecOld = SpineMsecNew;
        const float DeltaSec = (SpineMsecOld == 0)?
            0 : (SpineMsecNew - SpineMsecOld) * 0.001f;
        ZAY::SpineBuilder::Update(mSpineInstance, DeltaSec);
    }
}

Rect MapSpine::GetBoundRect(chars name) const
{
    if(!mSpineInstance) return Rect(0, 0, 0, 0);
    return ZAY::SpineBuilder::GetBoundRect(mSpineInstance, name);
}

void MapSpine::SetSeekSec(float sec)
{
    mSeekSec = sec;
}

bool MapSpine::IsSeekUpdated() const
{
    return (mSeekSec != mSeekSecOld);
}

void MapSpine::Staff_TryIdle()
{
    if(mSpineInstance && !mStaffIdleMode)
    {
        mStaffIdleMode = true;
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "idle", true);
    }
}

void MapSpine::Staff_Start()
{
    if(mSpineInstance && !mStaffStartMode)
    {
        mStaffIdleMode = true;
        mStaffStartMode = true;
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "start", false);
    }
}

////////////////////////////////////////////////////////////////////////////////
MapObject::MapObject()
{
    mType = nullptr;
    mVisible = true;
}

MapObject::~MapObject()
{
}

MapObject::MapObject(const MapObject& rhs)
{
    operator=(rhs);
}

MapObject& MapObject::operator=(const MapObject& rhs)
{
    MapSpine::operator=(rhs);
    mType = rhs.mType;
    mVisible = rhs.mVisible;
    mRect = rhs.mRect;
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
    mVisible = rhs.mVisible;
    mRect = rhs.mRect;
    return *this;
}

void MapObject::Hit() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "hit", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "hit", "idle", true);
    }
}

void MapObject::Dead() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "dead", false);
    }
}

////////////////////////////////////////////////////////////////////////////////
MapMonster::MapMonster()
{
    mType = nullptr;
    mEntranceSec = 0;
    mHP = 0;
    mAttackMode = false;
    mAttackCount = 0;
    mAttackTimeMsec = 0;
    mFlipMode = false;
    mLastFlip = false;
    mLastFlipPos = Point(0, 0);
    mDeathCount = 0;
    mTargetId = -1;
    mTargetPath = nullptr;
    mTargetPathScore = 0;
}

MapMonster::~MapMonster()
{
    TryWorld::Path::Release(mTargetPath);
}

MapMonster::MapMonster(MapMonster&& rhs)
{
    operator=(ToReference(rhs));
}

MapMonster& MapMonster::operator=(MapMonster&& rhs)
{
    MapSpine::operator=(ToReference(rhs));
    mType = rhs.mType;
    mEntranceSec = rhs.mEntranceSec;
    mHP = rhs.mHP;
    mAttackMode = rhs.mAttackMode;
    mAttackCount = rhs.mAttackCount;
    mAttackTimeMsec = rhs.mAttackTimeMsec;
    mFlipMode = rhs.mFlipMode;
    mLastFlip = rhs.mLastFlip;
    mLastFlipPos = rhs.mLastFlipPos;
    mDeathCount = rhs.mDeathCount;
    mPos = rhs.mPos;
    mTargetId = rhs.mTargetId;
    mTargetPath = rhs.mTargetPath;
    rhs.mTargetPath = nullptr;
    mTargetPathScore = rhs.mTargetPathScore;
    return *this;
}

void MapMonster::Init(const MonsterType* type, sint32 timesec, float x, float y)
{
    mType = type;
    mEntranceSec = timesec;
    mHP = type->mHP;
    mAttackMode = false;
    mAttackCount = 0;
    mAttackTimeMsec = 0;
    mFlipMode = (x < 0);
    mLastFlip = mFlipMode;
    mDeathCount = 0;
    mPos.x = x;
    mPos.y = y;
}

void MapMonster::Kill() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "dead", false);
    }
}

void MapMonster::Turn() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "flip", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "flip", (mAttackMode)? "attack" : "run", true);
    }
}

void MapMonster::Hit() const
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "hit", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "hit", (mAttackMode)? "attack" : "run", true);
    }
}

sint32 MapMonster::TryAttack()
{
    if(!mSpineInstance)
        return 0;

    if(!mAttackMode)
    {
        mAttackMode = true;
        mAttackCount = 0;
        mAttackTimeMsec = Platform::Utility::CurrentTimeMsec();
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "attack", true);
    }

    const sint32 CurAttackCount = (Platform::Utility::CurrentTimeMsec() - mAttackTimeMsec) / mType->mAttackSpeed;
    const sint32 Result = CurAttackCount - mAttackCount;
    mAttackCount = CurAttackCount;
    return Result;
}

void MapMonster::CancelAttack()
{
    if(mSpineInstance && mAttackMode)
    {
        mAttackMode = false;
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "run", true);
    }
}

////////////////////////////////////////////////////////////////////////////////
TargetZone::TargetZone()
{
    mLayerId = 0;
    mObjectId = 0;
    mHP = 0;
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
    mLayerId = rhs.mLayerId;
    mObjectId = rhs.mObjectId;
    mHP = rhs.mHP;
    mPos = rhs.mPos;
    mSizeR = rhs.mSizeR;
    return *this;
}

void TargetZone::Init(sint32 layerid, sint32 objectid, sint32 hp, float x, float y, float size_r)
{
    mLayerId = layerid;
    mObjectId = objectid;
    mHP = hp;
    mPos.x = x;
    mPos.y = y;
    mSizeR = size_r;
}

////////////////////////////////////////////////////////////////////////////////
F1State::F1State()
{
    Map<String> GlobalWeightMap;
    if(auto GlobalWeightTable = Context(ST_Json, SO_NeedCopy, String::FromFile("table/globalweight_table.json")))
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

    mViewRate = 5000.0f / 6000.0f;

    mBreathScale = Parser::GetInt(GlobalWeightMap("BreathScale")) / 1000.0f;
    mBreathMinDamage = Parser::GetInt(GlobalWeightMap("BreathMinDamage"));
    mBreathMaxDamage = Parser::GetInt(GlobalWeightMap("BreathMaxDamage"));
    mEggHPbarDeleteTime = Parser::GetInt(GlobalWeightMap("EggHPbarDeleteTime")); // HP가 다시 투명이되는데 걸리는 시간
    mEggHPRegenValue = Parser::GetInt(GlobalWeightMap("EggHPRegenValue")); // HP의 초당 재생량
    mEggHP = Parser::GetInt(GlobalWeightMap("EggHP"));
    m1StarHpRate = Parser::GetInt(GlobalWeightMap("1StarHpRate"));
    m2StarHpRate = Parser::GetInt(GlobalWeightMap("2StarHpRate"));
    m3StarHpRate = Parser::GetInt(GlobalWeightMap("3StarHpRate"));
    mMonsterScale = Parser::GetInt(GlobalWeightMap("MonsterScale")) / 1000.0f;
    mToolGrid = Parser::GetInt(GlobalWeightMap("ToolGrid"));
    mDragonEntryTime = Parser::GetInt(GlobalWeightMap("DragonEntryTime"));
    mDragonBreathTime = Parser::GetInt(GlobalWeightMap("DragonBreathTime"));
    mDragonExitTime = Parser::GetInt(GlobalWeightMap("DragonExitTime"));

    auto ObjectTable = Context(ST_Json, SO_NeedCopy, String::FromFile("table/object_table.json"));
    if(ObjectTable.IsValid())
    {
        for(sint32 i = 0, iend = ObjectTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewObjectType = mObjectTypes.AtAdding();
            NewObjectType.mID = ObjectTable[i]("ID").GetString("");
            NewObjectType.mType = ObjectTable[i]("Type").GetString("Null");
            NewObjectType.mAsset = ObjectTable[i]("Asset").GetString("noname");
        }
    }
    else BOSS_ASSERT("object_table.json의 로딩에 실패하였습니다", false);

    auto PolygonTable = Context(ST_Json, SO_NeedCopy, String::FromFile("table/polygon_table.json"));
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

    auto MonsterTable = Context(ST_Json, SO_NeedCopy, String::FromFile("table/monster_table.json"));
    if(MonsterTable.IsValid())
    {
        for(sint32 i = 0, iend = MonsterTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewMonsterType = mMonsterTypes.AtAdding();
            NewMonsterType.mID = MonsterTable[i]("ID").GetString("");
            NewMonsterType.mHP = MonsterTable[i]("HP").GetInt(100);
            NewMonsterType.mMoveType = MonsterTable[i]("MoveType").GetString("Null");
            NewMonsterType.mMoveSpeed = MonsterTable[i]("MoveSpeed").GetInt(1000);
            NewMonsterType.mMoveSight = Math::Max(1, MonsterTable[i]("MoveSight").GetInt(5));
            NewMonsterType.mTurnDistance = MonsterTable[i]("TurnDistance").GetInt(10);
            NewMonsterType.mAttackPower = MonsterTable[i]("AttackPower").GetInt(100);
            NewMonsterType.mAttackSpeed = MonsterTable[i]("AttackSpeed").GetInt(1000);
            NewMonsterType.mAttackRange = MonsterTable[i]("AttackRange").GetInt(100);
            NewMonsterType.mAsset = MonsterTable[i]("Asset").GetString("noname");
        }
    }
    else BOSS_ASSERT("monster_table.json의 로딩에 실패하였습니다", false);

    Platform::File::Search("assets:/spine",
        [](chars name, payload data)->void
        {
            F1State* This = (F1State*) data;
            This->mSpines(name).Create(String::Format("spine/%s/spine.json", name), String::Format("spine/%s/path.json", name));
        }, this, false);

    mScreenW = 0;
    mScreenH = 0;
    mInGameW = 0;
    mInGameH = 0;
    mInGameX = 0;
    mInGameY = 0;
    mBreathSizeR = 0;
    mMonsterSizeR = 0;

    mBGName = "";
    mLayers.AtDumpingAdded(mLayerLength);

    mHurdle = nullptr;
    mMap = nullptr;
}

F1State::~F1State()
{
    TryWorld::Hurdle::Release(mHurdle);
    TryWorld::Map::Release(mMap);
}

void F1State::LoadMap(chars json)
{
    mBGName = "";
    mTargets.SubtractionAll();
    mLayers.SubtractionAll();
    mLayers.AtDumpingAdded(mLayerLength);

    Context JsonLayer(ST_Json, SO_OnlyReference, json);
    mBGName = JsonLayer("BGName").GetString();
    for(sint32 layer = 0, layer_end = JsonLayer("Layers").LengthOfIndexable(); layer < layer_end; ++layer)
    {
        auto& CurJsonLayer = JsonLayer("Layers")[layer];
        const sint32 LayerID = CurJsonLayer("LayerID").GetInt();
        if(mLayerLength <= LayerID) continue;

        auto& CurLayer = mLayers.At(LayerID);
        for(sint32 obj = 0, obj_end = CurJsonLayer("Objects").LengthOfIndexable(); obj < obj_end; ++obj)
        {
            const sint32 NewObjectId = CurLayer.mObjects.Count();
            auto& NewObject = CurLayer.mObjects.AtAdding();
            auto& CurJsonObject = CurJsonLayer("Objects")[obj];

            const String CurJsonObjectID = CurJsonObject("ID").GetString();
            NewObject.mType = &mObjectTypes[0];
            for(sint32 i = 0, iend = mObjectTypes.Count(); i < iend; ++i)
                if(!CurJsonObjectID.Compare(mObjectTypes[i].mID))
                {
                    NewObject.mType = &mObjectTypes[i];
                    break;
                }
            NewObject.mVisible = (CurJsonObject("Visible").GetInt(1) != 0)? true : false;
            NewObject.mRect.l = CurJsonObject("RectL").GetFloat();
            NewObject.mRect.t = CurJsonObject("RectT").GetFloat();
            NewObject.mRect.r = CurJsonObject("RectR").GetFloat();
            NewObject.mRect.b = CurJsonObject("RectB").GetFloat();
            if(auto CurSpine = mSpines.Access(NewObject.mType->mAsset))
                NewObject.InitSpine(CurSpine, "idle");
            if(NewObject.mType->mType == ObjectType::TypeClass::Target)
            {
                auto& NewTarget = mTargets.AtAdding();
                NewTarget.Init(LayerID, NewObjectId, mEggHP,
                    NewObject.mRect.CenterX(), NewObject.mRect.CenterY(),
                    Math::Sqrt(Math::Pow(NewObject.mRect.Width()) * 2) / 2);
            }
        }
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
            NewPolygon.mVisible = (CurJsonPolygon("Visible").GetInt(1) != 0)? true : false;
            NewPolygon.mPolygon.AtDumpingAdded(CurJsonPolygon("Points").LengthOfIndexable());
            for(sint32 i = 0, iend = NewPolygon.mPolygon.Count(); i < iend; ++i)
            {
                NewPolygon.mPolygon.At(i).x = CurJsonPolygon("Points")[i]("X").GetFloat();
                NewPolygon.mPolygon.At(i).y = CurJsonPolygon("Points")[i]("Y").GetFloat();
            }
        }
    }
}

String F1State::SaveMap()
{
    Context JsonLayer;
    JsonLayer.At("BGName").Set(mBGName);
    for(sint32 layer = 0; layer < mLayerLength; ++layer)
    {
        auto& CurLayer = mLayers.At(layer);
        if(CurLayer.mObjects.Count() == 0 && CurLayer.mPolygons.Count() == 0)
            continue;

        auto& NewJsonLayer = JsonLayer.At("Layers").At(layer);
        NewJsonLayer.At("LayerID").Set(String::FromInteger(layer));
        for(sint32 obj = 0, obj_end = CurLayer.mObjects.Count(); obj < obj_end; ++obj)
        {
            auto& CurObject = CurLayer.mObjects[obj];
            auto& NewJsonObject = NewJsonLayer.At("Objects").At(obj);

            NewJsonObject.At("ID").Set(CurObject.mType->mID);
            NewJsonObject.At("Visible").Set((CurObject.mVisible)? "1" : "0");
            NewJsonObject.At("RectL").Set(String::FromFloat(CurObject.mRect.l));
            NewJsonObject.At("RectT").Set(String::FromFloat(CurObject.mRect.t));
            NewJsonObject.At("RectR").Set(String::FromFloat(CurObject.mRect.r));
            NewJsonObject.At("RectB").Set(String::FromFloat(CurObject.mRect.b));
        }
        for(sint32 plg = 0, plg_end = CurLayer.mPolygons.Count(); plg < plg_end; ++plg)
        {
            auto& CurPolygon = CurLayer.mPolygons[plg];
            auto& NewJsonPolygon = NewJsonLayer.At("Polygons").At(plg);

            NewJsonPolygon.At("ID").Set(CurPolygon.mType->mID);
            NewJsonPolygon.At("Visible").Set((CurPolygon.mVisible)? "1" : "0");
            for(sint32 i = 0, iend = CurPolygon.mPolygon.Count(); i < iend; ++i)
            {
                NewJsonPolygon.At("Points").At(i).At("X").Set(String::FromFloat(CurPolygon.mPolygon[i].x));
                NewJsonPolygon.At("Points").At(i).At("Y").Set(String::FromFloat(CurPolygon.mPolygon[i].y));
            }
        }
    }
    return JsonLayer.SaveJson();
}

void F1State::RebuildTryWorld()
{
    TryWorld::Hurdle::Release(mHurdle);
    TryWorld::Map::Release(mMap);

    mHurdle = TryWorld::Hurdle::Create();
    for(sint32 i = 0; i < mLayerLength; ++i)
    {
        auto& CurPolygons = mLayers[i].mPolygons;
        for(sint32 j = 0, jend = CurPolygons.Count(); j < jend; ++j)
        {
            if(CurPolygons[j].mType->mType != PolygonType::TypeClass::Wall)
                continue;
            auto& CurPoints = CurPolygons[j].mPolygon;
            Points NewPoints;
            NewPoints.AtDumpingAdded(CurPoints.Count());
            for(sint32 k = 0, kend = CurPoints.Count(); k < kend; ++k)
            {
                const float x = mInGameW * (CurPoints[k].x + 0.5f);
                const float y = mInGameH * (CurPoints[k].y + 0.5f);
                NewPoints.At(k) = Point(x, y);
            }
            mHurdle->Add(NewPoints);
        }
    }
    mMap = mHurdle->BuildMap(Rect(-mInGameW, -mInGameH, mInGameW * 2, mInGameH * 2));
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
    const sint32 ViewL = (sint32) mUILeft.result();
    const sint32 ViewT = (sint32) mUITop.result();
    const sint32 ViewR = (sint32) mUIRight.result();
    const sint32 ViewB = (sint32) mUIBottom.result();
    const sint32 ViewWidth = Math::Max(0, width - (ViewL + ViewR));
    const sint32 ViewHeight = Math::Max(0, height - (ViewT + ViewB));
    const float CurRate = ViewWidth / (float) ViewHeight;
    mScreenW = width;
    mScreenH = height;
    mInGameW = (sint32) (ViewWidth * ((CurRate < mViewRate)? 1 : mViewRate / CurRate));
    mInGameH = (sint32) (ViewHeight * ((CurRate < mViewRate)? CurRate / mViewRate : 1));
    mInGameX = ViewL + (ViewWidth - mInGameW) / 2;
    mInGameY = ViewT + (ViewHeight - mInGameH) / 2;
    mBreathSizeR = mInGameW * mBreathScale / 2;
    mMonsterSizeR = mInGameW * mMonsterScale / 2;
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

void F1State::RenderObject(bool editmode, ZayPanel& panel, const MapSpine& spine, sint32 sx, sint32 sy, sint32 sw, sint32 sh, bool flip,
    chars uiname, ZayPanel::SubGestureCB cb)
{
    if(spine.mSeekMode)
    {
        if(spine.IsSeekUpdated())
            spine.Seek();
    }
    else spine.Update();

    const Rect AreaRect = spine.GetBoundRect("area");
    const float Width = Math::MaxF(0.001, AreaRect.Width());
    const float Height = Math::MaxF(0.001, AreaRect.Height());
    const float Rate = Math::MinF(panel.w() / Width, panel.h() / Height);
    const float CX = (flip)? -AreaRect.CenterX() : AreaRect.CenterX();
    const float CY = AreaRect.CenterY();

    ZAY_XYRR(panel, panel.w() / 2 - CX * Rate, panel.h() / 2 - CY * Rate, 0, 0)
    {
        Platform::Graphics::BeginGL();
        spine.renderer()->Render(spine.mSpineInstance, panel, sx, mScreenH - (sy + sh), sw, sh, mScreenH, Rate, flip);
        Platform::Graphics::EndGL();
    }

    if(editmode)
    ZAY_RGBA(panel, 255, 255, 255, 128)
        panel.fill();

    if(editmode || uiname)
    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, 0, 0)
        spine.renderer()->RenderBound(spine.mSpineInstance, panel, editmode, CX, CY, Rate, flip, uiname, cb);

    if(editmode)
    ZAY_RGBA(panel, 255, 0, 0, 128)
        panel.rect(1);
}

void F1State::RenderLayer(bool editmode, bool titlemode, ZayPanel& panel, const MapLayer& layer,
    const MapMonsters* monsters, const sint32 wavesec, const sint32 SX, const sint32 SY, const sint32 SW, const sint32 SH)
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
        if(0 < (*monsters)[i].mHP || 0 < (*monsters)[i].mDeathCount)
        {
            auto& NewNode = Nodes.AtAdding();
            NewNode.mIsMonster = true;
            NewNode.mData = &(*monsters)[i];
            NewNode.mRect.l = mInGameW * ((*monsters)[i].mPos.x + 0.5f) - mMonsterSizeR;
            NewNode.mRect.t = mInGameH * ((*monsters)[i].mPos.y + 0.5f) - mMonsterSizeR;
            NewNode.mRect.r = NewNode.mRect.l + mMonsterSizeR * 2;
            NewNode.mRect.b = NewNode.mRect.t + mMonsterSizeR * 2;
            NewNode.BindUpdater(&Head);
            Head.Sort(&NewNode, OrderNode::Sorter);
        }
    }

    // 오브젝트 수집
    auto& CurObjects = layer.mObjects;
    for(sint32 i = 0, iend = CurObjects.Count(); i < iend; ++i)
    {
        if(!editmode && !CurObjects[i].mVisible)
            continue;
        auto& NewNode = Nodes.AtAdding();
        NewNode.mIsMonster = false;
        NewNode.mData = &CurObjects[i];
        NewNode.mRect.l = mInGameW * (CurObjects[i].mRect.l + 0.5f);
        NewNode.mRect.t = mInGameH * (CurObjects[i].mRect.t + 0.5f);
        NewNode.mRect.r = mInGameW * (CurObjects[i].mRect.r + 0.5f);
        NewNode.mRect.b = mInGameH * (CurObjects[i].mRect.b + 0.5f);
        NewNode.BindUpdater(&Head);
        Head.Sort(&NewNode, OrderNode::Sorter);
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
                if(auto CurRenderer = CurMonster->renderer())
                    RenderObject(editmode, panel, *CurMonster, SX, SY, SW, SH, CurMonster->mFlipMode);
                else RenderImage(editmode, panel, R(CurMonster->mType->mAsset));
                // 몬스터HP
                if(!titlemode)
                {
                    const String HP = String::Format("HP-%d", CurMonster->mHP);
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.text(panel.w() / 2 + 1, panel.h() + 1, HP, UIFA_CenterTop);
                    ZAY_RGB(panel, 255, 255, 255)
                        panel.text(panel.w() / 2, panel.h(), HP, UIFA_CenterTop);
                }
            }
            else
            {
                const MapObject* CurObject = (const MapObject*) CurOrderNode->mData;
                if(CurObject->mVisible)
                {
                    if(auto CurRenderer = CurObject->renderer())
                        RenderObject(editmode, panel, *CurObject, SX, SY, SW, SH, false);
                    else RenderImage(editmode, panel, R(CurObject->mType->mAsset));
                    // 타입ID
                    if(editmode)
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

    // 폴리곤
    if(editmode)
    {
        auto& CurPolygons = layer.mPolygons;
        for(sint32 j = 0, jend = CurPolygons.Count(); j < jend; ++j)
        {
            auto& CurPoints = CurPolygons[j].mPolygon;
            Points NewPoints;
            NewPoints.AtDumpingAdded(CurPoints.Count());
            for(sint32 k = 0, kend = CurPoints.Count(); k < kend; ++k)
            {
                const float x = mInGameW * (CurPoints[k].x + 0.5f);
                const float y = mInGameH * (CurPoints[k].y + 0.5f);
                NewPoints.At(k) = Point(x, y);
            }
            if(CurPolygons[j].mVisible)
            {
                const sint32 ColorR = CurPolygons[j].mType->mColorR;
                const sint32 ColorG = CurPolygons[j].mType->mColorG;
                const sint32 ColorB = CurPolygons[j].mType->mColorB;
                ZAY_RGBA(panel, ColorR, ColorG, ColorB, 128)
                    panel.polygon(NewPoints);
            }
            else
            {
                ZAY_RGBA(panel, 64, 64, 64, 64)
                    panel.polygon(NewPoints);
            }
        }
    }

    // 몬스터와 오브젝트 순서출력
    /*if(editmode)
    {
        sint32 OrderId = 0;
        CurNode = &Head;
        ZAY_FONT(panel, 0.9)
        while((CurNode = CurNode->Next()) != &Head)
        {
            OrderNode* CurOrderNode = (OrderNode*) CurNode;
            ZAY_RECT(panel, CurOrderNode->mRect)
            {
                ZAY_RGB(panel, 0, 0, 0)
                    panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, String::Format("%d", OrderId), UIFA_CenterMiddle);
                ZAY_RGB(panel, 0, 0, 255)
                    panel.text(panel.w() / 2, panel.h() / 2, String::Format("%d", OrderId), UIFA_CenterMiddle);
            }
            OrderId++;
        }
    }*/
}

void F1State::Render(bool editmode, bool titlemode, ZayPanel& panel, const MapMonsters* monsters, sint32 wavesec)
{
    const Point XY = panel.toview(0, 0);
    const sint32 SX = (sint32) (XY.x * panel.zoom());
    const sint32 SY = (sint32) (XY.y * panel.zoom());
    const sint32 SW = (sint32) (panel.w() * panel.zoom());
    const sint32 SH = (sint32) (panel.h() * panel.zoom());

    // 배경
    const Image& BGImage = R(mBGName);
    const float XRate = SW / (float) BGImage.GetWidth();
    const float YRate = SH / (float) BGImage.GetHeight();
    const sint32 DstX = (sint32) (SX - BGImage.L() * XRate);
    const sint32 DstY = (sint32) (SY - BGImage.T() * YRate);
    const sint32 DstWidth = (sint32) (BGImage.GetImageWidth() * XRate);
    const sint32 DstHeight = (sint32) (BGImage.GetImageHeight() * YRate);
    panel.stretch(BGImage, true);

    // 배경의 아웃라인(전)
    const Rect OutlineRect(Math::Max(0, DstX) - SX, Math::Max(0, DstY) - SY,
        Math::Min(DstX + DstWidth, mScreenW) - SX, Math::Min(DstY + DstHeight, mScreenH) - SY);
    const sint32 OutlineHeightHalf = OutlineRect.Height() / 2;
    ZAY_LTRB_SCISSOR(panel, OutlineRect.l, OutlineRect.t, OutlineRect.r, OutlineRect.t + OutlineHeightHalf)
    ZAY_LTRB(panel, 0, 0, OutlineRect.Width(), OutlineRect.Height())
        panel.stretch(R("black_aera"), true);

    // 레이어
    ZAY_FONT(panel, 1.1, "Arial Black")
    for(sint32 i = 0; i < mLayerLength; ++i)
    {
        if(!mLayers[i].mShowFlag) continue;
        RenderLayer(editmode, titlemode, panel, mLayers[i],
            (i == 2)? monsters : nullptr, wavesec, SX, SY, SW, SH);
    }

    // 타겟
    if(!titlemode)
    for(sint32 i = 0, iend = mTargets.Count(); i < iend; ++i)
    {
        const float x = mInGameW * (mTargets[i].mPos.x + 0.5f);
        const float y = mInGameH * (mTargets[i].mPos.y + 0.5f);
        ZAY_XYRR(panel, x, y, 0, 0)
        ZAY_FONT(panel, 1.5, "Arial Black")
        {
            const String Text = String::Format("%d", mTargets[i].mHP);
            ZAY_RGBA(panel, 0, 0, 0, -128)
                panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, Text, UIFA_CenterMiddle);
            ZAY_RGB(panel, 64, 255, 0)
                panel.text(panel.w() / 2, panel.h() / 2, Text, UIFA_CenterMiddle);
        }
    }

    // 배경의 아웃라인(후)
    ZAY_LTRB_SCISSOR(panel, OutlineRect.l, OutlineRect.t + OutlineHeightHalf, OutlineRect.r, OutlineRect.b)
    ZAY_LTRB(panel, 0, -OutlineHeightHalf, OutlineRect.Width(), OutlineRect.Height() - OutlineHeightHalf)
        panel.stretch(R("black_aera"), true);

    // 길찾기 정보
    if(editmode)
    {
        if(mMap)
        ZAY_RGBA(panel, 0, 0, 255, 128)
        for(sint32 i = 0, iend = mMap->Lines.Count(); i < iend; ++i)
        {
            const Point DotA = mMap->Dots[mMap->Lines[i].DotA];
            const Point DotB = mMap->Dots[mMap->Lines[i].DotB];
            panel.line(DotA, DotB, 1);
        }
        if(monsters)
        for(sint32 i = 0, iend = monsters->Count(); i < iend; ++i)
        {
            if(wavesec < (*monsters)[i].mEntranceSec) continue;
            if(0 < (*monsters)[i].mHP || 0 < (*monsters)[i].mDeathCount)
            {
                if(auto CurPath = (*monsters)[i].mTargetPath)
                if(0 < CurPath->Dots.Count())
                {
                    Point OldPoint = CurPath->Dots[0];
                    for(sint32 j = 1, jend = CurPath->Dots.Count(); j < jend; ++j)
                    {
                        Point NewPoint = CurPath->Dots[j];
                        ZAY_RGBA(panel, 255, 0, 255, 128)
                        {
                            if(j & 1) panel.line(OldPoint, NewPoint, 2);
                            if(jend - 1 - j == CurPath->DotFocus)
                            ZAY_RGB(panel, 255, 0, 0)
                            ZAY_XYRR(panel, NewPoint.x, NewPoint.y, 4, 4)
                                panel.circle();
                        }
                        OldPoint = NewPoint;
                    }
                    ZAY_RGB(panel, 255, 0, 0)
                    {
                        const Point LastPoint = CurPath->Dots[0];
                        panel.text(LastPoint.x, LastPoint.y,
                            String::Format("[%d]", (*monsters)[i].mTargetPathScore), UIFA_CenterMiddle);
                    }
                }
            }
        }
    }

    // 게임영역 표시
    if(editmode)
    ZAY_RGB(panel, 255, 255, 128)
        panel.rect(1);
}

////////////////////////////////////////////////////////////////////////////////
F1Tool::F1Tool() : mUITween(updater())
{
    mCursorInWindow = false;
    mLockedUI = false;
    mGridMode = false;
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
    const float fxmax = 1000;
    const float fymax = 1000 * mState.mInGameH / mState.mInGameW;
    sint32 nx = 0, ny = 0;
    if(0 <= fx) nx = (((sint32) (fx * fxmax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid;
    else nx = -((((sint32) (-fx * fxmax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid);
    if(0 <= fy) ny = (((sint32) (fy * fymax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid;
    else ny = -((((sint32) (-fy * fymax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid);
    return Point(nx / fxmax, ny / fymax);
}

void F1Tool::RenderGrid(ZayPanel& panel)
{
    // 중간 -> 좌측
    for(sint32 x = 500 - mState.mToolGrid, ix = 0; 0 < (ix = x * panel.w() / 1000); x -= mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, ix - 1, 0, 1, panel.h())
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_XYWH(panel, ix, 0, 1, panel.h())
            panel.fill();
    }
    // 중간 -> 우측
    for(sint32 x = 500, ix = 0; (ix = x * panel.w() / 1000) < panel.w(); x += mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, ix - 1, 0, 1, panel.h())
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_XYWH(panel, ix, 0, 1, panel.h())
            panel.fill();
    }
    // 중간 -> 상측
    for(sint32 y = 500 - mState.mToolGrid, iy = 0; 0 < (iy = y * panel.w() / 1000); y -= mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, 0, iy - 1, panel.w(), 1)
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
        ZAY_XYWH(panel, 0, iy, panel.w(), 1)
            panel.fill();
    }
    // 중간 -> 하측
    for(sint32 y = 500, iy = 0; (iy = y * panel.w() / 1000) < panel.h(); y += mState.mToolGrid)
    {
        ZAY_RGBA(panel, 255, 255, 255, 64)
        ZAY_XYWH(panel, 0, iy - 1, panel.w(), 1)
            panel.fill();
        ZAY_RGBA(panel, 0, 0, 0, 64)
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
