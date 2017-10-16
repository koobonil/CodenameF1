#include <boss.hpp>
#include "classes.hpp"

#include <r.hpp>

////////////////////////////////////////////////////////////////////////////////
MapMonster::MapMonster()
{
    mUIName = "";
    mEntranceSec = 0;
    mType = nullptr;
    mHP = 0;
    mAttackMode = false;
    mFlipMode = false;
    mLastFlip = false;
    mLastFlipPos = Point(0, 0);
    mDeathCount = 0;
    mPath = nullptr;
    mSpineRenderer = nullptr;
    mSpineInstance = nullptr;
    mSpineMsecOld = 0;
}

MapMonster::~MapMonster()
{
    TryWorld::Path::Release(mPath);
    ZAY::SpineBuilder::Release(mSpineInstance);
}

MapMonster::MapMonster(MapMonster&& rhs)
{
    operator=(ToReference(rhs));
}

MapMonster& MapMonster::operator=(MapMonster&& rhs)
{
    mUIName = ToReference(rhs.mUIName);
    mEntranceSec = rhs.mEntranceSec;
    mType = rhs.mType;
    mHP = rhs.mHP;
    mAttackMode = rhs.mAttackMode;
    mFlipMode = rhs.mFlipMode;
    mLastFlip = rhs.mLastFlip;
    mLastFlipPos = rhs.mLastFlipPos;
    mDeathCount = rhs.mDeathCount;
    mPos = rhs.mPos;
    mPath = rhs.mPath;
    rhs.mPath = nullptr;
    mSpineRenderer = rhs.mSpineRenderer;
    mSpineInstance = rhs.mSpineInstance;
    rhs.mSpineInstance = nullptr;
    mSpineMsecOld = rhs.mSpineMsecOld;
    return *this;
}

void MapMonster::Init(chars uiname, sint32 timesec, const MonsterType* type, float x, float y, const SpineRenderer* renderer)
{
    mUIName = uiname;
    mEntranceSec = timesec;
    mType = type;
    mHP = type->mHP;
    mAttackMode = false;
    mFlipMode = (x < 0);
    mLastFlip = mFlipMode;
    mDeathCount = 0;
    mPos.x = x;
    mPos.y = y;
    if(mSpineRenderer = renderer)
    {
        mSpineInstance = ZAY::SpineBuilder::Create(mSpineRenderer->spine(), "default",
            [this](chars motionname)
            {
            },
            [this](chars eventname)
            {
            });
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "run", true);
    }
}

void MapMonster::Kill()
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "dead", false);
    }
}

void MapMonster::Turn()
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "flip", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "flip", (mAttackMode)? "attack" : "run", true);
    }
}

void MapMonster::Hit()
{
    if(mSpineInstance)
    {
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "hit", false);
        ZAY::SpineBuilder::SetMotionOnAttached(mSpineInstance, "hit", (mAttackMode)? "attack" : "run", true);
    }
}

void MapMonster::Attacking()
{
    if(mSpineInstance && !mAttackMode)
    {
        mAttackMode = true;
        ZAY::SpineBuilder::SetMotionOffAll(mSpineInstance, true);
        ZAY::SpineBuilder::SetMotionOn(mSpineInstance, "attack", true);
    }
}

float MapMonster::CalcDeltaSec() const
{
    const uint64 SpineMsecOld = mSpineMsecOld;
    const uint64 SpineMsecNew = Platform::Utility::CurrentTimeMsec();
    mSpineMsecOld = SpineMsecNew;
    if(SpineMsecOld == 0) return 0;
    return (SpineMsecNew - SpineMsecOld) * 0.001f;
}

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

Rect SpineRenderer::Update(const MapMonster& monster) const
{
    if(!monster.mSpineInstance) return Rect(0, 0, 0, 0);
    ZAY::SpineBuilder::Update(monster.mSpineInstance, monster.CalcDeltaSec());
    return ZAY::SpineBuilder::GetBoundRect(monster.mSpineInstance, "area");
}

void SpineRenderer::Render(const MapMonster& monster, ZayPanel& panel, sint32 sx, sint32 sy, sint32 sw, sint32 sh, sint32 h, float scale, bool flip) const
{
    if(!monster.mSpineInstance) return;
    const Point XY = panel.toview(0, 0);
    const sint32 X = (sint32) (XY.x * panel.zoom());
    const sint32 Y = (sint32) ((h - (XY.y + panel.h())) * panel.zoom());
    const sint32 W = (sint32) (panel.w() * panel.zoom());
    const sint32 H = (sint32) (panel.h() * panel.zoom());
    const float cx = ((X + W * 0.5f) - sx) / sw;
    const float cy = ((Y + H * 0.5f) - sy) / sh;
    ZAY::SpineBuilder::Render(panel, monster.mSpineInstance, flip, cx, cy, scale, sx, sy, sw, sh);
}

void SpineRenderer::RenderBound(const MapMonster& monster, ZayPanel& panel, bool guideline, float ox, float oy, float scale, bool flip) const
{
    if(!monster.mSpineInstance) return;
    ZAY::SpineBuilder::RenderBound(panel, monster.mSpineInstance, ox, oy, scale, flip, guideline);
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

    mUILeft.Link("Meteo", "UILeft", false);
    mUITop.Link("Meteo", "UITop", false);
    mUIRight.Link("Meteo", "UIRight", false);
    mUIBottom.Link("Meteo", "UIBottom", false);
    mUILeft.Parse(GlobalWeightMap("UILeft"));
    mUITop.Parse(GlobalWeightMap("UITop"));
    mUIRight.Parse(GlobalWeightMap("UIRight"));
    mUIBottom.Parse(GlobalWeightMap("UIBottom"));
    mUILeft.Execute();
    mUITop.Execute();
    mUIRight.Execute();
    mUIBottom.Execute();

    mViewRate = 5000.0f / 6000.0f;
    mMeteoMinScale = Parser::GetInt(GlobalWeightMap("MeteoMinScale")) / 1000.0f;
    mMeteoMaxScale = Parser::GetInt(GlobalWeightMap("MeteoMaxScale")) / 1000.0f;
    mMeteoMinDamage = Parser::GetInt(GlobalWeightMap("MeteoMinDamage"));
    mMeteoMaxDamage = Parser::GetInt(GlobalWeightMap("MeteoMaxDamage"));
    mEggHPbarDeleteTime = Parser::GetInt(GlobalWeightMap("EggHPbarDeleteTime")); // HP가 다시 투명이되는데 걸리는 시간
    mEggHPRegenValue = Parser::GetInt(GlobalWeightMap("EggHPRegenValue")); // HP의 초당 재생량
    mEggHPValue = Parser::GetInt(GlobalWeightMap("EggHP"));
    mEggHP = mEggHPValue;
    m1StarHpRate = Parser::GetInt(GlobalWeightMap("1StarHpRate"));
    m2StarHpRate = Parser::GetInt(GlobalWeightMap("2StarHpRate"));
    m3StarHpRate = Parser::GetInt(GlobalWeightMap("3StarHpRate"));
    if((mToolGrid = Parser::GetInt(GlobalWeightMap("ToolGrid"))) == 0) mToolGrid = 50;

    auto ObjectTable = Context(ST_Json, SO_NeedCopy, String::FromFile("table/object_table.json"));
    if(ObjectTable.IsValid())
    {
        for(sint32 i = 0, iend = ObjectTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewObjectType = mObjectTypes.AtAdding();
            NewObjectType.mID = ObjectTable[i]("ID").GetString("");
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
            NewMonsterType.mScale = MonsterTable[i]("Scale").GetInt(1000);
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
    mTreeSizeR = 0;
    mMonsterSizeR = 0;
    mMeteoSizeMinR = 0;
    mMeteoSizeMaxR = 0;

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
            NewObject.mRect.l = CurJsonObject("RectL").GetFloat();
            NewObject.mRect.t = CurJsonObject("RectT").GetFloat();
            NewObject.mRect.r = CurJsonObject("RectR").GetFloat();
            NewObject.mRect.b = CurJsonObject("RectB").GetFloat();
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
            if(CurPolygons[j].mType->mType != PolygonType::ZoneType::Wall)
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
    ScreenWidth.Link("Meteo", "ScreenWidth", false);
    ScreenHeight.Link("Meteo", "ScreenHeight", false);
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
    mTreeSizeR = mInGameW / 10;
    mMonsterSizeR = mInGameW / 40;
    mMeteoSizeMinR = mInGameW / 2 * mMeteoMinScale;
    mMeteoSizeMaxR = mInGameW / 2 * mMeteoMaxScale;
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

void F1State::RenderObject(bool editmode, ZayPanel& panel, const SpineRenderer* renderer, const MapMonster& monster, sint32 sx, sint32 sy, sint32 sw, sint32 sh, bool flip)
{
    const Rect AreaRect = renderer->Update(monster);
    const float Width = Math::MaxF(0.001, AreaRect.Width());
    const float Height = Math::MaxF(0.001, AreaRect.Height());
    const float Rate = Math::MinF(panel.w() / Width, panel.h() / Height);
    const float CX = (flip)? -AreaRect.CenterX() : AreaRect.CenterX();
    const float CY = AreaRect.CenterY();

    ZAY_XYRR(panel, panel.w() / 2 - CX * Rate, panel.h() / 2 - CY * Rate, 0, 0)
    {
        Platform::Graphics::BeginGL();
        renderer->Render(monster, panel, sx, mScreenH - (sy + sh), sw, sh, mScreenH, Rate, flip);
        Platform::Graphics::EndGL();
    }

    if(editmode)
    {
        ZAY_RGBA(panel, 255, 255, 255, 128)
            panel.fill();
        ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, 0, 0)
            renderer->RenderBound(monster, panel, true, CX, CY, Rate, flip);
        ZAY_RGBA(panel, 255, 0, 0, 128)
            panel.rect(1);
    }
}

void F1State::Render(bool editmode, ZayPanel& panel, const MapMonsters* monsters, sint32 wavesec)
{
    const Point XY = panel.toview(0, 0);
    const sint32 ScreenX = (sint32) (XY.x * panel.zoom());
    const sint32 ScreenY = (sint32) (XY.y * panel.zoom());
    const sint32 ScreenW = (sint32) (panel.w() * panel.zoom());
    const sint32 ScreenH = (sint32) (panel.h() * panel.zoom());

    panel.stretch(R(mBGName), true);

    // 레이어
    ZAY_FONT(panel, 1.1, "Arial Black")
    for(sint32 i = 0; i < mLayerLength; ++i)
    {
        if(!mLayers[i].mShowFlag)
            continue;
        // 오브젝트
        auto& CurObjects = mLayers[i].mObjects;
        for(sint32 j = 0, jend = CurObjects.Count(); j < jend; ++j)
        {
            const float l = mInGameW * (CurObjects[j].mRect.l + 0.5f);
            const float t = mInGameH * (CurObjects[j].mRect.t + 0.5f);
            const float r = mInGameW * (CurObjects[j].mRect.r + 0.5f);
            const float b = mInGameH * (CurObjects[j].mRect.b + 0.5f);
            ZAY_LTRB(panel, l, t, r, b)
            {
                RenderImage(editmode, panel, R(CurObjects[j].mType->mAsset));
                // 타입ID
                if(editmode)
                ZAY_RGB(panel, 255, 255, 0)
                    panel.text(CurObjects[j].mType->mID, UIFA_CenterMiddle, UIFE_Right);
            }
        }
        // 폴리곤
        auto& CurPolygons = mLayers[i].mPolygons;
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
            const sint32 ColorR = CurPolygons[j].mType->mColorR;
            const sint32 ColorG = CurPolygons[j].mType->mColorG;
            const sint32 ColorB = CurPolygons[j].mType->mColorB;
            ZAY_RGBA(panel, ColorR, ColorG, ColorB, 128)
                panel.polygon(NewPoints);
        }
    }

    // 알
    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, mTreeSizeR, mTreeSizeR)
    {
        RenderImage(editmode, panel, R("egg"));
        // HP스코어
        ZAY_FONT(panel, 1.5, "Arial Black")
        {
            const String Text = String::Format("%d", mEggHP);
            ZAY_RGBA(panel, 0, 0, 0, -128)
                panel.text(panel.w() / 2 + 1, panel.h() / 2 + 1, Text, UIFA_CenterMiddle);
            ZAY_RGB(panel, 64, 255, 0)
                panel.text(panel.w() / 2, panel.h() / 2, Text, UIFA_CenterMiddle);
        }
    }

    // 몬스터
    if(monsters)
    for(sint32 i = 0, iend = monsters->Count(); i < iend; ++i)
    {
        if(wavesec < (*monsters)[i].mEntranceSec)
            continue;
        const float x = mInGameW * ((*monsters)[i].mPos.x + 0.5f);
        const float y = mInGameH * ((*monsters)[i].mPos.y + 0.5f);
        if(0 < (*monsters)[i].mHP || 0 < (*monsters)[i].mDeathCount)
        {
            ZAY_XYRR(panel, x, y, mMonsterSizeR, mMonsterSizeR)
            {
                if(auto CurRenderer = (*monsters)[i].renderer())
                    RenderObject(editmode, panel, CurRenderer, (*monsters)[i], ScreenX, ScreenY, ScreenW, ScreenH, (*monsters)[i].mFlipMode);
                else RenderImage(editmode, panel, R((*monsters)[i].mType->mAsset));
                // 몬스터HP
                const String HP = String::Format("HP-%d", (*monsters)[i].mHP);
                ZAY_RGB(panel, 0, 0, 0)
                    panel.text(panel.w() / 2 + 1, 1, HP, UIFA_CenterBottom);
                ZAY_RGB(panel, 255, 255, 255)
                    panel.text(panel.w() / 2, 0, HP, UIFA_CenterBottom);
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
    {
        const float fxmax = 1000;
        const float fymax = 1000 * mState.mInGameH / mState.mInGameW;
        sint32 nx = 0, ny = 0;
        if(0 <= fx) nx = (((sint32) (fx * fxmax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid;
        else nx = -((((sint32) (-fx * fxmax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid);
        if(0 <= fy) ny = (((sint32) (fy * fymax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid;
        else ny = -((((sint32) (-fy * fymax)) + mState.mToolGrid / 2) / mState.mToolGrid * mState.mToolGrid);
        fx = nx / fxmax;
        fy = ny / fymax;
    }
    return Point(fx, fy);
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
