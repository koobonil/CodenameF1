#include <boss.hpp>
#include "outgame.hpp"

#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("outgameView", outgameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        // 자동화면갱신
        m->invalidate();
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->SetSize(Width, Height);
        if(!m->mIsSpineInited)
        {
            m->mIsSpineInited = true;
            m->InitForSpine();
        }
        // 윈도우 타이틀
        Platform::SetWindowName(String::Format("Codename FX [%dx%d:%.03f]", Width, Height, Height / (float) Width));
    }
}

ZAY_VIEW_API OnNotify(chars sender, chars topic, id_share in, id_cloned_share* out)
{
}

ZAY_VIEW_API OnGesture(GestureType type, sint32 x, sint32 y)
{
    if(type == GT_Pressed)
    {
    }
    else if(type == GT_InDragging || type == GT_OutDragging)
    {
    }
    else if(type == GT_InReleased)
    {
    }
}

ZAY_VIEW_API OnRender(ZayPanel& panel)
{
    #if BOSS_WINDOWS | BOSS_LINUX | BOSS_MAC_OSX
        // 출력미비영역 표현용
        //Color TestColor(Platform::Utility::Random() | 0xFF000000);
        //ZAY_COLOR(panel, TestColor)
        //    panel.fill();
        ZAY_RGB(panel, 192, 192, 192)
            panel.fill();
    #endif
    m->Render(panel);
}

outgameData::outgameData() : mLandscape(landscape())
{
    Map<String> GlobalWeightMap;
    if(auto GlobalWeightTable = Context(ST_Json, SO_NeedCopy, String::FromFile("fx/table/globalweight_table.json")))
    {
        for(sint32 i = 0, iend = GlobalWeightTable.LengthOfIndexable(); i < iend; ++i)
        {
            chars CurID = GlobalWeightTable[i]("ID").GetString("noname");
            chars CurValue = GlobalWeightTable[i]("Value").GetString("0");
            GlobalWeightMap(CurID) = CurValue;
        }
    }
    else BOSS_ASSERT("globalweight_table.json의 로딩에 실패하였습니다", false);

    mUILeft.Link("FX", "UILeft", false);
    mUITop.Link("FX", "UITop", false);
    mUIRight.Link("FX", "UIRight", false);
    mUIBottom.Link("FX", "UIBottom", false);
    mUILeft.Parse(GlobalWeightMap("UILeft"));
    mUITop.Parse(GlobalWeightMap("UITop"));
    mUIRight.Parse(GlobalWeightMap("UIRight"));
    mUIBottom.Parse(GlobalWeightMap("UIBottom"));
    mUILeft.Execute();
    mUITop.Execute();
    mUIRight.Execute();
    mUIBottom.Execute();

    mUIL = 0;
    mUIT = 0;
    mUIR = 0;
    mUIB = 0;
    mScreenW = 0;
    mScreenH = 0;
    mInGameW = 0;
    mInGameH = 0;
    mInGameX = 0;
    mInGameY = 0;
    mInGameSize = 0;

    mIsSpineInited = false;
    mChapterMax = 2;
    mCardMax = 3;
    mCurChapter = 0;
    mCurCard = -1;

    Platform::File::Search("assets:/fx/spine",
        [](chars name, payload data)->void
        {
            outgameData* This = (outgameData*) data;
            This->mAllSpines(name).Create("fx/",
                String::Format("spine/%s/spine.json", name),
                String::Format("spine/%s/path.json", name));
        }, this, false);
}

outgameData::~outgameData()
{
}

void outgameData::SetSize(sint32 width, sint32 height)
{
    Solver ScreenWidth;
    Solver ScreenHeight;
    ScreenWidth.Link("FX", "ScreenWidth", false);
    ScreenHeight.Link("FX", "ScreenHeight", false);
    ScreenWidth.Parse(String::FromInteger(width));
    ScreenHeight.Parse(String::FromInteger(height));
    ScreenWidth.Execute();
    ScreenHeight.Execute();

    // 리사이징
    mUIL = (sint32) mUILeft.result();
    mUIT = (sint32) mUITop.result();
    mUIR = (sint32) mUIRight.result();
    mUIB = (sint32) mUIBottom.result();
    const sint32 ViewWidth = Math::Max(0, width - (mUIL + mUIR));
    const sint32 ViewHeight = Math::Max(0, height - (mUIT + mUIB));
    const float CurRate = ViewWidth / (float) ViewHeight;
    const float ViewRate = (mLandscape)? 8000.0f / 5000.0f : 5000.0f / 8000.0f;
    mScreenW = width;
    mScreenH = height;
    mInGameW = (sint32) (ViewWidth * ((CurRate < ViewRate)? 1 : ViewRate / CurRate));
    mInGameH = (sint32) (ViewHeight * ((CurRate < ViewRate)? CurRate / ViewRate : 1));
    mInGameX = mUIL + (ViewWidth - mInGameW) / 2;
    mInGameY = mUIT + (ViewHeight - mInGameH) / 2;
    mInGameSize = (mLandscape)? mInGameH : mInGameW;
}

void outgameData::InitForSpine()
{
    switch(mCurChapter)
    {
    case 0: mUILobby.InitSpine(&mAllSpines("ui_lobby_mid")).PlayMotion("mid_forest_idle", true); break;
    case 1: mUILobby.InitSpine(&mAllSpines("ui_lobby_mid")).PlayMotion("mid_ice_idle", true); break;
    }
    mUILobbyTL.InitSpine(&mAllSpines("ui_lobby_top_left")).PlayMotion("idle", true);
    mUILobbyTR.InitSpine(&mAllSpines("ui_lobby_top_right")).PlayMotion("idle", true);
    mUILobbyBL.InitSpine(&mAllSpines("ui_lobby_bottom_left")).PlayMotion("idle", true);
    mUILobbyBR.InitSpine(&mAllSpines("ui_lobby_bottom_right")).PlayMotion("idle", true);
}

void outgameData::Render(ZayPanel& panel)
{
    // 아웃게임
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
    {
        // 로비: 스테이지 캡쳐/카드
        mUILobby.RenderObject(true, false, panel, false, "Lobby_",
            ZAY_GESTURE_NT(n, t, this)
            {
                if(t == GT_Pressed)
                {
                    if(mCurCard != -1)
                    {
                        if(!String::Compare(n, "Lobby_backbutten_area"))
                        {
                            if(0 < mCurCard)
                            {
                                mCurCard--;
                                mUILobby.StopMotionAll();
                                mUILobby.PlayMotionOnce("mid_butten_back");
                                mUILobby.PlayMotionAttached("mid_card_left", "mid_card_idle", true);
                            }
                        }
                        else if(!String::Compare(n, "Lobby_nextbutten_area"))
                        {
                            if(mCurCard < mCardMax - 1)
                            {
                                mCurCard++;
                                mUILobby.StopMotionAll();
                                mUILobby.PlayMotionOnce("mid_butten_next");
                                mUILobby.PlayMotionAttached("mid_card_right", "mid_card_idle", true);
                            }
                        }
                        else if(!String::Compare(n, "Lobby_up_area"))
                        {
                            mCurCard = -1;
                            mUILobby.StopMotionAll();
                            switch(mCurChapter)
                            {
                            case 0: mUILobby.PlayMotionAttached("mid_card_to_forest", "mid_forest_idle", true); break;
                            case 1: mUILobby.PlayMotionAttached("mid_card_to_ice", "mid_ice_idle", true); break;
                            }
                        }
                    }
                    else if(!String::Compare(n, "Lobby_backbutten_area"))
                    {
                        if(0 < mCurChapter)
                        {
                            mUILobby.StopMotionAll();
                            mUILobby.PlayMotionOnce("mid_butten_back");
                            switch(mCurChapter--)
                            {
                            case 0: break;
                            case 1: mUILobby.PlayMotionAttached("mid_ice_to_forest", "mid_forest_idle", true); break;
                            }
                        }
                    }
                    else if(!String::Compare(n, "Lobby_nextbutten_area"))
                    {
                        if(mCurChapter < mChapterMax - 1)
                        {
                            mUILobby.StopMotionAll();
                            mUILobby.PlayMotionOnce("mid_butten_next");
                            switch(mCurChapter++)
                            {
                            case 0: mUILobby.PlayMotionAttached("mid_forest_to_ice", "mid_ice_idle", true); break;
                            case 1: break;
                            }
                        }
                    }
                    else if(mCurChapter == 0 && !String::Compare(n, "Lobby_mid_stage_forest_area"))
                    {
                        mCurCard = 0;
                        mUILobby.StopMotionAll();
                        mUILobby.PlayMotionAttached("mid_forest_to_card", "mid_card_idle", true);
                    }
                    else if(mCurChapter == 1 && !String::Compare(n, "Lobby_mid_stage_ice_area"))
                    {
                        mCurCard = 0;
                        mUILobby.StopMotionAll();
                        mUILobby.PlayMotionAttached("mid_ice_to_card", "mid_card_idle", true);
                    }
                }
            });

        //ZAY_RGB(panel, 255, 0, 0)
        //    panel.rect(2);
    }

    // 상단
    sint32 BaseHeight = 0;
    ZAY_LTRB(panel, 0, 0, panel.w(), mInGameY)
    {
        // 로비: 하트, 알게이지
        const sint32 TLWidthMax = panel.w() * 0.7;
        if(const Rect* Area = mUILobbyTL.GetBoundRect("area"))
        {
            const sint32 AreaWidth = Math::Min(TLWidthMax, panel.h() * Area->Width() / Area->Height());
            const sint32 AreaHeight = AreaWidth * Area->Height() / Area->Width();
            const Rect AreaRect(Point(0, 0), Size(AreaWidth, AreaHeight));
            ZAY_RECT(panel, AreaRect)
                mUILobbyTL.RenderObject(true, false, panel, false);
            BaseHeight = AreaHeight;
        }

        // 로비: 옵션
        if(const Rect* Area = mUILobbyTR.GetBoundRect("area"))
        {
            const sint32 AreaWidth = BaseHeight * Area->Width() / Area->Height();
            const Rect AreaRect(Point(panel.w() - AreaWidth, 0), Size(AreaWidth, BaseHeight));
            ZAY_RECT(panel, AreaRect)
                mUILobbyTR.RenderObject(true, false, panel, false);
        }
    }

    // 하단
    ZAY_LTRB(panel, 0, mInGameY + mInGameH, panel.w(), panel.h())
    {
        // 로비: 하트충전
        if(const Rect* Area = mUILobbyBL.GetBoundRect("area"))
        {
            const sint32 AreaWidth = BaseHeight * Area->Width() / Area->Height();
            const Rect AreaRect(Point(0, panel.h() - BaseHeight), Size(AreaWidth, BaseHeight));
            ZAY_RECT(panel, AreaRect)
                mUILobbyBL.RenderObject(true, false, panel, false);
        }

        // 로비: 아이템상점
        if(const Rect* Area = mUILobbyBR.GetBoundRect("area"))
        {
            const sint32 AreaWidth = BaseHeight * Area->Width() / Area->Height();
            const Rect AreaRect(Point(panel.w() - AreaWidth, panel.h() - BaseHeight), Size(AreaWidth, BaseHeight));
            ZAY_RECT(panel, AreaRect)
                mUILobbyBR.RenderObject(true, false, panel, false);
        }
    }
}
