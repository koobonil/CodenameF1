#pragma once

#include "zay_math.h"

namespace ZAY
{
    class Vector2
    {
    public:
        float x, y;

    public:
        inline Vector2()
        {
        }

        inline explicit Vector2(const float fX, const float fY )
            : x( fX ), y( fY )
        {
        }

        inline explicit Vector2( const float scaler )
            : x( scaler), y( scaler )
        {
        }

        inline explicit Vector2( const float afCoordinate[2] )
            : x( afCoordinate[0] ),
              y( afCoordinate[1] )
        {
        }

        inline explicit Vector2( const int afCoordinate[2] )
        {
            x = static_cast<float>(afCoordinate[0]);
            y = static_cast<float>(afCoordinate[1]);
        }

        inline explicit Vector2( float* const r )
            : x( r[0] ), y( r[1] )
        {
        }

        inline void swap(Vector2& other)
        {
            std::swap(x, other.x);
            std::swap(y, other.y);
        }

        inline float operator [] ( const size_t i ) const
        {
            assert( i < 2 );

            return *(&x+i);
        }

        inline float& operator [] ( const size_t i )
        {
            assert( i < 2 );

            return *(&x+i);
        }

        inline float* ptr()
        {
            return &x;
        }

        inline const float* ptr() const
        {
            return &x;
        }

        inline Vector2& operator = ( const Vector2& rkVector )
        {
            x = rkVector.x;
            y = rkVector.y;

            return *this;
        }

        inline Vector2& operator = ( const float fScalar)
        {
            x = fScalar;
            y = fScalar;

            return *this;
        }

        inline bool operator == ( const Vector2& rkVector ) const
        {
            return ( x == rkVector.x && y == rkVector.y );
        }

        inline bool operator != ( const Vector2& rkVector ) const
        {
            return ( x != rkVector.x || y != rkVector.y  );
        }

        // arithmetic operations
        inline Vector2 operator + ( const Vector2& rkVector ) const
        {
            return Vector2(
                x + rkVector.x,
                y + rkVector.y);
        }

        inline Vector2 operator - ( const Vector2& rkVector ) const
        {
            return Vector2(
                x - rkVector.x,
                y - rkVector.y);
        }

        inline Vector2 operator * ( const float fScalar ) const
        {
            return Vector2(
                x * fScalar,
                y * fScalar);
        }

        inline Vector2 operator * ( const Vector2& rhs) const
        {
            return Vector2(
                x * rhs.x,
                y * rhs.y);
        }

        inline Vector2 operator / ( const float fScalar ) const
        {
            assert( fScalar != 0.0f );

            float fInv = 1.0f / fScalar;

            return Vector2(
                x * fInv,
                y * fInv);
        }

        inline Vector2 operator / ( const Vector2& rhs) const
        {
            return Vector2(
                x / rhs.x,
                y / rhs.y);
        }

        inline const Vector2& operator + () const
        {
            return *this;
        }

        inline Vector2 operator - () const
        {
            return Vector2(-x, -y);
        }

        inline friend Vector2 operator * ( const float fScalar, const Vector2& rkVector )
        {
            return Vector2(
                fScalar * rkVector.x,
                fScalar * rkVector.y);
        }

        inline friend Vector2 operator / ( const float fScalar, const Vector2& rkVector )
        {
            return Vector2(
                fScalar / rkVector.x,
                fScalar / rkVector.y);
        }

        inline friend Vector2 operator + (const Vector2& lhs, const float rhs)
        {
            return Vector2(
                lhs.x + rhs,
                lhs.y + rhs);
        }

        inline friend Vector2 operator + (const float lhs, const Vector2& rhs)
        {
            return Vector2(
                lhs + rhs.x,
                lhs + rhs.y);
        }

        inline friend Vector2 operator - (const Vector2& lhs, const float rhs)
        {
            return Vector2(
                lhs.x - rhs,
                lhs.y - rhs);
        }

        inline friend Vector2 operator - (const float lhs, const Vector2& rhs)
        {
            return Vector2(
                lhs - rhs.x,
                lhs - rhs.y);
        }

        inline Vector2& operator += ( const Vector2& rkVector )
        {
            x += rkVector.x;
            y += rkVector.y;

            return *this;
        }

        inline Vector2& operator += ( const float fScaler )
        {
            x += fScaler;
            y += fScaler;

            return *this;
        }

        inline Vector2& operator -= ( const Vector2& rkVector )
        {
            x -= rkVector.x;
            y -= rkVector.y;

            return *this;
        }

        inline Vector2& operator -= ( const float fScaler )
        {
            x -= fScaler;
            y -= fScaler;

            return *this;
        }

        inline Vector2& operator *= ( const float fScalar )
        {
            x *= fScalar;
            y *= fScalar;

            return *this;
        }

        inline Vector2& operator *= ( const Vector2& rkVector )
        {
            x *= rkVector.x;
            y *= rkVector.y;

            return *this;
        }

        inline Vector2& operator /= ( const float fScalar )
        {
            assert( fScalar != 0.0f );

            float fInv = 1.0f / fScalar;

            x *= fInv;
            y *= fInv;

            return *this;
        }

        inline Vector2& operator /= ( const Vector2& rkVector )
        {
            x /= rkVector.x;
            y /= rkVector.y;

            return *this;
        }

        inline float length () const
        {
            return std::sqrt( x * x + y * y );
        }

        inline float squaredLength () const
        {
            return x * x + y * y;
        }

        inline float distance(const Vector2& rhs) const
        {
            return (*this - rhs).length();
        }

        inline float squaredDistance(const Vector2& rhs) const
        {
            return (*this - rhs).squaredLength();
        }

        inline float dotProduct(const Vector2& vec) const
        {
            return x * vec.x + y * vec.y;
        }

        inline float normalise()
        {
            float fLength = std::sqrt( x * x + y * y);

            if ( fLength > 0.0f )
            {
                float fInvLength = 1.0f / fLength;
                x *= fInvLength;
                y *= fInvLength;
            }

            return fLength;
        }

        inline Vector2 midPoint( const Vector2& vec ) const
        {
            return Vector2(( x + vec.x ) * 0.5f,
                           ( y + vec.y ) * 0.5f );
        }

        inline bool operator < ( const Vector2& rhs ) const
        {
            if( x < rhs.x && y < rhs.y )
                return true;
            return false;
        }

        inline bool operator > ( const Vector2& rhs ) const
        {
            if( x > rhs.x && y > rhs.y )
                return true;
            return false;
        }

        inline void makeFloor( const Vector2& cmp )
        {
            if( cmp.x < x ) x = cmp.x;
            if( cmp.y < y ) y = cmp.y;
        }

        inline void makeCeil( const Vector2& cmp )
        {
            if( cmp.x > x ) x = cmp.x;
            if( cmp.y > y ) y = cmp.y;
        }

        inline Vector2 perpendicular(void) const
        {
            return Vector2 (-y, x);
        }

        inline float crossProduct( const Vector2& rkVector ) const
        {
            return x * rkVector.y - y * rkVector.x;
        }

        inline Vector2 randomDeviant(float angle) const
        {
            angle *= Math::UnitRandom() * Math::TWO_PI;
            float cosa = std::cos(angle);
            float sina = std::sin(angle);
            return  Vector2(cosa * x - sina * y,
                            sina * x + cosa * y);
        }

        inline bool isZeroLength(void) const
        {
            float sqlen = (x * x) + (y * y);
            return (sqlen < (1e-06f * 1e-06f));

        }

        inline Vector2 normalisedCopy(void) const
        {
            Vector2 ret = *this;
            ret.normalise();
            return ret;
        }

        inline Vector2 reflect(Vector2* normal) const
        {
            return Vector2( *this - ( 2 * this->dotProduct(*normal) * *normal ) );
        }

        inline bool isNaN() const
        {
            return Math::isNaN(x) || Math::isNaN(y);
        }

        inline Radian angleBetween(const Vector2& other) const
        {        
            float lenProduct = length() * other.length();
            // Divide by zero check
            if(lenProduct < 1e-6f)
                lenProduct = 1e-6f;
        
            float f = dotProduct(other) / lenProduct;
    
            f = Math::Clamp(f, -1.0f, 1.0f);
            return Radian(std::cos(f));
        }

        inline Radian angleTo(const Vector2& other) const
        {
            Radian angle = angleBetween(other);
        
            if (crossProduct(other)<0)            
                angle = static_cast<Radian>(Math::TWO_PI) - angle;

            return angle;
        }

        // special points
        static const Vector2 ZERO;
        static const Vector2 UNIT_X;
        static const Vector2 UNIT_Y;
        static const Vector2 NEGATIVE_UNIT_X;
        static const Vector2 NEGATIVE_UNIT_Y;
        static const Vector2 UNIT_SCALE;

        /** Function for writing to a stream.
        */
        inline friend std::ostream& operator << ( std::ostream& o, const Vector2& v )
        {
            o << "Vector2(" << v.x << ", " << v.y <<  ")";
            return o;
        }
    };
}
