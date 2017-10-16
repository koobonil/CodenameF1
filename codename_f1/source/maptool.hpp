#pragma once
#include <service/boss_zay.hpp>
#include "classes.hpp"

class maptoolData : public F1Tool
{
    BOSS_DECLARE_NONCOPYABLE_CLASS(maptoolData)
public:
    maptoolData();
    ~maptoolData();

public:
    void Load(chars filename);
    void Save(chars filename);
    void Render(ZayPanel& panel);

public:
    sint32 mCurObject;
    sint32 mCurPolygon;
    sint32 mCurLayer;
    Points mCurDrawingPoints;
};
