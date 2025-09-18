#pragma once
struct FMatrix;

struct FVector
{
	float X;
	float Y;
	float Z;

	/**
	 * @brief FVector 기본 생성자
	 */
	FVector();


	/**
	 * @brief FVector의 멤버값을 Param으로 넘기는 생성자
	 */
	FVector(float InX, float InY, float InZ);


	/**
	 * @brief FVector를 Param으로 넘기는 생성자
	 */
	FVector(const FVector& InOther);

	void operator=(const FVector4& InOther);
	/**
	 * @brief 두 벡터를 더한 새로운 벡터를 반환하는 함수
	 */
	FVector operator+(const FVector& InOther) const;

	/**
	 * @brief 두 벡터를 뺀 새로운 벡터를 반환하는 함수
	 */
	FVector operator-(const FVector& InOther) const;

	/**
	 * @brief 자신의 벡터에서 배율을 곱한 백테를 반환하는 함수
	 */
	FVector operator*(const float Ratio) const;

	/**
	 * @brief 자신의 벡터에 다른 벡터를 가산하는 함수
	 */
	FVector& operator+=(const FVector& InOther);

	/**
	 * @brief 자신의 벡터에서 다른 벡터를 감산하는 함수
	 */
	FVector& operator-=(const FVector& InOther);

	/**
	 * @brief 자신의 벡터에서 배율을 곱한 뒤 자신을 반환
	 */
	FVector& operator*=(const float Ratio);

	/**
	 * @brief 자신의 벡터의 각 성분의 부호를 반전한 값을 반환
	 */
	inline FVector operator-() const { return FVector(-X, -Y, -Z); }


	/**
	 * @brief 벡터의 길이 연산 함수
	 * @return 벡터의 길이
	 */
	inline float Length() const { return sqrtf(X * X + Y * Y + Z * Z); }

	/**
	 * @brief 자신의 벡터의 각 성분을 제곱하여 더한 값을 반환하는 함수 (루트 사용 X)
	 */
	inline float LengthSquared() const { return (X * X) + (Y * Y) + (Z * Z); }

	/**
	 * @brief 두 벡터를 내적하여 결과의 스칼라 값을 반환하는 함수
	 */
	inline float Dot(const FVector& OtherVector) const { return (X * OtherVector.X) + (Y * OtherVector.Y) + (Z * OtherVector.Z); }

	/**
	 * @brief 두 벡터를 외적한 결과의 벡터 값을 반환하는 함수
	 */
	inline FVector Cross(const FVector& OtherVector) const
	{
		return FVector(
			Z * OtherVector.Y - Y * OtherVector.Z,
			X * OtherVector.Z - Z * OtherVector.X,
			Y * OtherVector.X - X * OtherVector.Y
		);
	}

	/**
	 * @brief 단위 벡터로 변경하는 함수
	 */
	inline void Normalize()
	{
		float Length = sqrt(LengthSquared());
		if (Length > 0.00000001f)
		{
			X /= Length;
			Y /= Length;
			Z /= Length;
		}
	}

	bool operator==(const FVector& vector) const
	{
		return (X == vector.X) && (Y == vector.Y) && (Z == vector.Z);
	}

	/**
	 * @brief 각도를 라디안으로 변환한 값을 반환하는 함수
	 */
	inline static float GetDegreeToRadian(const float Degree) { return (Degree * Pi) / 180.f; }
	inline static FVector GetDegreeToRadian(const FVector& Rotation)
	{
		return FVector{ (Rotation.X * Pi) / 180.f, (Rotation.Y * Pi) / 180.f, (Rotation.Z * Pi) / 180.f };
	}
	/**
	 * @brief 라디안를 각도로 변환한 값을 반환하는 함수
	 */
	inline static float GetRadianToDegree(const float Radian) { return (Radian * 180.f) / Pi; }

	/**
	 * @brief 제로 벡터 (0, 0, 0) - 전역 참조 변수
	 */
	static const FVector ZeroVector;

	/**
	 * @brief 단위 벡터 (1, 1, 1) - 전역 참조 변수
	 */
	static const FVector OneVector;
};


struct FVector4
{
	float X;
	float Y;
	float Z;
	float W;

	/**
	 * @brief FVector 기본 생성자
	 */
	FVector4();

	/**
	 * @brief FVector의 멤버값을 Param으로 넘기는 생성자
	 */
	FVector4(const float InX, const float InY, const float InZ, const float InW);


	/**
	 * @brief FVector를 Param으로 넘기는 생성자
	 */
	FVector4(const FVector4& InOther);

	/**
	 * @brief 두 벡터를 더한 새로운 벡터를 반환하는 함수
	 */
	FVector4 operator+(const FVector4& OtherVector) const;

	/**
	 * @brief 벡터와 행렬곱
	 */
	FVector4 operator*(const FMatrix& Matrix) const;
	/**
	 * @brief 두 벡터를 뺀 새로운 벡터를 반환하는 함수
	 */
	FVector4 operator-(const FVector4& OtherVector) const;

	/**
	 * @brief 자신의 벡터에 배율을 곱한 값을 반환하는 함수
	 */
	FVector4 operator*(const float Ratio) const;


	/**
	 * @brief 자신의 벡터에 다른 벡터를 가산하는 함수
	 */
	void operator+=(const FVector4& OtherVector);

	/**
	 * @brief 자신의 벡터에 다른 벡터를 감산하는 함수
	 */
	void operator-=(const FVector4& OtherVector);

	/**
	 * @brief 자신의 벡터에 배율을 곱하는 함수
	 */
	void operator*=(const float Ratio);

	inline float Length() const
	{
		return sqrtf(X * X + Y * Y + Z * Z + W * W);
	}

	inline void Normalize()
	{
		float Mag = this->Length();
		X /= Mag;
		Y /= Mag;
		Z /= Mag;
		W /= Mag;
	}


	/**
	 * @brief W성분 무시하고 dot product 진행하는 함수
	 */
	inline float Dot3(const FVector4& OtherVector) const
	{
		return X * OtherVector.X + Y * OtherVector.Y + Z * OtherVector.Z;
	}
	inline float Dot3(const FVector& OtherVector) const
	{
		return X * OtherVector.X + Y * OtherVector.Y + Z * OtherVector.Z;
	}

	bool operator==(const FVector4& color) const
	{
		return (X == color.X) && (Y == color.Y) && (Z == color.Z) && (W == color.W);
	}

	/**
	 * @brief 제로 벡터 (0, 0, 0, 0) - 전역 참조 변수
	 */
	static const FVector4 ZeroVector;

	/**
	 * @brief 단위 벡터 (1, 1, 1, 1) - 전역 참조 변수
	 */
	static const FVector4 OneVector;
};

struct FVector2D
{
	float X;
	float Y;

	/**
	 * @brief FVector2D 기본 생성자
	 */
	FVector2D();

	/**
	 * @brief FVector2D의 멤버값을 Param으로 넘기는 생성자
	 */
	FVector2D(float InX, float InY);

	/**
	 * @brief FVector2D를 Param으로 넘기는 생성자
	 */
	FVector2D(const FVector2D& InOther);

	/**
	 * @brief FVector를 Param으로 넘기는 생성자
	 */
	FVector2D(const FVector& InOther);

	/**
	 * @brief 두 벡터를 더한 새로운 벡터를 반환하는 함수
	 */
	FVector2D operator+(const FVector2D& InOther) const;

	/**
	 * @brief 두 벡터를 뺀 새로운 벡터를 반환하는 함수
	 */
	FVector2D operator-(const FVector2D& InOther) const;

	/**
	 * @brief 자신의 벡터에서 배율을 곱한 벡터를 반환하는 함수
	 */
	FVector2D operator*(const float Ratio) const;

	/**
	 * @brief 자신의 벡터에 다른 벡터를 가산하는 함수
	 */
	FVector2D& operator+=(const FVector2D& InOther);

	/**
	 * @brief 자신의 벡터에서 다른 벡터를 감산하는 함수
	 */
	FVector2D& operator-=(const FVector2D& InOther);

	/**
	 * @brief 자신의 벡터에서 배율을 곱한 뒤 자신을 반환
	 */
	FVector2D& operator*=(const float Ratio);

	/**
	 * @brief 자신의 벡터의 각 성분의 부호를 반전한 값을 반환
	 */
	inline FVector2D operator-() const { return FVector2D(-X, -Y); }

	/**
	 * @brief 벡터의 길이 연산 함수
	 * @return 벡터의 길이
	 */
	inline float Length() const { return sqrtf(X * X + Y * Y); }

	/**
	 * @brief 자신의 벡터의 각 성분을 제곱하여 더한 값을 반환하는 함수 (루트 사용 X)
	 */
	inline float LengthSquared() const { return (X * X) + (Y * Y); }

	/**
	 * @brief 두 벡터를 내적하여 결과의 스칼라 값을 반환하는 함수
	 */
	inline float Dot(const FVector2D& OtherVector) const { return (X * OtherVector.X) + (Y * OtherVector.Y); }

	/**
	 * @brief 두 벡터의 외적 연산 (2D에서는 스칼라 값 반환)
	 * @return Z축 방향의 외적 크기
	 */
	inline float Cross(const FVector2D& OtherVector) const
	{
		return X * OtherVector.Y - Y * OtherVector.X;
	}

	/**
	 * @brief 단위 벡터로 변경하는 함수
	 */
	inline void Normalize()
	{
		float VectorLength = sqrt(LengthSquared());
		if (VectorLength > 0.00000001f)
		{
			X /= VectorLength;
			Y /= VectorLength;
		}
	}

	/**
	 * @brief 정규화된 벡터를 반환하는 함수 (원본 수정하지 않음)
	 */
	inline FVector2D GetNormalized() const
	{
		float VectorLength = Length();
		if (VectorLength > 0.00000001f)
		{
			return FVector2D(X / VectorLength, Y / VectorLength);
		}
		return FVector2D(0.0f, 0.0f);
	}

	/**
	 * @brief 두 벡터 사이의 거리를 반환하는 함수
	 */
	inline float Distance(const FVector2D& OtherVector) const
	{
		return (*this - OtherVector).Length();
	}

	/**
	 * @brief 두 벡터 사이의 거리의 제곱을 반환하는 함수
	 */
	inline float DistanceSquared(const FVector2D& OtherVector) const
	{
		return (*this - OtherVector).LengthSquared();
	}

	/**
	 * @brief 벡터 동등성 비교 연산자
	 */
	bool operator==(const FVector2D& OtherVector) const
	{
		return (X == OtherVector.X) && (Y == OtherVector.Y);
	}

	/**
	 * @brief 벡터 비동등성 비교 연산자
	 */
	bool operator!=(const FVector2D& OtherVector) const
	{
		return !(*this == OtherVector);
	}

	/**
	 * @brief 두 벡터를 선형 보간하는 함수
	 */
	inline static FVector2D Lerp(const FVector2D& A, const FVector2D& B, float T)
	{
		return A + (B - A) * T;
	}

	/**
	 * @brief 제로 벡터 (0, 0) - 전역 참조 변수
	 */
	static const FVector2D ZeroVector;

	/**
	 * @brief 단위 벡터 (1, 1) - 전역 참조 변수
	 */
	static const FVector2D OneVector;

	/**
	 * @brief X축 단위 벡터 (1, 0) - 전역 참조 변수
	 */
	static const FVector2D UnitX;

	/**
	 * @brief Y축 단위 벡터 (0, 1) - 전역 참조 변수
	 */
	static const FVector2D UnitY;
};
