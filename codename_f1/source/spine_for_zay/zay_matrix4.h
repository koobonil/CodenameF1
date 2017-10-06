#pragma once

#include "zay_types.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_matrix3.h"
#include "zay_vector4.h"
#include "zay_plane.h"

namespace ZAY
{
    class Matrix4
    {
    protected:
        union {
            float m[4][4];
            float _m[16];
        };
    public:
        inline Matrix4()
        {
        }
        
        inline Matrix4(float m00, float m01, float m02, float m03,
                       float m10, float m11, float m12, float m13,
                       float m20, float m21, float m22, float m23,
                       float m30, float m31, float m32, float m33 )
        {
            m[0][0] = m00;
            m[0][1] = m01;
            m[0][2] = m02;
            m[0][3] = m03;
            m[1][0] = m10;
            m[1][1] = m11;
            m[1][2] = m12;
            m[1][3] = m13;
            m[2][0] = m20;
            m[2][1] = m21;
            m[2][2] = m22;
            m[2][3] = m23;
            m[3][0] = m30;
            m[3][1] = m31;
            m[3][2] = m32;
            m[3][3] = m33;
        }
        
        inline Matrix4(const Matrix3& m3x3)
        {
            operator=(IDENTITY);
            operator=(m3x3);
        }
        
        inline Matrix4(const Quaternion& rot)
        {
            Matrix3 m3x3;
            rot.ToRotationMatrix(m3x3);
            operator=(IDENTITY);
            operator=(m3x3);
        }
        
        inline const float (&_getRaws() const) [4][4]
        {
            return m;
        }
        
        
        inline void swap(Matrix4& other)
        {
            std::swap(m[0][0], other.m[0][0]);
            std::swap(m[0][1], other.m[0][1]);
            std::swap(m[0][2], other.m[0][2]);
            std::swap(m[0][3], other.m[0][3]);
            std::swap(m[1][0], other.m[1][0]);
            std::swap(m[1][1], other.m[1][1]);
            std::swap(m[1][2], other.m[1][2]);
            std::swap(m[1][3], other.m[1][3]);
            std::swap(m[2][0], other.m[2][0]);
            std::swap(m[2][1], other.m[2][1]);
            std::swap(m[2][2], other.m[2][2]);
            std::swap(m[2][3], other.m[2][3]);
            std::swap(m[3][0], other.m[3][0]);
            std::swap(m[3][1], other.m[3][1]);
            std::swap(m[3][2], other.m[3][2]);
            std::swap(m[3][3], other.m[3][3]);
        }
        
        inline float* operator [] ( size_t iRow )
        {
            assert( iRow < 4 );
            return m[iRow];
        }
        
        inline const float *operator [] ( size_t iRow ) const
        {
            assert( iRow < 4 );
            return m[iRow];
        }
        
        inline Matrix4 concatenate(const Matrix4 &m2) const
        {
            Matrix4 r;
            
            r.m[0][0] = m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0] + m[0][3] * m2.m[3][0];
            r.m[0][1] = m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1] + m[0][3] * m2.m[3][1];
            r.m[0][2] = m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2] + m[0][3] * m2.m[3][2];
            r.m[0][3] = m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3] * m2.m[3][3];
            
            r.m[1][0] = m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0] + m[1][3] * m2.m[3][0];
            r.m[1][1] = m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1] + m[1][3] * m2.m[3][1];
            r.m[1][2] = m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2] + m[1][3] * m2.m[3][2];
            r.m[1][3] = m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3] * m2.m[3][3];
            
            r.m[2][0] = m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0] + m[2][3] * m2.m[3][0];
            r.m[2][1] = m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1] + m[2][3] * m2.m[3][1];
            r.m[2][2] = m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2] + m[2][3] * m2.m[3][2];
            r.m[2][3] = m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3] * m2.m[3][3];
            
            r.m[3][0] = m[3][0] * m2.m[0][0] + m[3][1] * m2.m[1][0] + m[3][2] * m2.m[2][0] + m[3][3] * m2.m[3][0];
            r.m[3][1] = m[3][0] * m2.m[0][1] + m[3][1] * m2.m[1][1] + m[3][2] * m2.m[2][1] + m[3][3] * m2.m[3][1];
            r.m[3][2] = m[3][0] * m2.m[0][2] + m[3][1] * m2.m[1][2] + m[3][2] * m2.m[2][2] + m[3][3] * m2.m[3][2];
            r.m[3][3] = m[3][0] * m2.m[0][3] + m[3][1] * m2.m[1][3] + m[3][2] * m2.m[2][3] + m[3][3] * m2.m[3][3];
            
            return r;
        }

        inline Matrix4 operator * ( const Matrix4 &m2 ) const
        {
            return concatenate( m2 );
        }

        inline Vector3 operator * ( const Vector3 &v ) const
        {
            Vector3 r;

            float fInvW = 1.0f / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );

            r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * fInvW;
            r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * fInvW;
            r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * fInvW;

            return r;
        }

        inline Vector2 operator * ( const Vector2 &v ) const
        {
            Vector2 r;
            
            float fInvW = 1.0f / ( m[3][0] * v.x + m[3][1] * v.y + m[3][3] );
            
            r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][3] ) * fInvW;
            r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][3] ) * fInvW;
            
            return r;
        }

        inline Vector4 operator * (const Vector4& v) const
        {
            return Vector4(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
                           m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                           m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                           m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
                           );
        }
        
        inline Plane operator * (const Plane& p) const
        {
            Plane ret;
            Matrix4 invTrans = inverse().transpose();
            Vector4 v4( p.normal.x, p.normal.y, p.normal.z, p.d );
            v4 = invTrans * v4;
            ret.normal.x = v4.x;
            ret.normal.y = v4.y;
            ret.normal.z = v4.z;
            ret.d = v4.w / ret.normal.normalise();
            
            return ret;
        }
        
        inline Matrix4 operator + ( const Matrix4 &m2 ) const
        {
            Matrix4 r;
            
            r.m[0][0] = m[0][0] + m2.m[0][0];
            r.m[0][1] = m[0][1] + m2.m[0][1];
            r.m[0][2] = m[0][2] + m2.m[0][2];
            r.m[0][3] = m[0][3] + m2.m[0][3];
            
            r.m[1][0] = m[1][0] + m2.m[1][0];
            r.m[1][1] = m[1][1] + m2.m[1][1];
            r.m[1][2] = m[1][2] + m2.m[1][2];
            r.m[1][3] = m[1][3] + m2.m[1][3];
            
            r.m[2][0] = m[2][0] + m2.m[2][0];
            r.m[2][1] = m[2][1] + m2.m[2][1];
            r.m[2][2] = m[2][2] + m2.m[2][2];
            r.m[2][3] = m[2][3] + m2.m[2][3];
            
            r.m[3][0] = m[3][0] + m2.m[3][0];
            r.m[3][1] = m[3][1] + m2.m[3][1];
            r.m[3][2] = m[3][2] + m2.m[3][2];
            r.m[3][3] = m[3][3] + m2.m[3][3];
            
            return r;
        }
        
        inline Matrix4 operator - ( const Matrix4 &m2 ) const
        {
            Matrix4 r;
            r.m[0][0] = m[0][0] - m2.m[0][0];
            r.m[0][1] = m[0][1] - m2.m[0][1];
            r.m[0][2] = m[0][2] - m2.m[0][2];
            r.m[0][3] = m[0][3] - m2.m[0][3];
            
            r.m[1][0] = m[1][0] - m2.m[1][0];
            r.m[1][1] = m[1][1] - m2.m[1][1];
            r.m[1][2] = m[1][2] - m2.m[1][2];
            r.m[1][3] = m[1][3] - m2.m[1][3];
            
            r.m[2][0] = m[2][0] - m2.m[2][0];
            r.m[2][1] = m[2][1] - m2.m[2][1];
            r.m[2][2] = m[2][2] - m2.m[2][2];
            r.m[2][3] = m[2][3] - m2.m[2][3];
            
            r.m[3][0] = m[3][0] - m2.m[3][0];
            r.m[3][1] = m[3][1] - m2.m[3][1];
            r.m[3][2] = m[3][2] - m2.m[3][2];
            r.m[3][3] = m[3][3] - m2.m[3][3];
            
            return r;
        }
        
        inline bool operator == ( const Matrix4& m2 ) const
        {
            if(m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
               m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
               m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
               m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3] )
                return false;
            return true;
        }
        
        inline bool operator != ( const Matrix4& m2 ) const
        {
            if(m[0][0] != m2.m[0][0] || m[0][1] != m2.m[0][1] || m[0][2] != m2.m[0][2] || m[0][3] != m2.m[0][3] ||
               m[1][0] != m2.m[1][0] || m[1][1] != m2.m[1][1] || m[1][2] != m2.m[1][2] || m[1][3] != m2.m[1][3] ||
               m[2][0] != m2.m[2][0] || m[2][1] != m2.m[2][1] || m[2][2] != m2.m[2][2] || m[2][3] != m2.m[2][3] ||
               m[3][0] != m2.m[3][0] || m[3][1] != m2.m[3][1] || m[3][2] != m2.m[3][2] || m[3][3] != m2.m[3][3] )
                return true;
            return false;
        }
        
        inline void operator = ( const Matrix3& mat3 )
        {
            m[0][0] = mat3.m[0][0]; m[0][1] = mat3.m[0][1]; m[0][2] = mat3.m[0][2];
            m[1][0] = mat3.m[1][0]; m[1][1] = mat3.m[1][1]; m[1][2] = mat3.m[1][2];
            m[2][0] = mat3.m[2][0]; m[2][1] = mat3.m[2][1]; m[2][2] = mat3.m[2][2];
        }
        
        inline Matrix4 transpose(void) const
        {
            return Matrix4(m[0][0], m[1][0], m[2][0], m[3][0],
                           m[0][1], m[1][1], m[2][1], m[3][1],
                           m[0][2], m[1][2], m[2][2], m[3][2],
                           m[0][3], m[1][3], m[2][3], m[3][3]);
        }
        
        inline void setTrans( const Vector3& v )
        {
            m[0][3] = v.x;
            m[1][3] = v.y;
            m[2][3] = v.z;
        }
        
        inline Vector3 getTrans() const
        {
            return Vector3(m[0][3], m[1][3], m[2][3]);
        }
        
        inline void makeTrans( const Vector3& v )
        {
            m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = v.x;
            m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = v.y;
            m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = v.z;
            m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
        }
        
        inline void makeTrans( float tx, float ty, float tz )
        {
            m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = tx;
            m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = ty;
            m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = tz;
            m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
        }
        
        inline static Matrix4 getTrans( const Vector3& v )
        {
            Matrix4 r;
            
            r.m[0][0] = 1.0f; r.m[0][1] = 0.0f; r.m[0][2] = 0.0f; r.m[0][3] = v.x;
            r.m[1][0] = 0.0f; r.m[1][1] = 1.0f; r.m[1][2] = 0.0f; r.m[1][3] = v.y;
            r.m[2][0] = 0.0f; r.m[2][1] = 0.0f; r.m[2][2] = 1.0f; r.m[2][3] = v.z;
            r.m[3][0] = 0.0f; r.m[3][1] = 0.0f; r.m[3][2] = 0.0f; r.m[3][3] = 1.0f;
            
            return r;
        }
        
        inline static Matrix4 getTrans( float t_x, float t_y, float t_z )
        {
            Matrix4 r;
            
            r.m[0][0] = 1.0f; r.m[0][1] = 0.0f; r.m[0][2] = 0.0f; r.m[0][3] = t_x;
            r.m[1][0] = 0.0f; r.m[1][1] = 1.0f; r.m[1][2] = 0.0f; r.m[1][3] = t_y;
            r.m[2][0] = 0.0f; r.m[2][1] = 0.0f; r.m[2][2] = 1.0f; r.m[2][3] = t_z;
            r.m[3][0] = 0.0f; r.m[3][1] = 0.0f; r.m[3][2] = 0.0f; r.m[3][3] = 1.0f;
            
            return r;
        }
        
        inline void setScale( const Vector3& v )
        {
            m[0][0] = v.x;
            m[1][1] = v.y;
            m[2][2] = v.z;
        }
        
        inline static Matrix4 getScale( const Vector3& v )
        {
            Matrix4 r;
            r.m[0][0] = v.x;  r.m[0][1] = 0.0f; r.m[0][2] = 0.0f; r.m[0][3] = 0.0f;
            r.m[1][0] = 0.0f; r.m[1][1] = v.y;  r.m[1][2] = 0.0f; r.m[1][3] = 0.0f;
            r.m[2][0] = 0.0f; r.m[2][1] = 0.0f; r.m[2][2] = v.z;  r.m[2][3] = 0.0f;
            r.m[3][0] = 0.0f; r.m[3][1] = 0.0f; r.m[3][2] = 0.0f; r.m[3][3] = 1.0f;
            
            return r;
        }
        
        inline static Matrix4 getScale( float s_x, float s_y, float s_z )
        {
            Matrix4 r;
            r.m[0][0] = s_x;  r.m[0][1] = 0.0f; r.m[0][2] = 0.0f; r.m[0][3] = 0.0f;
            r.m[1][0] = 0.0f; r.m[1][1] = s_y;  r.m[1][2] = 0.0f; r.m[1][3] = 0.0f;
            r.m[2][0] = 0.0f; r.m[2][1] = 0.0f; r.m[2][2] = s_z;  r.m[2][3] = 0.0f;
            r.m[3][0] = 0.0f; r.m[3][1] = 0.0f; r.m[3][2] = 0.0f; r.m[3][3] = 1.0f;
            
            return r;
        }
        
        inline void extract3x3Matrix(Matrix3& m3x3) const
        {
            m3x3.m[0][0] = m[0][0];
            m3x3.m[0][1] = m[0][1];
            m3x3.m[0][2] = m[0][2];
            m3x3.m[1][0] = m[1][0];
            m3x3.m[1][1] = m[1][1];
            m3x3.m[1][2] = m[1][2];
            m3x3.m[2][0] = m[2][0];
            m3x3.m[2][1] = m[2][1];
            m3x3.m[2][2] = m[2][2];
            
        }
        
        inline bool hasScale() const
        {
            // check magnitude of column vectors (==local axes)
            float t = m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0];
            if (!Math::RealEqual(t, 1.0f, 1e-04f))
                return true;
            t = m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1];
            if (!Math::RealEqual(t, 1.0f, 1e-04f))
                return true;
            t = m[0][2] * m[0][2] + m[1][2] * m[1][2] + m[2][2] * m[2][2];
            if (!Math::RealEqual(t, 1.0f, 1e-04f))
                return true;

            return false;
        }
        
        inline bool hasNegativeScale() const
        {
            return determinant() < 0.0f;
        }
        
        inline Quaternion extractQuaternion() const
        {
            Matrix3 m3x3;
            extract3x3Matrix(m3x3);
            return Quaternion(m3x3);
        }
        
        static const Matrix4 ZERO;
        static const Matrix4 ZEROAFFINE;
        static const Matrix4 IDENTITY;
        static const Matrix4 CLIPSPACE2DTOIMAGESPACE;
        
        inline Matrix4 operator*(float scalar) const
        {
            return Matrix4(scalar*m[0][0], scalar*m[0][1], scalar*m[0][2], scalar*m[0][3],
                           scalar*m[1][0], scalar*m[1][1], scalar*m[1][2], scalar*m[1][3],
                           scalar*m[2][0], scalar*m[2][1], scalar*m[2][2], scalar*m[2][3],
                           scalar*m[3][0], scalar*m[3][1], scalar*m[3][2], scalar*m[3][3]);
        }
        
        inline friend std::ostream& operator << ( std::ostream& o, const Matrix4& mat )
        {
            o << "Matrix4(";
            for (size_t i = 0; i < 4; ++i)
            {
                o << " row" << (unsigned)i << "{";
                for(size_t j = 0; j < 4; ++j)
                {
                    o << mat[i][j] << " ";
                }
                o << "}";
            }
            o << ")";
            return o;
        }
        
        Matrix4 adjoint() const;
        float determinant() const;
        Matrix4 inverse() const;
        
        void makeTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation);

        void makeTransform(const Vector3& position, const Vector3& scale, float rotation, bool flipX, bool flipY);
        
        void makeInverseTransform(const Vector3& position, const Vector3& scale, const Quaternion& orientation);

        void makeInverseTransform(const Vector3& position, const Vector3& scale, float rotation, bool flipX, bool flipY);

        void decomposition(Vector3& position, Vector3& scale, Quaternion& orientation) const;
        
        inline bool isAffine(void) const
        {
            return m[3][0] == 0.0f && m[3][1] == 0.0f && m[3][2] == 0.0f && m[3][3] == 1.0f;
        }
        
        Matrix4 inverseAffine(void) const;

        inline Matrix4 concatenateAffine(const Matrix4 &m2) const
        {
            assert(isAffine() && m2.isAffine());
            
            return Matrix4(m[0][0] * m2.m[0][0] + m[0][1] * m2.m[1][0] + m[0][2] * m2.m[2][0],
                           m[0][0] * m2.m[0][1] + m[0][1] * m2.m[1][1] + m[0][2] * m2.m[2][1],
                           m[0][0] * m2.m[0][2] + m[0][1] * m2.m[1][2] + m[0][2] * m2.m[2][2],
                           m[0][0] * m2.m[0][3] + m[0][1] * m2.m[1][3] + m[0][2] * m2.m[2][3] + m[0][3],
                           
                           m[1][0] * m2.m[0][0] + m[1][1] * m2.m[1][0] + m[1][2] * m2.m[2][0],
                           m[1][0] * m2.m[0][1] + m[1][1] * m2.m[1][1] + m[1][2] * m2.m[2][1],
                           m[1][0] * m2.m[0][2] + m[1][1] * m2.m[1][2] + m[1][2] * m2.m[2][2],
                           m[1][0] * m2.m[0][3] + m[1][1] * m2.m[1][3] + m[1][2] * m2.m[2][3] + m[1][3],
                           
                           m[2][0] * m2.m[0][0] + m[2][1] * m2.m[1][0] + m[2][2] * m2.m[2][0],
                           m[2][0] * m2.m[0][1] + m[2][1] * m2.m[1][1] + m[2][2] * m2.m[2][1],
                           m[2][0] * m2.m[0][2] + m[2][1] * m2.m[1][2] + m[2][2] * m2.m[2][2],
                           m[2][0] * m2.m[0][3] + m[2][1] * m2.m[1][3] + m[2][2] * m2.m[2][3] + m[2][3],
                           
                           0.0f, 0.0f, 0.0f, 1.0f);
        }

        inline Vector3 transformAffine(const Vector3& v) const
        {
            assert(isAffine());
            
            return Vector3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
                           m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
                           m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
        }
        
        inline Vector3 transformVector(const Vector3& v) const
        {
            assert(isAffine());
            
            return Vector3(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                           m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                           m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
        }
        
        inline Vector4 transformAffine(const Vector4& v) const
        {
            assert(isAffine());
            
            return Vector4(m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
                           m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
                           m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
                           v.w);
        }
    };
    
    inline Vector4 operator * (const Vector4& v, const Matrix4& mat)
    {
        return Vector4(v.x*mat[0][0] + v.y*mat[1][0] + v.z*mat[2][0] + v.w*mat[3][0],
                       v.x*mat[0][1] + v.y*mat[1][1] + v.z*mat[2][1] + v.w*mat[3][1],
                       v.x*mat[0][2] + v.y*mat[1][2] + v.z*mat[2][2] + v.w*mat[3][2],
                       v.x*mat[0][3] + v.y*mat[1][3] + v.z*mat[2][3] + v.w*mat[3][3]);
    }
}
