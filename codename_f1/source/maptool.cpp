#include <boss.hpp>
#include "maptool.hpp"

#include <r.hpp>

ZAY_DECLARE_VIEW_CLASS("maptoolView", maptoolData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        bool PosInRect = m->mLockedUI;
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
        if(m->mCursorInWindow != PosInRect)
        {
            m->mCursorInWindow = PosInRect;
            m->mUITween.MoveTo((PosInRect)? 0 : 100, 0.5);
        }
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->mState.SetSize(Width, Height);
    }
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    float fx = ((x - m->mMapPos.x) - m->mState.mInGameX) / m->mState.mInGameW - 0.5f;
    float fy = ((y - m->mMapPos.y) - m->mState.mInGameY) / m->mState.mInGameH - 0.5f;
    if(m->mUseGrid)
    {
        const float fxmax = 1000;
        const float fymax = 1000 * m->mState.mInGameH / m->mState.mInGameW;
        sint32 nx = 0, ny = 0;
        if(0 <= fx) nx = (((sint32) (fx * fxmax)) + m->mState.mToolGrid / 2) / m->mState.mToolGrid * m->mState.mToolGrid;
        else nx = -((((sint32) (-fx * fxmax)) + m->mState.mToolGrid / 2) / m->mState.mToolGrid * m->mState.mToolGrid);
        if(0 <= fy) ny = (((sint32) (fy * fymax)) + m->mState.mToolGrid / 2) / m->mState.mToolGrid * m->mState.mToolGrid;
        else ny = -((((sint32) (-fy * fymax)) + m->mState.mToolGrid / 2) / m->mState.mToolGrid * m->mState.mToolGrid);
        fx = nx / fxmax;
        fy = ny / fymax;
    }

    if(type == GT_Pressed)
    {
        // 작업중인 폴리곤에 점추가
        auto& NewPoint = m->mCurDrawingPoints.AtAdding();
        NewPoint.x = fx;
        NewPoint.y = fy;
    }
    else if(type == GT_InDragging || type == GT_OutDragging)
    {
        // 마지막 점을 이동
        if(0 < m->mCurDrawingPoints.Count())
        {
            auto& LastPoint = m->mCurDrawingPoints.At(-1);
            LastPoint.x = fx;
            LastPoint.y = fy;
            m->invalidate();
        }
    }
    else if(type == GT_InReleased || type == GT_OutReleased)
    {
        if(m->mCurObject != -1 && 2 <= m->mCurDrawingPoints.Count())
        {
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

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    // 출력미비영역 표현용
    ZAY_RGB(panel, 128, 128, 128)
        panel.fill();
    m->Render(panel);
}

maptoolData::maptoolData() : mUITween(updater())
{
    mCurObject = 0;
    mCurPolygon = -1;
    mCurLayer = 2;
    mCursorInWindow = false;
    mLockedUI = false;
    mUseGrid = false;
    mUITween.Reset(100);
}

maptoolData::~maptoolData()
{
}

void maptoolData::Load(chars filename)
{
    mCurObject = 0;
    mCurPolygon = -1;
    mCurLayer = 2;

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
        mState.Render(true, panel);

        // 작업중인 폴리곤
        if(mCurObject != -1)
        {
            if(2 <= mCurDrawingPoints.Count())
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
                if(i == 0 && 2 <= iend)
                {
                    ZAY_XYRR_UI(panel, x, y, 10, 10, "drawing_polygon",
                        ZAY_GESTURE_T(t, this)
                        {
                            if(t == GT_Pressed)
                            {
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
        if(mUseGrid)
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
    }

    // 툴UI
    const sint32 InnerGap = 10, ButtonSize = 80, IconSize = 50;
    const float OuterSize = ButtonSize * 1.5 * mUITween.value() / 100;
    ZAY_LTRB(panel, -OuterSize, -OuterSize, panel.w() + OuterSize, panel.h() + OuterSize)
    ZAY_FONT(panel, 1.2, "Arial Black")
    {
        // 고정버튼
        ZAY_XYWH(panel, ButtonSize * 1, 0, ButtonSize, ButtonSize)
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
                panel.text("Lock", UIFA_CenterMiddle);
            }
        }

        // 그리드버튼
        ZAY_XYWH(panel, ButtonSize * 2, 0, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "grid",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    mUseGrid = !mUseGrid;
                }
            })
        {
            ZAY_RGBA_IF(panel, 255, 128, 255, 192, mUseGrid)
            ZAY_RGBA_IF(panel, 128, 255, 255, 192, !mUseGrid)
                panel.fill();
            ZAY_RGB(panel, 0, 0, 0)
            {
                panel.rect(2);
                panel.text("Grid", UIFA_CenterMiddle);
            }
        }

        // 이동버튼
        ZAY_XYWH(panel, panel.w() - ButtonSize, 0, ButtonSize, ButtonSize)
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
        ZAY_XYWH(panel, panel.w() - InnerGap - IconSize, ButtonSize * 3 + InnerGap, IconSize, IconSize * PolygonCount)
        {
            for(sint32 i = 0; i < PolygonCount; ++i)
            {
                ZAY_XYWH_UI(panel, 0, IconSize * i, IconSize, IconSize, String::Format("polygon-%d", i),
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

        // BG로드버튼
        ZAY_XYWH(panel, 0, ButtonSize, ButtonSize, ButtonSize)
        ZAY_INNER_UI(panel, InnerGap, "load_bg",
            ZAY_GESTURE_T(t, this)
            {
                if(t == GT_InReleased)
                {
                    String BGName = mState.mBGName;
                    if(Platform::Popup::TextDialog(BGName, "Load BG", "Typing the asset name"))
                        mState.mBGName = BGName;
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
        ZAY_XYWH(panel, InnerGap, ButtonSize * 2 + InnerGap, IconSize, IconSize * ObjectCount)
        {
            for(sint32 i = 0; i < ObjectCount; ++i)
            {
                ZAY_XYWH_UI(panel, 0, IconSize * i, IconSize, IconSize, String::Format("object-%d", i),
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
