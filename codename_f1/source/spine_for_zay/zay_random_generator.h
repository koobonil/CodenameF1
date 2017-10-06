#pragma once

#include "zay_types.h"
#include "zay_vector3.h"

namespace ZAY
{
    class RandomGenerator
    {
    public:
        static int32_t irand(int32_t minValue, int32_t maxValue);
        static float frand(float minValue, float maxValue);
        static Vector3 vrand(const Vector3& min, const Vector3& max);
        static Vector3 vrandAxisY(float size);
        static Vector3 vrandAxisZ(float size);
        static Vector3 vrandUnitScale();
        static float frandInError(float value, float errorRatio);
    private:
        static std::mt19937_64 s_randomEngine;
    };
}
