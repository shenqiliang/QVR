//
//  Matrix.hpp
//  
//
//  Created by 谌启亮 on 2017/4/6.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#ifndef Matrix_hpp
#define Matrix_hpp

#include <stdio.h>
#include <math.h>

#if defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

namespace QVR {
    
    union _Vec3
    {
        struct { float x, y, z; };
        struct { float r, g, b; };
        struct { float s, t, p; };
        float v[3];
    };
    typedef union _Vec3 Vec3;
    
    union _Mat4
    {
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
        float m[16];
    } __attribute__((aligned(16)));
    typedef union _Mat4 Mat4;
    
    
    static inline Vec3 Vec3Make(float x, float y, float z)
    {
        Vec3 v = { x, y, z };
        return v;
    }
    
    static inline Vec3 Vec3Normalize(Vec3 vector)
    {
        float length = sqrt(vector.v[0] * vector.v[0] + vector.v[1] * vector.v[1] + vector.v[2] * vector.v[2]);
        float scale = 1.0f / length;
        Vec3 v = { vector.v[0] * scale, vector.v[1] * scale, vector.v[2] * scale };
        return v;
    }
    
    static inline Mat4 Mat4MakeRotation(float radians, float x, float y, float z)
    {
        Vec3 v = Vec3Normalize(Vec3Make(x, y, z));
        float cos = cosf(radians);
        float cosp = 1.0f - cos;
        float sin = sinf(radians);
        
        Mat4 m = { cos + cosp * v.v[0] * v.v[0],
            cosp * v.v[0] * v.v[1] + v.v[2] * sin,
            cosp * v.v[0] * v.v[2] - v.v[1] * sin,
            0.0f,
            cosp * v.v[0] * v.v[1] - v.v[2] * sin,
            cos + cosp * v.v[1] * v.v[1],
            cosp * v.v[1] * v.v[2] + v.v[0] * sin,
            0.0f,
            cosp * v.v[0] * v.v[2] + v.v[1] * sin,
            cosp * v.v[1] * v.v[2] - v.v[0] * sin,
            cos + cosp * v.v[2] * v.v[2],
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f };
        
        return m;
    }
    
    static inline Mat4 Mat4Multiply(Mat4 matrixLeft, Mat4 matrixRight)
    {
#if defined(__ARM_NEON__)
        float32x4x4_t iMatrixLeft = *(float32x4x4_t *)&matrixLeft;
        float32x4x4_t iMatrixRight = *(float32x4x4_t *)&matrixRight;
        float32x4x4_t m;
        
        m.val[0] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[0], 0));
        m.val[1] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[1], 0));
        m.val[2] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[2], 0));
        m.val[3] = vmulq_n_f32(iMatrixLeft.val[0], vgetq_lane_f32(iMatrixRight.val[3], 0));
        
        m.val[0] = vmlaq_n_f32(m.val[0], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[0], 1));
        m.val[1] = vmlaq_n_f32(m.val[1], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[1], 1));
        m.val[2] = vmlaq_n_f32(m.val[2], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[2], 1));
        m.val[3] = vmlaq_n_f32(m.val[3], iMatrixLeft.val[1], vgetq_lane_f32(iMatrixRight.val[3], 1));
        
        m.val[0] = vmlaq_n_f32(m.val[0], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[0], 2));
        m.val[1] = vmlaq_n_f32(m.val[1], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[1], 2));
        m.val[2] = vmlaq_n_f32(m.val[2], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[2], 2));
        m.val[3] = vmlaq_n_f32(m.val[3], iMatrixLeft.val[2], vgetq_lane_f32(iMatrixRight.val[3], 2));
        
        m.val[0] = vmlaq_n_f32(m.val[0], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[0], 3));
        m.val[1] = vmlaq_n_f32(m.val[1], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[1], 3));
        m.val[2] = vmlaq_n_f32(m.val[2], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[2], 3));
        m.val[3] = vmlaq_n_f32(m.val[3], iMatrixLeft.val[3], vgetq_lane_f32(iMatrixRight.val[3], 3));
        
        return *(Mat4 *)&m;
#elif defined(GLK_SSE3_INTRINSICS)
        
        const __m128 l0 = _mm_load_ps(&matrixLeft.m[0]);
        const __m128 l1 = _mm_load_ps(&matrixLeft.m[4]);
        const __m128 l2 = _mm_load_ps(&matrixLeft.m[8]);
        const __m128 l3 = _mm_load_ps(&matrixLeft.m[12]);
        
        const __m128 r0 = _mm_load_ps(&matrixRight.m[0]);
        const __m128 r1 = _mm_load_ps(&matrixRight.m[4]);
        const __m128 r2 = _mm_load_ps(&matrixRight.m[8]);
        const __m128 r3 = _mm_load_ps(&matrixRight.m[12]);
        
        const __m128 m0 = l0 * _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(0, 0, 0, 0))
        + l1 * _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(1, 1, 1, 1))
        + l2 * _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(2, 2, 2, 2))
        + l3 * _mm_shuffle_ps(r0, r0, _MM_SHUFFLE(3, 3, 3, 3));
        
        const __m128 m1 = l0 * _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(0, 0, 0, 0))
        + l1 * _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(1, 1, 1, 1))
        + l2 * _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 2, 2, 2))
        + l3 * _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(3, 3, 3, 3));
        
        const __m128 m2 = l0 * _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(0, 0, 0, 0))
        + l1 * _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(1, 1, 1, 1))
        + l2 * _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(2, 2, 2, 2))
        + l3 * _mm_shuffle_ps(r2, r2, _MM_SHUFFLE(3, 3, 3, 3));
        
        const __m128 m3 = l0 * _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(0, 0, 0, 0))
        + l1 * _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(1, 1, 1, 1))
        + l2 * _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(2, 2, 2, 2))
        + l3 * _mm_shuffle_ps(r3, r3, _MM_SHUFFLE(3, 3, 3, 3));
        
        Mat4 m;
        _mm_store_ps(&m.m[0], m0);
        _mm_store_ps(&m.m[4], m1);
        _mm_store_ps(&m.m[8], m2);
        _mm_store_ps(&m.m[12], m3);
        return m;
        
#else
        Mat4 m;
        
        m.m[0]  = matrixLeft.m[0] * matrixRight.m[0]  + matrixLeft.m[4] * matrixRight.m[1]  + matrixLeft.m[8] * matrixRight.m[2]   + matrixLeft.m[12] * matrixRight.m[3];
        m.m[4]  = matrixLeft.m[0] * matrixRight.m[4]  + matrixLeft.m[4] * matrixRight.m[5]  + matrixLeft.m[8] * matrixRight.m[6]   + matrixLeft.m[12] * matrixRight.m[7];
        m.m[8]  = matrixLeft.m[0] * matrixRight.m[8]  + matrixLeft.m[4] * matrixRight.m[9]  + matrixLeft.m[8] * matrixRight.m[10]  + matrixLeft.m[12] * matrixRight.m[11];
        m.m[12] = matrixLeft.m[0] * matrixRight.m[12] + matrixLeft.m[4] * matrixRight.m[13] + matrixLeft.m[8] * matrixRight.m[14]  + matrixLeft.m[12] * matrixRight.m[15];
        
        m.m[1]  = matrixLeft.m[1] * matrixRight.m[0]  + matrixLeft.m[5] * matrixRight.m[1]  + matrixLeft.m[9] * matrixRight.m[2]   + matrixLeft.m[13] * matrixRight.m[3];
        m.m[5]  = matrixLeft.m[1] * matrixRight.m[4]  + matrixLeft.m[5] * matrixRight.m[5]  + matrixLeft.m[9] * matrixRight.m[6]   + matrixLeft.m[13] * matrixRight.m[7];
        m.m[9]  = matrixLeft.m[1] * matrixRight.m[8]  + matrixLeft.m[5] * matrixRight.m[9]  + matrixLeft.m[9] * matrixRight.m[10]  + matrixLeft.m[13] * matrixRight.m[11];
        m.m[13] = matrixLeft.m[1] * matrixRight.m[12] + matrixLeft.m[5] * matrixRight.m[13] + matrixLeft.m[9] * matrixRight.m[14]  + matrixLeft.m[13] * matrixRight.m[15];
        
        m.m[2]  = matrixLeft.m[2] * matrixRight.m[0]  + matrixLeft.m[6] * matrixRight.m[1]  + matrixLeft.m[10] * matrixRight.m[2]  + matrixLeft.m[14] * matrixRight.m[3];
        m.m[6]  = matrixLeft.m[2] * matrixRight.m[4]  + matrixLeft.m[6] * matrixRight.m[5]  + matrixLeft.m[10] * matrixRight.m[6]  + matrixLeft.m[14] * matrixRight.m[7];
        m.m[10] = matrixLeft.m[2] * matrixRight.m[8]  + matrixLeft.m[6] * matrixRight.m[9]  + matrixLeft.m[10] * matrixRight.m[10] + matrixLeft.m[14] * matrixRight.m[11];
        m.m[14] = matrixLeft.m[2] * matrixRight.m[12] + matrixLeft.m[6] * matrixRight.m[13] + matrixLeft.m[10] * matrixRight.m[14] + matrixLeft.m[14] * matrixRight.m[15];
        
        m.m[3]  = matrixLeft.m[3] * matrixRight.m[0]  + matrixLeft.m[7] * matrixRight.m[1]  + matrixLeft.m[11] * matrixRight.m[2]  + matrixLeft.m[15] * matrixRight.m[3];
        m.m[7]  = matrixLeft.m[3] * matrixRight.m[4]  + matrixLeft.m[7] * matrixRight.m[5]  + matrixLeft.m[11] * matrixRight.m[6]  + matrixLeft.m[15] * matrixRight.m[7];
        m.m[11] = matrixLeft.m[3] * matrixRight.m[8]  + matrixLeft.m[7] * matrixRight.m[9]  + matrixLeft.m[11] * matrixRight.m[10] + matrixLeft.m[15] * matrixRight.m[11];
        m.m[15] = matrixLeft.m[3] * matrixRight.m[12] + matrixLeft.m[7] * matrixRight.m[13] + matrixLeft.m[11] * matrixRight.m[14] + matrixLeft.m[15] * matrixRight.m[15];
        
        return m;
#endif
    }
    
    static inline Mat4 Mat4MakePerspective(float fovyRadians, float aspect, float nearZ, float farZ)
    {
        float cotan = 1.0f / tanf(fovyRadians / 2.0f);
        
        Mat4 m = { cotan / aspect, 0.0f, 0.0f, 0.0f,
            0.0f, cotan, 0.0f, 0.0f,
            0.0f, 0.0f, (farZ + nearZ) / (nearZ - farZ), -1.0f,
            0.0f, 0.0f, (2.0f * farZ * nearZ) / (nearZ - farZ), 0.0f };
        
        return m;
    }
    
    static inline Mat4 Mat4MakeOrtho(float left, float right,
                                              float bottom, float top,
                                              float nearZ, float farZ)
    {
        float ral = right + left;
        float rsl = right - left;
        float tab = top + bottom;
        float tsb = top - bottom;
        float fan = farZ + nearZ;
        float fsn = farZ - nearZ;
        
        Mat4 m = { 2.0f / rsl, 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / tsb, 0.0f, 0.0f,
            0.0f, 0.0f, -2.0f / fsn, 0.0f,
            -ral / rsl, -tab / tsb, -fan / fsn, 1.0f };
        
        return m;
    }

    
    static inline Mat4 Mat4MakeXRotation(float radians)
    {
        float cos = cosf(radians);
        float sin = sinf(radians);
        
        Mat4 m = { 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cos, sin, 0.0f,
            0.0f, -sin, cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
        
        return m;
    }
    
    static inline Mat4 Mat4MakeYRotation(float radians)
    {
        float cos = cosf(radians);
        float sin = sinf(radians);
        
        Mat4 m = { cos, 0.0f, -sin, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            sin, 0.0f, cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
        
        return m;
    }
    
    static inline Mat4 Mat4MakeZRotation(float radians)
    {
        float cos = cosf(radians);
        float sin = sinf(radians);
        
        Mat4 m = { cos, sin, 0.0f, 0.0f,
            -sin, cos, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
        
        return m;
    }
    
    static inline Mat4 Mat4RotateX(Mat4 matrix, float radians)
    {
        Mat4 rm = Mat4MakeXRotation(radians);
        return Mat4Multiply(matrix, rm);
    }
    
    
    static inline Mat4 Mat4RotateY(Mat4 matrix, float radians)
    {
        Mat4 rm = Mat4MakeYRotation(radians);
        return Mat4Multiply(matrix, rm);
    }
    
    static inline Mat4 Mat4RotateZ(Mat4 matrix, float radians)
    {
        Mat4 rm = Mat4MakeZRotation(radians);
        return Mat4Multiply(matrix, rm);
    }
    
    static inline Mat4 Mat4Rotate(Mat4 matrix, float radians, float x, float y, float z)
    {
        Mat4 rm = Mat4MakeRotation(radians, x, y, z);
        return Mat4Multiply(matrix, rm);
    }
    
    static inline Mat4 Mat4Scale(Mat4 matrix, float sx, float sy, float sz)
    {
#if defined(__ARM_NEON__)
        float32x4x4_t iMatrix = *(float32x4x4_t *)&matrix;
        float32x4x4_t m;
        
        m.val[0] = vmulq_n_f32(iMatrix.val[0], (float32_t)sx);
        m.val[1] = vmulq_n_f32(iMatrix.val[1], (float32_t)sy);
        m.val[2] = vmulq_n_f32(iMatrix.val[2], (float32_t)sz);
        m.val[3] = iMatrix.val[3];
        
        return *(Mat4 *)&m;
#elif defined(GLK_SSE3_INTRINSICS)
        Mat4 m;
        
        _mm_store_ps(&m.m[0],  _mm_load_ps(&matrix.m[0])  * _mm_load1_ps(&sx));
        _mm_store_ps(&m.m[4],  _mm_load_ps(&matrix.m[4])  * _mm_load1_ps(&sy));
        _mm_store_ps(&m.m[8],  _mm_load_ps(&matrix.m[8])  * _mm_load1_ps(&sz));
        _mm_store_ps(&m.m[12], _mm_load_ps(&matrix.m[12]));
        
        return m;
#else
        Mat4 m = { matrix.m[0] * sx, matrix.m[1] * sx, matrix.m[2] * sx, matrix.m[3] * sx,
            matrix.m[4] * sy, matrix.m[5] * sy, matrix.m[6] * sy, matrix.m[7] * sy,
            matrix.m[8] * sz, matrix.m[9] * sz, matrix.m[10] * sz, matrix.m[11] * sz,
            matrix.m[12], matrix.m[13], matrix.m[14], matrix.m[15] };
        return m;
#endif
    }

}


#endif /* Matrix_hpp */
