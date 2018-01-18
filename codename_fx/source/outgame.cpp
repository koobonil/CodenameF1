#include <boss.hpp>
#include "outgame.hpp"

#include <service/boss_parasource.hpp>
#include <resource.hpp>

ZAY_DECLARE_VIEW_CLASS("outgameView", outgameData)

ZAY_VIEW_API OnCommand(CommandType type, chars topic, id_share in, id_cloned_share* out)
{
    if(type == CT_Tick)
    {
        // 하트/광고 업데이트
        m->UpdateHeartAdSec(true);
        // 세이브파일 업데이트
        if(m->mNeedUpdateSaveFile)
        {
            m->mNeedUpdateSaveFile = false;
            m->mSaveFile.SaveJson().ToFile("save.json");
        }
        // 아웃게임종료
        if(0 < m->mClosing && --m->mClosing == 0)
        {
            if(m->mClosingOption == 0)
            {
                Platform::Option::SetFlag("LandscapeMode", false);
                Platform::Option::SetFlag("DirectPlay", m->mStartMode != outgameMode::StaffRoll);
                m->next("ingameView");
            }
            else if(m->mClosingOption == 1)
            {
                Platform::Option::SetText("StartMode", "Lobby");
                m->next("outgameView");
            }
        }
        // 자동화면갱신
        m->invalidate();
    }
    else if(type == CT_Size)
    {
        sint32s WH = in;
        const sint32 Width = WH[0];
        const sint32 Height = WH[1];
        m->SetSize(Width, Height);
        if(!m->mSpineInited)
        {
            m->mSpineInited = true;
            m->InitForSpine();
        }
        // 윈도우 타이틀
        if(Platform::Option::GetFlag("DevMode"))
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

outgameData::outgameData() : FXState("fx/"),
    mLandscape(Platform::Option::GetFlag("LandscapeMode")),
    mStartMode(Platform::Option::GetText("StartMode"))
{
    Map<String> GlobalWeightMap;
    if(auto GlobalWeightTable = Context(ST_Json, SO_NeedCopy, String::FromFile("fx/table/globalweightfx_table.json")))
    {
        for(sint32 i = 0, iend = GlobalWeightTable.LengthOfIndexable(); i < iend; ++i)
        {
            chars CurID = GlobalWeightTable[i]("ID").GetString("noname");
            chars CurValue = GlobalWeightTable[i]("Value").GetString("0");
            GlobalWeightMap(CurID) = CurValue;
        }
    }
    else BOSS_ASSERT("globalweight_table.json의 로딩에 실패하였습니다", false);

    static sint32 FXID = 0;
    mChain = String::Format("FX%d", FXID++);
    mUILeft.Link(mChain, "UILeft", false);
    mUITop.Link(mChain, "UITop", false);
    mUIRight.Link(mChain, "UIRight", false);
    mUIBottom.Link(mChain, "UIBottom", false);
    mUILeft.Parse(GlobalWeightMap("UILeft"));
    mUITop.Parse(GlobalWeightMap("UITop"));
    mUIRight.Parse(GlobalWeightMap("UIRight"));
    mUIBottom.Parse(GlobalWeightMap("UIBottom"));
    mUILeft.Execute();
    mUITop.Execute();
    mUIRight.Execute();
    mUIBottom.Execute();

    mDefaultHeartCount = Parser::GetInt(GlobalWeightMap("DefaultHeartCount"));
    mHeartRegenSec = Parser::GetInt(GlobalWeightMap("HeartRegenTime")) / 1000;
    mHeartCountMax = Parser::GetInt(GlobalWeightMap("HeartCountMax"));
    mVideoCoolSec = Parser::GetInt(GlobalWeightMap("VideoCoolTime")) / 1000;

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

    mSpineInited = false;
    mShowingPopupId = -1;
    mClosing = -1;
    mClosingOption = 0;
    mChapterMax = 2;
    mCardMax = 3;
    mCurChapter = 0;
    mCurCard = -1;
    mNextCard = -1;
    mResultIsWin = false;
    mHeart = mDefaultHeartCount;
    mAdEnabled = false;
    mHeartUpdatedSec = (sint32) (Platform::Utility::CurrentTimeMsec() / 1000);
    mAdUpdatedSec = (sint32) (Platform::Utility::CurrentTimeMsec() / 1000);
    mCalcedHeartSec = 0;
    mCalcedAdSec = 0;

    mNeedUpdateSaveFile = false;
    String SaveString = String::FromFile("save.json");
    if(0 < SaveString.Length())
        mSaveFile.LoadJson(SO_NeedCopy, SaveString, SaveString.Length());

    // 하트정리
    if(!mSaveFile("SumHeart").IsValid())
    {
        mSaveFile.At("SumHeart").Set(String::FromInteger(mHeart));
        mSaveFile.At("HeartUpdatedSecond").Set(String::FromInteger(mHeartUpdatedSec));
        mSaveFile.At("AdUpdatedSecond").Set(String::FromInteger(mAdUpdatedSec));
        mNeedUpdateSaveFile = true;
    }
    else
    {
        mHeart = mSaveFile("SumHeart").GetInt(mHeart);
        mHeartUpdatedSec = mSaveFile("HeartUpdatedSecond").GetInt(mHeartUpdatedSec);
        mAdUpdatedSec = mSaveFile("AdUpdatedSecond").GetInt(mAdUpdatedSec);
    }
    UpdateHeartAdSec(false);

    // 결과정리
    chars LastStageID = Platform::Option::GetText("LastStageID");
    if(!*LastStageID) LastStageID = "Stage1";
    chars LastResult = Platform::Option::GetText("LastResult");
    if(!String::Compare(LastResult, "LOSE")) mResultIsWin = false;
    else if(!String::Compare(LastResult, "WIN-", 4))
    {
        mResultIsWin = true;
        const sint32 NewEgg = Math::Min(3, Parser::GetInt(LastResult + 4));
        const sint32 OldEgg = mSaveFile(LastStageID)("Egg").GetInt(0);
        if(OldEgg < NewEgg)
        {
            mSaveFile.At(LastStageID).At("Egg").Set(String::FromInteger(NewEgg));
            const sint32 SumEgg = mSaveFile("SumEgg").GetInt(0) + NewEgg - OldEgg;
            mSaveFile.At("SumEgg").Set(String::FromInteger(SumEgg));
            mNeedUpdateSaveFile = true;
        }
    }
    Platform::Option::SetText("LastResult", "");
}

outgameData::~outgameData()
{
    // 세이브파일 업데이트
    if(mNeedUpdateSaveFile)
    {
        mNeedUpdateSaveFile = false;
        mSaveFile.SaveJson().ToFile("save.json");
    }
}

void outgameData::SetSize(sint32 width, sint32 height)
{
    Solver ScreenWidth;
    Solver ScreenHeight;
    ScreenWidth.Link(mChain, "ScreenWidth", false);
    ScreenHeight.Link(mChain, "ScreenHeight", false);
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
    if(mStartMode == outgameMode::Lobby)
    {
        auto FinishedCB = [this](chars motionname)
        {
            if(!String::Compare("mid_card_left", motionname))
                mCurCard = mNextCard;
            else if(!String::Compare("mid_card_right", motionname))
                mCurCard = mNextCard;
            else if(!String::Compare("mid_card_to_", motionname, 12))
                mCurCard = mNextCard;
        };
        ReloadAllCards(true);
        switch(mCurChapter)
        {
        case 0: mUILobby.InitSpine(GetSpine("ui_lobby_mid"), "default", FinishedCB).PlayMotion("mid_forest_idle", true); break;
        case 1: mUILobby.InitSpine(GetSpine("ui_lobby_mid"), "default", FinishedCB).PlayMotion("mid_ice_idle", true); break;
        }
        mUILobbyTL.InitSpine(GetSpine("ui_lobby_top_left"));
        mUILobbyTR.InitSpine(GetSpine("ui_lobby_top_right")).PlayMotion("idle", true);
        mUILobbyBL.InitSpine(GetSpine("ui_lobby_bottom_left")).PlayMotion("idle_no", true);
        mUILobbyBR.InitSpine(GetSpine("ui_lobby_bottom_right")).PlayMotion("idle", true);
        UpdateHeartAd(true);

        auto PopupCB = [this](chars motionname)
        {
            if(mShowingPopupId != -1)
            {
                chars NameTail = motionname + boss_strlen(motionname) - 6;
                if(!String::Compare("pop_heart_ad_close", motionname))
                {
                    mShowingPopupId = -1;
                    Popup("heart get");
                }
                else if(!String::Compare("_close", NameTail))
                    mShowingPopupId = -1;
                else if(!String::Compare("pop_get_butten", motionname))
                {
                    mShowingPopupId = -1;
                    AdToHeart();
                }
            }
        };
        mUIPopups[0].InitSpine(GetSpine("ui_popup_heart"), "default", PopupCB);
        mUIPopups[1].InitSpine(GetSpine("ui_popup_get"), "default", PopupCB);
        mUIPopups[2].InitSpine(GetSpine("ui_popup_set"), "default", PopupCB);
    }
    else if(mStartMode == outgameMode::Result)
    {
        mUIResult.InitSpine(GetSpine("ui_result")).PlayMotionAttached(
            (mResultIsWin)? "result_win_loading" : "result_lose_loading",
            (mResultIsWin)? "result_win_idle" : "result_lose_idle", true);
    }
    else if(mStartMode == outgameMode::StaffRoll)
    {
        mUIStaffRoll.InitSpine(GetSpine("ui_staffroll"), "default",
            [this](chars motionname)
            {
                if(mClosing == -1 && !String::Compare("end", motionname))
                    mClosing = 50;
            }).PlayMotion("show", false);
    }
}

void outgameData::UpdateHeartAd(bool idle_only)
{
    const sint32 HeartCount = Math::Min(5 + 1, mHeart);
    mUILobbyTL.StopMotionAll();
    if(idle_only)
    {
        switch(HeartCount)
        {
        case 0: mUILobbyTL.PlayMotion("idle", true); break;
        case 1: mUILobbyTL.PlayMotion("oneheart_idle", true); break;
        case 2: mUILobbyTL.PlayMotion("twoheart_idle", true); break;
        case 3: mUILobbyTL.PlayMotion("threeheart_idle", true); break;
        case 4: mUILobbyTL.PlayMotion("fourheart_idle", true); break;
        case 5: mUILobbyTL.PlayMotion("fiveheart_idle", true); break;
        case 6: mUILobbyTL.PlayMotion("moreheart_idle", true); break;
        }
    }
    else
    {
        switch(HeartCount)
        {
        case 0: mUILobbyTL.PlayMotion("idle", true); break;
        case 1: mUILobbyTL.PlayMotionAttached("oneheart_show", "oneheart_idle", true); break;
        case 2: mUILobbyTL.PlayMotionAttached("twoheart_show", "twoheart_idle", true); break;
        case 3: mUILobbyTL.PlayMotionAttached("threeheart_show", "threeheart_idle", true); break;
        case 4: mUILobbyTL.PlayMotionAttached("fourheart_show", "fourheart_idle", true); break;
        case 5: mUILobbyTL.PlayMotionAttached("fiveheart_show", "fiveheart_idle", true); break;
        case 6: mUILobbyTL.PlayMotion("moreheart_idle", true); break;
        }
    }

    mUILobbyBL.StopMotionAll();
    if(idle_only)
    {
        if(mAdEnabled) mUILobbyBL.PlayMotion("idle_ok", true);
        else mUILobbyBL.PlayMotion("idle_no", true);
    }
    else
    {
        if(mAdEnabled) mUILobbyBL.PlayMotionAttached("no_to_ok", "idle_ok", true);
        else mUILobbyBL.PlayMotion("idle_no", true);
    }
}

void outgameData::UpdateHeartAdSec(bool animate)
{
    const sint32 CurSec = (sint32) (Platform::Utility::CurrentTimeMsec() / 1000);

    bool DoUpdate = false;
    sint32 HeartSec = CurSec - mHeartUpdatedSec;
    while(mHeartRegenSec <= HeartSec)
    {
        if(mHeart < mHeartCountMax)
        {
            mHeart++;
            if(animate)
                DoUpdate = true;
        }
        mHeartUpdatedSec += mHeartRegenSec;
        HeartSec -= mHeartRegenSec;
        mNeedUpdateSaveFile = true;
    }
    sint32 AdSec = CurSec - mAdUpdatedSec;
    if(!mAdEnabled && mVideoCoolSec <= AdSec)
    {
        mAdEnabled = true;
        if(animate)
            DoUpdate = true;
        AdSec = 0;
        mNeedUpdateSaveFile = true;
    }
    if(DoUpdate)
        UpdateHeartAd(false);

    mCalcedHeartSec = mHeartRegenSec - 1 - HeartSec;
    mCalcedAdSec = mVideoCoolSec - 1 - AdSec;
    if(mNeedUpdateSaveFile)
    {
        mSaveFile.At("SumHeart").Set(String::FromInteger(mHeart));
        mSaveFile.At("HeartUpdatedSecond").Set(String::FromInteger(mHeartUpdatedSec));
        mSaveFile.At("AdUpdatedSecond").Set(String::FromInteger(mAdUpdatedSec));
    }
}

void outgameData::AdToHeart()
{
    if(mAdEnabled)
    {
        mHeart++;
        mAdEnabled = false;
        mAdUpdatedSec = (sint32) (Platform::Utility::CurrentTimeMsec() / 1000);
        mCalcedAdSec = mVideoCoolSec - 1;
        UpdateHeartAd(true);

        mNeedUpdateSaveFile = true;
        mSaveFile.At("SumHeart").Set(String::FromInteger(mHeart));
        mSaveFile.At("HeartUpdatedSecond").Set(String::FromInteger(mHeartUpdatedSec));
        mSaveFile.At("AdUpdatedSecond").Set(String::FromInteger(mAdUpdatedSec));
    }
}

void outgameData::ReloadAllCards(bool create)
{
    for(sint32 i = 0; i < 48; ++i)
    {
        if(create) mCards[i].mSpine.InitSpine(GetSpine("ui_card"));
        else
        {
            const sint32 CurIndex = i + 48 * mCurChapter;
            const String CurID = String::Format("%d", CurIndex + 1);
            sint32 Egg = mSaveFile("Stage" + CurID)("Egg").GetInt(-1);

            const Context& CurStage = GetStage(CurIndex);
            const bool ExistUrl = (CurStage("ParaSource").GetString()[0] != '\0');
            if(!ExistUrl) Egg = -3;
            else if(Egg == -1)
            {
                const sint32 NeedEgg = CurStage("NeedEgg").GetInt(0);
                if(NeedEgg <= mSaveFile("SumEgg").GetInt(0))
                {
                    Egg = (NeedEgg == 0)? 0 : -2;
                    mSaveFile.At("Stage" + CurID).At("Egg").Set("0");
                    mNeedUpdateSaveFile = true;
                }
            }

            mCards[i].mLocked = (Egg == -1 || Egg == -3);
            mCards[i].mId = CurIndex + 1;
            mCards[i].mSpine.StopMotionAll();
            switch(Egg)
            {
            case -3: mCards[i].mSpine.PlayMotion("blank", true); break;
            case -2: mCards[i].mSpine.PlayMotionAttached("unlock", "noegg", true); break;
            case -1: mCards[i].mSpine.PlayMotion("idle", true); break;
            case 0: mCards[i].mSpine.PlayMotion("noegg", true); break;
            case 1: mCards[i].mSpine.PlayMotion("oneegg", true); break;
            case 2: mCards[i].mSpine.PlayMotion("twoegg", true); break;
            case 3: mCards[i].mSpine.PlayMotion("threeegg", true); break;
            }
        }
    }
}

bool outgameData::GoStage(sint32 id)
{
    if(mHeart == 0)
    {
        if(mAdEnabled)
            Popup("no heart");
        else Popup("no heart no ad");
        return false;
    }
    else
    {
        const Context& CurStage = GetStage(id - 1);
        String URL = CurStage("ParaSource").GetString();
        if(!String::CompareNoCase(URL, "http://", 7))
        {
            URL = URL.Right(URL.Length() - 7);
            sint32 SlashPos = URL.Find(0, '/');
            if(SlashPos != -1)
            {
                mClosing = 50;
                ParaSource Source(ParaSource::NaverCafe);
                Source.SetContact(URL.Left(SlashPos), 80);
                Contexts MapAndStage;
                Source.GetJsons(MapAndStage, URL.Right(URL.Length() - 1 - SlashPos));
                if(MapAndStage.Count() == 2)
                {
                    const String Params[2] = {"MapJson", "StageJson"};
                    for(sint32 i = 0; i < 2; ++i)
                    {
                        id_asset NewAsset = Asset::OpenForWrite(CurStage(Params[i]).GetString(), true);
                        const String NewJson = MapAndStage[i].SaveJson();
                        Asset::Write(NewAsset, (bytes)(chars) NewJson, NewJson.Length());
                        Asset::Close(NewAsset);
                    }
                    Platform::Option::SetText("StageName", CurStage("StageJson").GetString());
                    Platform::Option::SetText("LastStageID", String::Format("Stage%d", id));

                    // 어플종료후 재시작을 위한 저장
                    mSaveFile.At("LastStageJson").Set(Platform::Option::GetText("StageName"));
                    mSaveFile.At("LastStageID").Set(Platform::Option::GetText("LastStageID"));
                    // 하트감소
                    mSaveFile.At("SumHeart").Set(String::FromInteger(--mHeart));
                    UpdateHeartAd(true);
                    mNeedUpdateSaveFile = true;

                    // 파라토크/파라뷰 댓글전달
                    Context Comment;
                    if(Source.GetLastSpecialJson(Comment))
                    {
                        static const String ParaViewText = "[paraview]";

                        const Context& List = Comment("result")("list");
                        sint32 ParaTalkCount = 0, ParaViewCount = 0;
                        for(sint32 i = 0, iend = List.LengthOfIndexable(); i < iend; ++i)
                        {
                            const String ContentText = List[i]("content").GetString();
                            const sint32 ParaViewPos = ContentText.Find(0, ParaViewText);
                            if(ParaViewPos == -1)
                                Platform::Option::SetText(String::Format("ParaTalkText_%d", ParaTalkCount++), ContentText);
                            else Platform::Option::SetText(String::Format("ParaViewText_%d", ParaViewCount++),
                                ((chars) ContentText) + ParaViewPos + ParaViewText.Length());
                        }
                        Platform::Option::SetText("ParaTalkCount", String::FromInteger(ParaTalkCount));
                        Platform::Option::SetText("ParaViewCount", String::FromInteger(ParaViewCount));
                    }
                    else
                    {
                        Platform::Option::SetText("ParaTalkCount", "0");
                        Platform::Option::SetText("ParaViewCount", "0");
                    }
                    return true;
                }
                else mClosing = -1;
            }
        }
    }
    return false;
}

void outgameData::Render(ZayPanel& panel)
{
    // 배경색
    if(mStartMode == outgameMode::Result)
    {
        ZAY_RGB(panel, 39, 39, 39)
            panel.fill();
    }

    // 아웃게임
    ZAY_XYWH(panel, mInGameX, mInGameY, mInGameW, mInGameH)
    {
        if(mStartMode == outgameMode::Lobby)
        {
            // 로비: 스테이지 캡쳐/카드
            mUILobby.RenderObject(true, false, panel, false, "Lobby_",
                ZAY_GESTURE_NT(n, t, this)
                {
                    if(t == GT_Pressed && mClosing == -1)
                    {
                        if(mNextCard != -1)
                        {
                            if(!String::Compare(n, "Lobby_backbutten_area"))
                            {
                                if(0 < mNextCard && mNextCard == mCurCard) // 연타방지
                                {
                                    mNextCard--;
                                    mUILobby.StopMotionAll();
                                    mUILobby.PlayMotionOnce("mid_butten_back");
                                    mUILobby.PlayMotionAttached("mid_card_left", "mid_card_idle", true);
                                }
                            }
                            else if(!String::Compare(n, "Lobby_nextbutten_area"))
                            {
                                if(mNextCard < mCardMax - 1 && mNextCard == mCurCard) // 연타방지
                                {
                                    mNextCard++;
                                    mUILobby.StopMotionAll();
                                    mUILobby.PlayMotionOnce("mid_butten_next");
                                    mUILobby.PlayMotionAttached("mid_card_right", "mid_card_idle", true);
                                }
                            }
                            else if(!String::Compare(n, "Lobby_up_area"))
                            {
                                if(mNextCard == mCurCard) // 연타방지
                                {
                                    mNextCard = -1;
                                    mUILobby.StopMotionAll();
                                    switch(mCurChapter)
                                    {
                                    case 0: mUILobby.PlayMotionAttached("mid_card_to_forest", "mid_forest_idle", true); break;
                                    case 1: mUILobby.PlayMotionAttached("mid_card_to_ice", "mid_ice_idle", true); break;
                                    }
                                }
                            }
                            else if(!String::Compare(n, "Lobby_card_", 11))
                            {
                                const sint32 CardIndex = Parser::GetInt(n + 11) - 1;
                                if(!mCards[CardIndex].mLocked)
                                {
                                    const sint32 StageID = CardIndex + 1 + 16 * mCurCard + 48 * mCurChapter;
                                    if(0 < StageID) GoStage(StageID);
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
                            mNextCard = 0;
                            mUILobby.StopMotionAll();
                            mUILobby.PlayMotionAttached("mid_forest_to_card", "mid_card_idle", true);
                            ReloadAllCards(false);
                        }
                        else if(mCurChapter == 1 && !String::Compare(n, "Lobby_mid_stage_ice_area"))
                        {
                            mCurCard = 0;
                            mNextCard = 0;
                            mUILobby.StopMotionAll();
                            mUILobby.PlayMotionAttached("mid_ice_to_card", "mid_card_idle", true);
                            ReloadAllCards(false);
                        }
                    }
                },
                ZAY_RENDER_PN(p, n, this)
                {
                    sint32 CardID = -1;
                    if(!String::Compare(n, "card_", 5))
                        CardID = Parser::GetInt(n + 5) - 1 + 16 * mCurCard;
                    else if(!String::Compare(n, "leftcard_", 9))
                        CardID = Parser::GetInt(n + 9) - 1 + 16 * (mCurCard - 1);
                    else if(!String::Compare(n, "rightcard_", 10))
                        CardID = Parser::GetInt(n + 10) - 1 + 16 * (mCurCard + 1);

                    if(0 <= CardID && CardID < 48)
                    if(auto Area = mUILobby.GetBoundRect(n))
                    {
                        const float CardWidth = p.h() * Area->Width() / Area->Height();
                        ZAY_XYWH(p, (p.w() - CardWidth) / 2, 0, CardWidth, p.h())
                        {
                            mCards[CardID].mSpine.RenderObject(true, false, p, false, nullptr, nullptr,
                                ZAY_RENDER_PN(p, n, this, CardID)
                                {
                                    if(!String::Compare(n, "stage_number_area"))
                                    {
                                        const sint32 ImageW = R("s_a").w();
                                        const sint32 ImageH = R("s_a").h();
                                        const float DestRateR = ((sint32) (10 * p.h() / ImageH)) * 0.1f / 2;
                                        ZAY_COLOR_CLEAR(p)
                                        {
                                            if(mCards[CardID].mId < 10)
                                            {
                                                const sint32 Id = mCards[CardID].mId % 10;
                                                ZAY_XYRR(p, p.w() / 2, p.h() / 2, ImageW * DestRateR, ImageH * DestRateR)
                                                    p.stretch(R(String::Format("s_%c", 'a' + Id)), true);
                                            }
                                            else
                                            {
                                                const sint32 IdA = (mCards[CardID].mId / 10) % 10;
                                                const sint32 IdB = mCards[CardID].mId % 10;
                                                ZAY_XYRR(p, p.w() / 2 - ImageW * DestRateR, p.h() / 2, ImageW * DestRateR, ImageH * DestRateR)
                                                    p.stretch(R(String::Format("s_%c", 'a' + IdA)), true);
                                                ZAY_XYRR(p, p.w() / 2 + ImageW * DestRateR, p.h() / 2, ImageW * DestRateR, ImageH * DestRateR)
                                                    p.stretch(R(String::Format("s_%c", 'a' + IdB)), true);
                                            }
                                        }
                                    }
                                });
                        }
                    }
                });
        }
        else if(mStartMode == outgameMode::Result)
        {
            mUIResult.RenderObject(true, false, panel, false, "Result_",
                ZAY_GESTURE_NT(n, t, this)
                {
                    if(t == GT_Pressed && mClosing == -1)
                    {
                        if(!String::Compare(n, "Result_result_replay_area"))
                        {
                            chars StageID = Platform::Option::GetText("LastStageID");
                            const sint32 ID = Parser::GetInt(StageID + 5);
                            GoStage(ID);
                            mUIResult.PlayMotionOnce("result_replay");
                        }
                        else if(!String::Compare(n, "Result_result_lobby_area"))
                        {
                            mClosing = 50;
                            mClosingOption = 1;
                            mUIResult.PlayMotionOnce("result_lobby");
                        }
                        else if(!String::Compare(n, "Result_result_nextstage_area"))
                        {
                            chars StageID = Platform::Option::GetText("LastStageID");
                            const sint32 ID = Parser::GetInt(StageID + 5);
                            GoStage(ID + 1);
                            mUIResult.PlayMotionOnce("result_next_stage");
                        }
                    }
                }, mSubRenderer);
        }
        else if(mStartMode == outgameMode::StaffRoll)
        {
            mUIStaffRoll.RenderObject(true, false, panel, false, "StaffRoll_",
                ZAY_GESTURE_NT(n, t, this)
                {
                    if(t == GT_Pressed && mClosing == -1)
                    {
                        if(!String::Compare(n, "StaffRoll_staff roll_x_area"))
                            mUIStaffRoll.PlayMotion("end", false);
                    }
                },
                ZAY_RENDER_PN(p, n, this)
                {
                    if(!String::Compare(n, "str_", 4))
                    {
                        ZAY_FONT(p, p.h() / 14)
                            p.text(GetString(Parser::GetInt(n + 4)), UIFA_CenterMiddle, UIFE_Right);
                    }
                });
        }
    }

    if(mStartMode == outgameMode::Lobby)
    {
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
                {
                    mUILobbyTL.RenderObject(true, false, panel, false, nullptr, nullptr,
                        ZAY_RENDER_PN(p, n, this)
                        {
                            if(!String::Compare(n, "heart_time_area"))
                            {
                                ZAY_FONT(p, p.h() / 14)
                                {
                                    const sint32 TotalMin = mCalcedHeartSec / 60;
                                    const sint32 TotalSec = mCalcedHeartSec % 60;
                                    p.text(String::Format("%02d:%02d", TotalMin, TotalSec), UIFA_CenterMiddle, UIFE_Right);
                                }
                            }
                            else if(!String::Compare(n, "heart_score_a_area"))
                            {
                                if(5 < mHeart)
                                ZAY_FONT(p, p.h() / 14)
                                    p.text(String::Format("+%d", mHeart - 5), UIFA_CenterMiddle, UIFE_Right);
                            }
                            else if(!String::Compare(n, "egg_score_area"))
                            {
                                const sint32 SumEgg = mSaveFile("SumEgg").GetInt(0);
                                ZAY_FONT(p, p.h() / 14)
                                    p.text(String::FromInteger(SumEgg), UIFA_CenterMiddle, UIFE_Right);
                            }
                        });
                }
                BaseHeight = AreaHeight;
            }

            // 로비: 옵션
            if(const Rect* Area = mUILobbyTR.GetBoundRect("area"))
            {
                const sint32 AreaWidth = BaseHeight * Area->Width() / Area->Height();
                const Rect AreaRect(Point(panel.w() - AreaWidth, 0), Size(AreaWidth, BaseHeight));
                ZAY_RECT(panel, AreaRect)
                {
                    mUILobbyTR.RenderObject(true, false, panel, false, "Option_",
                        ZAY_GESTURE_NT(n, t, this)
                        {
                            if(t == GT_Pressed)
                            {
                                if(!String::Compare(n, "Option_set_area"))
                                {
                                    mUILobbyTR.PlayMotionOnce("top_setbutten");
                                    Popup("option");
                                }
                            }
                        });
                }
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
                {
                    mUILobbyBL.RenderObject(true, false, panel, false, "Ad_",
                        ZAY_GESTURE_NT(n, t, this)
                        {
                            if(t == GT_Pressed)
                            {
                                if(!String::Compare(n, "Ad_touch_area"))
                                {
                                    if(mAdEnabled)
                                    {
                                        if(mHeart < 5)
                                        {
                                            mUILobbyBL.PlayMotionOnce("touch");
                                            Popup("heart get");
                                        }
                                        else
                                        {
                                            mUILobbyBL.PlayMotionOnce("touch");
                                            Popup("heart is max");
                                        }
                                    }
                                }
                            }
                        },
                        ZAY_RENDER_PN(p, n, this)
                        {
                            if(!String::Compare(n, "ad_time_area"))
                            {
                                ZAY_FONT(p, p.h() / 14)
                                {
                                    const sint32 TotalMin = mCalcedAdSec / 60;
                                    const sint32 TotalSec = mCalcedAdSec % 60;
                                    p.text(String::Format("%02d:%02d", TotalMin, TotalSec), UIFA_CenterMiddle, UIFE_Right);
                                }
                            }
                        });
                }
            }

            // 로비: 아이템상점
            if(const Rect* Area = mUILobbyBR.GetBoundRect("area"))
            {
                const sint32 AreaWidth = BaseHeight * Area->Width() / Area->Height();
                const Rect AreaRect(Point(panel.w() - AreaWidth, panel.h() - BaseHeight), Size(AreaWidth, BaseHeight));
                ZAY_RECT(panel, AreaRect)
                    mUILobbyBR.RenderObject(true, false, panel, false, nullptr, nullptr, mSubRenderer);
            }
        }
    }

    // 팝업
    if(mShowingPopupId != -1)
    {
        ZAY_INNER_UI(panel, 0, "Popup")
        ZAY_RGBA(panel, 22, 40, 42, 160)
            panel.fill();
        mUIPopups[mShowingPopupId].RenderObject(true, false, panel, false, "Popup_",
            ZAY_GESTURE_NT(n, t, this)
            {
                if(t == GT_Pressed)
                {
                    chars NameTail = n + boss_strlen(n) - 6;
                    if(!String::Compare(NameTail, "x_area")
                        || !String::Compare(n, "Popup_pop_get_butten_area")
                        || !String::Compare(n, "Popup_str_27")
                        || !String::Compare(n, "Popup_watch_area"))
                        mUIPopups[mShowingPopupId].PlayMotion(mPopupCloseName, false);
                }
            }, mSubRenderer);
    }

    // 아웃게임클로징
    if(0 <= mClosing && mClosing < 50)
        ZAY_RGBA(panel, 0, 0, 0, 255 * (50 - mClosing) / 50)
            panel.fill();
}

void outgameData::Popup(chars name)
{
    if(!String::Compare(name, "no heart no ad"))
    {
        mShowingPopupId = 0;
        mUIPopups[mShowingPopupId].StopMotionAll();
        mUIPopups[mShowingPopupId].PlayMotionAttached("pop_no_heart_ad_loading", "pop_no_heart_ad_idle", true);
        mPopupCloseName = "pop_no_heart_ad_close";
    }
    else if(!String::Compare(name, "no heart"))
    {
        mShowingPopupId = 0;
        mUIPopups[mShowingPopupId].StopMotionAll();
        mUIPopups[mShowingPopupId].PlayMotionAttached("pop_heart_ad_loading", "pop_heart_ad_idle", true);
        mPopupCloseName = "pop_heart_ad_close";
    }
    else if(!String::Compare(name, "heart is max"))
    {
        mShowingPopupId = 0;
        mUIPopups[mShowingPopupId].StopMotionAll();
        mUIPopups[mShowingPopupId].PlayMotionAttached("pop_heart_max_loading", "pop_heart_max_idle", true);
        mPopupCloseName = "pop_heart_max_close";
    }
    else if(!String::Compare(name, "heart get"))
    {
        mShowingPopupId = 1;
        mUIPopups[mShowingPopupId].StopMotionAll();
        mUIPopups[mShowingPopupId].PlayMotionAttached("pop_get_loading", "pop_get_idle", true);
        mPopupCloseName = "pop_get_butten";
    }
    else if(!String::Compare(name, "option"))
    {
        mShowingPopupId = 2;
        mUIPopups[mShowingPopupId].StopMotionAll();
        mUIPopups[mShowingPopupId].PlayMotionAttached("pop_set_loading", "pop_set_idle", true);
        mPopupCloseName = "pop_set_close";
    }
    else BOSS_ASSERT("Popup의 명칭이 잘못되었습니다", false);
}
