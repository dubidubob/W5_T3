#pragma once
#include "pch.h"

#if SIMD_LEVEL >= 1
/* 16바이트 정렬 반드시 보장!*/
static void AddArrays_ref
	(int count,
	float* results,
	const float* dataA,
	const float* dataB)
{
	/* Three array's size is same, and they are 4*n */
	assert(count % 4 == 0);

	for (int i = 0; i < count; i+=4)
	{
		__m128 a = _mm_load_ps(&dataA[i]);
		__m128 b = _mm_load_ps(&dataB[i]);
		__m128 r = _mm_add_ps(a,b);
		_mm_store_ps(&results[i], r);
	}
}

static void DotsArrays_ref
(int count /* float 개수 */,
	float r[],
	const float dataA[],
	const float dataB[])
{
	/* Three array's size is same, and they are 4*n */
	assert(count % 4 == 0);
	for (int i = 0; i + 3 < count; i += 4) {
		__m128 A0 = _mm_loadu_ps(&dataA[(i + 0) * 4]);
		__m128 A1 = _mm_loadu_ps(&dataA[(i + 1) * 4]);
		__m128 A2 = _mm_loadu_ps(&dataA[(i + 2) * 4]);
		__m128 A3 = _mm_loadu_ps(&dataA[(i + 3) * 4]);

		__m128 B0 = _mm_loadu_ps(&dataB[(i + 0) * 4]);
		__m128 B1 = _mm_loadu_ps(&dataB[(i + 1) * 4]);
		__m128 B2 = _mm_loadu_ps(&dataB[(i + 2) * 4]);
		__m128 B3 = _mm_loadu_ps(&dataB[(i + 3) * 4]);

		// 4x4 transpose: (A0..A3) → (Ax, Ay, Az, Aw)
		__m128 Ax = A0, Ay = A1, Az = A2, Aw = A3;
		_MM_TRANSPOSE4_PS(Ax, Ay, Az, Aw);
		__m128 Bx = B0, By = B1, Bz = B2, Bw = B3;
		_MM_TRANSPOSE4_PS(Bx, By, Bz, Bw);

		__m128 R = _mm_mul_ps(Ax, Bx);
		R = _mm_add_ps(R, _mm_mul_ps(Ay, By));
		R = _mm_add_ps(R, _mm_mul_ps(Az, Bz));
		R = _mm_add_ps(R, _mm_mul_ps(Aw, Bw)); // FMA면 fmadd로 체인

		_mm_storeu_ps(&r[i], R); // r[i+0..i+3] = (a·b) 4개
	}
}

union Mat44
{
	float c[4][4]; // elements
	__m128 row[4]; // rows;
};

__m128 MulVecMat(const __m128& v, const Mat44& M)
{
	__m128 vX = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0,0,0,0));
	__m128 vY = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,1,1,1));
	__m128 vZ = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,2,2,2));
	__m128 vW = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3,3,3,3));

	__m128 r = _mm_mul_ps(vX, M.row[0]);
	r = _mm_add_ps(r, _mm_mul_ps(vY, M.row[1]));
	r = _mm_add_ps(r, _mm_mul_ps(vZ, M.row[2]));
	r = _mm_add_ps(r, _mm_mul_ps(vW, M.row[3]));
	return r;
}

void MulMatMat(Mat44& R, const Mat44& A, const Mat44& B)
{
	R.row[0] = MulVecMat(A.row[0], B);
	R.row[1] = MulVecMat(A.row[1], B);
	R.row[2] = MulVecMat(A.row[2], B);
	R.row[3] = MulVecMat(A.row[3], B);
}
#endif
