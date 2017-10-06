#pragma once
#include <service/boss_zay.hpp>
#include <element/boss_tween.hpp>
#include "ingame.hpp"

class maptoolData : public ZayObject
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(maptoolData)
public:
    maptoolData();
    ~maptoolData();

public:
    void Load(chars filename);
    void Save(chars filename);
    void Render(ZayPanel& panel);

public: // 맵툴상태
    F1State mState;
    Point mMapPos;
    sint32 mCurObject;
    sint32 mCurPolygon;
    sint32 mCurLayer;
    Points mCurDrawingPoints;
    bool mCursorInWindow;
    Tween1D mUITween;
};
