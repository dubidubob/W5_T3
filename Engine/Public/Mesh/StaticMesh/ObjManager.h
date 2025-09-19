#pragma once

struct FStaticMesh;
class UStaticMesh;

class FObjManager
{
	DECLARE_SINGLETON(FObjManager)

public:
	/**
	* @brief Load FStaticMesh
	* @param PathFileName Obj 파일 경로
	* @return Obj 파일의 정보를 담은 FStaticMesh 포인터
	*/
	FStaticMesh* LoadObjStaticMeshAsset(const FString& PathFileName);

	/**
	* @brief Load UStaticMesh
	* @param PathFileName Obj 파일 경로
	* @return Obj 파일을 담는 FStaticMesh를 담는 UStaticMesh 포인터
	*/
	UStaticMesh* LoadObjStaticMesh(const FString& PathFileName);

private:
	TMap<FString, FStaticMesh*> StaticMeshAssetMap;
	TMap<FString, UStaticMesh*> StaticMeshMap;
};
