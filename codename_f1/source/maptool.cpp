﻿#include <boss.hpp>
#include "maptool.hpp"

#include <r.hpp>

ZAY_DECLARE_VIEW_CLASS("maptoolView", maptoolData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Create)
    {
        // 윈도우 타이틀
        Platform::SetWindowName("Codename F1 [MapTool]");
    }
    else m->Command(type, in);
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
        auto& NewPoint = m->mCurDrawingPoints.AtAdding();
        NewPoint.x = Pos.x;
        NewPoint.y = Pos.y;
    }
    else if(type == GT_InDragging || type == GT_OutDragging)
    {
        // 마지막 점을 이동
        if(0 < m->mCurDrawingPoints.Count())
        {
            auto& LastPoint = m->mCurDrawingPoints.At(-1);
            LastPoint.x = Pos.x;
            LastPoint.y = Pos.y;
            m->invalidate();
        }
    }
    else if(type == GT_InReleased || type == GT_OutReleased)
    {
        if(m->mCurDrawingPoints.Count() == 2)
        {
            if(m->mSelectMode)
            {
                // 선택박스 완성
                auto& NewBox = m->mSelectBoxes.AtAdding();
                NewBox.mX = Math::MinF(m->mCurDrawingPoints[0].x, m->mCurDrawingPoints[1].x);
                NewBox.mY = Math::MinF(m->mCurDrawingPoints[0].y, m->mCurDrawingPoints[1].y);
                NewBox.mWidth = Math::MaxF(m->mCurDrawingPoints[0].x, m->mCurDrawingPoints[1].x) - NewBox.mX;
                NewBox.mHeight = Math::MaxF(m->mCurDrawingPoints[0].y, m->mCurDrawingPoints[1].y) - NewBox.mY;
                m->InitSelectBox(m->mSelectBoxes.Count() - 1);
                m->mCurDrawingPoints.SubtractionAll();

                // 빈 선택박스 일괄삭제
                for(sint32 i = m->mSelectBoxes.Count() - 1; 0 <= i; --i)
                {
                    bool HasChild = false;
                    for(sint32 j = 0; j < m->mSelectBoxes[i].mLayers.Count(); ++j)
                    {
                        HasChild |= (0 < m->mSelectBoxes[i].mLayers[j].mObjects.Count());
                        HasChild |= (0 < m->mSelectBoxes[i].mLayers[j].mPolygons.Count());
                    }
                    if(!HasChild)
                    {
                        m->mSelectBoxes.SubtractionSection(i);
                        if(i == m->mCurSelectBox)
                            m->mCurSelectBox = -1;
                    }
                }
            }
            else if(m->mCurObject != -1)
            {
                // 오브젝트 완성
                auto& CurObjects = m->mState.mLayers.At(m->mCurLayer).mObjects;
                auto& NewObject = CurObjects.AtAdding();
                NewObject.mType = &m->mState.mObjectTypes[m->mCurObject];
                NewObject.mRect.l = Math::MinF(m->mCurDrawingPoints[0].x, m->mCurDrawingPoints[1].x);
                NewObject.mRect.t = Math::MinF(m->mCurDrawingPoints[0].y, m->mCurDrawingPoints[1].y);
                NewObject.mRect.r = Math::MaxF(m->mCurDrawingPoints[0].x, m->mCurDrawingPoints[1].x);
                NewObject.mRect.b = Math::MaxF(m->mCurDrawingPoints[0].y, m->mCurDrawingPoints[1].y);
                m->mCurDrawingPoints.SubtractionAll();
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

MapSelectBox::MapSelectBox()
{
    mVisibled = true;
    mX = 0;
    mY = 0;
    mWidth = 0;
    mHeight = 0;
    mLayerX = 0;
    mLayerY = 0;
}

MapSelectBox::~MapSelectBox()
{
}

MapSelectBox::MapSelectBox(MapSelectBox&& rhs)
{
    operator=(ToReference(rhs));
}

MapSelectBox& MapSelectBox::operator=(MapSelectBox&& rhs)
{
    mVisibled = rhs.mVisibled;
    mX = rhs.mX;
    mY = rhs.mY;
    mWidth = rhs.mWidth;
    mHeight = rhs.mHeight;
    mLayerX = rhs.mLayerX;
    mLayerY = rhs.mLayerY;
    mLayers = ToReference(rhs.mLayers);
    return *this;
}

void MapSelectBox::CopyFrom(const MapSelectBox& rhs)
{
    mVisibled = rhs.mVisibled;
    mX = rhs.mX;
    mY = rhs.mY;
    mWidth = rhs.mWidth;
    mHeight = rhs.mHeight;
    mLayerX = rhs.mLayerX;
    mLayerY = rhs.mLayerY;
    mLayers = rhs.mLayers;
}

maptoolData::maptoolData()
{
    mCurObject = 0;
    mCurPolygon = -1;
    mCurLayer = 2;
    mCurSelectBox = -1;
}

maptoolData::~maptoolData()
{
}

void maptoolData::Load(chars filename)
{
    mCurObject = 0;
    mCurPolygon = -1;
    mCurLayer = 2;
    mCurSelectBox = -1;

    id_file_read TextFile = Platform::File::OpenForRead(filename);
    if(TextFile)
    {
        const sint32 TextSize = Platform::File::Size(TextFile);
        buffer TextBuffer = Buffer::Alloc(BOSS_DBG TextSize + 1);
        Platform::File::Read(TextFile, (uint08*) TextBuffer, TextSize);
        Platform::File::Close(TextFile);
        ((char*) TextBuffer)[TextSize] = '\0';
        mState.LoadMap((chars) TextBuffer);
        Buffer::Free(TextBuffer);
    }
}

void maptoolData::Save(chars filename)
{
    id_file TextFile = Platform::File::OpenForWrite(filename);
    if(TextFile)
    {
        String TextString = mState.SaveMap();
        Platform::File::Write(TextFile, (bytes)(chars) TextString, TextString.Length());
        Platform::File::Close(TextFile);
    }
}

void maptoolData::Render(ZayPanel& panel)
{
    // 인게임
    ZAY_XYWH(panel, mMapPos.x + mState.mInGameX, mMapPos.y + mState.mInGameY, mState.mInGameW, mState.mInGameH)
    {
        mState.Render(true, false, panel);

        // 작업중인 오브젝트
        if(mSelectMode || mCurObject != -1)
        {
            if(mCurDrawingPoints.Count() == 2)
            {
                Rect CurRect;
                CurRect.l = Math::MinF(mCurDrawingPoints[0].x, mCurDrawingPoints[1].x);
                CurRect.t = Math::MinF(mCurDrawingPoints[0].y, mCurDrawingPoints[1].y);
                CurRect.r = Math::MaxF(mCurDrawingPoints[0].x, mCurDrawingPoints[1].x);
                CurRect.b = Math::MaxF(mCurDrawingPoints[0].y, mCurDrawingPoints[1].y);
                const float l = mState.mInGameW * (CurRect.l + 0.5f);
                const float t = mState.mInGameH * (CurRect.t + 0.5f);
                const float r = mState.mInGameW * (CurRect.r + 0.5f);
                const float b = mState.mInGameH * (CurRect.b + 0.5f);
                ZAY_RGBA(panel, 0, 0, 0, 128)
                ZAY_LTRB(panel, l, t, r, b)
                    panel.fill();
            }
            ZAY_RGBA(panel, 0, 0, 0, 48)
            for(sint32 i = 0, iend = mCurDrawingPoints.Count(); i < iend; ++i)
            {
                const float x = mState.mInGameW * (mCurDrawingPoints[i].x + 0.5f);
                const float y = mState.mInGameH * (mCurDrawingPoints[i].y + 0.5f);
                ZAY_XYRR(panel, x, y, 10, 10)
                    panel.circle();
            }
        }
        // 작업중인 폴리곤
        else
        {
            const sint32 ColorR = mState.mPolygonTypes[mCurPolygon].mColorR;
            const sint32 ColorG = mState.mPolygonTypes[mCurPolygon].mColorG;
            const sint32 ColorB = mState.mPolygonTypes[mCurPolygon].mColorB;
            float OldX = 0, OldY = 0;
            ZAY_RGBA(panel, ColorR, ColorG, ColorB, 128)
            for(sint32 i = 0, iend = mCurDrawingPoints.Count(); i < iend; ++i)
            {
                const float NewX = mState.mInGameW * (mCurDrawingPoints[i].x + 0.5f);
                const float NewY = mState.mInGameH * (mCurDrawingPoints[i].y + 0.5f);
                if(0 < i)
                    panel.line(Point(OldX, OldY), Point(NewX, NewY), 2);
                OldX = NewX;
                OldY = NewY;
            }
            ZAY_RGBA(panel, ColorR, ColorG, ColorB, 48)
            for(sint32 i = 0, iend = mCurDrawingPoints.Count(); i < iend; ++i)
            {
                const float x = mState.mInGameW * (mCurDrawingPoints[i].x + 0.5f);
                const float y = mState.mInGameH * (mCurDrawingPoints[i].y + 0.5f);
                if(i == 0 && 2 < iend)
                {
                    ZAY_XYRR_UI(panel, x, y, 10, 10, "drawing_polygon",
                        ZAY_GESTURE_T(t, this)
                        {
                            if(t == GT_Pressed)
                            {
                                // 폴리곤 완성
                                auto& CurPolygons = mState.mLayers.At(mCurLayer).mPolygons;
                                auto& NewPolygon = CurPolygons.AtAdding();
                                NewPolygon.mType = &mState.mPolygonTypes[mCurPolygon];
                                NewPolygon.mPolygon = mCurDrawingPoints;
                                mCurDrawingPoints.SubtractionAll();
                                invalidate();
                            }
                        })
                    ZAY_RGBA_IF(panel, 192, 192, 192, 128, panel.state("drawing_polygon") & PS_Focused)
                        panel.circle();
                }
                else ZAY_XYRR(panel, x, y, 10, 10)
                    panel.circle();
            }
        }

        // 그리드
        if(mGridMode)
            RenderGrid(panel);

        // 선택박스
        for(sint32 i = 0, iend = mSelectBoxes.Count(); i < iend; ++i)
        {
            auto& CurBox = mSelectBoxes[i];
            const float AddX = mState.mInGameW * (CurBox.mX - CurBox.mLayerX);
            const float AddY = mState.mInGameH * (CurBox.mY - CurBox.mLayerY);
            ZAY_RGBA_IF(panel, 0, 128, 192, 128, i == mCurSelectBox)
            ZAY_RGBA_IF(panel, 192, 0, 128, 128, i != mCurSelectBox)
            {
                String Info;
                ZAY_XYWH(panel, AddX, AddY, panel.w(), panel.h())
                for(sint32 j = 0, jend = CurBox.mLayers.Count(); j < jend; ++j)
                {
                    const sint32 CurObjectCount = CurBox.mLayers[j].mObjects.Count();
                    const sint32 CurPolygonCount = CurBox.mLayers[j].mPolygons.Count();
                    if(0 < CurObjectCount || 0 < CurPolygonCount)
                    {
                        mState.RenderLayer(true, false, panel, CurBox.mLayers[j]);
                        if(0 < Info.Length()) Info += "/";
                        if(j < 2) Info += String::Format("B%d", 2 - j);
                        else Info += String::Format("%dF", j - 1);
                    }
                }
                if(Info.Length() == 0) Info = "(No Item)";
                const float x = mState.mInGameW * (CurBox.mX + 0.5f);
                const float y = mState.mInGameH * (CurBox.mY + 0.5f);
                const float w = mState.mInGameW * CurBox.mWidth;
                const float h = mState.mInGameH * CurBox.mHeight;
                ZAY_XYWH(panel, x, y, w, h)
                    RenderSelectBox(panel, i, Info);
            }
        }
    }

    // 툴UI
    const float OuterSize = ButtonSize * 1.5 * mUITween.value() / 100;
    ZAY_LTRB(panel, -OuterSize, -OuterSize, panel.w() + OuterSize, panel.h() + OuterSize)
    ZAY_FONT(panel, 1.0, "Arial Black")
    {
        // 고정버튼
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
            RenderLockToggle(panel);

        // 그리드모드
        ZAY_XYWH(panel, ButtonSize * 2, 0, ButtonSize, ButtonSize)
            RenderGridToggle(panel);

        // 선택모드
        ZAY_XYWH(panel, ButtonSize * 3, 0, ButtonSize, ButtonSize)
            RenderSelectToggle(panel);

        // 선택모드 부가버튼
        if(mCurSelectBox != -1)
        {
            // 선택해제
            ZAY_XYWH(panel, ButtonSize * 4 + ButtonSize * 0, 0, ButtonSize, ButtonSizeSmall)
            ZAY_RGBA(panel, 128, 192, 255, 192)
                RenderSelect_SubButton(panel, "Release");

            // Visible 상태변경
            ZAY_XYWH(panel, ButtonSize * 4 + ButtonSize * 1, 0, ButtonSize, ButtonSizeSmall)
            ZAY_RGBA(panel, 128, 255, 192, 192)
                RenderSelect_SubButton(panel, (mSelectBoxes[mCurSelectBox].mVisibled)? "Show" : "Hide");

            // 삭제
            ZAY_XYWH(panel, ButtonSize * 4 + ButtonSize * 2, 0, ButtonSizeSmall, ButtonSizeSmall)
            ZAY_RGBA(panel, 255, 64, 64, 192)
                RenderSelect_SubButton(panel, "×");
        }

        // 이동버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, 0, ButtonSize, ButtonSize)
            RenderDragButton(panel);

        // 로드버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, ButtonSize * 1, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "load",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String FileName;
                    if(Platform::Popup::FileDialog(FileName, nullptr, "Load Map(json)"))
                        Load(FileName);
                }
            })
        {
            ZAY_RGBA(panel, 64, 128, 255, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Load\nMap", UIFA_CenterMiddle);
            }
        }

        // 세이브버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, ButtonSize * 2, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "save",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String FileName;
                    if(Platform::Popup::FileDialog(FileName, nullptr, "Save Map(json)"))
                        Save(FileName);
                }
            })
        {
            ZAY_RGBA(panel, 64, 128, 255, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Save\nMap", UIFA_CenterMiddle);
            }
        }

        // 폴리곤리스트
        const sint32 PolygonCount = mState.mPolygonTypes.Count();
        ZAY_XYWH(panel, panel.w() - InnerGap - IconSize * 2, ButtonSize * 3 + InnerGap, IconSize * 2, IconSize * PolygonCount)
        {
            for(sint32 i = 0; i < PolygonCount; ++i)
            {
                ZAY_XYWH_UI(panel, 0, IconSize * i, panel.w(), IconSize, String::Format("polygon-%d", i),
                    ZAY_GESTURE_NT(n, t, this)
                    {
                        if(t == GT_InReleased)
                        {
                            mCurPolygon = Parser::GetInt(&n[8]);
                            if(mCurObject != -1)
                            {
                                mCurObject = -1;
                                mCurDrawingPoints.SubtractionAll();
                            }
                        }
                    })
                {
                    ZAY_RGBA_IF(panel, 255, 128, 64, 192, i == mCurPolygon)
                    ZAY_RGBA_IF(panel, 128, 128, 128, 192, i != mCurPolygon)
                        panel.fill();
                    ZAY_RGB(panel, 0, 0, 0)
                    {
                        panel.rect(2);
                        panel.text(mState.mPolygonTypes[i].mID, UIFA_CenterMiddle, UIFE_Right);
                    }
                }
            }
        }

        // 홈버튼
        ZAY_XYWH(panel, 0, 0, ButtonSize, ButtonSize)
            RenderHomeButton(panel);

        // BG로드버튼
        ZAY_XYWH(panel, 0, ButtonSize, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "load_bg",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String BGName = mState.mBGNameA;
                    if(Platform::Popup::TextDialog(BGName, "Load BG", "Typing the asset name"))
                        mState.mBGNameA = BGName;
                }
            })
        {
            ZAY_RGBA(panel, 64, 255, 128, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Load\nBG", UIFA_CenterMiddle);
            }
        }

        // 오브젝트리스트
        const sint32 ObjectCount = mState.mObjectTypes.Count();
        ZAY_XYWH(panel, InnerGap, ButtonSize * 2 + InnerGap, IconSize * 2, IconSize * ObjectCount)
        {
            for(sint32 i = 0; i < ObjectCount; ++i)
            {
                ZAY_XYWH_UI(panel, 0, IconSize * i, panel.w(), IconSize, String::Format("object-%d", i),
                    ZAY_GESTURE_NT(n, t, this)
                    {
                        if(t == GT_InReleased)
                        {
                            mCurObject = Parser::GetInt(&n[7]);
                            if(mCurPolygon != -1)
                            {
                                mCurPolygon = -1;
                                mCurDrawingPoints.SubtractionAll();
                            }
                        }
                    })
                {
                    ZAY_RGBA_IF(panel, 255, 128, 64, 192, i == mCurObject)
                    ZAY_RGBA_IF(panel, 128, 128, 128, 192, i != mCurObject)
                        panel.fill();
                    ZAY_RGB(panel, 0, 0, 0)
                    {
                        panel.rect(2);
                        panel.text(mState.mObjectTypes[i].mID, UIFA_CenterMiddle, UIFE_Right);
                    }
                }
            }
        }

        // 레이어
        ZAY_LTRB(panel, 0, panel.h() - ButtonSize, panel.w(), panel.h())
        ZAY_INNER_UI(panel, InnerGap, "layer_board")
        {
            ZAY_RGBA(panel, 64, 255, 128, 192)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                // 프로그레스바
                const sint32 ProgressInnerSize = (panel.h() - InnerGap) / 2;
                ZAY_INNER_UI(panel, ProgressInnerSize, "layer",
                    ZAY_GESTURE_NTXY(n, t, x, y, this)
                    {
                        if(t == GT_Pressed || t == GT_InDragging || t == GT_OutDragging)
                        {
                            const rect128& CurRect = rect(n);
                            const sint32 CalcedLayer = (x - CurRect.l) * mState.mLayerLength / (CurRect.r - CurRect.l);
                            mCurLayer = Math::Clamp(CalcedLayer, 0, mState.mLayerLength - 1);
                            invalidate();
                        }
                    })
                {
                    for(sint32 i = 0; i < mState.mLayerLength; ++i)
                    {
                        const bool IsOdd = i & 1;
                        ZAY_LTRB(panel, panel.w() * i / mState.mLayerLength, 0, panel.w() * (i + 1) / mState.mLayerLength, panel.h())
                        ZAY_RGB_IF(panel, 240, 240, 240, IsOdd)
                        ZAY_RGB_IF(panel, 255, 255, 255, !IsOdd)
                        ZAY_RGB_IF(panel, 150, 150, 100, i == mCurLayer)
                        ZAY_RGB_IF(panel, 200, 50, 50, 0 < mState.mLayers[i].mObjects.Count() || 0 < mState.mLayers[i].mPolygons.Count())
                            panel.fill();
                    }
                    ZAY_RGB(panel, 0, 0, 0)
                        panel.rect(2);
                    ZAY_RGBA(panel, 255, 128, 64, 192)
                    {
                        // 쇼잉상태
                        for(sint32 i = 0; i < mState.mLayerLength; ++i)
                        {
                            ZAY_LTRB(panel, panel.w() * i / mState.mLayerLength, -ProgressInnerSize, panel.w() * (i + 1) / mState.mLayerLength, 0)
                            ZAY_INNER_UI(panel, 2, String::Format("showflag-%d", i),
                                ZAY_GESTURE_NT(n, t, this)
                                {
                                    if(t == GT_Pressed)
                                    {
                                        const sint32 i = Parser::GetInt(&n[9]);
                                        mState.mLayers.At(i).mShowFlag ^= 1;
                                    }
                                })
                            {
                                ZAY_RGBA_IF(panel, -128, -128, -128, 128, !mState.mLayers[i].mShowFlag)
                                    panel.text((mState.mLayers[i].mShowFlag)? "SHOW" : "HIDE", UIFA_CenterMiddle, UIFE_Right);
                            }
                        }
                        // 커서위치
                        ZAY_FONT(panel, 0.8)
                        ZAY_XYRR(panel, panel.w() * (mCurLayer + 0.5) / mState.mLayerLength, panel.h(), 0, 0)
                        {
                            if(mCurLayer < 2)
                                panel.text(0, 3, String::Format("▲ B%d", 2 - mCurLayer), UIFA_CenterTop);
                            else panel.text(0, 3, String::Format("▲ %dF", mCurLayer - 1), UIFA_CenterTop);
                        }
                    }
                }
            }
        }
    }
}

void maptoolData::OnModeChanged()
{
    for(sint32 i = 0; i < mSelectBoxes.Count(); ++i)
        QuitSelectBox(i);
    mSelectBoxes.Clear();
    mCurSelectBox = -1;
    mCurDrawingPoints.SubtractionAll();
}

void maptoolData::OnSelectSub(chars name)
{
    if(!String::Compare(name, "Release"))
    {
        if(mCurSelectBox != -1)
        {
            QuitSelectBox(mCurSelectBox);
            mSelectBoxes.SubtractionSection(mCurSelectBox, 1);
            mCurSelectBox = -1;
        }
    }
    else if(!String::Compare(name, "Show") || !String::Compare(name, "Hide"))
    {
        if(mCurSelectBox != -1)
        {
            mSelectBoxes.At(mCurSelectBox).mVisibled ^= true;
            ChangeSelectBox(mCurSelectBox);
        }
    }
    else if(!String::Compare(name, "×"))
    {
        if(mCurSelectBox != -1)
        {
            mSelectBoxes.SubtractionSection(mCurSelectBox, 1);
            mCurSelectBox = -1;
        }
    }
}

void maptoolData::InitSelectBox(sint32 index)
{
    mCurSelectBox = index;
    auto& CurBox = mSelectBoxes.At(index);
    CurBox.mLayerX = CurBox.mX;
    CurBox.mLayerY = CurBox.mY;
    // 맵의 요소들
    for(sint32 i = 0; i < mState.mLayerLength; ++i)
    {
        if(!mState.mLayers[i].mShowFlag)
            continue;
        auto& CurLayer = CurBox.mLayers.AtWherever(i);
        // 오브젝트
        auto& CurObjects = mState.mLayers.At(i).mObjects;
        for(sint32 j = 0; j < CurObjects.Count(); ++j)
        {
            if(CurObjects[j].mRect.r < CurBox.mX) continue;
            if(CurObjects[j].mRect.b < CurBox.mY) continue;
            if(CurBox.mX + CurBox.mWidth <= CurObjects[j].mRect.l) continue;
            if(CurBox.mY + CurBox.mHeight <= CurObjects[j].mRect.t) continue;
            CurLayer.mObjects.AtAdding() = ToReference(CurObjects.At(j));
            CurObjects.SubtractionSection(j, 1);
            j--;
        }
        // 폴리곤
        auto& CurPolygons = mState.mLayers.At(i).mPolygons;
        for(sint32 j = 0; j < CurPolygons.Count(); ++j)
        {
            auto& CurPoints = CurPolygons[j].mPolygon;
            for(sint32 k = 0, kend = CurPoints.Count(); k < kend; ++k)
            {
                if(CurPoints[k].x < CurBox.mX) continue;
                if(CurPoints[k].y < CurBox.mY) continue;
                if(CurBox.mX + CurBox.mWidth <= CurPoints[k].x) continue;
                if(CurBox.mY + CurBox.mHeight <= CurPoints[k].y) continue;
                CurLayer.mPolygons.AtAdding() = ToReference(CurPolygons.At(j));
                CurPolygons.SubtractionSection(j, 1);
                j--;
                break;
            }
        }
    }
    // 타 선택박스
    for(sint32 i = 0; i < mSelectBoxes.Count(); ++i)
    {
        if(i == mCurSelectBox) continue;
        auto& OtherBox = mSelectBoxes.At(i);
        const float AddX = OtherBox.mX - OtherBox.mLayerX;
        const float AddY = OtherBox.mY - OtherBox.mLayerY;
        for(sint32 j = 0; j < OtherBox.mLayers.Count(); ++j)
        {
            auto& CurLayer = CurBox.mLayers.AtWherever(j);
            // 오브젝트
            auto& CurObjects = OtherBox.mLayers.At(j).mObjects;
            for(sint32 k = 0; k < CurObjects.Count(); ++k)
            {
                if(CurObjects[k].mRect.r + AddX < CurBox.mX) continue;
                if(CurObjects[k].mRect.b + AddY < CurBox.mY) continue;
                if(CurBox.mX + CurBox.mWidth <= CurObjects[k].mRect.l + AddX) continue;
                if(CurBox.mY + CurBox.mHeight <= CurObjects[k].mRect.t + AddY) continue;
                auto& NewObject = CurLayer.mObjects.AtAdding();
                NewObject = ToReference(CurObjects.At(k));
                NewObject.mRect += Point(AddX, AddY);
                CurObjects.SubtractionSection(k, 1);
                k--;
            }
            // 폴리곤
            auto& CurPolygons = OtherBox.mLayers.At(j).mPolygons;
            for(sint32 k = 0; k < CurPolygons.Count(); ++k)
            {
                auto& CurPoints = CurPolygons[k].mPolygon;
                for(sint32 l = 0, lend = CurPoints.Count(); l < lend; ++l)
                {
                    if(CurPoints[l].x + AddX < CurBox.mX) continue;
                    if(CurPoints[l].y + AddY < CurBox.mY) continue;
                    if(CurBox.mX + CurBox.mWidth <= CurPoints[l].x + AddX) continue;
                    if(CurBox.mY + CurBox.mHeight <= CurPoints[l].y + AddY) continue;
                    auto& NewPolygon = CurLayer.mPolygons.AtAdding();
                    NewPolygon = ToReference(CurPolygons.At(k));
                    for(sint32 m = 0, mend = NewPolygon.mPolygon.Count(); m < mend; ++m)
                        NewPolygon.mPolygon.At(m) += Point(AddX, AddY);
                    CurPolygons.SubtractionSection(k, 1);
                    k--;
                    break;
                }
            }
        }
    }
}

void maptoolData::QuitSelectBox(sint32 index)
{
    auto& CurBox = mSelectBoxes.At(index);
    const float AddX = CurBox.mX - CurBox.mLayerX;
    const float AddY = CurBox.mY - CurBox.mLayerY;
    for(sint32 i = 0; i < mState.mLayerLength; ++i)
    {
        auto& CurLayer = CurBox.mLayers.AtWherever(i);
        // 오브젝트
        for(sint32 j = 0; j < CurLayer.mObjects.Count(); ++j)
        {
            auto& NewObject = mState.mLayers.At(i).mObjects.AtAdding();
            NewObject = ToReference(CurLayer.mObjects.At(j));
            NewObject.mRect.l += AddX;
            NewObject.mRect.t += AddY;
            NewObject.mRect.r += AddX;
            NewObject.mRect.b += AddY;
        }
        // 폴리곤
        for(sint32 j = 0; j < CurLayer.mPolygons.Count(); ++j)
        {
            auto& NewPolygon = mState.mLayers.At(i).mPolygons.AtAdding();
            NewPolygon = ToReference(CurLayer.mPolygons.At(j));
            for(sint32 k = 0; k < NewPolygon.mPolygon.Count(); ++k)
            {
                NewPolygon.mPolygon.At(k).x += AddX;
                NewPolygon.mPolygon.At(k).y += AddY;
            }
        }
    }
}

void maptoolData::ChangeSelectBox(sint32 index)
{
    auto& CurBox = mSelectBoxes.At(index);
    for(sint32 i = 0; i < mState.mLayerLength; ++i)
    {
        auto& CurLayer = CurBox.mLayers.AtWherever(i);
        // 오브젝트
        for(sint32 j = 0; j < CurLayer.mObjects.Count(); ++j)
            CurLayer.mObjects.At(j).mVisible = CurBox.mVisibled;
        // 폴리곤
        for(sint32 j = 0; j < CurLayer.mPolygons.Count(); ++j)
            CurLayer.mPolygons.At(j).mVisible = CurBox.mVisibled;
    }
}

void maptoolData::OnSelectBoxMoving(sint32 index, float addx, float addy)
{
    mCurSelectBox = index;
    auto& CurBox = mSelectBoxes.At(index);
    CurBox.mX += addx / mState.mInGameW;
    CurBox.mY += addy / mState.mInGameH;
}

void maptoolData::OnSelectBoxMoved(sint32 index)
{
    mCurSelectBox = index;
    if(mGridMode)
    {
        auto& CurBox = mSelectBoxes.At(index);
        const Point NewPos = AttachGrid(CurBox.mX, CurBox.mY);
        CurBox.mX = NewPos.x;
        CurBox.mY = NewPos.y;
    }
}

void maptoolData::OnSelectBoxSizing(sint32 index, float addx, float addy)
{
    mCurSelectBox = index;
    auto& CurBox = mSelectBoxes.At(index);
    CurBox.mWidth = Math::MaxF(0, CurBox.mWidth + addx / mState.mInGameW);
    CurBox.mHeight = Math::MaxF(0, CurBox.mHeight + addy / mState.mInGameH);
}

void maptoolData::OnSelectBoxSized(sint32 index)
{
    mCurSelectBox = index;
    if(mGridMode)
    {
        auto& CurBox = mSelectBoxes.At(index);
        const Point NewPos = AttachGrid(CurBox.mX + CurBox.mWidth, CurBox.mY + CurBox.mHeight);
        CurBox.mWidth = Math::MaxF(0, NewPos.x - CurBox.mX);
        CurBox.mHeight = Math::MaxF(0, NewPos.y - CurBox.mY);
    }
}

void maptoolData::OnSelectBoxClone(sint32 index)
{
    mCurSelectBox = index;
    auto& NewBox = mSelectBoxes.AtAdding();
    auto& CurBox = mSelectBoxes.At(index);
    NewBox.CopyFrom(CurBox);
}
