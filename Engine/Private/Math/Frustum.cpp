#include "pch.h"
#include "Math/Frustum.h"
#include "Math/AABB.h"

static inline void NormalizePlane(FVector4& P)
{
#if SIMD_LEVEL >= 1
	assert((reinterpret_cast<uintptr_t>(&P) % 16) == 0);

	// v = [x y z w], len = sqrt(x^2+y^2+z^2)
	__m128 v = _mm_load_ps(&P.X);
	const __m128 zeroW = _mm_castsi128_ps(_mm_set_epi32(0, -1, -1, -1)); // w=0 만드는 용도(XYZ만)
	__m128 xyz = _mm_and_ps(v, zeroW);  // w=0

	// len2 = dot(xyz, xyz)
	__m128 mul = _mm_mul_ps(xyz, xyz);
	// 수평합(모든 SSE2 호환; w=0 가정)
	__m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 1, 0, 3));
	__m128 sums = _mm_add_ps(mul, shuf);
	shuf = _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(1, 0, 3, 2));
	sums = _mm_add_ps(sums, shuf);
	float len2 = _mm_cvtss_f32(sums);

	if (len2 > 0.0f)
	{
		// invLen = 1/sqrt(len2)
		__m128 invLen = _mm_rsqrt_ps(_mm_set_ss(len2));
		// 정밀 보정(원하면 NR 1회)
		// invLen = 0.5f * invLen * (3 - len2 * invLen * invLen);
		// 전체 성분에 동일 스칼라 곱
		__m128 s = _mm_shuffle_ps(invLen, invLen, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 out = _mm_mul_ps(v, s);
		_mm_store_ps(&P.X, out);
	}
#else
    float L = std::sqrtf(P.X * P.X + P.Y * P.Y + P.Z * P.Z);
    if (L > 0.0f)
    {
        float R = 1.0f / L;
        P.X *= R; P.Y *= R; P.Z *= R; P.W *= R;
    }
#endif
}

void ExtractFrustumPlanes(const FMatrix& View, const FMatrix& Projection, TStaticArray<FVector4, 6>& OutPlanes)
{
    FMatrix VP = View * Projection;

    FVector4 C0(VP.Data[0][0], VP.Data[1][0], VP.Data[2][0], VP.Data[3][0]);
    FVector4 C1(VP.Data[0][1], VP.Data[1][1], VP.Data[2][1], VP.Data[3][1]);
    FVector4 C2(VP.Data[0][2], VP.Data[1][2], VP.Data[2][2], VP.Data[3][2]);
    FVector4 C3(VP.Data[0][3], VP.Data[1][3], VP.Data[2][3], VP.Data[3][3]);

    OutPlanes[0] = C3 + C0;
    OutPlanes[1] = C3 - C0;
    OutPlanes[2] = C3 + C1;
    OutPlanes[3] = C3 - C1;
    OutPlanes[4] = C2;
    OutPlanes[5] = C3 - C2;

    for (auto& P : OutPlanes) NormalizePlane(P);
}

void ExtractFrustumPlanes(const FViewProjConstants& VP, TStaticArray<FVector4, 6>& OutPlanes)
{
    ExtractFrustumPlanes(VP.View, VP.Projection, OutPlanes);
}

bool TestAABBFrustum(const FAABB& Box, const TStaticArray<FVector4, 6>& Planes)
{
	if (!Box.IsValid()) return false;

	// 스칼라 폴백 (원래 코드)
	const FVector c = (Box.Min + Box.Max) * 0.5f;
	const FVector e = (Box.Max - Box.Min) * 0.5f;
	for (const auto& P : Planes)
	{
		const float r = std::fabs(P.X) * e.X + std::fabs(P.Y) * e.Y + std::fabs(P.Z) * e.Z;
		const float s = P.X * c.X + P.Y * c.Y + P.Z * c.Z + P.W;
		if (s + r < 0.0f) return false;
	}
	return true;
}

__m128 TestAABBFrustum_Chunk_SIMD(const FAABB_SIMD_Chunk& Chunk, const TStaticArray<FVector4, 6>& Planes)
{
	// 1. AABB 데이터 로드
	// AABB의 [MinX, MinY, MinZ]와 [MaxX, MaxY, MaxZ]를 SIMD 레지스터에 로드
	// (AABB_SIMD_Chunk의 필드들이 16바이트 정렬되어 있다고 가정)
	__m128 min_x = _mm_load_ps(Chunk.MinX);
	__m128 min_y = _mm_load_ps(Chunk.MinY);
	__m128 min_z = _mm_load_ps(Chunk.MinZ);
	__m128 max_x = _mm_load_ps(Chunk.MaxX);
	__m128 max_y = _mm_load_ps(Chunk.MaxY);
	__m128 max_z = _mm_load_ps(Chunk.MaxZ);

	// 2. 초기 마스크 설정: 모두 '렌더링 대상' (0xFFFFFFFF)
	// 이 마스크는 6개 평면 검사 중 하나라도 Outside로 판정되면 해당 비트가 0으로 바뀝니다.
	__m128 final_render_mask = _mm_castsi128_ps(_mm_set1_epi32(-1)); // All 1s

	// 3. 6개 평면 순회 (주요 루프)
	for (int i = 0; i < 6; ++i)
	{
		// 3-1. 평면 계수 로드 및 브로드캐스트
		// p_all: [A, B, C, D]
		__m128 p_all = _mm_loadu_ps(&Planes[i].X); // FVector4는 정렬되지 않았을 수 있으므로 _mm_loadu_ps 사용

		// 계수 브로드캐스트: 각 AABB 4개에 대해 동일한 A, B, C, D 계수 사용
		// SSE4.1 이상의 PSHUFD 명령어 또는 VPERMILPS 명령어가 있다면 더 최적화될 수 있지만, SSE2 호환으로 SHUFFLEPS 사용
		__m128 n_x = _mm_shuffle_ps(p_all, p_all, _MM_SHUFFLE(0, 0, 0, 0)); // [A, A, A, A]
		__m128 n_y = _mm_shuffle_ps(p_all, p_all, _MM_SHUFFLE(1, 1, 1, 1)); // [B, B, B, B]
		__m128 n_z = _mm_shuffle_ps(p_all, p_all, _MM_SHUFFLE(2, 2, 2, 2)); // [C, C, C, C]
		__m128 n_d = _mm_shuffle_ps(p_all, p_all, _MM_SHUFFLE(3, 3, 3, 3)); // [D, D, D, D]

		// 3-2. Positive Vertex (P) 계산
		// P: 평면 노멀과 같은 방향으로 가장 멀리 떨어진 꼭짓점 (Outside 검사에 사용)

		// n_i의 부호가 0보다 크거나 같은지 검사 (마스크 생성)
		__m128 mask_nx = _mm_cmpge_ps(n_x, _mm_setzero_ps());
		__m128 mask_ny = _mm_cmpge_ps(n_y, _mm_setzero_ps());
		__m128 mask_nz = _mm_cmpge_ps(n_z, _mm_setzero_ps());

		// P_x = (n_x >= 0) ? max_x : min_x; (4개 AABB 동시)
		// _mm_blendv_ps(false_case, true_case, mask)
		__m128 p_x_coord = _mm_blendv_ps(min_x, max_x, mask_nx);
		__m128 p_y_coord = _mm_blendv_ps(min_y, max_y, mask_ny);
		__m128 p_z_coord = _mm_blendv_ps(min_z, max_z, mask_nz);

		// 3-3. Outside 검사: r_outside = dot(N, P) + D
		// dot(N, P) = N_x*P_x + N_y*P_y + N_z*P_z

		__m128 dot_x = _mm_mul_ps(n_x, p_x_coord);
		__m128 dot_y = _mm_mul_ps(n_y, p_y_coord);
		__m128 dot_z = _mm_mul_ps(n_z, p_z_coord);

		// 3-4. 최종 거리 계산 및 마스크 업데이트
		// r_dot = dot(N, P)
		__m128 r_dot = _mm_add_ps(dot_x, dot_y);
		r_dot = _mm_add_ps(r_dot, dot_z);

		__m128 distance = _mm_add_ps(r_dot, n_d); // dot(N, P) + D

		// Outside_Mask: distance < 0 이면 완전히 바깥 (컬링 대상)
		__m128 outside_mask = _mm_cmplt_ps(distance, _mm_setzero_ps());

		// 최종 렌더링 마스크 업데이트:
		// outside_mask의 비트가 1이면 컬링되어야 하므로, final_render_mask의 해당 비트를 0으로 만듦
		// final_render_mask &= ~outside_mask
		final_render_mask = _mm_andnot_ps(outside_mask, final_render_mask);

		// 최적화: 4개 AABB 모두 컬링(final_render_mask == 0)되었다면 나머지 평면 검사 생략 가능
		if (_mm_movemask_ps(final_render_mask) == 0)
		{
			return final_render_mask;
		}
	}

	// 4. 최종 마스크 반환 (컬링되지 않은 AABB는 비트 1)
	return final_render_mask;
}
