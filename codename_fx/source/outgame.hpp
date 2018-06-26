#pragma once
#include <service/boss_zay.hpp>
#include <element/boss_solver.hpp>
#include <element/boss_tween.hpp>
#include "classes.hpp"

class outgameMode
{
public:
    enum Type {Lobby, Result, StaffRoll, Max, Null = -1};
public:
    outgameMode() {mValue = Null;}
    outgameMode(const outgameMode& rhs) {operator=(rhs);}
    outgameMode(chars rhs) {operator=(rhs);}
    outgameMode& operator=(const outgameMode& rhs) {mValue = rhs.mValue; return *this;}
    outgameMode& operator=(Type rhs) {mValue = rhs; return *this;}
    outgameMode& operator=(chars rhs)
    {
        if(!String::Compare(rhs, "Lobby"))
            mValue = Lobby;
        else if(!String::Compare(rhs, "Result"))
            mValue = Result;
        else if(!String::Compare(rhs, "StaffRoll"))
            mValue = StaffRoll;
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

class outgameCard
{
public:
    outgameCard() {mLocked = true; mId = 0;}
    ~outgameCard() {}

public:
    bool mLocked;
    sint32 mId;
    MapSpine mSpine;
};

class outgameData : public ZayObject, public FXState
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(outgameData, mLandscape(false))

public:
    outgameData();
    ~outgameData();

public:
    void SetSize(sint32 width, sint32 height);
    void InitForSpine();
    void UpdateHeartAd(bool idle_only);
    void UpdateHeartAdSec(bool animate);
    void AdToHeart();
    void ReloadAllCards(bool create);
    bool GoStage(chars id, bool nopopup, bool paratalk);
    void SavePara(const Context& stage, ParaSource& source, bool paratalk);
    void SaveOption(const Context& stage);
    void Render(ZayPanel& panel);
    void Popup(chars name);

public: // 상수요소
    const bool mLandscape;
    const outgameMode mStartMode;

public: // 기획요소
    String mChain;
    Solver mUILeft;
    Solver mUITop;
    Solver mUIRight;
    Solver mUIBottom;
    String mDefaultLanguages;
    sint32 mDefaultHeartCount;
    sint32 mHeartRegenSec;
    sint32 mHeartCountMax;
    sint32 mVideoCoolSec;

public: // UI요소
    sint32 mUIL;
    sint32 mUIT;
    sint32 mUIR;
    sint32 mUIB;
    sint32 mScreenW;
    sint32 mScreenH;
    sint32 mInGameW;
    sint32 mInGameH;
    sint32 mInGameX;
    sint32 mInGameY;
    sint32 mInGameSize;

public: // 게임상태
    bool mSpineInited;
    sint32 mShowingPopupId;
    String mPopupCloseName;
    sint32 mClosing;
    sint32 mClosingOption;
    sint32 mChapterMax;
    sint32 mCardMax;
    sint32 mCurChapter;
    sint32 mCurCard;
    sint32 mNextCard;
    sint32 mHeart;
    bool mAdEnabled;
    sint32 mHeartUpdatedSec;
    sint32 mAdUpdatedSec;
    sint32 mCalcedHeartSec;
    sint32 mCalcedAdSec;
    bool mResultIsWin;
    outgameCard mCards[48];
    Tween1D mScreenLockTween;
    //id_purchase mDonate;

public: // 스파인
    MapSpine mUILobby;
    MapSpine mUILobbyTL;
    MapSpine mUILobbyTR;
    MapSpine mUILobbyBL;
    MapSpine mUILobbyBR;
    MapSpine mUIResult;
    MapSpine mUIStaffRoll;
    MapSpine mUIPopups[3];
};
