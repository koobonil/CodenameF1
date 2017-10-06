#include "zay_math.h"
#include "zay_random_generator.h"
#include "zay_vector3.h"

namespace ZAY
{
    int32_t RandomGenerator::irand(int32_t minValue, int32_t maxValue)
    {
        std::uniform_int_distribution<int32_t> distribution(minValue, maxValue);

        return distribution(s_randomEngine);
    }
    
    float RandomGenerator::frand(float minValue, float maxValue)
    {
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        
        return distribution(s_randomEngine);
    }
    
    Vector3 RandomGenerator::vrand(const Vector3& min, const Vector3& max)
    {
        Vector3 ret;
        
        ret.x = std::uniform_real_distribution<float>(min.x, max.x)(s_randomEngine);
        ret.y = std::uniform_real_distribution<float>(min.y, max.y)(s_randomEngine);
        ret.z = std::uniform_real_distribution<float>(min.z, max.z)(s_randomEngine);
        
        return ret;
    }
    
    Vector3 RandomGenerator::vrandAxisY(float size)
    {
        Vector3 ret;
        
        float rad = std::uniform_real_distribution<float>(static_cast<float>(-M_PI), static_cast<float>(M_PI))(s_randomEngine);
        
        ret.x = std::cos(rad) * size;
        ret.y = 0.0f;
        ret.z = std::sin(rad) * size;
        
        return ret;
    }
    
    Vector3 RandomGenerator::vrandAxisZ(float size)
    {
        Vector3 ret;
        
        float rad = std::uniform_real_distribution<float>(static_cast<float>(-M_PI), static_cast<float>(M_PI))(s_randomEngine);
        
        ret.x = std::cos(rad) * size;
        ret.y = std::sin(rad) * size;
        ret.z = 0.0f;
        
        return ret;
    }
    
    Vector3 RandomGenerator::vrandUnitScale()
    {
        Vector3 ret;
        
        float radian0 = std::uniform_real_distribution<float>(static_cast<float>(-M_PI), static_cast<float>(M_PI))(s_randomEngine);
        float radian1 = std::uniform_real_distribution<float>(static_cast<float>(-M_PI), static_cast<float>(M_PI))(s_randomEngine);
        
        ret.x = std::cos(radian1) * std::cos(radian0);
        ret.y = std::cos(radian1) * std::sin(radian0);
        ret.z = std::sin(radian1);
        
        return ret;
        
    }

    float RandomGenerator::frandInError(float value, float errorRatio)
    {
        return value * std::uniform_real_distribution<float>(1.0f - errorRatio, 1.0f + errorRatio)(s_randomEngine);
    }

    std::mt19937_64 RandomGenerator::s_randomEngine;
}
