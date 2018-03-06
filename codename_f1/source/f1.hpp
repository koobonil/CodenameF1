#pragma once
#include <service/boss_zay.hpp>

class f1Data : public ZayObject
{
public:
    f1Data();
    ~f1Data();

public:
    inline static String& FirstStage() {static String _ = ""; return _;}

public:
    bool mIsLandscape;
    sint32 mGoNextCounter;
};
