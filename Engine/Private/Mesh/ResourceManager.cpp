#include "pch.h"
#include "Mesh/ResourceManager.h"
#include "Mesh/VertexDatas.h"
#include "Render/Renderer/Renderer.h"

#include <ranges>

IMPLEMENT_CLASS(UResourceManager, UObject)
IMPLEMENT_SINGLETON(UResourceManager)

UResourceManager::UResourceManager() = default;

UResourceManager::~UResourceManager() = default;

void UResourceManager::Initialize()
{
	URenderer& Renderer = URenderer::GetInstance();
	// TMap.Add()
	VertexData.emplace(EPrimitiveType::Cube, &VerticesCube);
	VertexData.emplace(EPrimitiveType::Sphere, &VerticesSphere);
	VertexData.emplace(EPrimitiveType::Triangle, &VerticesTriangle);
	VertexData.emplace(EPrimitiveType::Square, &VerticesSquare);
	VertexData.emplace(EPrimitiveType::Torus, &VerticesTorus);
	VertexData.emplace(EPrimitiveType::Arrow, &VerticesArrow);
	VertexData.emplace(EPrimitiveType::CubeArrow, &VerticesCubeArrow);
	VertexData.emplace(EPrimitiveType::Ring, &VerticesRing);
	VertexData.emplace(EPrimitiveType::Line, &VerticesLine);

	// TArray.GetData(), TArray.Num()*sizeof(FVertexSimple), TArray.GetTypeSize()
	VertexBuffers.emplace(EPrimitiveType::Cube, Renderer.CreateVertexBuffer(VerticesCube));
	VertexBuffers.emplace(EPrimitiveType::Sphere, Renderer.CreateVertexBuffer(VerticesSphere));
	VertexBuffers.emplace(EPrimitiveType::Triangle, Renderer.CreateVertexBuffer(VerticesTriangle));
	VertexBuffers.emplace(EPrimitiveType::Square, Renderer.CreateVertexBuffer(VerticesSquare));
	VertexBuffers.emplace(EPrimitiveType::Torus, Renderer.CreateVertexBuffer(VerticesTorus));
	VertexBuffers.emplace(EPrimitiveType::Arrow, Renderer.CreateVertexBuffer(VerticesArrow));
	VertexBuffers.emplace(EPrimitiveType::CubeArrow, Renderer.CreateVertexBuffer(VerticesCubeArrow));
	VertexBuffers.emplace(EPrimitiveType::Ring, Renderer.CreateVertexBuffer(VerticesRing));
	VertexBuffers.emplace(EPrimitiveType::Line, Renderer.CreateVertexBuffer(VerticesLine));

	VertexNum.emplace(EPrimitiveType::Cube, static_cast<uint32>(VerticesCube.size()));
	VertexNum.emplace(EPrimitiveType::Sphere, static_cast<uint32>(VerticesSphere.size()));
	VertexNum.emplace(EPrimitiveType::Triangle, static_cast<uint32>(VerticesTriangle.size()));
	VertexNum.emplace(EPrimitiveType::Square, static_cast<uint32>(VerticesSquare.size()));
	VertexNum.emplace(EPrimitiveType::Torus, static_cast<uint32>(VerticesTorus.size()));
	VertexNum.emplace(EPrimitiveType::Arrow, static_cast<uint32>(VerticesArrow.size()));
	VertexNum.emplace(EPrimitiveType::CubeArrow, static_cast<uint32>(VerticesCubeArrow.size()));
	VertexNum.emplace(EPrimitiveType::Ring, static_cast<uint32>(VerticesRing.size()));
	VertexNum.emplace(EPrimitiveType::Line, static_cast<uint32>(VerticesLine.size()));

	TextVertexData = &VerticesText;
	TextVertexBuffer = Renderer.CreateVertexBuffer(VerticesText);
	TexVertexNum = static_cast<uint32>(VerticesText.size());

	CreateTextSampler();


	// Create Reduced Vertex Data and Index Data
	ReducedVertexData.emplace(EPrimitiveType::Cube, &ReducedVerticesCube);
	ReducedVertexData.emplace(EPrimitiveType::Sphere, &ReducedVerticesSphere);
	ReducedVertexData.emplace(EPrimitiveType::Triangle, &ReducedVerticesTriangle);
	ReducedVertexData.emplace(EPrimitiveType::Square, &ReducedVerticesSquare);
	ReducedVertexData.emplace(EPrimitiveType::Torus, &ReducedVerticesTorus);
	ReducedVertexData.emplace(EPrimitiveType::Arrow, &ReducedVerticesArrow);
	ReducedVertexData.emplace(EPrimitiveType::CubeArrow, &ReducedVerticesCubeArrow);
	ReducedVertexData.emplace(EPrimitiveType::Ring, &ReducedVerticesRing);
	ReducedVertexData.emplace(EPrimitiveType::Line, &ReducedVerticesLine);

	ReducedVertexBuffers.emplace(EPrimitiveType::Cube, Renderer.CreateVertexBuffer(ReducedVerticesCube));
	ReducedVertexBuffers.emplace(EPrimitiveType::Sphere, Renderer.CreateVertexBuffer(ReducedVerticesSphere));
	ReducedVertexBuffers.emplace(EPrimitiveType::Triangle, Renderer.CreateVertexBuffer(ReducedVerticesTriangle));
	ReducedVertexBuffers.emplace(EPrimitiveType::Square, Renderer.CreateVertexBuffer(ReducedVerticesSquare));
	ReducedVertexBuffers.emplace(EPrimitiveType::Torus, Renderer.CreateVertexBuffer(ReducedVerticesTorus));
	ReducedVertexBuffers.emplace(EPrimitiveType::Arrow, Renderer.CreateVertexBuffer(ReducedVerticesArrow));
	ReducedVertexBuffers.emplace(EPrimitiveType::CubeArrow, Renderer.CreateVertexBuffer(ReducedVerticesCubeArrow));
	ReducedVertexBuffers.emplace(EPrimitiveType::Ring, Renderer.CreateVertexBuffer(ReducedVerticesRing));
	ReducedVertexBuffers.emplace(EPrimitiveType::Line, Renderer.CreateVertexBuffer(ReducedVerticesLine));

	ReducedVertexNum.emplace(EPrimitiveType::Cube, static_cast<uint32>(ReducedVerticesCube.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Sphere, static_cast<uint32>(ReducedVerticesSphere.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Triangle, static_cast<uint32>(ReducedVerticesTriangle.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Square, static_cast<uint32>(ReducedVerticesSquare.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Torus, static_cast<uint32>(ReducedVerticesTorus.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Arrow, static_cast<uint32>(ReducedVerticesArrow.size()));
	ReducedVertexNum.emplace(EPrimitiveType::CubeArrow, static_cast<uint32>(ReducedVerticesCubeArrow.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Ring, static_cast<uint32>(ReducedVerticesRing.size()));
	ReducedVertexNum.emplace(EPrimitiveType::Line, static_cast<uint32>(ReducedVerticesLine.size()));


	// Create Index Data from Vertex Data and Reduced Vertex Data
	// IndexData = TMap<EPrimitiveType, TArray<uint32>>
	// Build index buffers by mapping each original vertex to its index in the reduced (unique) vertex list.
	// Compare by value (FVertex::operator==), not by pointer address.
	for (const auto& key : VertexData | std::views::keys)
	{
		EPrimitiveType Type = key;
		const TArray<FVertex>& SourceVertices = *VertexData[Type];
		const TArray<FVertex>& ReducedVertices = *ReducedVertexData[Type];

		IndexData[Type].clear();
		IndexData[Type].reserve(SourceVertices.size());

		for (const FVertex& Vertex : SourceVertices)
		{
			for (uint32 Index = 0; Index < ReducedVertices.size(); ++Index)
			{
				if (Vertex == ReducedVertices[Index])
				{
					IndexData[Type].push_back(Index);
					break;
				}
			}
		}
	}

	// Create Index Buffer from Index Data
	for (auto& Pair : IndexData)
	{
		IndexBuffers.emplace(Pair.first, Renderer.CreateIndexBuffer(Pair.second));
		IndexNum.emplace(Pair.first, static_cast<uint32>(IndexData[Pair.first].size()));
	}

}

void UResourceManager::Release()
{
	URenderer& Renderer = URenderer::GetInstance();
	//TMap.Value()
	for (auto& Pair : VertexBuffers)
	{
		Renderer.ReleaseVertexBuffer(Pair.second);
	}
	Renderer.ReleaseVertexBuffer(TextVertexBuffer);

	//TMap.Empty()
	VertexBuffers.clear();

	for (auto& Pair : SamplerStates)
	{
		Pair.second->Release();
	}
	SamplerStates.clear();
	for (auto& Pair : ShaderResourceViews)
	{
		Pair.second->Release();
	}
	ShaderResourceViews.clear();
}

TArray<FVertex>* UResourceManager::GetVertexData(EPrimitiveType Type)
{
	return VertexData[Type];
}

ID3D11Buffer* UResourceManager::GetVertexBuffer(EPrimitiveType Type)
{
	return VertexBuffers[Type];
}

uint32 UResourceManager::GetVertexNum(EPrimitiveType Type)
{
	return VertexNum[Type];
}

TArray<FVertex>* UResourceManager::GetReducedVertexData(EPrimitiveType Type)
{
	return ReducedVertexData[Type];
}

ID3D11Buffer* UResourceManager::GetReducedVertexBuffer(EPrimitiveType Type)
{
	return ReducedVertexBuffers[Type];
}

uint32 UResourceManager::GetReducedVertexNum(EPrimitiveType Type)
{
	return ReducedVertexNum[Type];
}

TArray<uint32>* UResourceManager::GetIndexData(EPrimitiveType Type)
{
	return &IndexData[Type];
}

ID3D11Buffer* UResourceManager::GetIndexBuffer(EPrimitiveType Type)
{
	return IndexBuffers[Type];
}

uint32 UResourceManager::GetIndexNum(EPrimitiveType Type)
{
	return IndexNum[Type];
}


void UResourceManager::CreateTextSampler()
{
	URenderer& Renderer = URenderer::GetInstance();
	ID3D11Device* Device = Renderer.GetDevice();

	ID3D11SamplerState* SamplerState = nullptr;
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = 0;

	Device->CreateSamplerState(&SamplerDesc, &SamplerState);

	SamplerStates.emplace(ESamplerType::Text, SamplerState);

}
ID3D11ShaderResourceView* UResourceManager::LoadTexture(const FString& Path)
{
	URenderer& Renderer = URenderer::GetInstance();
	ID3D11Device* Device = Renderer.GetDevice();
	const wstring WidePath = StringToWideString(Path);

	ID3D11Resource* Texture;
	ID3D11ShaderResourceView* NewResourceView;
	HRESULT Hr = DirectX::CreateDDSTextureFromFile(Device, WidePath.c_str(), &Texture, &NewResourceView);

	Texture->Release();

	return NewResourceView;
}

ID3D11ShaderResourceView* UResourceManager::GetTexture(const FString& Path)
{
	if (ShaderResourceViews.count(Path) > 0)
	{
		return ShaderResourceViews[Path];
	}
	else
	{
		ID3D11ShaderResourceView* NewResourceView = LoadTexture(Path);
		ShaderResourceViews.emplace(Path, NewResourceView);
		return NewResourceView;
	}

}

ID3D11SamplerState* UResourceManager::GetSamplerState(ESamplerType Type)
{
	return SamplerStates[Type];
}

FCharacterInfo* UResourceManager::LoadCharTable()
{
	const int NumCharSet = 95;
	const int CellWidth = 32;
	const int CellHeight = 64;
	const int CellsPerRow = 16;
	const int BitMapWidth = 512;
	const int BitMapHeight = 512;


	const char CharSet[NumCharSet + 1] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


	for (int Index = 0; Index < 95; Index++)
	{
		int Key = CharSet[Index];

		int Row = Index / CellsPerRow;
		int Col = Index % CellsPerRow;

		FCharacterInfo Info;
		Info.U = Col * CellWidth / (float)BitMapWidth;
		Info.V = Row * CellHeight / (float)BitMapHeight;
		Info.Width = CellWidth / (float)BitMapWidth;
		Info.Height = CellHeight / (float)BitMapHeight;


		CharTable[Index] = Info;
		//CharInfoMap[Index] = Info;
	}

	return CharTable;
}
