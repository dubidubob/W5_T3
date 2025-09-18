#pragma once

struct FNormalVertex;
struct FStaticMesh;

struct FObjInfo
{
	TArray<FVector> Positions;
	TArray<FVector2> UVs;
	TArray<FVector> Normals;
	TArray<uint32> PositionIndices;
	TArray<uint32> UVIndices;
	TArray<uint32> NormalIndices;
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
	* @brief FObjInfo To FStaticMesh
	*/
	static void ConvertObjToStaticMesh(const FObjInfo& ObjInfo, FStaticMesh& OutStaticMesh);
};

