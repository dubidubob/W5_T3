#include "pch.h"
#include "Mesh/ResourceManager.h"
#include "Mesh/VertexDatas.h"
#include "Render/Renderer/Renderer.h"

IMPLEMENT_SINGLETON(UResourceManager)

UResourceManager::UResourceManager() = default;

UResourceManager::~UResourceManager() = default;

void UResourceManager::Initialize()
{
	URenderer& Renderer = URenderer::GetInstance();
	//TMap.Add()
	VertexDatas.emplace(EPrimitiveType::Cube, &VerticesCube);
	VertexDatas.emplace(EPrimitiveType::Sphere, &VerticesSphere);
	VertexDatas.emplace(EPrimitiveType::Triangle, &VerticesTriangle);
	VertexDatas.emplace(EPrimitiveType::Square, &VerticesSquare);
	VertexDatas.emplace(EPrimitiveType::Torus, &VerticesTorus);
	VertexDatas.emplace(EPrimitiveType::Arrow, &VerticesArrow);
	VertexDatas.emplace(EPrimitiveType::CubeArrow, &VerticesCubeArrow);
	VertexDatas.emplace(EPrimitiveType::Ring, &VerticesRing);
	VertexDatas.emplace(EPrimitiveType::Line, &VerticesLine);

	//TArray.GetData(), TArray.Num()*sizeof(FVertexSimple), TArray.GetTypeSize()
	Vertexbuffers.emplace(EPrimitiveType::Cube, Renderer.CreateVertexBuffer(
		VerticesCube.data(), static_cast<int>(VerticesCube.size()) * sizeof(FVertex)));
	Vertexbuffers.emplace(EPrimitiveType::Sphere, Renderer.CreateVertexBuffer(
		VerticesSphere.data(), static_cast<int>(VerticesSphere.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::Triangle, Renderer.CreateVertexBuffer(
		VerticesTriangle.data(), static_cast<int>(VerticesTriangle.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::Square, Renderer.CreateVertexBuffer(
		VerticesSquare.data(), static_cast<int>(VerticesSquare.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::Torus, Renderer.CreateVertexBuffer(
		VerticesTorus.data(), static_cast<int>(VerticesTorus.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::Arrow, Renderer.CreateVertexBuffer(
		VerticesArrow.data(), static_cast<int>(VerticesArrow.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::CubeArrow, Renderer.CreateVertexBuffer(
		VerticesCubeArrow.data(), static_cast<int>(VerticesCubeArrow.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::Ring, Renderer.CreateVertexBuffer(
		VerticesRing.data(), static_cast<int>(VerticesRing.size() * sizeof(FVertex))));
	Vertexbuffers.emplace(EPrimitiveType::Line, Renderer.CreateVertexBuffer(
		VerticesLine.data(), static_cast<int>(VerticesLine.size() * sizeof(FVertex))));

	NumVertices.emplace(EPrimitiveType::Cube, static_cast<uint32>(VerticesCube.size()));
	NumVertices.emplace(EPrimitiveType::Sphere, static_cast<uint32>(VerticesSphere.size()));
	NumVertices.emplace(EPrimitiveType::Triangle, static_cast<uint32>(VerticesTriangle.size()));
	NumVertices.emplace(EPrimitiveType::Square, static_cast<uint32>(VerticesSquare.size()));
	NumVertices.emplace(EPrimitiveType::Torus, static_cast<uint32>(VerticesTorus.size()));
	NumVertices.emplace(EPrimitiveType::Arrow, static_cast<uint32>(VerticesArrow.size()));
	NumVertices.emplace(EPrimitiveType::CubeArrow, static_cast<uint32>(VerticesCubeArrow.size()));
	NumVertices.emplace(EPrimitiveType::Ring, static_cast<uint32>(VerticesRing.size()));
	NumVertices.emplace(EPrimitiveType::Line, static_cast<uint32>(VerticesLine.size()));
}

void UResourceManager::Release()
{
	URenderer& Renderer = URenderer::GetInstance();
	//TMap.Value()
	for (auto& Pair : Vertexbuffers)
	{
		Renderer.ReleaseVertexBuffer(Pair.second);
	}
	//TMap.Empty()
	Vertexbuffers.clear();

	for (auto& Pair : SamplerStates)
	{
		Renderer.ReleaseSamplerState(Pair.second);
	}
	SamplerStates.clear();
	for (auto& Pair : ShaderResourceViews)
	{
		Renderer.ReleaseTexture(Pair.second);
	}
	ShaderResourceViews.clear();
}

TArray<FVertex>* UResourceManager::GetVertexData(EPrimitiveType Type)
{
	return VertexDatas[Type];
}

ID3D11Buffer* UResourceManager::GetVertexbuffer(EPrimitiveType Type)
{
	return Vertexbuffers[Type];
}

uint32 UResourceManager::GetNumVertices(EPrimitiveType Type)
{
	return NumVertices[Type];
}

ID3D11ShaderResourceView* UResourceManager::LoadTexture(const FString& Path)
{
	URenderer& Renderer = URenderer::GetInstance();
	ID3D11Device* Device = Renderer.GetDevice();
	const wstring WidePath = StringToWideString(Path);

	ID3D11Resource* Texture;
	ID3D11ShaderResourceView* NewResourceView;
	HRESULT Hr = DirectX::CreateDDSTextureFromFile(Device, WidePath.c_str(), &Texture, &NewResourceView);


	ID3D11SamplerState* SamplerState = nullptr;
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	SamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerDesc.MinLOD = 0;
	SamplerDesc.MaxLOD = 2.0;

	Device->CreateSamplerState(&SamplerDesc, &SamplerState);

	SamplerStates.emplace(ESamplerType::Text, SamplerState);

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

	FCharacterInfo ResultTable[NumCharSet];


	const char CharSet[NumCharSet + 1] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


	for (int Index = 0; Index < 95; Index++)
	{
		int Key = CharSet[Index];

		int Row = Index / CellsPerRow;
		int Col = Index % CellsPerRow;

		FCharacterInfo Info;
		Info.U = Col * CellWidth / BitMapWidth;
		Info.V = Row * CellHeight / BitMapHeight;
		Info.Width = CellWidth / BitMapWidth;
		Info.Height = CellHeight / BitMapHeight;

		
		//CharInfoMap[Index] = Info;
	}

	return ResultTable;
}
