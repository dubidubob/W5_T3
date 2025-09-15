#include "pch.h"
#include "Render/AABBWireframeComponent.h"
#include "Render/Renderer/Renderer.h"

IMPLEMENT_CLASS(UAABBWireframeComponent, UPrimitiveComponent)

UAABBWireframeComponent::UAABBWireframeComponent()
{
	Type = EPrimitiveType::Cube;
	Topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::WireFrame;

	SetColor(FVector4(1.0f, 0.0f, 0.0f, 1.0f));

	GenerateWireframeVertices();
}

UAABBWireframeComponent::~UAABBWireframeComponent()
{
	if (Vertexbuffer)
	{
		Vertexbuffer->Release();
		Vertexbuffer = nullptr;
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
		NumVertices = 0;
		return;
	}

	WireframeVertices.clear();

	FVector Min = BoundingBox.Min;
	FVector Max = BoundingBox.Max;

	// AABB의 8개 꼭짓점
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

	// 12개 엣지를 위한 24개 버텍스 (각 엣지마다 2개 버텍스)
	int EdgeIndices[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 뒤쪽 면
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 앞쪽 면
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 연결 엣지들
	};

	for (int i = 0; i < 12; ++i)
	{
		FVector Start = Corners[EdgeIndices[i][0]];
		FVector End = Corners[EdgeIndices[i][1]];

		FVertex StartVertex = {};
		StartVertex.Position = FVector(Start.X, Start.Y, Start.Z);
		StartVertex.Color = FVector4(1.0f, 0.0f, 0.0f, 1.0f);

		FVertex EndVertex = {};
		EndVertex.Position = FVector(End.X, End.Y, End.Z);
		EndVertex.Color = FVector4(1.0f, 0.0f, 0.0f, 1.0f);

		WireframeVertices.push_back(StartVertex);
		WireframeVertices.push_back(EndVertex);
	}

	NumVertices = static_cast<uint32>(WireframeVertices.size());
	Vertices = &WireframeVertices;

	if (Vertexbuffer)
	{
		Vertexbuffer->Release();
		Vertexbuffer = nullptr;
	}

	URenderer& Renderer = URenderer::GetInstance();
	//Vertexbuffer = Renderer.CreateVertexBuffer(WireframeVertices.data(), NumVertices * sizeof(FVertex));
	Vertexbuffer = Renderer.CreateVertexBuffer(WireframeVertices);

	bNeedsUpdate = false;
}
