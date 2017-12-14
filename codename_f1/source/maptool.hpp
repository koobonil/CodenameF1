#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

////////////////////////////////////////////////////////////////////////////////
class MapSelectBox
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(MapSelectBox)
public:
    MapSelectBox();
    ~MapSelectBox();
    MapSelectBox(MapSelectBox&& rhs);
    MapSelectBox& operator=(MapSelectBox&& rhs);

public:
    void CopyFrom(const MapSelectBox& rhs);

public:
    bool mFlagShow;
    bool mFlagCW;
    float mX;
    float mY;
    float mWidth;
    float mHeight;
    float mLayerX;
    float mLayerY;
    MapLayers mLayers;
};
typedef Array<MapSelectBox> MapSelectBoxes;

////////////////////////////////////////////////////////////////////////////////
class maptoolData : public F1Tool
{
    BOSS_DECLARE_NONCOPYABLE_INITIALIZED_CLASS(maptoolData, mObjectScrollMax(0))
public:
    maptoolData();
    ~maptoolData();

public:
    void Load(chars filename);
    void Save(chars filename);
    void Render(ZayPanel& panel);

public:
    void OnModeChanged() override;
    void OnSelectSub(chars name) override;
    void InitSelectBox(sint32 index) override;
    void QuitSelectBox(sint32 index) override;
    void ChangeSelectBox(sint32 type, sint32 index) override;
    void OnSelectBoxMoving(sint32 index, float addx, float addy) override;
    void OnSelectBoxMoved(sint32 index) override;
    void OnSelectBoxSizing(sint32 index, float addx, float addy) override;
    void OnSelectBoxSized(sint32 index) override;
    void OnSelectBoxClone(sint32 index) override;

public:
    sint32 mCurObject;
    Tween1D mObjectScroll;
    const sint32 mObjectScrollMax;
    sint32 mCurPolygon;
    sint32 mCurLayer;
    TryWorld::DotList mCurDrawingPoints;
    MapSelectBoxes mSelectBoxes;
    sint32 mCurSelectBox;
};
