#pragma once
#include <service/boss_zay.hpp>
#include "classes.hpp"

class ingameData : public ZayObject, public F1State
{
public:
    ingameData();
    ~ingameData();

public:
    void AnimationOnce(sint32 timespan);
    void ClearPath();
    void Render(ZayPanel& panel);
    void ReadyForNextWave();
    void SetTouchSizeR(float size);
    void TouchAttack();

public: // 게임상태
    bool mShowDebug;
    Context mWaveData;
    sint32 mWave; // -1: 게임전, 0~N: 게임중
    String mWaveTitle;
    sint32 mWaveSec;
    MapMonsters mMonsters;
    Point mTouchPos;
    float mTouchSizeR;
    sint32 mTouchDamage;
};
