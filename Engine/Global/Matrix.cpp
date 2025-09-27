#include "pch.h"

/**
* @brief float 타입의 배열을 사용한 FMatrix의 기본 생성자
*/
FMatrix::FMatrix()
	: Data{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
{
}


/**
* @brief float 타입의 param을 사용한 FMatrix의 기본 생성자
*/
FMatrix::FMatrix(
	float M00, float M01, float M02, float M03,
	float M10, float M11, float M12, float M13,
	float M20, float M21, float M22, float M23,
	float M30, float M31, float M32, float M33)
	: Data{
		{M00, M01, M02, M03},
		{M10, M11, M12, M13},
		{M20, M21, M22, M23},
		{M30, M31, M32, M33}
	}
{
}

/**
 * @brief 행렬의 전치행렬을 반환하는 함수
 */
FMatrix FMatrix::Transpose(const FMatrix& InOtherMatrix)
{
#if SIMD_LEVEL >= 1
	FMatrix Result;

	// InOtherMatrix의 4개 행을 4개의 SIMD 레지스터에 로드합니다.
	__m128 row0 = InOtherMatrix.row[0];
	__m128 row1 = InOtherMatrix.row[1];
	__m128 row2 = InOtherMatrix.row[2];
	__m128 row3 = InOtherMatrix.row[3];

	_MM_TRANSPOSE4_PS(row0, row1, row2, row3);

	// 전치된 결과를 Result 행렬의 행에 저장합니다.
	Result.row[0] = row0;
	Result.row[1] = row1;
	Result.row[2] = row2;
	Result.row[3] = row3;

	return Result;
#elif SIMD_LEVEL ==0
	return {
		InOtherMatrix.Data[0][0], InOtherMatrix.Data[1][0], InOtherMatrix.Data[2][0], InOtherMatrix.Data[3][0],
		InOtherMatrix.Data[0][1], InOtherMatrix.Data[1][1], InOtherMatrix.Data[2][1], InOtherMatrix.Data[3][1],
		InOtherMatrix.Data[0][2], InOtherMatrix.Data[1][2], InOtherMatrix.Data[2][2], InOtherMatrix.Data[3][2],
		InOtherMatrix.Data[0][3], InOtherMatrix.Data[1][3], InOtherMatrix.Data[2][3], InOtherMatrix.Data[3][3]
	};
#endif
}
#if SIMD_LEVEL >= 1
__m128 FMatrix::MulVecMat(const __m128& v, const FMatrix& M)
{
	// v의 각 요소를 4개씩 복사 (브로드캐스트)
	__m128 vX = _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 vY = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1));
	__m128 vZ = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2));
	__m128 vW = _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3));

	// M.row[i] * v.i 를 계산하여 모두 더함 (Dot Product의 병렬 확장)
	__m128 r = _mm_mul_ps(vX, M.row[0]);
	r = _mm_add_ps(r, _mm_mul_ps(vY, M.row[1]));
	r = _mm_add_ps(r, _mm_mul_ps(vZ, M.row[2]));
	r = _mm_add_ps(r, _mm_mul_ps(vW, M.row[3]));
	return r;
}
#endif

/**
* @brief 두 행렬곱을 진행한 행렬을 반환하는 연산자 함수 (SIMD 최적화)
*/
FMatrix FMatrix::operator*(const FMatrix& InOtherMatrix) const
{
	FMatrix Result;

#if SIMD_LEVEL>=1
	Result.row[0] = MulVecMat(this->row[0], InOtherMatrix);
	Result.row[1] = MulVecMat(this->row[1], InOtherMatrix);
	Result.row[2] = MulVecMat(this->row[2], InOtherMatrix);
	Result.row[3] = MulVecMat(this->row[3], InOtherMatrix);
#elif SIMD_LEVEL==0
	for (int32 i = 0; i < 4; ++i)
	{
		for (int32 j = 0; j < 4; ++j)
		{
			for (int32 k = 0; k < 4; ++k)
			{
				Result.Data[i][j] += Data[i][k] * InOtherMatrix.Data[k][j];
			}
		}
	}
#endif

	return Result;
}

void FMatrix::operator*=(const FMatrix& InOtherMatrix)
{
	*this = (*this) * InOtherMatrix;
}

/**
* @brief Position의 정보를 행렬로 변환하여 제공하는 함수
*/
FMatrix FMatrix::TranslationMatrix(const FVector& InOtherVector)
{
	FMatrix Result = FMatrix::Identity;
	Result.Data[3][0] = InOtherVector.X;
	Result.Data[3][1] = InOtherVector.Y;
	Result.Data[3][2] = InOtherVector.Z;
	Result.Data[3][3] = 1;

	return Result;
}

FMatrix FMatrix::TranslationMatrixInverse(const FVector& InOtherVector)
{
	FMatrix Result = FMatrix::Identity;
	Result.Data[3][0] = -InOtherVector.X;
	Result.Data[3][1] = -InOtherVector.Y;
	Result.Data[3][2] = -InOtherVector.Z;
	Result.Data[3][3] = 1;

	return Result;
}

/**
* @brief Scale의 정보를 행렬로 변환하여 제공하는 함수
*/
FMatrix FMatrix::ScaleMatrix(const FVector& InOtherVector)
{
	FMatrix Result = FMatrix::Identity;
	Result.Data[0][0] = InOtherVector.X;
	Result.Data[1][1] = InOtherVector.Y;
	Result.Data[2][2] = InOtherVector.Z;
	Result.Data[3][3] = 1;

	return Result;
}

FMatrix FMatrix::ScaleMatrixInverse(const FVector& InOtherVector)
{
	FMatrix Result = FMatrix::Identity;
	Result.Data[0][0] = 1 / InOtherVector.X;
	Result.Data[1][1] = 1 / InOtherVector.Y;
	Result.Data[2][2] = 1 / InOtherVector.Z;
	Result.Data[3][3] = 1;

	return Result;
}

/**
* @brief Rotation의 정보를 행렬로 변환하여 제공하는 함수
*/
FMatrix FMatrix::RotationMatrix(const FVector& InOtherVector)
{
    return RotationX(InOtherVector.X) * RotationY(InOtherVector.Y) * RotationZ(InOtherVector.Z);
}

FMatrix FMatrix::RotationMatrixInverse(const FVector& InOtherVector)
{
	return RotationZ(-InOtherVector.Z) * RotationY(-InOtherVector.Y) * RotationX(-InOtherVector.X);
}

/**
* @brief Camera용 Rotation의 정보를 행렬로 변환하여 제공하는 함수
*		 카메라의 경우 YXZ 순서로 회전해야 일반적인 카메라 회전과 동일
*/

FMatrix FMatrix::RotationMatrixCamera(const FVector& InOtherVector)
{
	// Roll -> Pitch -> Yaw 순서
	return RotationX(InOtherVector.X) * RotationY(InOtherVector.Y) * RotationZ(InOtherVector.Z);
}

FMatrix FMatrix::RotationMatrixInverseCamera(const FVector& InOtherVector)
{
	// 역행렬: Yaw^-1 -> Pitch^-1 -> Roll^-1
	return RotationZ(-InOtherVector.Z) * RotationY(-InOtherVector.Y) * RotationX(-InOtherVector.X);
}

/**
* @brief X의 회전 정보를 행렬로 변환
*/
FMatrix FMatrix::RotationX(float Radian)
{
	FMatrix Result = FMatrix::Identity;
	const float C = std::cosf(Radian);
	const float S = std::sinf(Radian);

	Result.Data[1][1] = C;
	Result.Data[1][2] = S;
	Result.Data[2][1] = -S;
	Result.Data[2][2] = C;

	return Result;
}

/**
* @brief Y의 회전 정보를 행렬로 변환
*/
FMatrix FMatrix::RotationY(float Radian)
{
	FMatrix Result = FMatrix::Identity;
	const float C = std::cosf(Radian);
	const float S = std::sinf(Radian);

	Result.Data[0][0] = C;
	Result.Data[0][2] = -S;
	Result.Data[2][0] = S;
	Result.Data[2][2] = C;

	return Result;
}

/**
* @brief Y의 회전 정보를 행렬로 변환
*/
FMatrix FMatrix::RotationZ(float Radian)
{
	FMatrix Result = FMatrix::Identity;
	const float C = std::cosf(Radian);
	const float S = std::sinf(Radian);

	Result.Data[0][0] = C;
	Result.Data[0][1] = S;
	Result.Data[1][0] = -S;
	Result.Data[1][1] = C;

	return Result;
}

// Quaternion 기반 회전행렬 (row-major)
FMatrix FMatrix::RotationMatrix(const FQuat& Q)
{
    return QuatToRotationMatrix(Q);
}

FMatrix FMatrix::RotationMatrixInverse(const FQuat& Q)
{
    return QuatToRotationMatrixInverse(Q);
}

FMatrix FMatrix::GetModelMatrix(const FVector& Location, const FVector& Rotation, const FVector& Scale)
{
    FMatrix T = TranslationMatrix(Location);
    FMatrix R = RotationMatrix(Rotation);
    FMatrix S = ScaleMatrix(Scale);

    return S * R * T;
}

FMatrix FMatrix::GetModelMatrixInverse(const FVector& Location, const FVector& Rotation, const FVector& Scale)
{
	FMatrix T = TranslationMatrixInverse(Location);
	FMatrix R = RotationMatrixInverse(Rotation);
	FMatrix S = ScaleMatrixInverse(Scale);

    return T * R * S;
}

FMatrix FMatrix::GetModelMatrix(const FVector& Location, const FQuat& Rotation, const FVector& Scale)
{
    FMatrix T = TranslationMatrix(Location);
    FMatrix R = RotationMatrix(Rotation);
    FMatrix S = ScaleMatrix(Scale);
    return S * R * T;
}

FMatrix FMatrix::GetModelMatrixInverse(const FVector& Location, const FQuat& Rotation, const FVector& Scale)
{
    FMatrix T = TranslationMatrixInverse(Location);
    FMatrix R = RotationMatrixInverse(Rotation);
    FMatrix S = ScaleMatrixInverse(Scale);
    return T * R * S;
}

/**
 * @brief 좌표계 기준변환: LHY+ -> UE(LHZ+, X-forward)
 * (x,y,z) -> (z,x,y) 로 순열 전환하는 행렬
 */
FMatrix FMatrix::BasisLHYToUE()
{
	// row-major, row-vector mul(p, M) 기준
	return {
		0, 0, 1, 0,
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1
	};
}

/**
 * @brief 좌표계 기준변환의 역행렬: UE(LHZ+, X-forward) -> LHY+
 * 직교 순열행렬의 역행렬은 전치행렬과 동일
 */
FMatrix FMatrix::BasisUEToLHY()
{
	// transpose of BasisLHYToUE
	return {
		0, 1, 0, 0,
		0, 0, 1, 0,
		1, 0, 0, 0,
		0, 0, 0, 1
	};
}

const FMatrix FMatrix::Identity = FMatrix(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
);

const FMatrix FMatrix::Zero = FMatrix();
