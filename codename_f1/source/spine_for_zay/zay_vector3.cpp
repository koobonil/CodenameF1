#include "zay_types.h"
#include "zay_vector3.h"

namespace ZAY
{
    const Vector3 Vector3::ZERO( 0.0f, 0.0f, 0.0f );
    const Vector3 Vector3::HALF( 0.5f, 0.5f, 0.5f );

    const Vector3 Vector3::UNIT_X( 1.0f, 0.0f, 0.0f );
    const Vector3 Vector3::UNIT_Y( 0.0f, 1.0f, 0.0f );
    const Vector3 Vector3::UNIT_Z( 0.0f, 0.0f, 1.0f );
    const Vector3 Vector3::NEGATIVE_UNIT_X( -1.0f,  0.0f,  0.0f );
    const Vector3 Vector3::NEGATIVE_UNIT_Y(  0.0f, -1.0f,  0.0f );
    const Vector3 Vector3::NEGATIVE_UNIT_Z(  0.0f,  0.0f, -1.0f );
    const Vector3 Vector3::UNIT_SCALE(1.0f, 1.0f, 1.0f);
}


