#pragma once

#include "zay_types.h"

#ifndef M_PI
    #define M_PI (3.14159265358979323846)
#endif

namespace ZAY
{
    class Radian;
    class Degree;
    class Angle;

    class Radian
    {
    private:
        float _rad;
        
    public:
        explicit Radian ( float r=0.0f ) : _rad(r) {}
        Radian ( const Degree& d );
        Radian& operator = ( const float& f ) { _rad = f; return *this; }
        Radian& operator = ( const Radian& r ) { _rad = r._rad; return *this; }
        Radian& operator = ( const Degree& d );
        
        float valueDegrees() const;
        float valueRadians() const { return _rad; }
        float valueAngleUnits() const;
        
        const Radian& operator + () const { return *this; }
        Radian operator + ( const Radian& r ) const { return Radian ( _rad + r._rad ); }
        Radian operator + ( const Degree& d ) const;
        Radian& operator += ( const Radian& r ) { _rad += r._rad; return *this; }
        Radian& operator += ( const Degree& d );
        Radian operator - () const { return Radian(-_rad); }
        Radian operator - ( const Radian& r ) const { return Radian ( _rad - r._rad ); }
        Radian operator - ( const Degree& d ) const;
        Radian& operator -= ( const Radian& r ) { _rad -= r._rad; return *this; }
        Radian& operator -= ( const Degree& d );
        Radian operator * ( float f ) const { return Radian ( _rad * f ); }
        Radian operator * ( const Radian& f ) const { return Radian ( _rad * f._rad ); }
        Radian& operator *= ( float f ) { _rad *= f; return *this; }
        Radian operator / ( float f ) const { return Radian ( _rad / f ); }
        Radian& operator /= ( float f ) { _rad /= f; return *this; }
        
        bool operator <  ( const Radian& r ) const { return _rad <  r._rad; }
        bool operator <= ( const Radian& r ) const { return _rad <= r._rad; }
        bool operator == ( const Radian& r ) const { return _rad == r._rad; }
        bool operator != ( const Radian& r ) const { return _rad != r._rad; }
        bool operator >= ( const Radian& r ) const { return _rad >= r._rad; }
        bool operator >  ( const Radian& r ) const { return _rad >  r._rad; }
        
        inline friend std::ostream& operator << ( std::ostream& o, const Radian& v )
        {
            o << "Radian(" << v.valueRadians() << ")";
            return o;
        }
    };
    
    
    
    class Degree
    {
    private:
        float _deg;
        
    public:
        explicit Degree ( float d=0.0f ) : _deg(d) {}
        Degree ( const Radian& r ) : _deg(r.valueDegrees()) {}
        Degree& operator = ( const float& f ) { _deg = f; return *this; }
        Degree& operator = ( const Degree& d ) { _deg = d._deg; return *this; }
        Degree& operator = ( const Radian& r ) { _deg = r.valueDegrees(); return *this; }
        
        float valueDegrees() const { return _deg; }
        float valueRadians() const;
        float valueAngleUnits() const;
        
        const Degree& operator + () const { return *this; }
        Degree operator + ( const Degree& d ) const { return Degree ( _deg + d._deg ); }
        Degree operator + ( const Radian& r ) const { return Degree ( _deg + r.valueDegrees() ); }
        Degree& operator += ( const Degree& d ) { _deg += d._deg; return *this; }
        Degree& operator += ( const Radian& r ) { _deg += r.valueDegrees(); return *this; }
        Degree operator - () const { return Degree(-_deg); }
        Degree operator - ( const Degree& d ) const { return Degree ( _deg - d._deg ); }
        Degree operator - ( const Radian& r ) const { return Degree ( _deg - r.valueDegrees() ); }
        Degree& operator -= ( const Degree& d ) { _deg -= d._deg; return *this; }
        Degree& operator -= ( const Radian& r ) { _deg -= r.valueDegrees(); return *this; }
        Degree operator * ( float f ) const { return Degree ( _deg * f ); }
        Degree operator * ( const Degree& f ) const { return Degree ( _deg * f._deg ); }
        Degree& operator *= ( float f ) { _deg *= f; return *this; }
        Degree operator / ( float f ) const { return Degree ( _deg / f ); }
        Degree& operator /= ( float f ) { _deg /= f; return *this; }
        
        bool operator <  ( const Degree& d ) const { return _deg <  d._deg; }
        bool operator <= ( const Degree& d ) const { return _deg <= d._deg; }
        bool operator == ( const Degree& d ) const { return _deg == d._deg; }
        bool operator != ( const Degree& d ) const { return _deg != d._deg; }
        bool operator >= ( const Degree& d ) const { return _deg >= d._deg; }
        bool operator >  ( const Degree& d ) const { return _deg >  d._deg; }
        
        inline friend std::ostream& operator << ( std::ostream& o, const Degree& v )
        {
            o << "Degree(" << v.valueDegrees() << ")";
            return o;
        }
    };
    
    
    
    class Angle
    {
    private:
        float _angle;
    public:
        explicit Angle ( float angle ) : _angle(angle) {}
        operator Radian() const;
        operator Degree() const;
    };

    inline Radian::Radian ( const Degree& d ) : _rad(d.valueRadians())
    {
    }
    
    inline Radian& Radian::operator = ( const Degree& d )
    {
        _rad = d.valueRadians(); return *this;
    }
    
    inline Radian Radian::operator + ( const Degree& d ) const
    {
        return Radian ( _rad + d.valueRadians() );
    }
    
    inline Radian& Radian::operator += ( const Degree& d )
    {
        _rad += d.valueRadians();
        return *this;
    }
    
    inline Radian Radian::operator - ( const Degree& d ) const
    {
        return Radian ( _rad - d.valueRadians() );
    }
    
    inline Radian& Radian::operator -= ( const Degree& d )
    {
        _rad -= d.valueRadians();
        return *this;
    }
    
    
    class Math
    {
    public:
        enum AngleUnit
        {
            AU_DEGREE,
            AU_RADIAN
        };

    protected:
        // angle units used by the api
        static AngleUnit msAngleUnit;
        
        /// Size of the trig tables as determined by constructor.
        static int mTrigTableSize;
        
        /// Radian -> index factor value ( mTrigTableSize / 2 * PI )
        static float mTrigTableFactor;
        static float* mSinTable;
        static float* mTanTable;
        
        /** Private function to build trig tables.
         */
        void buildTrigTables();
        
        static float SinTable (float fValue);
        static float TanTable (float fValue);
    public:
        /** Default constructor.
         @param
         trigTableSize Optional parameter to set the size of the
         tables used to implement Sin, Cos, Tan
         */
        Math(unsigned int trigTableSize = 4096);
        
        /** Default destructor.
         */
        virtual ~Math();
        
        static inline int IAbs (int iValue) { return ( iValue >= 0 ? iValue : -iValue ); }
        static inline int ICeil (float fValue) { return int(ceil(fValue)); }
        static inline int IFloor (float fValue) { return int(floor(fValue)); }
        static int ISign (int iValue);
        
        /** Absolute value function
         @param
         fValue The value whose absolute value will be returned.
         */
        static inline float Abs (float fValue) { return std::fabs(fValue); }
        
        /** Absolute value function
         @param dValue
         The value, in degrees, whose absolute value will be returned.
         */
        static inline Degree Abs (const Degree& dValue) { return Degree(fabsf(dValue.valueDegrees())); }
        
        /** Absolute value function
         @param rValue
         The value, in radians, whose absolute value will be returned.
         */
        static inline Radian Abs (const Radian& rValue) { return Radian(fabsf(rValue.valueRadians())); }
        
        /** Arc cosine function
         @param fValue
         The value whose arc cosine will be returned.
         */
        static Radian ACos (float fValue);
        
        /** Arc sine function
         @param fValue
         The value whose arc sine will be returned.
         */
        static Radian ASin (float fValue);
        
        /** Arc tangent function
         @param fValue
         The value whose arc tangent will be returned.
         */
        static inline Radian ATan (float fValue) { return Radian(atanf(fValue)); }
        
        /** Arc tangent between two values function
         @param fY
         The first value to calculate the arc tangent with.
         @param fX
         The second value to calculate the arc tangent with.
         */
        static inline Radian ATan2 (float fY, float fX) { return Radian(atan2f(fY,fX)); }
        
        /** Ceiling function
         Returns the smallest following integer. (example: Ceil(1.1) = 2)
         
         @param fValue
         The value to round up to the nearest integer.
         */
        static inline float Ceil (float fValue) { return std::ceil(fValue); }
        static inline bool isNaN(float f)
        {
            // std::isnan() is C99, not supported by all compilers
            // However NaN always fails this next test, no other number does.
            return f != f;
        }
        
        /** Cosine function.
         @param fValue
         Angle in radians
         @param useTables
         If true, uses lookup tables rather than
         calculation - faster but less accurate.
         */
        static inline float Cos (const Radian& fValue, bool useTables = false) {
            return (!useTables) ? std::cos(fValue.valueRadians()) : SinTable(fValue.valueRadians() + HALF_PI);
        }
        /** Cosine function.
         @param fValue
         Angle in radians
         @param useTables
         If true, uses lookup tables rather than
         calculation - faster but less accurate.
         */
        static inline float Cos (float fValue, bool useTables = false) {
            return (!useTables) ? std::cos(fValue) : SinTable(fValue + HALF_PI);
        }
        
        static inline float Exp (float fValue) { return std::exp(fValue); }
        
        /** Floor function
         Returns the largest previous integer. (example: Floor(1.9) = 1)
         
         @param fValue
         The value to round down to the nearest integer.
         */
        static inline float Floor (float fValue) { return std::floor(fValue); }
        
        static inline float Log (float fValue) { return std::log(fValue); }
        
        /// Stored value of log(2) for frequent use
        static const float LOG2;
        
        static inline float Log2 (float fValue) { return std::log(fValue)/LOG2; }
        
        static inline float LogN (float base, float fValue) { return std::log(fValue)/std::log(base); }
        
        static inline float Pow (float fBase, float fExponent) { return std::pow(fBase,fExponent); }
        
        static float Sign (float fValue);
        static inline Radian Sign ( const Radian& rValue )
        {
            return Radian(Sign(rValue.valueRadians()));
        }
        static inline Degree Sign ( const Degree& dValue )
        {
            return Degree(Sign(dValue.valueDegrees()));
        }
        
        //Simulate the shader function saturate that clamps a parameter value between 0 and 1
        static inline float saturate(float t) { return (t < 0) ? 0 : ((t > 1) ? 1 : t); }
        static inline double saturate(double t) { return (t < 0) ? 0 : ((t > 1) ? 1 : t); }
        
        //Simulate the shader function lerp which performers linear interpolation
        //given 3 parameters v0, v1 and t the function returns the value of (1 ñ t)* v0 + t * v1.
        //where v0 and v1 are matching vector or scalar types and t can be either a scalar or a vector of the same type as a and b.
        template<typename V, typename T> static V lerp(const V& v0, const V& v1, const T& t) {
            return v0 * (1 - t) + v1 * t; }
        
        /** Sine function.
         @param fValue
         Angle in radians
         @param useTables
         If true, uses lookup tables rather than
         calculation - faster but less accurate.
         */
        static inline float Sin (const Radian& fValue, bool useTables = false) {
            return (!useTables) ? std::sin(fValue.valueRadians()) : SinTable(fValue.valueRadians());
        }
        /** Sine function.
         @param fValue
         Angle in radians
         @param useTables
         If true, uses lookup tables rather than
         calculation - faster but less accurate.
         */
        static inline float Sin (float fValue, bool useTables = false) {
            return (!useTables) ? std::sin(fValue) : SinTable(fValue);
        }
        
        /** Squared function.
         @param fValue
         The value to be squared (fValue^2)
         */
        static inline float Sqr (float fValue) { return fValue*fValue; }
        
        /** Square root function.
         @param fValue
         The value whose square root will be calculated.
         */
        static inline float Sqrt (float fValue) { return std::sqrt(fValue); }
        
        /** Square root function.
         @param fValue
         The value, in radians, whose square root will be calculated.
         @return
         The square root of the angle in radians.
         */
        static inline Radian Sqrt (const Radian& fValue) { return Radian(sqrtf(fValue.valueRadians())); }
        
        /** Square root function.
         @param fValue
         The value, in degrees, whose square root will be calculated.
         @return
         The square root of the angle in degrees.
         */
        static inline Degree Sqrt (const Degree& fValue) { return Degree(sqrtf(fValue.valueDegrees())); }
        
        /** Inverse square root i.e. 1 / Sqrt(x), good for vector
         normalisation.
         @param fValue
         The value whose inverse square root will be calculated.
         */
        static float InvSqrt (float fValue);
        
        /** Generate a random number of unit length.
         @return
         A random number in the range from [0,1].
         */
        static float UnitRandom ();
        
        /** Generate a random number within the range provided.
         @param fLow
         The lower bound of the range.
         @param fHigh
         The upper bound of the range.
         @return
         A random number in the range from [fLow,fHigh].
         */
        static float RangeRandom (float fLow, float fHigh);
        
        /** Generate a random number in the range [-1,1].
         @return
         A random number in the range from [-1,1].
         */
        static float SymmetricRandom ();
        
        /** Tangent function.
         @param fValue
         Angle in radians
         @param useTables
         If true, uses lookup tables rather than
         calculation - faster but less accurate.
         */
        static inline float Tan (const Radian& fValue, bool useTables = false) {
            return (!useTables) ? std::tan(fValue.valueRadians()) : TanTable(fValue.valueRadians());
        }
        /** Tangent function.
         @param fValue
         Angle in radians
         @param useTables
         If true, uses lookup tables rather than
         calculation - faster but less accurate.
         */
        static inline float Tan (float fValue, bool useTables = false) {
            return (!useTables) ? std::tan(fValue) : TanTable(fValue);
        }
        
        static inline float DegreesToRadians(float degrees) { return degrees * fDeg2Rad; }
        static inline float RadiansToDegrees(float radians) { return radians * fRad2Deg; }
        
        /** These functions used to set the assumed angle units (radians or degrees)
         expected when using the Angle type.
         @par
         You can set this directly after creating a new Root, and also before/after resource creation,
         depending on whether you want the change to affect resource files.
         */
        static void setAngleUnit(AngleUnit unit);
        /** Get the unit being used for angles. */
        static AngleUnit getAngleUnit(void);
        
        /** Convert from the current AngleUnit to radians. */
        static float AngleUnitsToRadians(float units);
        /** Convert from radians to the current AngleUnit . */
        static float RadiansToAngleUnits(float radians);
        /** Convert from the current AngleUnit to degrees. */
        static float AngleUnitsToDegrees(float units);
        /** Convert from degrees to the current AngleUnit. */
        static float DegreesToAngleUnits(float degrees);
        
        /** Checks whether a given point is inside a triangle, in a
         2-dimensional (Cartesian) space.
         @remarks
         The vertices of the triangle must be given in either
         trigonometrical (anticlockwise) or inverse trigonometrical
         (clockwise) order.
         @param p
         The point.
         @param a
         The triangle's first vertex.
         @param b
         The triangle's second vertex.
         @param c
         The triangle's third vertex.
         @return
         If the point resides in the triangle, <b>true</b> is
         returned.
         @par
         If the point is outside the triangle, <b>false</b> is
         returned.
         */
        static bool pointInTri2D(const Vector2& p, const Vector2& a,
                                 const Vector2& b, const Vector2& c);
        
        /** Checks whether a given 3D point is inside a triangle.
         @remarks
         The vertices of the triangle must be given in either
         trigonometrical (anticlockwise) or inverse trigonometrical
         (clockwise) order, and the point must be guaranteed to be in the
         same plane as the triangle
         @param p
         p The point.
         @param a
         The triangle's first vertex.
         @param b
         The triangle's second vertex.
         @param c
         The triangle's third vertex.
         @param normal
         The triangle plane's normal (passed in rather than calculated
         on demand since the caller may already have it)
         @return
         If the point resides in the triangle, <b>true</b> is
         returned.
         @par
         If the point is outside the triangle, <b>false</b> is
         returned.
         */
        static bool pointInTri3D(const Vector3& p, const Vector3& a,
                                 const Vector3& b, const Vector3& c, const Vector3& normal);
        //                    /** Ray / plane intersection, returns boolean result and distance. */
        //                    static std::pair<bool, Real> intersects(const Ray& ray, const Plane& plane);
        //
        //                    /** Ray / sphere intersection, returns boolean result and distance. */
        //                    static std::pair<bool, Real> intersects(const Ray& ray, const Sphere& sphere,
        //                                                            bool discardInside = true);
        //
        //                    /** Ray / box intersection, returns boolean result and distance. */
        //                    static std::pair<bool, Real> intersects(const Ray& ray, const AxisAlignedBox& box);
        //
        //                    /** Ray / box intersection, returns boolean result and two intersection distance.
        //                     @param ray
        //                     The ray.
        //                     @param box
        //                     The box.
        //                     @param d1
        //                     A real pointer to retrieve the near intersection distance
        //                     from the ray origin, maybe <b>null</b> which means don't care
        //                     about the near intersection distance.
        //                     @param d2
        //                     A real pointer to retrieve the far intersection distance
        //                     from the ray origin, maybe <b>null</b> which means don't care
        //                     about the far intersection distance.
        //                     @return
        //                     If the ray is intersects the box, <b>true</b> is returned, and
        //                     the near intersection distance is return by <i>d1</i>, the
        //                     far intersection distance is return by <i>d2</i>. Guarantee
        //                     <b>0</b> <= <i>d1</i> <= <i>d2</i>.
        //                     @par
        //                     If the ray isn't intersects the box, <b>false</b> is returned, and
        //                     <i>d1</i> and <i>d2</i> is unmodified.
        //                     */
        //                    static bool intersects(const Ray& ray, const AxisAlignedBox& box,
        //                                           Real* d1, Real* d2);
        //
        //                    /** Ray / triangle intersection, returns boolean result and distance.
        //                     @param ray
        //                     The ray.
        //                     @param a
        //                     The triangle's first vertex.
        //                     @param b
        //                     The triangle's second vertex.
        //                     @param c
        //                     The triangle's third vertex.
        //                     @param normal
        //                     The triangle plane's normal (passed in rather than calculated
        //                     on demand since the caller may already have it), doesn't need
        //                     normalised since we don't care.
        //                     @param positiveSide
        //                     Intersect with "positive side" of the triangle
        //                     @param negativeSide
        //                     Intersect with "negative side" of the triangle
        //                     @return
        //                     If the ray is intersects the triangle, a pair of <b>true</b> and the
        //                     distance between intersection point and ray origin returned.
        //                     @par
        //                     If the ray isn't intersects the triangle, a pair of <b>false</b> and
        //                     <b>0</b> returned.
        //                     */
        //                    static std::pair<bool, Real> intersects(const Ray& ray, const Vector3& a,
        //                                                            const Vector3& b, const Vector3& c, const Vector3& normal,
        //                                                            bool positiveSide = true, bool negativeSide = true);
        //
        //                    /** Ray / triangle intersection, returns boolean result and distance.
        //                     @param ray
        //                     The ray.
        //                     @param a
        //                     The triangle's first vertex.
        //                     @param b
        //                     The triangle's second vertex.
        //                     @param c
        //                     The triangle's third vertex.
        //                     @param positiveSide
        //                     Intersect with "positive side" of the triangle
        //                     @param negativeSide
        //                     Intersect with "negative side" of the triangle
        //                     @return
        //                     If the ray is intersects the triangle, a pair of <b>true</b> and the
        //                     distance between intersection point and ray origin returned.
        //                     @par
        //                     If the ray isn't intersects the triangle, a pair of <b>false</b> and
        //                     <b>0</b> returned.
        //                     */
        //                    static std::pair<bool, Real> intersects(const Ray& ray, const Vector3& a,
        //                                                            const Vector3& b, const Vector3& c,
        //                                                            bool positiveSide = true, bool negativeSide = true);
        //
        //                    /** Sphere / box intersection test. */
        //                    static bool intersects(const Sphere& sphere, const AxisAlignedBox& box);
        //
        //                    /** Plane / box intersection test. */
        //                    static bool intersects(const Plane& plane, const AxisAlignedBox& box);
        //
        //                    /** Ray / convex plane list intersection test.
        //                     @param ray The ray to test with
        //                     @param planeList List of planes which form a convex volume
        //                     @param normalIsOutside Does the normal point outside the volume
        //                     */
        //                    static std::pair<bool, Real> intersects(
        //                                                            const Ray& ray, const vector<Plane>::type& planeList,
        //                                                            bool normalIsOutside);
        //                    /** Ray / convex plane list intersection test.
        //                     @param ray The ray to test with
        //                     @param planeList List of planes which form a convex volume
        //                     @param normalIsOutside Does the normal point outside the volume
        //                     */
        //                    static std::pair<bool, Real> intersects(
        //                                                            const Ray& ray, const list<Plane>::type& planeList,
        //                                                            bool normalIsOutside);
        //
        //                    /** Sphere / plane intersection test.
        //                     @remarks NB just do a plane.getDistance(sphere.getCenter()) for more detail!
        //                     */
        //                    static bool intersects(const Sphere& sphere, const Plane& plane);
        
        /** Compare 2 reals, using tolerance for inaccuracies.
         */
        static bool RealEqual(float a, float b,
                              float tolerance = std::numeric_limits<float>::epsilon());
        
        /** Calculates the tangent space vector for a given set of positions / texture coords. */
        static Vector3 calculateTangentSpaceVector(const Vector3& position1, const Vector3& position2, const Vector3& position3,
                                                   float u1, float v1, float u2, float v2, float u3, float v3);
        
        /** Build a reflection matrix for the passed in plane. */
        static Matrix4 buildReflectionMatrix(const Plane& p);
        /** Calculate a face normal, including the w component which is the offset from the origin. */
        static Vector4 calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        /** Calculate a face normal, no w-information. */
        static Vector3 calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        /** Calculate a face normal without normalize, including the w component which is the offset from the origin. */
        static Vector4 calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        /** Calculate a face normal without normalize, no w-information. */
        static Vector3 calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        
        /** Generates a value based on the Gaussian (normal) distribution function
         with the given offset and scale parameters.
         */
        static float gaussianDistribution(float x, float offset = 0.0f, float scale = 1.0f);
        
        /** Clamp a value within an inclusive range. */
        template <typename T>
        static T Clamp(T val, T minval, T maxval)
        {
            assert (minval <= maxval && "Invalid clamp range");
            return std::max(std::min(val, maxval), minval);
        }
        
        static Matrix4 makeViewMatrix(const Vector3& position, const Quaternion& orientation,
                                      const Matrix4* reflectMatrix = 0);
        
        static const float POS_INFINITY;
        static const float NEG_INFINITY;
        static const float ONE_PI;
        static const float TWO_PI;
        static const float HALF_PI;
        static const float fDeg2Rad;
        static const float fRad2Deg;
        
    };
    
    // these functions must be defined down here, because they rely on the
    // angle unit conversion functions in class Math:
    
    inline float Radian::valueDegrees() const
    {
        return Math::RadiansToDegrees ( _rad );
    }
    
    inline float Radian::valueAngleUnits() const
    {
        return Math::RadiansToAngleUnits ( _rad );
    }
    
    inline float Degree::valueRadians() const
    {
        return Math::DegreesToRadians ( _deg );
    }
    
    inline float Degree::valueAngleUnits() const
    {
        return Math::DegreesToAngleUnits ( _deg );
    }
    
    inline Angle::operator Radian() const
    {
        return Radian(Math::AngleUnitsToRadians(_angle));
    }
    
    inline Angle::operator Degree() const
    {
        return Degree(Math::AngleUnitsToDegrees(_angle));
    }
    
    inline Radian operator * ( float a, const Radian& b )
    {
        return Radian ( a * b.valueRadians() );
    }
    
    inline Radian operator / ( float a, const Radian& b )
    {
        return Radian ( a / b.valueRadians() );
    }
    
    inline Degree operator * ( float a, const Degree& b )
    {
        return Degree ( a * b.valueDegrees() );
    }
    
    inline Degree operator / ( float a, const Degree& b )
    {
        return Degree ( a / b.valueDegrees() );
    }
}
