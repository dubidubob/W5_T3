#include "pch.h"
#include "Editor/Camera.h"
#include "Manager/Input/InputManager.h"
#include "Manager/Time/TimeManager.h"
#include "Manager/Path/PathManager.h"
#include "Render/Renderer/Renderer.h"
#include <algorithm>

IMPLEMENT_CLASS(UCamera, UObject)

void UCamera::Reset()
{
	RelativeLocation = FVector(-10.0f, 0.0f, 0.0f);
	RelativeRotation = FVector(0, 0, 0);
	FovY = 90.f;
	NearZ = 0.1f;
	FarZ = 100.f;
	CameraViewType = EViewportViewType::Perspective;
	
	// Aspect는 현재 뷰포트 크기에 따라 계속 업데이트되므로 Reset에서 제외
	// Aspect = float(Render::INIT_SCREEN_WIDTH) / Render::INIT_SCREEN_HEIGHT;
}

void UCamera::Update(bool IsDivided)
{
	if (!IsDivided && !bIsSingleVP)
	{
		bIsSingleVP = true;
		LoadMainCameraInfo();
	}

	/*
	 * QE 상하는 카메라가 보는 방향과 관계 없이 월드 기준으로 상하로 움직인다.
	 */
	// UE 기준(X-forward, Z-up)으로 Forward 기준축을 X로 변경
	Forward = FVector4(1, 0, 0, 1) * FMatrix::RotationMatrixCamera(FVector::GetDegreeToRadian(RelativeRotation));
	Forward.Normalize();
	Up = FVector(0, 0, 1); // Z up
	Right = Up.Cross(Forward);

	/* Camera 조작*/
	Manipulate();

	// Update Main Camera Aspect, updating multi viewport is in OnWindowResized
	if (URenderer::GetInstance().GetDeviceResources())
	{
		float Width = URenderer::GetInstance().GetDeviceResources()->GetViewportInfo().Width;
		float Height = URenderer::GetInstance().GetDeviceResources()->GetViewportInfo().Height;
		SetAspect(Width / Height);
	}

	switch (CameraViewType)
	{
	case EViewportViewType::Perspective:
		UpdateMatrixByPers();
		break;
	default : /* Orthographic Mode */
		UpdateMatrixByOrth();
		break;
	}
}

/**
* @brief 마우스 우클릭을 하고 있는 동안 카메라 제어가 가능합니다.
*/
void UCamera::Manipulate()
{
	const UInputManager& Input = UInputManager::GetInstance();

	if (CameraViewType == EViewportViewType::Perspective)
	{
		if (Input.IsKeyDown(EKeyInput::MouseRight))
		{
			/**
			 * @brief W, A, S, D 는 각각 카메라의 상, 하, 좌, 우 이동을 담당합니다.
			 */
			FVector Direction = { 0, 0, 0 };

			if (Input.IsKeyDown(EKeyInput::A)) { Direction += Right; }
			if (Input.IsKeyDown(EKeyInput::D)) { Direction += -Right; }
			if (Input.IsKeyDown(EKeyInput::W)) { Direction += Forward; }
			if (Input.IsKeyDown(EKeyInput::S)) { Direction += -Forward; }
			if (Input.IsKeyDown(EKeyInput::Q)) { Direction += -Up; }
			if (Input.IsKeyDown(EKeyInput::E)) { Direction += Up; }
			Direction.Normalize();
			RelativeLocation += Direction * CurrentMoveSpeed * DT;

			// 오른쪽 마우스 버튼 + 마우스 휠로 카메라 이동속도 조절
			float WheelDelta = Input.GetMouseWheelDelta();
			if (WheelDelta != 0.0f)
			{
				// 휠 위로 돌리면 속도 증가, 아래로 돌리면 속도 감소
				AdjustMoveSpeed(WheelDelta * SPEED_ADJUST_STEP);
			}

			/**
			* @brief 마우스 위치 변화량을 감지하여 카메라의 회전을 담당합니다.
			*/
			const FVector2 MouseDelta = UInputManager::GetInstance().GetMouseDelta();
			RelativeRotation.Y = std::clamp(RelativeRotation.Y + MouseDelta.Y * CurrentMouseSensitivity, -89.0f, 89.0f);
			RelativeRotation.Z += MouseDelta.X * CurrentMouseSensitivity;

			// Pitch 클램프(짐벌 플립 방지)
			RelativeRotation.X = std::min(RelativeRotation.X, 89.0f);
			RelativeRotation.X = std::max(RelativeRotation.X, -89.0f);
		}
	}
	else
	{
		if (Input.IsKeyDown(EKeyInput::MouseRight))
		{
			const FVector2 MouseDelta = UInputManager::GetInstance().GetMouseDelta();

			float MouseDeltaX = MouseDelta.X * CurrentMouseSensitivity;
			float MouseDeltaY = MouseDelta.Y * CurrentMouseSensitivity;

			FVector OrthoMoveDirection(0.0f, 0.0f, 0.0f);

			switch (CameraViewType)
			{
			case EViewportViewType::Front:
				OrthoMoveDirection = FVector(0.0f, -MouseDeltaX, MouseDeltaY);
				break;
			case EViewportViewType::Back:
				OrthoMoveDirection = FVector(0.0f, MouseDeltaX, MouseDeltaY);
				break;
			case EViewportViewType::Top:
				OrthoMoveDirection = FVector(MouseDeltaY, MouseDeltaX, 0.0f);
				break;
			case EViewportViewType::Bottom:
				OrthoMoveDirection = FVector(-MouseDeltaY, MouseDeltaX, 0.0f);
				break;
			case EViewportViewType::Left:
				OrthoMoveDirection = FVector(MouseDeltaX, 0.0f, MouseDeltaY);
				break;
			case EViewportViewType::Right:
				OrthoMoveDirection = FVector(-MouseDeltaX, 0.0f, MouseDeltaY);
				break;
			}

			RelativeLocation += OrthoMoveDirection;
			OrthoMoveDelta = OrthoMoveDirection;
		}
	}	
}

void UCamera::UpdateMatrixByPers()

{
	/**
	 * @brief View 행렬 연산
	 */
	FMatrix T = FMatrix::TranslationMatrixInverse(RelativeLocation);
	FMatrix R = FMatrix::RotationMatrixInverseCamera(FVector::GetDegreeToRadian(RelativeRotation));
	// 좌표계 기준 변환(B)을 View에 접합(마지막에 적용): pos = pos * World * (View * B) * Proj
	ViewProjConstants.View = T * (R * FMatrix::BasisLHYToUE());

	/**
	 * @brief Projection 행렬 연산
	 * 원근 투영 행렬 (HLSL에서 row-major로 mul(p, M) 일관성 유지)
	 * f = 1 / tan(fovY/2)
	 */
	const float RadianFovY = FVector::GetDegreeToRadian(FovY);

	const float F = 1.0f / std::tanf(RadianFovY * 0.5f);

	FMatrix P = FMatrix::Identity;
	// | f/aspect   0        0         0 |
	// |    0       f        0         0 |
	// |    0       0   zf/(zf-zn)     1 |
	// |    0       0  -zn*zf/(zf-zn)  0 |
	P.Data[0][0] = F / Aspect;
	P.Data[1][1] = F;
	P.Data[2][2] = FarZ / (FarZ - NearZ);
	P.Data[2][3] = 1.0f;
	P.Data[3][2] = (-NearZ * FarZ) / (FarZ - NearZ);
	P.Data[3][3] = 0.0f;

	ViewProjConstants.Projection = P;
}

void UCamera::UpdateMatrixByOrth()
{
	/**
	 * @brief View 행렬 연산
	 */
	FMatrix T = FMatrix::TranslationMatrixInverse(RelativeLocation);
	FMatrix R = FMatrix::RotationMatrixInverseCamera(FVector::GetDegreeToRadian(RelativeRotation));
	// 좌표계 기준 변환(B)을 View에 접합(마지막에 적용)
	ViewProjConstants.View = T * (R * FMatrix::BasisLHYToUE());

	/**
	 * @brief Projection 행렬 연산
	 */
	OrthoWidth = OrthoDistance; // 2.0f * std::tanf(FVector::GetDegreeToRadian(FovY) * 0.5f);
	const float OrthoHeight = OrthoWidth / Aspect;
	const float Left = -OrthoWidth * 0.5f;
	const float Right = OrthoWidth * 0.5f;
	const float Bottom = -OrthoHeight * 0.5f;
	const float Top = OrthoHeight * 0.5f;

	FMatrix P = FMatrix::Identity;
	P.Data[0][0] = 2.0f / (Right - Left);
	P.Data[1][1] = 2.0f / (Top - Bottom);
	P.Data[2][2] = 1.0f / (FarZ - NearZ);
	P.Data[3][0] = -(Right + Left) / (Right - Left);
	P.Data[3][1] = -(Top + Bottom) / (Top - Bottom);
	P.Data[3][2] = -NearZ / (FarZ - NearZ);
	P.Data[3][3] = 1.0f;
	ViewProjConstants.Projection = P;
}

FViewProjConstants UCamera::GetFViewProjConstantsInverse() const
{
	/*
	* @brief View^(-1) = R * T
	*/
	FViewProjConstants Result = {};
	FMatrix R = FMatrix::RotationMatrixCamera(FVector::GetDegreeToRadian(RelativeRotation));
	FMatrix T = FMatrix::TranslationMatrix(RelativeLocation);
	// (View * B)^-1 = B^-1 * View^-1
	Result.View = (FMatrix::BasisUEToLHY() * R) * T;

	/*Ortho Matrix*/
	if (CameraViewType != EViewportViewType::Perspective) 
	{
		const float OrthoHeight = OrthoWidth / Aspect;
		const float Left = -OrthoWidth * 0.5f;
		const float Right1 = OrthoWidth * 0.5f;
		const float Bottom = -OrthoHeight * 0.5f;
		const float Top = OrthoHeight * 0.5f;

		FMatrix P = FMatrix::Identity;
		// A^{-1} (대각)
		P.Data[0][0] = (Right1 - Left) * 0.5f; // (r-l)/2
		P.Data[1][1] = (Top - Bottom) * 0.5f; // (t-b)/2
		P.Data[2][2] = (FarZ - NearZ); // (zf-zn)
		// -b A^{-1} (마지막 행의 x,y,z)
		P.Data[3][0] = (Right1 + Left) * 0.5f; // (r+l)/2
		P.Data[3][1] = (Top + Bottom) * 0.5f; // (t+b)/2
		P.Data[3][2] = NearZ; // zn
		P.Data[3][3] = 1.0f;
		Result.Projection = P;
	}
	else if ((CameraViewType == EViewportViewType::Perspective))
	{
		const float FovRadian = FVector::GetDegreeToRadian(FovY);
		const float F = 1.0f / std::tanf(FovRadian * 0.5f);
		FMatrix P = FMatrix::Identity;
		// | aspect/F   0      0         0 |
		// |    0      1/F     0         0 |
		// |    0       0      0   -(zf-zn)/(zn*zf) |
		// |    0       0      1        zf/(zn*zf)  |
		P.Data[0][0] = Aspect / F;
		P.Data[1][1] = 1.0f / F;
		P.Data[2][2] = 0.0f;
		P.Data[2][3] = -(FarZ - NearZ) / (NearZ * FarZ);
		P.Data[3][2] = 1.0f;
		P.Data[3][3] = FarZ / (NearZ * FarZ);
		Result.Projection = P;
	}

	return Result;
}


FRay UCamera::ConvertToWorldRay(float NdcX, float NdcY) const
{
	/* *
	 * @brief 반환할 타입의 객체 선언
	 */
	FRay Ray = {};

	FViewProjConstants ViewProjMatrix = GetFViewProjConstantsInverse();
	// ViewProjMatrix.View = ViewProjMatrix.View;

	/* *
	 * @brief NDC 좌표 정보를 행렬로 변환합니다.
	 */
	const FVector4 NdcNear(NdcX, NdcY, 0.0f, 1.0f);
	const FVector4 NdcFar(NdcX, NdcY, 1.0f, 1.0f);

	/* *
	 * @brief Projection 행렬을 View 행렬로 역투영합니다.
	 * Model -> View -> Projection -> NDC
	 */
	const FVector4 ViewNear = MultiplyPointWithMatrix(NdcNear, ViewProjMatrix.Projection);
	const FVector4 ViewFar = MultiplyPointWithMatrix(NdcFar, ViewProjMatrix.Projection);

	/* *
	 * @brief View 행렬을 World 행렬로 역투영합니다.
	 * Model -> View -> Projection -> NDC
	 */
	const FVector4 WorldNear = MultiplyPointWithMatrix(ViewNear, ViewProjMatrix.View);
	const FVector4 WorldFar = MultiplyPointWithMatrix(ViewFar, ViewProjMatrix.View);

	/* *
	 * @brief 카메라의 월드 좌표를 추출합니다.
	 * Row-major 기준, 마지막 행 벡터는 위치 정보를 가지고 있음
	 */
	const FVector4 CameraPosition(
		ViewProjMatrix.View.Data[3][0],
		ViewProjMatrix.View.Data[3][1],
		ViewProjMatrix.View.Data[3][2],
		ViewProjMatrix.View.Data[3][3]);

	if (CameraViewType == EViewportViewType::Perspective)
	{
		FVector4 DirectionVector = WorldFar - CameraPosition;
		DirectionVector.Normalize();

		Ray.Origin = CameraPosition;
		Ray.Direction = DirectionVector;
	}
	else if (CameraViewType != EViewportViewType::Perspective)
	{
		FVector4 DirectionVector = WorldFar - WorldNear;
		DirectionVector.Normalize();

		Ray.Origin = WorldNear;
		Ray.Direction = DirectionVector;
	}

	// 기준변환을 View에 흡수했으므로 별도 축 순열 변환 불필요
	Ray.Direction.Normalize();

	return Ray;
}

FVector UCamera::CalculatePlaneNormal(const FVector4& Axis)
{
	return Forward.Cross(FVector(Axis.X, Axis.Y, Axis.Z));
}

FVector UCamera::CalculatePlaneNormal(const FVector& Axis)
{
	return Forward.Cross(FVector(Axis.X, Axis.Y, Axis.Z));
}

void UCamera::SaveCameraSettings() const
{
	const path ConfigFilePath = UPathManager::GetInstance().GetEditorIniPath();

	WritePrivateProfileStringA(
		"Camera",
		"MoveSpeed",
		std::to_string(CurrentMoveSpeed).c_str(),
		ConfigFilePath.string().c_str()
	);

	WritePrivateProfileStringA(
		"Camera",
		"MouseSensitivity",
		std::to_string(CurrentMouseSensitivity).c_str(),
		ConfigFilePath.string().c_str()
	);
}

void UCamera::LoadCameraSettings()
{
	const path ConfigFilePath = UPathManager::GetInstance().GetEditorIniPath();

	// Check if config file exists
	if (!std::filesystem::exists(ConfigFilePath))
	{
		// Create default config if it doesn't exist
		SaveCameraSettings();
		return;
	}

	char Buffer[32];

	// Load Move Speed
	GetPrivateProfileStringA(
		"Camera",
		"MoveSpeed",
		std::to_string(DEFAULT_CAMERA_SPEED).c_str(),
		Buffer,
		sizeof(Buffer),
		ConfigFilePath.string().c_str()
	);

	float LoadedSpeed = std::stof(Buffer);
	// 로드한 값을 직접 설정 (SaveCameraSettings 호출하지 않음)
	CurrentMoveSpeed = max(LoadedSpeed, MIN_CAMERA_SPEED);
	CurrentMoveSpeed = min(CurrentMoveSpeed, MAX_CAMERA_SPEED);

	// Load Mouse Sensitivity
	GetPrivateProfileStringA(
		"Camera",
		"MouseSensitivity",
		std::to_string(DEFAULT_MOUSE_SENSITIVITY).c_str(),
		Buffer,
		sizeof(Buffer),
		ConfigFilePath.string().c_str()
	);

	float LoadedSensitivity = std::stof(Buffer);
	// 로드한 값을 직접 설정 (SaveCameraSettings 호출하지 않음)
	CurrentMouseSensitivity = max(LoadedSensitivity, MIN_MOUSE_SENSITIVITY);
	CurrentMouseSensitivity = min(CurrentMouseSensitivity, MAX_MOUSE_SENSITIVITY);
}

void UCamera::SetCameraType(const EViewportViewType InCameraType, bool bIsWindowDivided)
{
	// Single Perspective Viewport 상태만 저장.
	// Single Viewport에 Perspective 상태였고 + 곧 Divided로 전환되거나 다른 CameraType으로 전환될 때 Camera Info Update
	if (bIsSingleVP)
	{
		if (bIsWindowDivided)
		{
			bIsSingleVP = false;
			SaveMainCameraInfo();
		}
		if (CameraViewType == EViewportViewType::Perspective && InCameraType != EViewportViewType::Perspective)
		{
			SaveMainCameraInfo();
		}
	}

	CameraViewType = InCameraType;

	if (InCameraType == EViewportViewType::Perspective)
	{
		// Perspective camera logic (if any)
		return;
	}

	FVector MoveAxis = FVector::ZeroVector;
	FVector CameraRotation = FVector::ZeroVector;

	switch (InCameraType)
	{
	case EViewportViewType::Front:
		MoveAxis = FVector(1.0f, 0.0f, 0.0f);
		CameraRotation = FVector(0.0f, 0.0f, -2.0f);
		break;

	case EViewportViewType::Back:
		MoveAxis = FVector(-1.0f, 0.0f, 0.0f);
		CameraRotation = FVector(0.0f, 0.0f, 0.0f);
		break;

	case EViewportViewType::Top:
		MoveAxis = FVector(0.0f, 0.0f, 1.0f);
		CameraRotation = FVector(0.0f, 1.0f, 0.0f);
		break;

	case EViewportViewType::Bottom:
		MoveAxis = FVector(0.0f, 0.0f, -1.0f);
		CameraRotation = FVector(0.0f, -1.0f, 0.0f);
		break;

	case EViewportViewType::Left:
		MoveAxis = FVector(0.0f, -1.0f, 0.0f);
		CameraRotation = FVector(0.0f, 0.0f, 1.0f);
		break;

	case EViewportViewType::Right:
		MoveAxis = FVector(0.0f, 1.0f, 0.0f);
		CameraRotation = FVector(0.0f, 0.0f, -1.0f);
		break;
	}

	const float RotateStep = 90.0f;
	CameraRotation *= RotateStep;

	MoveAxis *= OrthoDistance;

	SetLocation(MoveAxis);
	SetRotation(CameraRotation);
}

void UCamera::SaveMainCameraInfo()
{
	SavedRelativeLocation = RelativeLocation;
	SavedRelativeRotation = RelativeRotation;
	SaveFovY = FovY;
	SaveAspect = Aspect;
	SaveNearZ = NearZ;
	SaveFarZ = FarZ;
	SaveCameraViewType = CameraViewType;
}

void UCamera::LoadMainCameraInfo()
{
	RelativeLocation = SavedRelativeLocation;
	RelativeRotation = SavedRelativeRotation;
	FovY = SaveFovY;
	Aspect = SaveAspect;
	NearZ = SaveNearZ;
	FarZ = SaveFarZ;
	CameraViewType = SaveCameraViewType;
}

/* Copy From Other Except Aspect & CameraViewType*/
void UCamera::CopyFrom(const UCamera& Other)
{
	SetLocation(Other.GetLocation());
	SetRotation(Other.GetRotation());

	FovY = Other.GetFovY();
	NearZ = Other.GetNearZ();
	FarZ = Other.GetFarZ();

	/*CurrentMoveSpeed = Other.GetMoveSpeed();
	CurrentMouseSensitivity = Other.GetMouseSensitivity();*/

	RefreshViewMatrices();
}

void UCamera::RefreshViewMatrices()
{
	Forward = FVector4(1, 0, 0, 1) * FMatrix::RotationMatrixCamera(FVector::GetDegreeToRadian(RelativeRotation));
	Forward.Normalize();
	Up = FVector(0, 0, 1);
	Right = Forward.Cross(Up);

	switch (CameraViewType)
	{
		case EViewportViewType::Perspective:
			UpdateMatrixByPers();
			break;
		default : /* Orthographic */
			UpdateMatrixByOrth();
			break;
	}
}
