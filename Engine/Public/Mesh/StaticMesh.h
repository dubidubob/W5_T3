#pragma once
class UStaticMesh : public UObject
{
	DECLARE_CLASS(UStaticMesh, UObject)
public:
	//Getter
	const FString& GetAssetPathFileName() { return StaticMeshAsset->PathFileName; }
	//Setter
	void SetStaticMeshAsset(FStaticMesh* InStaticMesh) {StaticMeshAsset = InStaticMesh;}

private:
	FStaticMesh* StaticMeshAsset;
};

