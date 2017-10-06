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

F1State::F1State()
{
    Map<String> GlobalWeightMap;
    if(auto GlobalWeightTable = Context(ST_Json, SO_NeedCopy, String::FromFile("json/GlobalWeightTable.json")))
    {
        for(sint32 i = 0, iend = GlobalWeightTable.LengthOfIndexable(); i < iend; ++i)
        {
            chars CurID = GlobalWeightTable[i]("ID").GetString("noname");
            chars CurValue = GlobalWeightTable[i]("Value").GetString("0");
            GlobalWeightMap(CurID) = CurValue;
        }
    }
    else BOSS_ASSERT("GlobalWeightTable.json의 로딩에 실패하였습니다", false);

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
    mTreeHPbarDeleteTime = Parser::GetInt(GlobalWeightMap("TreeHPbarDeleteTime")); // HP가 다시 투명이되는데 걸리는 시간
    mTreeHPRegenValue = Parser::GetInt(GlobalWeightMap("TreeHPRegenValue")); // HP의 초당 재생량
    mTreeHPValue = Parser::GetInt(GlobalWeightMap("TreeHP"));
    mTreeHP = mTreeHPValue;
    m1StarHpRate = Parser::GetInt(GlobalWeightMap("3StarHpRate"));
    m2StarHpRate = Parser::GetInt(GlobalWeightMap("3StarHpRate"));
    m3StarHpRate = Parser::GetInt(GlobalWeightMap("3StarHpRate"));

    if(auto ObjectTable = Context(ST_Json, SO_NeedCopy, String::FromFile("json/ObjectTable.json")))
    {
        for(sint32 i = 0, iend = ObjectTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewObjectType = mObjectTypes.AtAdding();
            NewObjectType.mID = ObjectTable[i]("ID").GetString("");
            NewObjectType.mAsset = ObjectTable[i]("Asset").GetString("noname");
        }
    }
    else BOSS_ASSERT("ObjectTable.json의 로딩에 실패하였습니다", false);

    if(auto PolygonTable = Context(ST_Json, SO_NeedCopy, String::FromFile("json/PolygonTable.json")))
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
    else BOSS_ASSERT("PolygonTable.json의 로딩에 실패하였습니다", false);

    if(auto MonsterTable = Context(ST_Json, SO_NeedCopy, String::FromFile("json/MonsterTable.json")))
    {
        for(sint32 i = 0, iend = MonsterTable.LengthOfIndexable(); i < iend; ++i)
        {
            auto& NewMonsterType = mMonsterTypes.AtAdding();
            NewMonsterType.mID = MonsterTable[i]("ID").GetString("");
            NewMonsterType.mHP = MonsterTable[i]("HP").GetInt(100);
            NewMonsterType.mMoveType = MonsterTable[i]("MoveType").GetString("Null");
            NewMonsterType.mMoveSpeed = MonsterTable[i]("MoveSpeed").GetInt(1000);
            NewMonsterType.mMoveSight = Math::Max(1, MonsterTable[i]("MoveSight").GetInt(5));
            NewMonsterType.mAttackPower = MonsterTable[i]("AttackPower").GetInt(100);
            NewMonsterType.mAttackSpeed = MonsterTable[i]("AttackSpeed").GetInt(1000);
            NewMonsterType.mAttackRange = MonsterTable[i]("AttackRange").GetInt(100);
            NewMonsterType.mAsset = MonsterTable[i]("Asset").GetString("noname");
            NewMonsterType.mScale = MonsterTable[i]("Scale").GetInt(1000);
        }
    }
    else BOSS_ASSERT("MonsterTable.json의 로딩에 실패하였습니다", false);

    mSpine.Create("spine/orc/orc.json", "spine/orc/path.json");

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

void F1State::RenderObject(bool editmode, ZayPanel& panel, const MapMonster& monster, sint32 sx, sint32 sy, sint32 sw, sint32 sh)
{
    const sint32 Width = 125;
    const sint32 Height = 46;
    const float RateR = Math::MinF(panel.w() / Width, panel.h() / Height) / 2;
    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, Width * RateR, Height * RateR)
    {
        mSpine.Render(panel, monster, editmode, sx, mScreenH - (sy + sh), sw, sh, mScreenH);
        if(editmode)
        ZAY_RGBA(panel, 255, 0, 0, 64)
            panel.fill();
    }
    if(editmode)
    ZAY_RGBA(panel, 255, 0, 0, 128)
        panel.rect(1);
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

    // 중앙나무
    ZAY_XYRR(panel, panel.w() / 2, panel.h() / 2, mTreeSizeR, mTreeSizeR)
    {
        RenderImage(editmode, panel, R("egg"));
        // HP스코어
        if(!editmode)
        ZAY_FONT(panel, 1.5, "Arial Black")
        {
            const String Text = String::Format("%d", mTreeHP);
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
        if(0 < (*monsters)[i].mHP)
        {
            // 이동중인 몬스터
            ZAY_XYRR(panel, x, y, mMonsterSizeR, mMonsterSizeR)
            {
                //RenderImage(editmode, panel, R((*monsters)[i].mType->mAsset));
                RenderObject(editmode, panel, (*monsters)[i], ScreenX, ScreenY, ScreenW, ScreenH);
                // 몬스터HP
                if(!editmode)
                {
                    const String HP = String::Format("HP-%d", (*monsters)[i].mHP);
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.text(panel.w() / 2 + 1, 1, HP, UIFA_CenterBottom);
                    ZAY_RGB(panel, 255, 255, 255)
                        panel.text(panel.w() / 2, 0, HP, UIFA_CenterBottom);
                }
            }
        }
        else if(0 < (*monsters)[i].mDeathCount)
        {
            // 죽는중인 몬스터
            const bool is_death_mark = ((*monsters)[i].mDeathCount & 1);
            ZAY_RGB_IF(panel, 192, 192, 192, is_death_mark)
            ZAY_XYRR(panel, x, y, mMonsterSizeR, mMonsterSizeR)
                RenderImage(editmode, panel, R((*monsters)[i].mType->mAsset));
        }
    }

    // 게임영역 표시
    if(editmode)
    ZAY_RGB(panel, 255, 255, 128)
        panel.rect(1);
}

ingameData::ingameData()
{
    mWaveData = Context(ST_Json, SO_NeedCopy, String::FromFile("json/Stage_0.json"));
    mWave = -1;
    mWaveTitle = "";
    mWaveSec = 0;
    mTouchPos = Point(0, 0);
    mTouchSizeR = 0;
    mTouchDamage = 0;

    const String MapName = mWaveData("MapName").GetString();
    id_asset_read TextAsset = Asset::OpenForRead("json/" + MapName + ".json");
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
            mTreeHP = Math::Max(0, mTreeHP - mMonsters[i].mType->mAttackPower * timespan * mMonsters[i].mType->mAttackSpeed / 1000);
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
            mMonsters.At(i).mPos = Point(NextPosX / mInGameW - 0.5f, NextPosY / mInGameH - 0.5f);
        }
    }

    // 트리회복
    if(0 < mTreeHP)
        mTreeHP = Math::Min(mTreeHP + mTreeHPRegenValue * timespan, mTreeHPValue);

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
        F1State::Render(false, panel, &mMonsters, mWaveSec);

    // 홈버튼
    const sint32 InnerGap = 10, ButtonSize = 80;
    ZAY_FONT(panel, 1.2, "Arial Black")
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
                        AllMonsters[j].Init(TimeSec, &mMonsterTypes[k],
                            JsonMonsters[j]("PosX").GetFloat(0),
                            JsonMonsters[j]("PosY").GetFloat(0), mSpine.spine());
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
                mMonsters.At(i).mDeathCount = 10;
        }
    }

    // 트리피해
    const float ox = mInGameX + mInGameW / 2;
    const float oy = mInGameY + mInGameH / 2;
    if(Math::Distance(ox, oy, mTouchPos.x, mTouchPos.y) < mTreeSizeR + mTouchSizeR)
    {
        mTreeHP = Math::Max(0, mTreeHP - mTouchDamage);
    }
}
