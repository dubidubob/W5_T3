#include "pch.h"
#include "Render/AABBWireframeComponent.h"
#include "Render/Renderer/Renderer.h"

IMPLEMENT_CLASS(UAABBWireframeComponent, UPrimitiveComponent)

UAABBWireframeComponent::UAABBWireframeComponent()
{
	Type = EPrimitiveType::Cube;
	Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::WireFrame;

	SetColor(FVector4(1.0f, 1.0f, 1.0f, 1.0f));

	GenerateWireframeVertices();
}

UAABBWireframeComponent::~UAABBWireframeComponent()
{
	if (Vertexbuffer)
	{
		Vertexbuffer->Release();
		Vertexbuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}
}

void UAABBWireframeComponent::SetAABB(const FAABB& InAABB)
{
	BoundingBox = InAABB;
	bNeedsUpdate = true;
	GenerateWireframeVertices();
}

FAABB UAABBWireframeComponent::GetLocalBounds() const
{
	return BoundingBox;
}

void UAABBWireframeComponent::GenerateWireframeVertices()
{
	if (!BoundingBox.IsValid())
	{
		WireframeVertices.clear();
		WireframeIndices.clear();
		NumVertices = 0;
		NumIndices = 0;
		return;
	}

	WireframeVertices.clear();
	WireframeIndices.clear();

	FVector Min = BoundingBox.Min;
	FVector Max = BoundingBox.Max;

	// AABB의 8개 꼭짓점 생성
	FVector Corners[8] = {
		FVector(Min.X, Min.Y, Min.Z), // 0: 왼쪽 아래 뒤
		FVector(Max.X, Min.Y, Min.Z), // 1: 오른쪽 아래 뒤
		FVector(Max.X, Max.Y, Min.Z), // 2: 오른쪽 위 뒤
		FVector(Min.X, Max.Y, Min.Z), // 3: 왼쪽 위 뒤
		FVector(Min.X, Min.Y, Max.Z), // 4: 왼쪽 아래 앞
		FVector(Max.X, Min.Y, Max.Z), // 5: 오른쪽 아래 앞
		FVector(Max.X, Max.Y, Max.Z), // 6: 오른쪽 위 앞
		FVector(Min.X, Max.Y, Max.Z)  // 7: 왼쪽 위 앞
	};

	// 8개 버텍스 생성
	for (int i = 0; i < 8; ++i)
	{
		FVertex Vertex = {};
		Vertex.Position = Corners[i];
		Vertex.Color = FVector4(1.0f, 0.0f, 0.0f, 1.0f);
		WireframeVertices.push_back(Vertex);
	}

	// 12개 엣지를 위한 인덱스 생성 (각 엣지마다 2개 인덱스)
	uint32 EdgeIndices[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 뒤쪽 면
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 앞쪽 면
		{0, 4}, {1, 5}, {2, 6}, {3, 7}, // 연결 엣지들
	};

	for (int i = 0; i < 12; ++i)
	{
		WireframeIndices.push_back(EdgeIndices[i][0]);
		WireframeIndices.push_back(EdgeIndices[i][1]);
	}

	NumVertices = static_cast<uint32>(WireframeVertices.size());
	NumIndices = static_cast<uint32>(WireframeIndices.size());
	Vertices = &WireframeVertices;

	// 기존 버퍼들 해제
	if (Vertexbuffer)
	{
		Vertexbuffer->Release();
		Vertexbuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	URenderer& Renderer = URenderer::GetInstance();
	Vertexbuffer = Renderer.CreateVertexBuffer(WireframeVertices);
	IndexBuffer = Renderer.CreateIndexBuffer(WireframeIndices.data(), NumIndices * sizeof(uint32));

	bNeedsUpdate = false;
}
