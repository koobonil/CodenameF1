#pragma once
#include <service/boss_zay.hpp>
#include <element/boss_solver.hpp>
#include "classes.hpp"

class outgameData : public ZayObject
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(outgameData, mLandscape(false))

public:
    outgameData();
    ~outgameData();

public:
    void SetSize(sint32 width, sint32 height);
    void InitForSpine();
    void Render(ZayPanel& panel);

public: // 상수요소
    const bool mLandscape;

public: // 기획요소
    Solver mUILeft;
    Solver mUITop;
    Solver mUIRight;
    Solver mUIBottom;
    float mViewRate; // 뷰비율 = 가로길이 / 세로길이

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
    bool mIsSpineInited;
    sint32 mChapterMax;
    sint32 mCardMax;
    sint32 mCurChapter;
    sint32 mCurCard;
    Contexts mStageList;

public: // 스파인
    SpineRendererMap mAllSpines;
    MapSpine mUILobby;
    MapSpine mUILobbyTL;
    MapSpine mUILobbyTR;
    MapSpine mUILobbyBL;
    MapSpine mUILobbyBR;
};
