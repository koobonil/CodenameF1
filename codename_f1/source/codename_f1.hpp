#pragma once
#include <service/boss_zay.hpp>

class codename_f1Data : public ZayObject
{
public:
    codename_f1Data();
    ~codename_f1Data();

public:
    bool mIsLandscape;
    String mFirstStage;
};
