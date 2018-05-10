#include <boss.hpp>
#include "pathtool_f1.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("pathtoolView", pathtoolData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Create)
    {
        // 윈도우 타이틀
        Platform::SetWindowName("Codename F1 [PathTool]");
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->mState.SetSize(Width, Height);
        m->mState.BuildMap();
    }
    m->Command(type, in);
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    const Point Pos = m->GestureToPos(x, y);

    if(type == GT_Pressed)
    {
        // 작업중인 폴리곤에 점추가
        auto& NewPoint = m->mCurPoints.AtAdding();
        NewPoint.x = Pos.x;
        NewPoint.y = Pos.y;
    }
    else if(type == GT_ExtendPress)
    {
        // 작업중인 폴리곤
        m->mCurPoints.SubtractionOne();
		m->invalidate();
    }
    else if(type == GT_InDragging || type == GT_OutDragging)
    {
        // 마지막 점을 이동
        if(0 < m->mCurPoints.Count())
        {
            auto& LastPoint = m->mCurPoints.At(-1);
            LastPoint.x = Pos.x;
            LastPoint.y = Pos.y;
            m->invalidate();
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

pathtoolData::pathtoolData()
{
}

pathtoolData::~pathtoolData()
{
}

void pathtoolData::Render(ZayPanel& panel)
{
    ZAY_RGB(panel, 128, 255, 255)
        panel.fill();
    auto PolygonType = &mState.mPolygonTypes[0];

    // 인게임
    ZAY_XYWH(panel, mMapPos.x + mState.mInGameX, mMapPos.y + mState.mInGameY, mState.mInGameW, mState.mInGameH)
    {
        // 길찾기 매핑결과
        if(auto* CurTryWorldZone = mState.mAllTryWorldZones.Access(PolygonType->mID))
        if(CurTryWorldZone->mMap)
        for(sint32 i = 0, iend = CurTryWorldZone->mMap->Lines.Count(); i < iend; ++i)
        {
            auto& DotA = CurTryWorldZone->mMap->Dots[CurTryWorldZone->mMap->Lines[i].DotA];
            auto& DotB = CurTryWorldZone->mMap->Dots[CurTryWorldZone->mMap->Lines[i].DotB];
            if(CurTryWorldZone->mMap->Lines[i].Type == TryWorld::linetype_wall)
            {
                ZAY_RGB(panel, 0, 0, 255)
                {
                    panel.line(Point(DotA.x, DotA.y), Point(DotB.x, DotB.y), 6);
                    ZAY_XYRR(panel, DotA.x, DotA.y, 6, 6)
                        panel.circle();
                }
            }
            else
            {
                ZAY_RGBA(panel, 128, 128, 128, 64)
                    panel.line(Point(DotA.x, DotA.y), Point(DotB.x, DotB.y), 1);
            }
        }

        // 등록된 폴리곤
        auto& CurPolygons = mState.mLayers.At(0).mPolygons;
        for(sint32 i = 0, iend = CurPolygons.Count(); i < iend; ++i)
        {
            auto& CurPoints = CurPolygons[i].mDots;
            Points NewPoints;
            sint32s NewPayloads;
            NewPoints.AtDumpingAdded(CurPoints.Count() + (!CurPolygons[i].mIsCCW));
            NewPayloads.AtDumpingAdded(CurPoints.Count());
            for(sint32 j = 0, jend = CurPoints.Count(); j < jend; ++j)
            {
                const float x = mState.mInGameW * (CurPoints[j].x + 0.5f);
                const float y = mState.mInGameH * (CurPoints[j].y + 0.5f);
                NewPoints.At(j) = Point(x, y);
                NewPayloads.At(j) = CurPoints[j].Payload;
            }
            if(!CurPolygons[i].mIsCCW)
                NewPoints.At(-1) = NewPoints.At(0);

            if(CurPolygons[i].mVisible)
            {
                const sint32 ColorR = CurPolygons[i].mType->mColorR;
                const sint32 ColorG = CurPolygons[i].mType->mColorG;
                const sint32 ColorB = CurPolygons[i].mType->mColorB;
                ZAY_RGBA(panel, ColorR, ColorG, ColorB, 128)
                {
                    if(CurPolygons[i].mIsCCW)
                        panel.polygon(NewPoints);
                    else ZAY_RGBA_IF(panel, 64, 192, 64, 128, CurPolygons[i].mDots.Count() < 3)
                        panel.polyline(NewPoints, 4);
                    // 오브젝트가 링크된 선분
                    ZAY_RGBA(panel, 64, 64, 192, 128)
                    for(sint32 j = 0, jend = CurPoints.Count(); j < jend; ++j)
                    {
                        const sint32 jNext = (j + 1) % jend;
                        if(NewPayloads[j] != -1 && NewPayloads[j] == NewPayloads[jNext])
                            panel.line(NewPoints[j], NewPoints[jNext], 2);
                    }
                }
            }
            else
            {
                ZAY_RGBA(panel, 64, 64, 64, 64)
                {
                    if(CurPolygons[i].mIsCCW)
                        panel.polygon(NewPoints);
                    else panel.polyline(NewPoints, 2);
                }
            }
        }

        // 게임영역 표시
        ZAY_RGB(panel, 255, 255, 128)
            panel.rect(1);

        // 에러표시
        if(0 < CurPolygons.Count())
        {
            bool IsSuccess = false;
            if(auto* CurTryWorldZone = mState.mAllTryWorldZones.Access(PolygonType->mID))
            if(CurTryWorldZone->mMap) IsSuccess = true;
            if(!IsSuccess)
            {
                ZAY_RGB(panel, 255, 0, 0)
                    panel.text(panel.w() / 2, -10, "(BUILD ERROR)", UIFA_CenterBottom);
            }
        }

        // 작업중인 폴리곤
        const sint32 ColorR = PolygonType->mColorR;
        const sint32 ColorG = PolygonType->mColorG;
        const sint32 ColorB = PolygonType->mColorB;
        float OldX = 0, OldY = 0;
        ZAY_RGBA(panel, ColorR, ColorG, ColorB, 128)
        for(sint32 i = 0, iend = mCurPoints.Count(); i < iend; ++i)
        {
            const float NewX = mState.mInGameW * (mCurPoints[i].x + 0.5f);
            const float NewY = mState.mInGameH * (mCurPoints[i].y + 0.5f);
            if(0 < i)
                panel.line(Point(OldX, OldY), Point(NewX, NewY), 2);
            OldX = NewX;
            OldY = NewY;
        }
        ZAY_RGBA(panel, ColorR, ColorG, ColorB, 48)
        for(sint32 i = 0, iend = mCurPoints.Count(); i < iend; ++i)
        {
            const float x = mState.mInGameW * (mCurPoints[i].x + 0.5f);
            const float y = mState.mInGameH * (mCurPoints[i].y + 0.5f);
            if(i == 0 && 1 < iend)
            {
                ZAY_XYRR_UI(panel, x, y, 10, 10, "drawing_polygon",
                    ZAY_GESTURE_T(t, this, PolygonType)
                    {
                        if(t == GT_Pressed)
                        {
                            // 폴리곤 완성
                            auto& CurPolygons = mState.mLayers.At(0).mPolygons;
                            auto& NewPolygon = CurPolygons.AtAdding();
                            NewPolygon.mType = PolygonType;
                            NewPolygon.mRID = ++mState.mPolygonLastRID;
                            NewPolygon.mDots = mCurPoints;
                            NewPolygon.UpdateCCW();
                            // 길찾기 빌드
                            mState.BuildMap();
                            mCurPoints.SubtractionAll();
                            invalidate();
                        }
                    })
                ZAY_RGBA_IF(panel, 192, 192, 192, 128, panel.state("drawing_polygon") & PS_Focused)
                    panel.circle();
            }
            else ZAY_XYRR(panel, x, y, 10, 10)
                panel.circle();
        }

        // 그리드
        if(mGridMode)
            RenderGrid(panel);
    }

    // 툴UI
    const float OuterSize = ButtonSize * 1.5 * mUITween.value() / 100;
    ZAY_LTRB(panel, -OuterSize, -OuterSize, panel.w() + OuterSize, panel.h() + OuterSize)
    ZAY_FONT(panel, 1.0, "Arial Black")
    {
        // 홈버튼
        ZAY_XYWH(panel, 0, 0, ButtonSize, ButtonSize)
            RenderHomeButton(panel);

        // 고정버튼
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
            RenderLockToggle(panel);

        // 그리드모드
        ZAY_XYWH(panel, ButtonSize * 2, 0, ButtonSize, ButtonSize)
            RenderGridToggle(panel);

        // 이동버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, 0, ButtonSize, ButtonSize)
            RenderDragButton(panel);
    }
}

void pathtoolData::OnModeChanged()
{
}

void pathtoolData::OnSelectSub(chars name)
{
}

void pathtoolData::InitSelectBox(sint32 index)
{
}

void pathtoolData::QuitSelectBox(sint32 index)
{
}

void pathtoolData::ChangeSelectBox(sint32 type, sint32 index)
{
}

void pathtoolData::OnSelectBoxMoving(sint32 index, float addx, float addy)
{
}

void pathtoolData::OnSelectBoxMoved(sint32 index)
{
}

void pathtoolData::OnSelectBoxSizing(sint32 index, float addx, float addy)
{
}

void pathtoolData::OnSelectBoxSized(sint32 index)
{
}

void pathtoolData::OnSelectBoxClone(sint32 index)
{
}
