#pragma once

#include "zay_vector3.h"

namespace ZAY
{
    class Matrix3
    {
    public:
        inline Matrix3 ()
        {}

        inline explicit Matrix3 (const float arr[3][3])
        {
            memcpy(m,arr,9*sizeof(float));
        }

        inline Matrix3 (const Matrix3& rkMatrix)
        {
            memcpy(m,rkMatrix.m,9*sizeof(float));
        }

        Matrix3 (float fEntry00, float fEntry01, float fEntry02,
                 float fEntry10, float fEntry11, float fEntry12,
                 float fEntry20, float fEntry21, float fEntry22)
        {
            m[0][0] = fEntry00;
            m[0][1] = fEntry01;
            m[0][2] = fEntry02;
            m[1][0] = fEntry10;
            m[1][1] = fEntry11;
            m[1][2] = fEntry12;
            m[2][0] = fEntry20;
            m[2][1] = fEntry21;
            m[2][2] = fEntry22;
        }

        inline void swap(Matrix3& other)
        {
            std::swap(m[0][0], other.m[0][0]);
            std::swap(m[0][1], other.m[0][1]);
            std::swap(m[0][2], other.m[0][2]);
            std::swap(m[1][0], other.m[1][0]);
            std::swap(m[1][1], other.m[1][1]);
            std::swap(m[1][2], other.m[1][2]);
            std::swap(m[2][0], other.m[2][0]);
            std::swap(m[2][1], other.m[2][1]);
            std::swap(m[2][2], other.m[2][2]);
        }

        inline float* operator[] (size_t iRow) const
        {
            return const_cast<float*>(m[iRow]);
        }
        
        Vector3 GetColumn (size_t iCol) const;
        
        void SetColumn(size_t iCol, const Vector3& vec);
        
        void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

        inline Matrix3& operator= (const Matrix3& rkMatrix)
        {
            memcpy(m,rkMatrix.m,9*sizeof(float));
            return *this;
        }

        bool operator== (const Matrix3& rkMatrix) const;

        inline bool operator!= (const Matrix3& rkMatrix) const
        {
            return !operator==(rkMatrix);
        }

        Matrix3 operator+ (const Matrix3& rkMatrix) const;
        Matrix3 operator- (const Matrix3& rkMatrix) const;
        Matrix3 operator* (const Matrix3& rkMatrix) const;
        Matrix3 operator- () const;
        Vector3 operator* (const Vector3& rkVector) const;
        friend Vector3 operator* (const Vector3& rkVector, const Matrix3& rkMatrix);
        Matrix3 operator* (float fScalar) const;
        friend Matrix3 operator* (float fScalar, const Matrix3& rkMatrix);
        Matrix3 Transpose () const;
        bool Inverse (Matrix3& rkInverse, float fTolerance = 1e-06f) const;
        Matrix3 Inverse (float fTolerance = 1e-06f) const;
        float Determinant () const;
        void SingularValueDecomposition (Matrix3& rkL, Vector3& rkS, Matrix3& rkR) const;
        void SingularValueComposition (const Matrix3& rkL, const Vector3& rkS, const Matrix3& rkR);
        void Orthonormalize ();
        void QDUDecomposition (Matrix3& rkQ, Vector3& rkD, Vector3& rkU) const;
        float SpectralNorm () const;
        void ToAngleAxis (Vector3& rkAxis, Radian& rfAngle) const;
        inline void ToAngleAxis (Vector3& rkAxis, Degree& rfAngle) const {
            Radian r;
            ToAngleAxis ( rkAxis, r );
            rfAngle = r;
        }
        void FromAngleAxis (const Vector3& rkAxis, const Radian& fRadians);

        bool ToEulerAnglesXYZ (Radian& rfYAngle, Radian& rfPAngle, Radian& rfRAngle) const;
        bool ToEulerAnglesXZY (Radian& rfYAngle, Radian& rfPAngle, Radian& rfRAngle) const;
        bool ToEulerAnglesYXZ (Radian& rfYAngle, Radian& rfPAngle, Radian& rfRAngle) const;
        bool ToEulerAnglesYZX (Radian& rfYAngle, Radian& rfPAngle, Radian& rfRAngle) const;
        bool ToEulerAnglesZXY (Radian& rfYAngle, Radian& rfPAngle, Radian& rfRAngle) const;
        bool ToEulerAnglesZYX (Radian& rfYAngle, Radian& rfPAngle, Radian& rfRAngle) const;
        void FromEulerAnglesXYZ (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesXZY (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesYXZ (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesYZX (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesZXY (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesZYX (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);

        void EigenSolveSymmetric (float afEigenvalue[3],
            Vector3 akEigenvector[3]) const;

        static void TensorProduct (const Vector3& rkU, const Vector3& rkV,
            Matrix3& rkProduct);

        inline bool hasScale() const
        {
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

        inline friend std::ostream& operator << ( std::ostream& o, const Matrix3& mat )
        {
            o << "Matrix3(" << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << ", " 
                            << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << ", " 
                            << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << ")";
            return o;
        }

        static const float EPSILON;
        static const Matrix3 ZERO;
        static const Matrix3 IDENTITY;

    protected:
        // support for eigensolver
        void Tridiagonal (float afDiag[3], float afSubDiag[3]);
        bool QLAlgorithm (float afDiag[3], float afSubDiag[3]);

        // support for singular value decomposition
        static const float msSvdEpsilon;
        static const unsigned int msSvdMaxIterations;
        static void Bidiagonalize (Matrix3& kA, Matrix3& kL,
            Matrix3& kR);
        static void GolubKahanStep (Matrix3& kA, Matrix3& kL,
            Matrix3& kR);

        // support for spectral norm
        static float MaxCubicRoot (float afCoeff[3]);

        float m[3][3];

        // for faster access
        friend class Matrix4;
    };
}
