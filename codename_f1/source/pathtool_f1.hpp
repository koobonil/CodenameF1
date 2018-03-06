#pragma once
#include <service/boss_zay.hpp>
#include "classes_f1.hpp"

class pathtoolData : public F1Tool
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(pathtoolData)
public:
    pathtoolData();
    ~pathtoolData();

public:
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
    TryWorld::DotList mCurPoints;
};
