#pragma once

/**
 * @brief UObject Meta Data Struct
 * @param ID 고유 ID
 * @param ObjStaticMeshAsset 스태틱 메쉬 에셋 경로(.obj)
 * @param Location 위치
 * @param Rotation 회전
 * @param Scale 스케일
 * @param Type 오브젝트 타입
 */
struct FPrimitiveMetadata
{
	uint32 ID;
	FString ObjStaticMeshAsset; 
	FVector Location;
	FVector Rotation;
	FVector Scale;
	EPrimitiveType Type;

	/**
	 * @brief Default Constructor
	 */
	FPrimitiveMetadata()
		: ID(0)
		  , ObjStaticMeshAsset("")
		  , Location(0.0f, 0.0f, 0.0f)
		  , Rotation(0.0f, 0.0f, 0.0f)
		  , Scale(1.0f, 1.0f, 1.0f)
		  , Type(EPrimitiveType::None)
	{
	}

	/**
	 * @brief Parameter Constructor
	 */
	FPrimitiveMetadata(uint32 InID, const FVector& InLocation, const FVector& InRotation,
	                   const FVector& InScale, EPrimitiveType InType)
		: ID(InID)
		  , Location(InLocation)
		  , Rotation(InRotation)
		  , Scale(InScale)
		  , Type(InType)
	{
	}

	/**
	 * @brief Equality Comparison Operator
	 */
	bool operator==(const FPrimitiveMetadata& InOther) const
	{
		return ID == InOther.ID;
	}
};

/**
 * @brief Level Meta Data Struct
 * @param Version 레벨 버전
 * @param NextUUID 다음으로 찍어낼 UUID
 * @param Primitives 레벨 내 Primitive를 모아놓은 Metadata Map
 * @param PerspectiveCamera 카메라 메타데이터
 */
struct FLevelMetadata
{
	uint32 Version;
	uint32 NextUUID;
	TMap<uint32, FPrimitiveMetadata> Primitives;
	FCameraMetaData PerspectiveCamera;
	/**
	 * @brief 기본 생성자
	 */
	FLevelMetadata()
		: Version(1)
		  , NextUUID(0)
	{
	}

	/**
	 * @brief Level Meta에 Primitive를 추가하는 함수
	 */
	uint32 AddPrimitive(const FPrimitiveMetadata& InPrimitiveData)
	{
		uint32 NewID = NextUUID++;
		FPrimitiveMetadata NewPrimitive = InPrimitiveData;
		NewPrimitive.ID = NewID;
		Primitives[NewID] = NewPrimitive;
		return NewID;
	}

	/**
	 * @brief Level Meta에서 특정 ID를 가진 Primitive를 제거하는 함수
	 */
	bool RemovePrimitive(uint32 InID)
	{
		return Primitives.erase(InID) > 0;
	}

	/**
	 * @brief Level Meta에서 특정 ID를 가진 Primitive를 검색하는 함수
	 */
	FPrimitiveMetadata* FindPrimitive(uint32 InID)
	{
		auto Iter = Primitives.find(InID);

		if (Iter != Primitives.end())
		{
			return &Iter->second;
		}
		else
		{
			return nullptr;
		}
	}

	/**
	 * @brief Primitive Clear Function
	 */
	void ClearPrimitives()
	{
		Primitives.clear();
		NextUUID = 0;
	}
};

/**
 * @brief Perspective Camera Meta Data Struct
 * @param Location 카메라 위치
 * @param Rotation 카메라 회전
 * @param FOV 카메라 fov
 * @param NearClip 카메라 near
 * @param FarClip 카메라 far
 */
struct FCameraMetaData
{
	FVector Location;
	FVector Rotation;
	float FOV;
	float NearClip;
	float FarClip;

	FCameraMetaData()
		: Location(0.0f, 2.0f, -5.0f),
		Rotation(0.0f, 0.0f, 0.0f),
		FOV(60.0f),
		NearClip(0.1f),
		FarClip(1000.0f)
	{
	}
};
