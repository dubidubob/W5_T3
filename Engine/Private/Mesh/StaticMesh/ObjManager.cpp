#include "pch.h"
#include "Mesh/StaticMesh/ObjManager.h"
#include "Core/ObjectIterator.h"
#include "Public/Mesh/StaticMesh/ObjImporter.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"
#include "Public/Render/Renderer/Renderer.h"
#include "Public/Manager/Path/PathManager.h"

IMPLEMENT_SINGLETON(FObjManager)

FObjManager::FObjManager() = default;
FObjManager::~FObjManager()
{
	// StaticMeshAssetMap 정리
	for (auto& Pair : StaticMeshAssetMap)
	{
		if (Pair.second)
		{
			delete Pair.second;
			Pair.second = nullptr;
		}
	}
	StaticMeshAssetMap.Empty();

	// StaticMeshMap 정리
	for (auto& Pair : StaticMeshMap)
	{
		if (Pair.second)
		{
			delete Pair.second;
			Pair.second = nullptr;
		}
	}
	StaticMeshMap.Empty();

	// TextureCache 정리
	for (auto& Pair : TextureCache)
	{
		if (Pair.second)
		{
			Pair.second->Release();   // DirectX COM 객체 해제
			Pair.second = nullptr;
		}
	}
	TextureCache.Empty();
}

// 이걸로 호출 되긴 함 
FStaticMesh* FObjManager::LoadObjStaticMeshAsset(const FString& PathFileName)
{
	auto It = StaticMeshAssetMap.find(PathFileName);
	if (It != StaticMeshAssetMap.end())
	{
		// Already Cached
		//UE_LOG("FStaticMesh asset '%s' found in cache.", PathFileName.c_str());
		return It->second;
	}

	FStaticMesh* NewStaticMesh = FObjImporter::ParseAndConvert(PathFileName);

	// vertex  & Index 
	CreateVertexBuffer(NewStaticMesh);
	CreateIndexBuffer(NewStaticMesh);

	// texture
	CreateTextureBuffer(NewStaticMesh);

	if (NewStaticMesh)
	{
		StaticMeshAssetMap[PathFileName] = NewStaticMesh;
		UE_LOG("Successfully loaded FStaticMesh asset: %s", PathFileName.c_str());
		UE_LOG(" - Vertices: %d, Indices: %d", NewStaticMesh->Vertices.Num(), NewStaticMesh->Indices.Num() / 3);
		UE_LOG(" - Materials: %d, Sections: %d", NewStaticMesh->Materials.Num(), NewStaticMesh->Sections.Num());
	}
	else
	{
		UE_LOG("Failed to load FStaticMesh asset: %s", PathFileName.c_str());
	}

	return NewStaticMesh;
}

//void FObjManager::Initialize(ID3D11Device* InDevice)
//{
//	Device = InDevice;
//}

UStaticMesh* FObjManager::LoadObjStaticMesh(path Path)
{
	FString PathFileName = Path.string();
	if (StaticMeshMap.Contains(PathFileName))
	{
		UStaticMesh* CachedMesh = StaticMeshMap[PathFileName];
		if (CachedMesh)
		{
			//UE_LOG("UStaticMesh object '%s' found in cache.", PathFileName.c_str());
			return CachedMesh;
		}
		StaticMeshMap.Remove(PathFileName);
	}

	FStaticMesh* StaticMeshAsset = LoadObjStaticMeshAsset(PathFileName);
	if (!StaticMeshAsset)
	{
		UE_LOG("Failed to generate UStaticMesh for '%s' due to failed asset loading.", PathFileName.c_str());
		return nullptr;
	}

	// Generate UStaticMesh
	UStaticMesh* NewStaticMesh = NewObject<UStaticMesh>();
	if (NewStaticMesh)
	{
		NewStaticMesh->SetStaticMeshAsset(StaticMeshAsset);
		UE_LOG("Successfully generated UStaticMesh for '%s'.", PathFileName.c_str());
	}
	else
	{
		UE_LOG("Failed to create UStaticMesh object for '%s'.", PathFileName.c_str());
	}

	StaticMeshMap.Add(PathFileName, NewStaticMesh);

	return NewStaticMesh;
}

void FObjManager::CreateVertexBuffer(FStaticMesh* OutStaticMesh)
{
	if (OutStaticMesh->Vertices.empty()) return;
	ID3D11Device* Device = URenderer::GetInstance().GetDevice();

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = static_cast<UINT>(sizeof(FNormalVertex) * OutStaticMesh->Vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vinitData = {};
	vinitData.pSysMem = OutStaticMesh->Vertices.data();

	HRESULT hr = Device->CreateBuffer(&vbd, &vinitData, &OutStaticMesh->VertexBuffer);

	OutStaticMesh->VertexCount = static_cast<uint32>(OutStaticMesh->Vertices.size());
	OutStaticMesh->ByteWidth = vbd.ByteWidth;
}


void FObjManager::CreateIndexBuffer(FStaticMesh* OutStaticMesh)
{
	if (OutStaticMesh->Indices.empty()) return;
	ID3D11Device* Device = URenderer::GetInstance().GetDevice();


	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = static_cast<UINT>(sizeof(uint32) * OutStaticMesh->Indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData = {};
	iinitData.pSysMem = OutStaticMesh->Indices.data();

	HRESULT hr = Device->CreateBuffer(&ibd, &iinitData, &OutStaticMesh->IndexBuffer);
	if (FAILED(hr))
	{
		return;
	}

	OutStaticMesh->IndexCount = static_cast<uint32>(OutStaticMesh->Indices.size());
}

void FObjManager::CreateTextureBuffer(FStaticMesh* OutStaticMesh)
{
	if (!OutStaticMesh) return;

	ID3D11Device* Device = URenderer::GetInstance().GetDevice();
	ID3D11DeviceContext* Context = URenderer::GetInstance().GetDeviceContext();

	for (FStaticMaterial& Mat : OutStaticMesh->Materials)
	{
		if (!Mat.DiffusePath.empty())
		{
			Mat.TextureSRV = LoadTexture(Device, Context, Mat.DiffusePath);
			Mat.bUseTexture = true;
		}
	}
}

ID3D11ShaderResourceView* FObjManager::LoadTexture(ID3D11Device* Device, ID3D11DeviceContext* Context, const FString& FilePath)
{
	const path TexturePath = UPathManager::GetInstance().GetDataPath() / FilePath;
	// 1) 캐시에 있는지 확인
	auto It = TextureCache.find(FilePath);
	if (It != TextureCache.end())
	{
		return It->second; // 이미 있으면 재사용
	}

	// 2) 새로 로드
	ID3D11ShaderResourceView* SRV = nullptr;
	HRESULT hr = DirectX::CreateWICTextureFromFile(
		Device,
		Context,
		TexturePath.wstring().c_str(),
		nullptr,
		&SRV
	);
	// 3) 캐시에 저장 후 반환
	TextureCache[FilePath] = SRV;

	return SRV;
}
