#pragma once

struct FNormalVertex;
struct FStaticMesh;

struct FObjMaterialInfo
{
	FString Name;
	FVector AmbientColor;   // Ka
	FVector DiffuseColor;   // Kd
	FVector SpecularColor;  // Ks
	float SpecularExponent; // Ns
	float Alpha; // d or Tr

	FString DiffuseTexturePath; // map_Kd
	FString NormalTexturePath;  // map_bump or norm
	FString SpecularPath;
};

struct FObjInfo
{
	TArray<FVector> Positions;
	TArray<FVector2> UVs;
	TArray<FVector> Normals;
	TArray<uint32> PositionIndices;
	TArray<uint32> UVIndices;
	TArray<uint32> NormalIndices;

	//Material List

	// OBJ의 Material/Group 정보를 저장하는 구조체
	struct FMaterialGroup
	{
		int32 MaterialIndex = -1; // Materials 배열의 인덱스
		uint32 FirstFaceIndex;
	};

	TArray<FMaterialGroup> MaterialGroups;
	TArray<FObjMaterialInfo> Materials;
};

class FObjImporter
{
public:
	/**
	* @brief Parsing OBJ File, Convert To FStaticMesh
	* @param FileName Data 폴더에 저장된 Obj 파일 이름
	* @return Obj 파일의 정보를 담은 FStaticMesh 포인터
	*/
	static FStaticMesh* ParseAndConvert(const FString& FileName);

private:
	/**
	* @brief Parsing Obj File, To FObjInfo
	*/
	static bool ParseObjFile(const FString& FileName, FObjInfo& OutObjInfo);
	/**
	* @brief Parsing Obj File, To FObjInfo
	*/
	static bool ParseMtlFile(const path& FilePath, TArray<FObjMaterialInfo>& OutMaterials);
	/**
	* @brief FObjInfo To FStaticMesh
	*/
	static void ConvertObjToStaticMesh(const FObjInfo& ObjInfo, FStaticMesh& OutStaticMesh);

	static FVector PositionToUEBasis(const FVector& InVector)
	{
		return FVector(InVector.X, -InVector.Y, InVector.Z);
	}

	static FVector2 UVToUEBasis(const FVector2& InVector)
	{
		return FVector2(InVector.X, 1.0f - InVector.Y);
	}
};

