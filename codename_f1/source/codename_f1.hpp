#pragma once
#include <service/boss_zay.hpp>

class codename_f1Data : public ZayObject
{
public:
    codename_f1Data();
    ~codename_f1Data();

public:
    inline static String& FirstStage() {static String _ = ""; return _;}

public:
    bool mIsLandscape;
};
