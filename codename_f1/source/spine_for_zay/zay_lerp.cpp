#include "zay_types.h"
#include "zay_lerp.h"
#include "zay_animation_keyframe.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_vector4.h"
#include "zay_colour_value.h"

namespace ZAY
{
    template<>
    float lerp<float>(const float& left, const float& right, float ratio)
    {
        return left * (1.0f - ratio) + right * ratio;
    }
    
    template<>
    Vector2 lerp<Vector2>(const Vector2& left, const Vector2& right, float ratio)
    {
        return left * (1.0f - ratio) + right * ratio;
    }
    
    template<>
    Vector3 lerp<Vector3>(const Vector3& left, const Vector3& right, float ratio)
    {
        return left * (1.0f - ratio) + right * ratio;
    }
    
    template<>
    Vector4 lerp<Vector4>(const Vector4& left, const Vector4& right, float ratio)
    {
        return left * (1.0f - ratio) + right * ratio;
    }

    template<>
    ColourValue lerp<ColourValue>(const ColourValue& left, const ColourValue& right, float ratio)
    {
        return left * (1.0f - ratio) + right * ratio;
    }
    
    template<>
    Quaternion lerp<Quaternion>(const Quaternion& left, const Quaternion& right, float ratio)
    {
        return Quaternion::Slerp(ratio, left, right);
    }

    float convertRotationToRegularRange(float rotation)
    {
        while (rotation > 180.0f)
        {
            rotation -= 360.0f;
        }
        while (rotation < -180.0f)
        {
            rotation += 360.0f;
        }
        return rotation;
    }
    
    float lerp_rotation_min_path(float left, float right, float ratio)
    {
        right = left + convertRotationToRegularRange(right-left);

        return lerp(left, right, ratio);
    }

}
