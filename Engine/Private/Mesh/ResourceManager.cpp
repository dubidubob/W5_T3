#include "pch.h"
#include "Mesh/ResourceManager.h"
#include "Mesh/VertexDatas.h"
#include "Render/Renderer/Renderer.h"

IMPLEMENT_CLASS(UResourceManager, UObject)
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
	Vertexbuffers.emplace(EPrimitiveType::Cube, Renderer.CreateVertexBuffer(VerticesCube));
	Vertexbuffers.emplace(EPrimitiveType::Sphere, Renderer.CreateVertexBuffer(VerticesSphere));
	Vertexbuffers.emplace(EPrimitiveType::Triangle, Renderer.CreateVertexBuffer(VerticesTriangle));
	Vertexbuffers.emplace(EPrimitiveType::Square, Renderer.CreateVertexBuffer(VerticesSquare));
	Vertexbuffers.emplace(EPrimitiveType::Torus, Renderer.CreateVertexBuffer(VerticesTorus));
	Vertexbuffers.emplace(EPrimitiveType::Arrow, Renderer.CreateVertexBuffer(VerticesArrow));
	Vertexbuffers.emplace(EPrimitiveType::CubeArrow, Renderer.CreateVertexBuffer(VerticesCubeArrow));
	Vertexbuffers.emplace(EPrimitiveType::Ring, Renderer.CreateVertexBuffer(VerticesRing));
	Vertexbuffers.emplace(EPrimitiveType::Line, Renderer.CreateVertexBuffer(VerticesLine));

	NumVertices.emplace(EPrimitiveType::Cube, static_cast<uint32>(VerticesCube.size()));
	NumVertices.emplace(EPrimitiveType::Sphere, static_cast<uint32>(VerticesSphere.size()));
	NumVertices.emplace(EPrimitiveType::Triangle, static_cast<uint32>(VerticesTriangle.size()));
	NumVertices.emplace(EPrimitiveType::Square, static_cast<uint32>(VerticesSquare.size()));
	NumVertices.emplace(EPrimitiveType::Torus, static_cast<uint32>(VerticesTorus.size()));
	NumVertices.emplace(EPrimitiveType::Arrow, static_cast<uint32>(VerticesArrow.size()));
	NumVertices.emplace(EPrimitiveType::CubeArrow, static_cast<uint32>(VerticesCubeArrow.size()));
	NumVertices.emplace(EPrimitiveType::Ring, static_cast<uint32>(VerticesRing.size()));
	NumVertices.emplace(EPrimitiveType::Line, static_cast<uint32>(VerticesLine.size()));

	TextVertexData = &VerticesText;
	TextVertexBuffer = Renderer.CreateVertexBuffer(VerticesText);
	TextNumVertices = static_cast<uint32>(VerticesText.size());

	CreateTextSampler();	
}

void UResourceManager::Release()
{
	URenderer& Renderer = URenderer::GetInstance();
	//TMap.Value()
	for (auto& Pair : Vertexbuffers)
	{
		Renderer.ReleaseVertexBuffer(Pair.second);
	}
	Renderer.ReleaseVertexBuffer(TextVertexBuffer);

	//TMap.Empty()
	Vertexbuffers.clear();

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
