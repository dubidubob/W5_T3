#include "pch.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Render/Renderer/Renderer.h"
#include "Render/Renderer/Pipeline.h"
#include "Global/Enum.h"

IMPLEMENT_CLASS(ULineBatchRenderer, UObject)
IMPLEMENT_SINGLETON(ULineBatchRenderer)

ULineBatchRenderer::ULineBatchRenderer() = default;
ULineBatchRenderer::~ULineBatchRenderer() = default;

void ULineBatchRenderer::Init()
{
    CreateBuffers();
    CreateAABBResources();

	/** 배칭 데이터 예약 */
	BatchedVertices.reserve(MaxVertices);
	BatchedIndices.reserve(MaxIndices);
}

void ULineBatchRenderer::Release()
{
    ReleaseBuffers();
    ReleaseAABBResources();
    BatchedVertices.clear();
    BatchedIndices.clear();
    AABBWorlds.clear();
    AABBColors.clear();
}

void ULineBatchRenderer::CreateBuffers()
{
	URenderer& Renderer = URenderer::GetInstance();
	ID3D11Device* Device = Renderer.GetDevice();

	/** 동적 Vertex Buffer 생성 */
	{
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.ByteWidth = MaxVertices * sizeof(FVertex);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		HRESULT HR = Device->CreateBuffer(&BufferDesc, nullptr, &DynamicVertexBuffer);
		if (FAILED(HR))
		{
			UE_LOG("Failed to create dynamic vertex buffer for LineBatchRenderer");
		}
	}

	/** 동적 Index Buffer 생성 */
	{
		D3D11_BUFFER_DESC BufferDesc = {};
		BufferDesc.ByteWidth = MaxIndices * sizeof(uint32);
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		HRESULT HR = Device->CreateBuffer(&BufferDesc, nullptr, &DynamicIndexBuffer);
		if (FAILED(HR))
		{
			UE_LOG("Failed to create dynamic index buffer for LineBatchRenderer");
		}
	}
}

void ULineBatchRenderer::ReleaseBuffers()
{
	if (DynamicVertexBuffer)
	{
		DynamicVertexBuffer->Release();
		DynamicVertexBuffer = nullptr;
	}

	if (DynamicIndexBuffer)
	{
		DynamicIndexBuffer->Release();
		DynamicIndexBuffer = nullptr;
	}
}

void ULineBatchRenderer::BeginBatch()
{
	if (bBatchStarted)
	{
		UE_LOG("Warning: BeginBatch called while batch already started");
		return;
	}

    ClearBatch();
    bBatchStarted = true;
}

void ULineBatchRenderer::ClearBatch()
{
    BatchedVertices.clear();
    BatchedIndices.clear();
    CurrentVertexCount = 0;
    CurrentIndexCount = 0;

    // AABB instances도 함께 초기화
    AABBWorlds.clear();
    AABBColors.clear();
}

void ULineBatchRenderer::FlushBatch()
{
	if (!bBatchStarted)
	{
		UE_LOG("Warning: FlushBatch called without BeginBatch");
		return;
	}

	if (CurrentVertexCount == 0)
	{
		bBatchStarted = false;
		return;
	}

	/** 버퍼 업데이트 및 렌더링 */
    UpdateVertexBuffer();
    UpdateIndexBuffer();
    RenderBatch();

    // AABB 인스턴싱 렌더링
    if (!AABBWorlds.empty())
    {
        UpdateAABBResources();
        RenderAABBInstances();
    }

	/** 배치 종료 */
	bBatchStarted = false;
}

void ULineBatchRenderer::AddLine(const FVector& Start, const FVector& End, const FVector4& Color)
{
	if (!bBatchStarted)
	{
		UE_LOG("Warning: AddLine called without BeginBatch");
		return;
	}

	if (!CanAddVertices(2))
	{
		UE_LOG("Warning: Line batch is full, cannot add more lines");
		return;
	}

	/** 시작점과 끝점 정점 추가 */
	BatchedVertices.push_back({Start, Color});
	BatchedVertices.push_back({End, Color});

	/** 라인 인덱스 생성 (2개 정점) */
	GenerateLineIndices(CurrentVertexCount, 2);
	CurrentVertexCount += 2;
}

void ULineBatchRenderer::AddLines(const TArray<FVertex>& Vertices)
{
	if (!bBatchStarted)
	{
		UE_LOG("Warning: AddLines called without BeginBatch");
		return;
	}

	uint32 VertexCount = static_cast<uint32>(Vertices.size());
	if (!CanAddVertices(VertexCount))
	{
		UE_LOG("Warning: Cannot add %d vertices to line batch (current: %d, max: %d)",
			VertexCount, CurrentVertexCount, MaxVertices);
		return;
	}

	/** 정점 데이터 추가 */
	for (const FVertex& Vertex : Vertices)
	{
		BatchedVertices.push_back(Vertex);
	}

	/** 라인 인덱스 생성 (정점 2개씩 페어) */
	GenerateLineIndices(CurrentVertexCount, VertexCount);
	CurrentVertexCount += VertexCount;
}

void ULineBatchRenderer::AddLineStrip(const TArray<FVertex>& Vertices, bool bClosed)
{
	if (!bBatchStarted)
	{
		UE_LOG("Warning: AddLineStrip called without BeginBatch");
		return;
	}

	uint32 VertexCount = static_cast<uint32>(Vertices.size());
	if (VertexCount < 2)
	{
		/** 최소 2개 정점 필요 */
		return;
	}

	if (!CanAddVertices(VertexCount))
	{
		UE_LOG("Warning: Cannot add %d vertices to line batch", VertexCount);
		return;
	}

	/** 정점 데이터 추가 */
	for (const FVertex& Vertex : Vertices)
	{
		BatchedVertices.push_back(Vertex);
	}

	/** 라인 스트립 인덱스 생성 */
	GenerateLineStripIndices(CurrentVertexCount, VertexCount, bClosed);
	CurrentVertexCount += VertexCount;
}

void ULineBatchRenderer::AddAABB(const FVector& Min, const FVector& Max, const FVector4& Color)
{
    if (!bBatchStarted)
    {
        UE_LOG("Warning: AddAABB called without BeginBatch");
        return;
    }

    // 유닛 박스 기준 [-0.5, +0.5]^3 을 사용
    // World = Scale(size) · Translate(center)
    FVector Size = Max - Min;
    FVector Center = (Min + Max) * 0.5f;

    FMatrix S = FMatrix::ScaleMatrix(Size);
    FMatrix T = FMatrix::TranslationMatrix(Center);
    FMatrix World = S * T; // row_major, pos=mul(pos, World) 가정

    if (AABBWorlds.size() >= MaxAABBInstances)
    {
        UE_LOG("Warning: exceeded MaxAABBInstances, skipping AABB");
        return;
    }

    AABBWorlds.push_back(World);
    AABBColors.push_back(Color);
}

void ULineBatchRenderer::UpdateVertexBuffer()
{
	if (!DynamicVertexBuffer || BatchedVertices.empty())
		return;

	URenderer& Renderer = URenderer::GetInstance();
	ID3D11DeviceContext* DeviceContext = Renderer.GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE MappedResource = {};
	HRESULT hr = DeviceContext->Map(DynamicVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

	if (SUCCEEDED(hr))
	{
		memcpy(MappedResource.pData, BatchedVertices.data(), CurrentVertexCount * sizeof(FVertex));
		DeviceContext->Unmap(DynamicVertexBuffer, 0);
	}
}

void ULineBatchRenderer::UpdateIndexBuffer()
{
	if (!DynamicIndexBuffer || BatchedIndices.empty())
		return;

	URenderer& Renderer = URenderer::GetInstance();
	ID3D11DeviceContext* DeviceContext = Renderer.GetDeviceContext();

	D3D11_MAPPED_SUBRESOURCE MappedResource = {};
	HRESULT hr = DeviceContext->Map(DynamicIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

	if (SUCCEEDED(hr))
	{
		memcpy(MappedResource.pData, BatchedIndices.data(), CurrentIndexCount * sizeof(uint32));
		DeviceContext->Unmap(DynamicIndexBuffer, 0);
	}
}

void ULineBatchRenderer::RenderBatch()
{
	URenderer& Renderer = URenderer::GetInstance();

	// 라인 렌더링용 렌더 상태 설정
	FRenderState LineRenderState;
	LineRenderState.CullMode = ECullMode::None;
	LineRenderState.FillMode = EFillMode::Solid;

	// EditorPrimitive 생성하여 기존 렌더링 파이프라인 재사용
	FEditorPrimitive LinePrimitive;
	LinePrimitive.Vertexbuffer = DynamicVertexBuffer;
	LinePrimitive.NumVertices = CurrentVertexCount;
	LinePrimitive.Topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	// Preserve per-vertex colors by disabling constant color override (alpha = 0)
	LinePrimitive.Color = FVector4(1.0f, 1.0f, 1.0f, 0.0f);
	LinePrimitive.Location = FVector::ZeroVector;
	LinePrimitive.Rotation = FVector::ZeroVector;
	LinePrimitive.Scale = FVector::OneVector;
	LinePrimitive.RenderState = LineRenderState;
	LinePrimitive.bShouldAlwaysVisible = false;

	// 기존 RenderPrimitive 사용 (인덱스 버퍼 미사용 버전)
	Renderer.RenderEditorPrimitive(LinePrimitive, LineRenderState);
}

void ULineBatchRenderer::GenerateLineIndices(uint32 StartVertex, uint32 VertexCount)
{
	// 정점 2개씩 페어로 라인 생성
	for (uint32 i = 0; i < VertexCount; i += 2)
	{
		if (i + 1 < VertexCount)
		{
			BatchedIndices.push_back(StartVertex + i);
			BatchedIndices.push_back(StartVertex + i + 1);
			CurrentIndexCount += 2;
		}
	}
}

void ULineBatchRenderer::GenerateLineStripIndices(uint32 StartVertex, uint32 VertexCount, bool bClosed)
{
	// 연속된 정점들을 라인으로 연결
	for (uint32 i = 0; i < VertexCount - 1; ++i)
	{
		BatchedIndices.push_back(StartVertex + i);
		BatchedIndices.push_back(StartVertex + i + 1);
		CurrentIndexCount += 2;
	}

	// 닫힌 형태라면 마지막 정점을 첫 정점과 연결
	if (bClosed && VertexCount > 2)
	{
		BatchedIndices.push_back(StartVertex + VertexCount - 1);
		BatchedIndices.push_back(StartVertex);
		CurrentIndexCount += 2;
	}
}

void ULineBatchRenderer::CreateAABBResources()
{
    URenderer& Renderer = URenderer::GetInstance();
    ID3D11Device* Device = Renderer.GetDevice();

    // 1) 유닛 AABB 라인 정점(24개) 생성: [-0.5, +0.5]^3 기준 엣지 쌍
    {
        TArray<FVector> UnitPositions;
        UnitPositions.reserve(24);

        const float h = 0.5f;
        FVector c000(-h, -h, -h);
        FVector c100( h, -h, -h);
        FVector c110( h,  h, -h);
        FVector c010(-h,  h, -h);
        FVector c001(-h, -h,  h);
        FVector c101( h, -h,  h);
        FVector c111( h,  h,  h);
        FVector c011(-h,  h,  h);

        auto push = [&](const FVector& a, const FVector& b){ UnitPositions.push_back(a); UnitPositions.push_back(b); };

        // 뒤면 사각형
        push(c000, c100); push(c100, c110); push(c110, c010); push(c010, c000);
        // 앞면 사각형
        push(c001, c101); push(c101, c111); push(c111, c011); push(c011, c001);
        // 수직 엣지
        push(c000, c001); push(c100, c101); push(c110, c111); push(c010, c011);

        UnitAABBVertexBuffer = Renderer.CreateVertexBuffer(UnitPositions);
    }

    // 2) 인스턴스 버퍼(Color만 보관, FLOAT4), DYNAMIC
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(FVector4) * MaxAABBInstances;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        HRESULT hr = Device->CreateBuffer(&desc, nullptr, &AABBInstanceBuffer);
        if (FAILED(hr))
        {
            UE_LOG("Failed to create AABB InstanceBuffer");
        }
    }

    // 3) StructuredBuffer(FMatrix) + SRV, DYNAMIC
    {
        D3D11_BUFFER_DESC sbDesc = {};
        sbDesc.ByteWidth = sizeof(FMatrix) * MaxAABBInstances;
        sbDesc.Usage = D3D11_USAGE_DYNAMIC;
        sbDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        sbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        sbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        sbDesc.StructureByteStride = sizeof(FMatrix);

        HRESULT hr = Device->CreateBuffer(&sbDesc, nullptr, &AABBWorldStructuredBuffer);
        if (FAILED(hr))
        {
            UE_LOG("Failed to create AABB World StructuredBuffer");
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement = 0;
        srvDesc.BufferEx.NumElements = MaxAABBInstances;
        srvDesc.BufferEx.Flags = 0;

        hr = Device->CreateShaderResourceView(AABBWorldStructuredBuffer, &srvDesc, &AABBWorldSRV);
        if (FAILED(hr))
        {
            UE_LOG("Failed to create AABB World SRV");
        }
    }
}

void ULineBatchRenderer::ReleaseAABBResources()
{
    if (UnitAABBVertexBuffer)
    {
        UnitAABBVertexBuffer->Release();
        UnitAABBVertexBuffer = nullptr;
    }
    if (AABBInstanceBuffer)
    {
        AABBInstanceBuffer->Release();
        AABBInstanceBuffer = nullptr;
    }
    if (AABBWorldSRV)
    {
        AABBWorldSRV->Release();
        AABBWorldSRV = nullptr;
    }
    if (AABBWorldStructuredBuffer)
    {
        AABBWorldStructuredBuffer->Release();
        AABBWorldStructuredBuffer = nullptr;
    }
}

void ULineBatchRenderer::UpdateAABBResources()
{
    URenderer& Renderer = URenderer::GetInstance();
    ID3D11DeviceContext* Context = Renderer.GetDeviceContext();

    if (AABBInstanceBuffer && !AABBColors.empty())
    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        if (SUCCEEDED(Context->Map(AABBInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr)))
        {
            memcpy(msr.pData, AABBColors.data(), sizeof(FVector4) * AABBColors.size());
            Context->Unmap(AABBInstanceBuffer, 0);
        }
    }

    if (AABBWorldStructuredBuffer && !AABBWorlds.empty())
    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        if (SUCCEEDED(Context->Map(AABBWorldStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr)))
        {
            memcpy(msr.pData, AABBWorlds.data(), sizeof(FMatrix) * AABBWorlds.size());
            Context->Unmap(AABBWorldStructuredBuffer, 0);
        }
    }
}

void ULineBatchRenderer::RenderAABBInstances()
{
    URenderer& Renderer = URenderer::GetInstance();
    UPipeline* Pipeline = Renderer.GetPipeline();

    // 라인용 상태
    FRenderState LineRenderState;
    LineRenderState.CullMode = ECullMode::None;
    LineRenderState.FillMode = EFillMode::Solid;

    ID3D11RasterizerState* RasterizerState = Renderer.GetRasterizerState(LineRenderState);

    FPipelineInfo Info = {
        Renderer.GetLineInstancedInputLayout(),
        Renderer.GetLineInstancedVertexShader(),
        RasterizerState,
        Renderer.GetDefaultDepthStencilState(),
        Renderer.GetLineInstancedPixelShader(),
        nullptr,
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST
    };

    Pipeline->UpdatePipeline(Info);

    // per-vertex: POSITION-only buffer (stride = sizeof(FVector))
    const uint32 StrideVertex = sizeof(FVector);
    Pipeline->SetVertexBuffer(UnitAABBVertexBuffer, StrideVertex);

    // per-instance: COLOR buffer
    const uint32 StrideInstance = sizeof(FVector4);
    Pipeline->SetInstanceBuffer(AABBInstanceBuffer, StrideInstance);

    // VS SRV t0: Worlds structured buffer
    Pipeline->SetShaderResourceView(0, true, AABBWorldSRV);

    // Draw
    Pipeline->DrawInstanced(24, static_cast<uint32>(AABBWorlds.size()), 0, 0);

    // Unbind SRV to avoid hazards
    ID3D11ShaderResourceView* NullSrv[1] = { nullptr };
    Renderer.GetDeviceContext()->VSSetShaderResources(0, 1, NullSrv);
}
