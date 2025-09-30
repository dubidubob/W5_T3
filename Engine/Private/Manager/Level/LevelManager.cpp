#include "pch.h"
#include "Manager/Level/LevelManager.h"

#include "Editor/Camera.h"
#include "Level/Level.h"
#include "Mesh/CubeActor.h"
#include "Mesh/SphereActor.h"
#include "Mesh/TriangleActor.h"
#include "Mesh/SquareActor.h"
#include "Mesh/StaticMeshActor.h"
#include "Mesh/StaticMeshComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Manager/Path/PathManager.h"
#include "Utility/LevelSerializer.h"
#include "Utility/Metadata.h"
#include "Public/Core/EngineStatics.h"
#include "Render/Renderer/Renderer.h"

IMPLEMENT_CLASS(ULevelManager, UObject)
IMPLEMENT_SINGLETON(ULevelManager)

ULevelManager::ULevelManager() = default;

ULevelManager::~ULevelManager() = default;

void ULevelManager::Update(float DeltaTime) const
{
	if (CurrentLevel)
	{
		CurrentLevel->Update(DeltaTime);
	}
}

void ULevelManager::Shutdown()
{
	if (CurrentLevel)
	{
		CurrentLevel->Cleanup();
		SafeDelete(CurrentLevel);
		CurrentLevel = nullptr;
	}
}

bool ULevelManager::Init(UCamera* InCamera)
{
	CurrentLevel = NewObject<ULevel>();
	CurrentLevel->SetCamera(InCamera);
	CurrentLevel->Init();

	////UE_LOG("LevelManager: Successfully Created New Level '%s'", CurrentLevel->GetName().c_str());
	return true;
}

/**
 * @brief 새로운 빈 레벨 생성
 */
bool ULevelManager::CreateNewLevel()
{
	UCamera* Camera = CurrentLevel->GetCamera();
	Camera->Reset();
	CurrentLevel->Cleanup();
	SafeDelete(CurrentLevel);

	// 새 레벨 생성
	CurrentLevel = NewObject<ULevel>();
	CurrentLevel->SetCamera(Camera);
	CurrentLevel->Init();
	URenderer::GetInstance().SetSortingBatchMapDirty();
	////UE_LOG("LevelManager: Successfully Created New Level '%s'", CurrentLevel->GetName().c_str());
	return true;
}

/**
 * @brief 파일로부터 레벨 로드
 */
bool ULevelManager::LoadLevel(const FString& InFilePath)
{
	//UE_LOG("LevelManager: Loading Level From: %s", InFilePath.c_str());

	try
	{
		FLevelMetadata Metadata;

		// 파일로부터 메타데이터 로드
		bool bLoadSuccess = FLevelSerializer::LoadLevelFromFile(Metadata, InFilePath);
		if (!bLoadSuccess)
		{
			//UE_LOG("LevelManager: Failed To Load Level From: %s", InFilePath.c_str());
			return false;
		}

		// 유효성 검사
		FString ErrorMessage;
		if (!FLevelSerializer::ValidateLevelData(Metadata, ErrorMessage))
		{
			//UE_LOG("LevelManager: Level Validation Failed: %s", ErrorMessage.c_str());
			return false;
		}
		UCamera* CameraPtr = CurrentLevel->GetCamera();
		// 기존 레벨 정리
		if (CurrentLevel)
		{
			CurrentLevel->Cleanup();
			SafeDelete(CurrentLevel);
		}

		// 새 레벨 생성
		CurrentLevel = NewObject<ULevel>();
		CurrentLevel->SetName("LoadedLevel");

		// 새 카메라 생성 및 설정
		//UCamera* NewCamera = NewObject<UCamera>();
		CurrentLevel->SetCamera(CameraPtr);

		// 메타데이터로부터 레벨 구성
		bool bSuccess = LoadLevelFromMetadata(CurrentLevel, Metadata);
		if (!bSuccess)
		{
			//UE_LOG("LevelManager: Failed To Create Level From Metadata");
			SafeDelete(CurrentLevel);
			CurrentLevel = nullptr;
			return false;
		}

		CurrentLevel->Init();
		URenderer::GetInstance().MarkForceReRenderPicking();
		//UE_LOG("LevelManager: Level Successfully Loaded");
		return true;
	}
	catch (const exception& Exception)
	{
		//UE_LOG("LevelManager: Exception During Load: %s", Exception.what());
		return false;
	}
}

/**
 * @brief 현재 레벨을 지정된 경로에 저장
 */
bool ULevelManager::SaveCurrentLevel(const FString& InFilePath) const
{
	if (!CurrentLevel)
	{
		//UE_LOG("LevelManager: No Current Level To Save");
		return false;
	}

	// 파일 경로 처리
	path FilePath = InFilePath;
	if (FilePath.empty())
	{
		FilePath = GenerateLevelFilePath(CurrentLevel->GetName().empty() ? "Untitled" : CurrentLevel->GetName());
	}

	//UE_LOG("LevelManager: Saving Current Level To: %s", FilePath.string().c_str());

	try
	{
		// 현재 레벨을 메타데이터로 변환
		FLevelMetadata Metadata = ConvertLevelToMetadata(CurrentLevel);

		// 파일에 저장
		bool bSuccess = FLevelSerializer::SaveLevelToFile(Metadata, FilePath.string());

		if (bSuccess)
		{
			//UE_LOG("LevelManager: Level Saved Successfully");
		}
		else
		{
			//UE_LOG("LevelManager: Failed To Save Level");
		}

		return bSuccess;
	}
	catch (const exception& Exception)
	{
		//UE_LOG("LevelManager: Exception During Save: %s", Exception.what());
		return false;
	}
}

/**
 * @brief 레벨 저장 디렉토리 경로 반환
 */
path ULevelManager::GetLevelDirectory()
{
	UPathManager& PathManager = UPathManager::GetInstance();
	return PathManager.GetWorldPath();
}

/**
 * @brief 레벨 이름을 바탕으로 전체 파일 경로 생성
 */
path ULevelManager::GenerateLevelFilePath(const FString& InLevelName)
{
	path LevelDirectory = GetLevelDirectory();
	path FileName = InLevelName + ".json";
	return LevelDirectory / FileName;
}

/**
 * @brief ULevel을 FLevelMetadata로 변환
 */
FLevelMetadata ULevelManager::ConvertLevelToMetadata(ULevel* InLevel)
{
	FLevelMetadata Metadata;
	Metadata.Version = 1;
	Metadata.NextUUID = UEngineStatics::GetNextUUID();

	if (!InLevel)
	{
		//UE_LOG("LevelManager: ConvertLevelToMetadata: Level Is Null");
		return Metadata;
	}

	// 레벨의 액터들을 순회하며 메타데이터로 변환
	uint32 CurrentID = 1;
	for (AActor* Actor : InLevel->GetLevelActors())
	{
		if (!Actor)
			continue;

		FPrimitiveMetadata PrimitiveMeta;
		PrimitiveMeta.ID = Actor->GetUUID();
		PrimitiveMeta.Location = Actor->GetActorLocation();
		PrimitiveMeta.Rotation = Actor->GetActorRotation();
		PrimitiveMeta.Scale = Actor->GetActorScale3D();

		// Actor 타입에 따라 EPrimitiveType 설정
		if (Cast<ACubeActor>(Actor))
		{
			PrimitiveMeta.Type = EPrimitiveType::Cube;
		}
		else if (Cast<ASphereActor>(Actor))
		{
			PrimitiveMeta.Type = EPrimitiveType::Sphere;
		}
		else if (Cast<ATriangleActor>(Actor))
		{
			PrimitiveMeta.Type = EPrimitiveType::Triangle;
		}
		else if (Cast<ASquareActor>(Actor))
		{
			PrimitiveMeta.Type = EPrimitiveType::Square;
		}
		else if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor))
		{
			PrimitiveMeta.Type = EPrimitiveType::StaticMeshComp;
			UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
			if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
			{
				PrimitiveMeta.ObjStaticMeshAsset = StaticMeshComponent->GetStaticMesh()->GetAssetPathFileName().ToString();
			}
		}
		else
		{
			//UE_LOG("LevelManager: Unknown Actor Type, Skipping...");
			continue;
		}

		Metadata.Primitives[PrimitiveMeta.ID] = PrimitiveMeta;
	}

	// 카메라 메타데이터 저장
	UCamera* CameraPtr = InLevel->GetCamera();
	if (CameraPtr)
	{
		FCameraMetadata CameraMetadata;
		CameraMetadata.Location = CameraPtr->GetLocation();
		CameraMetadata.Rotation = CameraPtr->GetRotation();
		CameraMetadata.FOV = CameraPtr->GetFovY();
		CameraMetadata.NearClip = CameraPtr->GetNearZ();
		CameraMetadata.FarClip = CameraPtr->GetFarZ();
		Metadata.PerspectiveCamera = CameraMetadata;
	}

	//Metadata.NextUUID = CurrentID;

	//UE_LOG("LevelManager: Converted %zu Actors To Metadata", Metadata.Primitives.size());
	return Metadata;
}

/**
 * @brief FLevelMetadata로부터 ULevel에 Actor 및 UCamera 로드
 */
bool ULevelManager::LoadLevelFromMetadata(ULevel* InLevel, const FLevelMetadata& InMetadata)
{
	if (!InLevel)
	{
		//UE_LOG("LevelManager: LoadLevelFromMetadata: InLevel Is Null");
		return false;
	}

	//UE_LOG("LevelManager: Loading %zu Primitives From Metadata", InMetadata.Primitives.size());

	// Metadata의 각 Primitive를 Actor로 생성
	for (const auto& [ID, PrimitiveMeta] : InMetadata.Primitives)
	{
		AActor* NewActor = nullptr;

		// 타입에 따라 적절한 액터 생성
		switch (PrimitiveMeta.Type)
		{
		case EPrimitiveType::Cube:
			NewActor = InLevel->SpawnActor<ACubeActor>();
			break;
		case EPrimitiveType::Sphere:
			NewActor = InLevel->SpawnActor<ASphereActor>();
			break;
		case EPrimitiveType::Triangle:
			NewActor = InLevel->SpawnActor<ATriangleActor>();
			break;
		case EPrimitiveType::Square:
			NewActor = InLevel->SpawnActor<ASquareActor>();
			break;
		case EPrimitiveType::StaticMeshComp:
		{
			AStaticMeshActor* StaticMeshActor = InLevel->SpawnActor<AStaticMeshActor>();
			NewActor = StaticMeshActor;
			if (StaticMeshActor)
			{
				UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent();
				StaticMeshComponent->SetStaticMeshByPath(PrimitiveMeta.ObjStaticMeshAsset);
			}
			break;
		}
		default:
			//UE_LOG("LevelManager: Unknown Primitive Type: %d", static_cast<int32>(PrimitiveMeta.Type));
			continue;
		}

		if (NewActor)
		{
			NewActor->SetUUID(ID);
			// Transform 정보 적용
			NewActor->SetActorLocation(PrimitiveMeta.Location);
			NewActor->SetActorRotation(PrimitiveMeta.Rotation);
			NewActor->SetActorScale3D(PrimitiveMeta.Scale);
		}
		else
		{
			//UE_LOG("LevelManager: Failed to create Actor (Primitive ID: %d)", ID);
		}
	}

	// 카메라 정보 적용
	UCamera* CameraPtr = InLevel->GetCamera();
	if (CameraPtr)
	{
		CameraPtr->SetLocation(InMetadata.PerspectiveCamera.Location);
		CameraPtr->SetRotation(InMetadata.PerspectiveCamera.Rotation);
		CameraPtr->SetFovY(InMetadata.PerspectiveCamera.FOV);
		CameraPtr->SetNearZ(InMetadata.PerspectiveCamera.NearClip);
		CameraPtr->SetFarZ(InMetadata.PerspectiveCamera.FarClip);
		CameraPtr->RefreshViewMatrices();
	}
	UEngineStatics::SetNextUUID(InMetadata.NextUUID);

	//UE_LOG("LevelManager: Level successfully loaded from metadata");
	return true;
}
