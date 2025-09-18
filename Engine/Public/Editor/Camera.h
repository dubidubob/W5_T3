#pragma once
#include "Core/Object.h"

enum class ECameraType
{
	ECT_Orthographic,
	ECT_Perspective
};

class UCamera : public UObject
{
	DECLARE_CLASS(UCamera, UObject)

public:
    UCamera() :
        ViewProjConstants(FViewProjConstants()),
        // UE 기준(X-forward) 원점 바라보도록 -X로 초기 위치 설정
        RelativeLocation(FVector(-10.0f, 0.0f, 0.0f)), RelativeRotation(FVector(0, 0, 0)),
        FovY(90.f), Aspect(float(Render::INIT_SCREEN_WIDTH) / Render::INIT_SCREEN_HEIGHT),
        NearZ(0.1f), FarZ(100.f), CameraType(ECameraType::ECT_Perspective),
        CurrentMoveSpeed(DEFAULT_CAMERA_SPEED), CurrentMouseSensitivity(DEFAULT_MOUSE_SENSITIVITY)
    {
		LoadCameraSettings();
	}
	~UCamera() override {}

	void Update();
	void UpdateMatrixByPers();
	void UpdateMatrixByOrth();

	/**
	 * @brief Setter
	 */
	void SetLocation(const FVector& InOtherPosition) { RelativeLocation = InOtherPosition; }
	void SetRotation(const FVector& InOtherRotation) { RelativeRotation = InOtherRotation; }
	void SetFovY(const float InOtherFovY) { FovY = InOtherFovY; }
	void SetAspect(const float InOtherAspect) { Aspect = InOtherAspect; }
	void SetNearZ(const float InOtherNearZ) { NearZ = InOtherNearZ; }
	void SetFarZ(const float InOtherFarZ) { FarZ = InOtherFarZ; }
	void SetCameraType(const ECameraType InCameraType) { CameraType = InCameraType; }

	/**
	 * @brief Getter
	 */
	const FViewProjConstants& GetFViewProjConstants() const { return ViewProjConstants; }
	FViewProjConstants GetFViewProjConstantsInverse() const;

	FRay ConvertToWorldRay(float NdcX, float NdcY) const;

	FVector CalculatePlaneNormal(const FVector4& Axis);
	FVector CalculatePlaneNormal(const FVector& Axis);
	FVector& GetLocation() { return RelativeLocation; }
	FVector& GetRotation() { return RelativeRotation; }
	const FVector& GetForward() const { return Forward; }
	const FVector& GetUp() const { return Up; }
	const FVector& GetRight() const { return Right; }
	const float GetFovY() const { return FovY; }
	const float GetAspect() const { return Aspect; }
	const float GetNearZ() const { return NearZ; }
	const float GetFarZ() const { return FarZ; }
	const ECameraType GetCameraType() const { return CameraType; }

	float GetMoveSpeed() const { return CurrentMoveSpeed; }
	void SetMoveSpeed(float InSpeed)
	{
		CurrentMoveSpeed = max(InSpeed, MIN_CAMERA_SPEED);
		CurrentMoveSpeed = min(InSpeed, MAX_CAMERA_SPEED);
		SaveCameraSettings();
	}
	void AdjustMoveSpeed(float InDelta) { SetMoveSpeed(CurrentMoveSpeed + InDelta); }

	float GetMouseSensitivity() const { return CurrentMouseSensitivity; }
	void SetMouseSensitivity(float InSensitivity)
	{
		CurrentMouseSensitivity = max(InSensitivity, MIN_MOUSE_SENSITIVITY);
		CurrentMouseSensitivity = min(InSensitivity, MAX_MOUSE_SENSITIVITY);
		SaveCameraSettings();
	}
	void AdjustMouseSensitivity(float InDelta) { SetMouseSensitivity(CurrentMouseSensitivity + InDelta); }

	/* *
	 * @brief Camera Settings Save/Load
	 */
	void SaveCameraSettings() const;
	void LoadCameraSettings();

	/* *
	 * @brief 행렬 형태로 저장된 좌표와 변환 행렬과의 연산한 결과를 반환합니다.
	 */
	inline FVector4 MultiplyPointWithMatrix(const FVector4& Point, const FMatrix& Matrix) const
	{
		FVector4 Result = Point * Matrix;
		/* *
		 * @brief 좌표가 왜곡된 공간에 남는 것을 방지합니다.
		 */
		if (Result.W != 0.f) { Result *= (1.f / Result.W); }

		return Result;
	}

private:
	// Camera Speed Constants
	static constexpr float MIN_CAMERA_SPEED = 0.5f;
	static constexpr float MAX_CAMERA_SPEED = 50.0f;
	static constexpr float DEFAULT_CAMERA_SPEED = 6.0f;
	static constexpr float SPEED_ADJUST_STEP = 0.5f;

	// Mouse Sensitivity Constants
	static constexpr float MIN_MOUSE_SENSITIVITY = 0.001f;
	static constexpr float MAX_MOUSE_SENSITIVITY = 0.5f;
	static constexpr float DEFAULT_MOUSE_SENSITIVITY = 0.05f;

private:
	FViewProjConstants ViewProjConstants = {};
	FVector RelativeLocation = {};
	FVector RelativeRotation = {};
    // UE 기준: X-forward
    FVector Forward = { 1,0,0 };
	FVector Up = {};
	FVector Right = {};
	float FovY = {};
	float Aspect = {};
	float NearZ = {};
	float FarZ = {};
	float OrthoWidth = {};
	ECameraType CameraType = {};

	// Dynamic Movement Speed
	float CurrentMoveSpeed;

	// Dynamic Mouse Sensitivity
	float CurrentMouseSensitivity;
};
