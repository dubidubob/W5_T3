#pragma once
struct Batch4
{
	float sx[4], sy[4], sz[4];
	float qx[4], qy[4], qz[4], qw[4];
	float tx[4], ty[4], tz[4];
};

class USceneComponent;
class TestSIMD
{
	std::array<FMatrix, 4> UpdateWorldTransformMatrix(USceneComponent* comps[4])
	{
		Batch4 b{};
		for (int i = 0;i < 4;i++)
		{
			// ⚠️ 너 코드에선 위치/스케일을 뒤집어 넣고 있었음 (버그)
			const auto& S = comps[i]->GetRelativeScale3D();
			const auto& T = comps[i]->GetRelativeLocation();
			const auto& Q = comps[i]->GetRelativeRotationQuat();

			b.sx[i] = S.X; b.sy[i] = S.Y; b.sz[i] = S.Z;   // Scale
			b.tx[i] = T.X; b.ty[i] = T.Y; b.tz[i] = T.Z;   // Translation
			b.qx[i] = Q.X; b.qy[i] = Q.Y; b.qz[i] = Q.Z; b.qw[i] = Q.W; // Rotation
		}

		std::array<FMatrix, 4> out{};
		BuildWorld4_SSE(b, out.data());
		return out;  // 값 반환(카피 일어날 수 있지만 RVO/NRVO로 제거됨)
	}

	static void BuildWorld4_SSE(const Batch4& b, FMatrix outM[4])
	{
		__m128 X = _mm_loadu_ps(b.qx);
		__m128 Y = _mm_loadu_ps(b.qy);
		__m128 Z = _mm_loadu_ps(b.qz);
		__m128 W = _mm_loadu_ps(b.qw);

		// 2배수 항들
		const __m128 two = _mm_set1_ps(2.0f);
		__m128 XX = _mm_mul_ps(X, X), YY = _mm_mul_ps(Y, Y), ZZ = _mm_mul_ps(Z, Z);
		__m128 XY = _mm_mul_ps(X, Y), XZ = _mm_mul_ps(X, Z), YZ = _mm_mul_ps(Y, Z);
		__m128 WX = _mm_mul_ps(W, X), WY = _mm_mul_ps(W, Y), WZ = _mm_mul_ps(W, Z);

		__m128 tXX = _mm_mul_ps(two, XX), tYY = _mm_mul_ps(two, YY), tZZ = _mm_mul_ps(two, ZZ);
		__m128 tXY = _mm_mul_ps(two, XY), tXZ = _mm_mul_ps(two, XZ), tYZ = _mm_mul_ps(two, YZ);
		__m128 tWX = _mm_mul_ps(two, WX), tWY = _mm_mul_ps(two, WY), tWZ = _mm_mul_ps(two, WZ);

		const __m128 one = _mm_set1_ps(1.0f);

		// R (row-major/row-vector)
		__m128 R00 = _mm_sub_ps(one, _mm_add_ps(tYY, tZZ));
		__m128 R01 = _mm_add_ps(tXY, tWZ);
		__m128 R02 = _mm_sub_ps(tXZ, tWY);

		__m128 R10 = _mm_sub_ps(tXY, tWZ);
		__m128 R11 = _mm_sub_ps(one, _mm_add_ps(tXX, tZZ));
		__m128 R12 = _mm_add_ps(tYZ, tWX);

		__m128 R20 = _mm_add_ps(tXZ, tWY);
		__m128 R21 = _mm_sub_ps(tYZ, tWX);
		__m128 R22 = _mm_sub_ps(one, _mm_add_ps(tXX, tYY));

		// 행 스케일 (row-vector 규약: 각 "행"에 스케일 곱)
		__m128 SX = _mm_loadu_ps(b.sx);
		__m128 SY = _mm_loadu_ps(b.sy);
		__m128 SZ = _mm_loadu_ps(b.sz);

		R00 = _mm_mul_ps(R00, SX);  R01 = _mm_mul_ps(R01, SX);  R02 = _mm_mul_ps(R02, SX);
		R10 = _mm_mul_ps(R10, SY);  R11 = _mm_mul_ps(R11, SY);  R12 = _mm_mul_ps(R12, SY);
		R20 = _mm_mul_ps(R20, SZ);  R21 = _mm_mul_ps(R21, SZ);  R22 = _mm_mul_ps(R22, SZ);

		// 이동 (row-vector: 마지막 행이 translation)
		__m128 TX = _mm_loadu_ps(b.tx);
		__m128 TY = _mm_loadu_ps(b.ty);
		__m128 TZ = _mm_loadu_ps(b.tz);

		// scatter (SSE엔 scatter가 없으니 임시 배열로 풀고 4 행렬에 저장)
		alignas(16) float r00[4], r01[4], r02[4],
			r10[4], r11[4], r12[4],
			r20[4], r21[4], r22[4],
			t0[4], t1[4], t2[4];

		_mm_store_ps(r00, R00); _mm_store_ps(r01, R01); _mm_store_ps(r02, R02);
		_mm_store_ps(r10, R10); _mm_store_ps(r11, R11); _mm_store_ps(r12, R12);
		_mm_store_ps(r20, R20); _mm_store_ps(r21, R21); _mm_store_ps(r22, R22);
		_mm_store_ps(t0, TX); _mm_store_ps(t1, TY); _mm_store_ps(t2, TZ);

		for (int i = 0;i < 4;++i)
		{
			FMatrix& M = outM[i];
			M.Data[0][0] = r00[i]; M.Data[0][1] = r01[i]; M.Data[0][2] = r02[i]; M.Data[0][3] = 0.f;
			M.Data[1][0] = r10[i]; M.Data[1][1] = r11[i]; M.Data[1][2] = r12[i]; M.Data[1][3] = 0.f;
			M.Data[2][0] = r20[i]; M.Data[2][1] = r21[i]; M.Data[2][2] = r22[i]; M.Data[2][3] = 0.f;
			M.Data[3][0] = t0[i];  M.Data[3][1] = t1[i];  M.Data[3][2] = t2[i];  M.Data[3][3] = 1.f;
		}
	}
};

