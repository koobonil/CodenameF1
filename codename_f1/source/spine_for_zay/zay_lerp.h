#pragma once

#include "zay_types.h"

namespace ZAY
{
    template<typename T>
    T lerp(const T& left, const T& right, float ratio);
    
    float convertRotationToRegularRange(float rotation);
    float lerp_rotation_min_path(float left, float right, float ratio);
}
