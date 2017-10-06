#include "zay_types.h"
#include "zay_math.h"
#include "zay_matrix3.h"
#include "zay_matrix4.h"
#include "zay_plane.h"
#include "zay_quaternion.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_vector4.h"
#include "zay_random_generator.h"

namespace ZAY
{
    static const float pi = 4.0f * atan( 1.0f );
//    static const float half_pi = 0.5f * pi;
    
    float asm_arccos(float r)
    {
        return acosf(r);
    }
    
    float asm_arcsin(float r)
    {
        return asinf(r);
    }
    
    float asm_arctan(float r)
    {
        return atanf(r);
    }
    
    float asm_sin(float r)
    {
        return sinf(r);
    }
    
    float asm_cos(float r)
    {
        return cosf(r);
    }
    
    float asm_tan(float r)
    {
        return tanf(r);
    }
    
    float asm_sqrt(float r)
    {
        return sqrtf(r);
    }
    
    float asm_rsq(float r)
    {
        return 1.0f / sqrtf(r);
    }
    
    float apx_rsq( float r )
    {
        return 1.0f / sqrtf(r);
    }
    
    float asm_rand()
    {
        return static_cast<float>(rand());
    }
    
    float asm_rand_max()
    {
        return RAND_MAX;
    }
    
    float asm_ln(float r)
    {
        return logf(r);
    }

    
    
    const float Math::POS_INFINITY = std::numeric_limits<float>::infinity();
    const float Math::NEG_INFINITY = -std::numeric_limits<float>::infinity();
    const float Math::ONE_PI = 4.0f * atanf(1.0f);
    const float Math::TWO_PI = static_cast<float>(2.0f * M_PI);
    const float Math::HALF_PI = static_cast<float>(0.5f * M_PI);
    const float Math::fDeg2Rad = static_cast<float>(M_PI / 180.0f);
    const float Math::fRad2Deg = static_cast<float>(180.0f / M_PI);
    const float Math::LOG2 = std::log(2.0f);

    int Math::mTrigTableSize;
    Math::AngleUnit Math::msAngleUnit;

    float Math::mTrigTableFactor;
    float *Math::mSinTable = NULL;
    float *Math::mTanTable = NULL;

    //-----------------------------------------------------------------------
    Math::Math( unsigned int trigTableSize )
    {
        msAngleUnit = AU_DEGREE;
        mTrigTableSize = trigTableSize;
        mTrigTableFactor = mTrigTableSize / Math::TWO_PI;

        mSinTable = new float[mTrigTableSize];
        mTanTable = new float[mTrigTableSize];

        buildTrigTables();
    }

    //-----------------------------------------------------------------------
    Math::~Math()
    {
        delete mSinTable;
        delete mTanTable;
    }

    //-----------------------------------------------------------------------
    void Math::buildTrigTables(void)
    {
        // Build trig lookup tables
        // Could get away with building only PI sized Sin table but simpler this 
        // way. Who cares, it'll ony use an extra 8k of memory anyway and I like 
        // simplicity.
        float angle;
        for (int i = 0; i < mTrigTableSize; ++i)
        {
            angle = Math::TWO_PI * i / mTrigTableSize;
            mSinTable[i] = sin(angle);
            mTanTable[i] = tan(angle);
        }
    }
    //-----------------------------------------------------------------------    
    float Math::SinTable (float fValue)
    {
        // Convert range to index values, wrap if required
        int idx;
        if (fValue >= 0)
        {
            idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
        }
        else
        {
            idx = mTrigTableSize - (int(-fValue * mTrigTableFactor) % mTrigTableSize) - 1;
        }

        return mSinTable[idx];
    }
    //-----------------------------------------------------------------------
    float Math::TanTable (float fValue)
    {
        // Convert range to index values, wrap if required
        int idx = int(fValue *= mTrigTableFactor) % mTrigTableSize;
        return mTanTable[idx];
    }
    //-----------------------------------------------------------------------
    int Math::ISign (int iValue)
    {
        return ( iValue > 0 ? +1 : ( iValue < 0 ? -1 : 0 ) );
    }
    //-----------------------------------------------------------------------
    Radian Math::ACos (float fValue)
    {
        if ( -1.0f < fValue )
        {
            if ( fValue < 1.0f )
                return Radian(acos(fValue));
            else
                return Radian(0.0f);
        }
        else
        {
            return Radian(static_cast<float>(M_PI));
        }
    }
    //-----------------------------------------------------------------------
    Radian Math::ASin (float fValue)
    {
        if ( -1.0f < fValue )
        {
            if ( fValue < 1.0f )
                return Radian(asin(fValue));
            else
                return Radian(HALF_PI);
        }
        else
        {
            return Radian(-HALF_PI);
        }
    }
    //-----------------------------------------------------------------------
    float Math::Sign (float fValue)
    {
        if ( fValue > 0.0f )
            return 1.0f;

        if ( fValue < 0.0f )
            return -1.0f;

        return 0.0f;
    }
    //-----------------------------------------------------------------------
    float Math::InvSqrt(float fValue)
    {
        return asm_rsq(fValue);
    }
    //-----------------------------------------------------------------------
    float Math::UnitRandom ()
    {
        return RandomGenerator::frand(0.0f, 1.0f);
    }
    
    //-----------------------------------------------------------------------
    float Math::RangeRandom (float fLow, float fHigh)
    {
        return (fHigh-fLow)*UnitRandom() + fLow;
    }

    //-----------------------------------------------------------------------
    float Math::SymmetricRandom ()
    {
        return 2.0f * UnitRandom() - 1.0f;
    }

   //-----------------------------------------------------------------------
    void Math::setAngleUnit(Math::AngleUnit unit)
   {
       msAngleUnit = unit;
   }
   //-----------------------------------------------------------------------
   Math::AngleUnit Math::getAngleUnit(void)
   {
       return msAngleUnit;
   }
    //-----------------------------------------------------------------------
    float Math::AngleUnitsToRadians(float angleunits)
    {
       if (msAngleUnit == AU_DEGREE)
           return angleunits * fDeg2Rad;
       else
           return angleunits;
    }

    //-----------------------------------------------------------------------
    float Math::RadiansToAngleUnits(float radians)
    {
       if (msAngleUnit == AU_DEGREE)
           return radians * fRad2Deg;
       else
           return radians;
    }

    //-----------------------------------------------------------------------
    float Math::AngleUnitsToDegrees(float angleunits)
    {
       if (msAngleUnit == AU_RADIAN)
           return angleunits * fRad2Deg;
       else
           return angleunits;
    }

    //-----------------------------------------------------------------------
    float Math::DegreesToAngleUnits(float degrees)
    {
       if (msAngleUnit == AU_RADIAN)
           return degrees * fDeg2Rad;
       else
           return degrees;
    }

    //-----------------------------------------------------------------------
    bool Math::pointInTri2D(const Vector2& p, const Vector2& a, 
        const Vector2& b, const Vector2& c)
    {
        // Winding must be consistent from all edges for point to be inside
        Vector2 v1, v2;
        float dot[3];
        bool zeroDot[3];

        v1 = b - a;
        v2 = p - a;

        // Note we don't care about normalisation here since sign is all we need
        // It means we don't have to worry about magnitude of cross products either
        dot[0] = v1.crossProduct(v2);
        zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3f);


        v1 = c - b;
        v2 = p - b;

        dot[1] = v1.crossProduct(v2);
        zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3f);

        // Compare signs (ignore colinear / coincident points)
        if(!zeroDot[0] && !zeroDot[1] 
        && Math::Sign(dot[0]) != Math::Sign(dot[1]))
        {
            return false;
        }

        v1 = a - c;
        v2 = p - c;

        dot[2] = v1.crossProduct(v2);
        zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3f);
        // Compare signs (ignore colinear / coincident points)
        if((!zeroDot[0] && !zeroDot[2] 
            && Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
            (!zeroDot[1] && !zeroDot[2] 
            && Math::Sign(dot[1]) != Math::Sign(dot[2])))
        {
            return false;
        }


        return true;
    }
    //-----------------------------------------------------------------------
    bool Math::pointInTri3D(const Vector3& p, const Vector3& a, 
        const Vector3& b, const Vector3& c, const Vector3& normal)
    {
        // Winding must be consistent from all edges for point to be inside
        Vector3 v1, v2;
        float dot[3];
        bool zeroDot[3];

        v1 = b - a;
        v2 = p - a;

        // Note we don't care about normalisation here since sign is all we need
        // It means we don't have to worry about magnitude of cross products either
        dot[0] = v1.crossProduct(v2).dotProduct(normal);
        zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3f);


        v1 = c - b;
        v2 = p - b;

        dot[1] = v1.crossProduct(v2).dotProduct(normal);
        zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3f);

        // Compare signs (ignore colinear / coincident points)
        if(!zeroDot[0] && !zeroDot[1] 
            && Math::Sign(dot[0]) != Math::Sign(dot[1]))
        {
            return false;
        }

        v1 = a - c;
        v2 = p - c;

        dot[2] = v1.crossProduct(v2).dotProduct(normal);
        zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3f);
        // Compare signs (ignore colinear / coincident points)
        if((!zeroDot[0] && !zeroDot[2] 
            && Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
            (!zeroDot[1] && !zeroDot[2] 
            && Math::Sign(dot[1]) != Math::Sign(dot[2])))
        {
            return false;
        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool Math::RealEqual( float a, float b, float tolerance )
    {
        if (std::fabs(b-a) <= tolerance)
            return true;
        else
            return false;
    }

    //-----------------------------------------------------------------------
    Vector3 Math::calculateTangentSpaceVector(
        const Vector3& position1, const Vector3& position2, const Vector3& position3,
        float u1, float v1, float u2, float v2, float u3, float v3)
    {
        //side0 is the vector along one side of the triangle of vertices passed in, 
        //and side1 is the vector along another side. Taking the cross product of these returns the normal.
        Vector3 side0 = position1 - position2;
        Vector3 side1 = position3 - position1;
        //Calculate face normal
        Vector3 normal = side1.crossProduct(side0);
        normal.normalise();
        //Now we use a formula to calculate the tangent. 
        float deltaV0 = v1 - v2;
        float deltaV1 = v3 - v1;
        Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
        tangent.normalise();
        //Calculate binormal
        float deltaU0 = u1 - u2;
        float deltaU1 = u3 - u1;
        Vector3 binormal = deltaU1 * side0 - deltaU0 * side1;
        binormal.normalise();
        //Now, we take the cross product of the tangents to get a vector which 
        //should point in the same direction as our normal calculated above. 
        //If it points in the opposite direction (the dot product between the normals is less than zero), 
        //then we need to reverse the s and t tangents. 
        //This is because the triangle has been mirrored when going from tangent space to object space.
        //reverse tangents if necessary
        Vector3 tangentCross = tangent.crossProduct(binormal);
        if (tangentCross.dotProduct(normal) < 0.0f)
        {
            tangent = -tangent;
            binormal = -binormal;
        }

        return tangent;

    }
    //-----------------------------------------------------------------------
    Matrix4 Math::buildReflectionMatrix(const Plane& p)
    {
        return Matrix4(
            -2.0f * p.normal.x * p.normal.x + 1.0f,     -2.0f * p.normal.x * p.normal.y,            -2.0f * p.normal.x * p.normal.z,            -2.0f * p.normal.x * p.d,
            -2.0f * p.normal.y * p.normal.x,            -2.0f * p.normal.y * p.normal.y + 1.0f,     -2.0f * p.normal.y * p.normal.z,            -2.0f * p.normal.y * p.d,
            -2.0f * p.normal.z * p.normal.x,            -2.0f * p.normal.z * p.normal.y,            -2.0f * p.normal.z * p.normal.z + 1.0f,     -2.0f * p.normal.z * p.d,
            0.0f,                                       0.0f,                                       0.0f,                                       1.0f);
    }
    //-----------------------------------------------------------------------
    Vector4 Math::calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = calculateBasicFaceNormal(v1, v2, v3);
        // Now set up the w (distance of tri from origin
        return Vector4(normal.x, normal.y, normal.z, -(normal.dotProduct(v1)));
    }
    //-----------------------------------------------------------------------
    Vector3 Math::calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = (v2 - v1).crossProduct(v3 - v1);
        normal.normalise();
        return normal;
    }
    //-----------------------------------------------------------------------
    Vector4 Math::calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = calculateBasicFaceNormalWithoutNormalize(v1, v2, v3);
        // Now set up the w (distance of tri from origin)
        return Vector4(normal.x, normal.y, normal.z, -(normal.dotProduct(v1)));
    }
    //-----------------------------------------------------------------------
    Vector3 Math::calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3)
    {
        Vector3 normal = (v2 - v1).crossProduct(v3 - v1);
        return normal;
    }
    //-----------------------------------------------------------------------
    float Math::gaussianDistribution(float x, float offset, float scale)
    {
        float nom = Math::Exp(-Math::Sqr(x - offset) / (2 * Math::Sqr(scale)));
        float denom = scale * Math::Sqrt(2 * Math::ONE_PI);

        return nom / denom;

    }
    //---------------------------------------------------------------------
    Matrix4 Math::makeViewMatrix(const Vector3& position, const Quaternion& orientation,
        const Matrix4* reflectMatrix)
    {
        Matrix4 viewMatrix;

        // View matrix is:
        //
        //  [ Lx  Uy  Dz  Tx  ]
        //  [ Lx  Uy  Dz  Ty  ]
        //  [ Lx  Uy  Dz  Tz  ]
        //  [ 0   0   0   1   ]
        //
        // Where T = -(Transposed(Rot) * Pos)

        // This is most efficiently done using 3x3 Matrices
        Matrix3 rot;
        orientation.ToRotationMatrix(rot);

        // Make the translation relative to new axes
        Matrix3 rotT = rot.Transpose();
        Vector3 trans = -rotT * position;

        // Make final matrix
        viewMatrix = Matrix4::IDENTITY;
        viewMatrix = rotT; // fills upper 3x3
        viewMatrix[0][3] = trans.x;
        viewMatrix[1][3] = trans.y;
        viewMatrix[2][3] = trans.z;

        // Deal with reflections
        if (reflectMatrix)
        {
            viewMatrix = viewMatrix * (*reflectMatrix);
        }

        return viewMatrix;

    }
}
