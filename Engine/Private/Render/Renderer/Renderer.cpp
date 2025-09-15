#include "pch.h"
#include "Render/Renderer/Renderer.h"

#include "Level/Level.h"

#include "Manager/Level/LevelManager.h"
#include "Manager/UI/UIManager.h"
#include "Mesh/Actor.h"
#include "Render/Renderer/Pipeline.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Editor/Editor.h"
#include "Mesh/TextComponent.h"
#include "Render/AABBWireframeComponent.h"

IMPLEMENT_CLASS(URenderer, UObject)
IMPLEMENT_SINGLETON(URenderer)

URenderer::URenderer() = default;

URenderer::~URenderer() = default;

void URenderer::Init(HWND InWindowHandle)
{
	DeviceResources = new UDeviceResources(InWindowHandle);
	Pipeline = new UPipeline(GetDeviceContext());

	/** 래스터라이저 상태 생성 */
	CreateRasterizerState();
	CreateDepthStencilState();
	CreateBlendState();
	CreateDefaultShader();
	CreateTextShader();
	CreateInstanceBuffer();

	CreateConstantBuffer();

	/** LineBatchRenderer 초기화 */
	ULineBatchRenderer::GetInstance().Init();
}

void URenderer::Release()
{
	/** LineBatchRenderer 해제 */
	ULineBatchRenderer::GetInstance().Release();

	ReleaseConstantBuffer();
	ReleaseDefaultShader();
	ReleaseResource();
	ReleaseTextShader();
	ReleaseInstanceBuffer();
	ReleaseBlendState();

	SafeDelete(Pipeline);
	SafeDelete(DeviceResources);
}

/**
 * @brief 래스터라이저 상태를 생성하는 함수
 */
void URenderer::CreateRasterizerState()
{

}

void URenderer::CreateDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC DescDefault = {};

	DescDefault.DepthEnable = TRUE;
	DescDefault.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	DescDefault.DepthFunc = D3D11_COMPARISON_LESS;

	DescDefault.StencilEnable = FALSE;
	DescDefault.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DescDefault.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	HRESULT hr = DeviceResources->GetDevice()->CreateDepthStencilState(
		&DescDefault,
		&DefaultDepthStencilState
	);

	D3D11_DEPTH_STENCIL_DESC descDisabled = {};

	descDisabled.DepthEnable = FALSE;

	DescDefault.StencilEnable = FALSE;
	DescDefault.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DescDefault.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	hr = DeviceResources->GetDevice()->CreateDepthStencilState(
		&descDisabled,
		&DisabledDepthStencilState
	);
}

void URenderer::CreateBlendState()
{
	D3D11_BLEND_DESC BlendDesc = {};

	D3D11_RENDER_TARGET_BLEND_DESC& RtBlendDesc = BlendDesc.RenderTarget[0];

	RtBlendDesc.BlendEnable = TRUE;

	RtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	RtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	RtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;

	RtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	RtBlendDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	RtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	RtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	GetDevice()->CreateBlendState(&BlendDesc, &TextBlendState);

}

/**
 * @brief 래스터라이저 상태를 해제하는 함수
 */
void URenderer::ReleaseRasterizerState()
{
	for (auto& Cache : RasterCache)
	{
		if (Cache.second != nullptr)
		{
			Cache.second->Release();
		}
	}
	RasterCache.clear();
}

/**
 * @brief 렌더러에 사용된 모든 리소스를 해제하는 함수
 */
void URenderer::ReleaseResource()
{
	for (auto& Cache : RasterCache)
	{
		if (Cache.second != nullptr)
		{
			Cache.second->Release();
		}
	}
	RasterCache.clear();

	if (DefaultDepthStencilState)
	{
		DefaultDepthStencilState->Release();
		DefaultDepthStencilState = nullptr;
	}

	if (DisabledDepthStencilState)
	{
		DisabledDepthStencilState->Release();
		DisabledDepthStencilState = nullptr;
	}

	/** 렌더 타겟을 초기화 */
	if (GetDeviceContext())
	{
		GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);
	}
}

/**
 * @brief Shader 기반의 CSO 생성 함수
 */
void URenderer::CreateDefaultShader()
{
	ID3DBlob* VertexShaderCSO;
	ID3DBlob* PixelShaderCSO;

	D3DCompileFromFile(L"Asset/Shader/SampleShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0,
	                   &VertexShaderCSO, nullptr);

	GetDevice()->CreateVertexShader(VertexShaderCSO->GetBufferPointer(),
	                           VertexShaderCSO->GetBufferSize(), nullptr, &DefaultVertexShader);

	D3DCompileFromFile(L"Asset/Shader/SampleShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0,
	                   &PixelShaderCSO, nullptr);

	GetDevice()->CreatePixelShader(PixelShaderCSO->GetBufferPointer(),
	                          PixelShaderCSO->GetBufferSize(), nullptr, &DefaultPixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(),
	                          VertexShaderCSO->GetBufferSize(), &DefaultInputLayout);

	Stride = sizeof(FVertex);

	VertexShaderCSO->Release();
	PixelShaderCSO->Release();
}

void URenderer::CreateTextShader()
{
	ID3DBlob* VertexShaderCSO;
	ID3DBlob* PixelShaderCSO;

	D3DCompileFromFile(L"Asset/Shader/TextShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0,
		&VertexShaderCSO, nullptr);

	GetDevice()->CreateVertexShader(VertexShaderCSO->GetBufferPointer(),
		VertexShaderCSO->GetBufferSize(), nullptr, &TextVertexShader);

	D3DCompileFromFile(L"Asset/Shader/TextShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0,
		&PixelShaderCSO, nullptr);

	GetDevice()->CreatePixelShader(PixelShaderCSO->GetBufferPointer(),
		PixelShaderCSO->GetBufferSize(), nullptr, &TextPixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA,1},
		{"OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA,1},
		{"TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA,1},
	};

	GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(),
		VertexShaderCSO->GetBufferSize(), &TextInputLayout);

	StrideTextVertex = sizeof(FTextVertex);
	StrideTextInstance = sizeof(FTextInstance);

	VertexShaderCSO->Release();
	PixelShaderCSO->Release();
}
/**
 * @brief Shader Release
 */
void URenderer::ReleaseDefaultShader()
{
	if (DefaultInputLayout)
	{
		DefaultInputLayout->Release();
		DefaultInputLayout = nullptr;
	}

	if (DefaultPixelShader)
	{
		DefaultPixelShader->Release();
		DefaultPixelShader = nullptr;
	}

	if (DefaultVertexShader)
	{
		DefaultVertexShader->Release();
		DefaultVertexShader = nullptr;
	}
}

void URenderer::ReleaseTextShader()
{
	if (TextInputLayout)
	{
		TextInputLayout->Release();
		TextInputLayout = nullptr;
	}

	if (TextPixelShader)
	{
		TextPixelShader->Release();
		TextPixelShader = nullptr;
	}

	if (TextVertexShader)
	{	
		TextVertexShader->Release();
		TextVertexShader = nullptr;
	}
}

void URenderer::ReleaseBlendState()
{
	TextBlendState->Release();
}

void URenderer::Update(UEditor* Editor)
{
	RenderBegin();

	RenderLevel();
	Editor->RenderEditor();
	RenderTest();

	//RenderLines();

	UUIManager::GetInstance().Render();

	RenderEnd();
}

/**
 * @brief Render Prepare Step
 */
void URenderer::RenderBegin()
{
	auto* rtv = DeviceResources->GetRenderTargetView();
	GetDeviceContext()->ClearRenderTargetView(rtv, ClearColor);
	auto* dsv = DeviceResources->GetDepthStencilView();
	GetDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	GetDeviceContext()->RSSetViewports(1, &DeviceResources->GetViewportInfo());

	ID3D11RenderTargetView* rtvs[] = { rtv };  // 배열 생성

	GetDeviceContext()->OMSetRenderTargets(1, rtvs, DeviceResources->GetDepthStencilView());
	DeviceResources->UpdateViewport();
}

/**
 * @brief Buffer에 데이터 입력 및 Draw
 */
void URenderer::RenderLevel()
{
	//
	// 여기에 카메라 VP 업데이트 한 번 싹
	//
	if (!ULevelManager::GetInstance().GetCurrentLevel())
		return;

	for (auto& PrimitiveComponent : ULevelManager::GetInstance().GetCurrentLevel()->GetLevelPrimitiveComponents())
	{
		// Check show flags for primitive components
		if (IsShowFlagEnabled(EEngineShowFlags::SF_Primitives) == false) { break; }

		if (!PrimitiveComponent) { continue; }
		
		Pipeline->UpdatePipeline(CreatePipelineInfo(PrimitiveComponent->GetRenderState()));

		Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
		UpdateConstant(
			PrimitiveComponent->GetRelativeLocation(),
			PrimitiveComponent->GetRelativeRotation(),
			PrimitiveComponent->GetRelativeScale3D() );

		Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
		UpdateConstant(PrimitiveComponent->GetColor());

		Pipeline->SetVertexBuffer(PrimitiveComponent->GetVertexBuffer(), Stride);
		Pipeline->Draw(static_cast<uint32>(PrimitiveComponent->GetVerticesData()->size()), 0);

		// Render bounding boxes if enabled
		if (IsShowFlagEnabled(EEngineShowFlags::SF_Bounds))
		{
			RenderBoundingBox(PrimitiveComponent);
		}
	}
}

void URenderer::RenderBoundingBox(UPrimitiveComponent* PrimitiveComponent)
{
	if (!PrimitiveComponent) return;

	static TMap<UPrimitiveComponent*, UAABBWireframeComponent*> WireframeCache;

	UAABBWireframeComponent* WireframeComponent = nullptr;

	if (WireframeCache.count(PrimitiveComponent))
	{
		WireframeComponent = WireframeCache[PrimitiveComponent];
	}
	else
	{
		WireframeComponent = new UAABBWireframeComponent();
		WireframeCache[PrimitiveComponent] = WireframeComponent;
	}

	FAABB WorldBounds = PrimitiveComponent->GetWorldBounds();
	if (!WorldBounds.IsValid())
	{
		return; // 유효하지 않은 바운딩 박스는 렌더링하지 않음
	}

	//FVector Center = WorldBounds.GetCenter();
	//FVector Size = WorldBounds.GetSize();

	WireframeComponent->SetAABB(WorldBounds);

	if (WireframeComponent->GetVertexBuffer() && WireframeComponent->GetIndexBuffer() && WireframeComponent->GetNumIndices() > 0)
	{
		// AABB Wireframe을 위한 특별한 PipelineInfo 생성
		ID3D11RasterizerState* RasterizerState = GetRasterizerState(WireframeComponent->GetRenderState());
		FPipelineInfo WireframePipelineInfo = {
			DefaultInputLayout, DefaultVertexShader,
			RasterizerState, DefaultDepthStencilState, DefaultPixelShader, nullptr,
			WireframeComponent->GetTopology()  // LINELIST 사용
		};
		Pipeline->UpdatePipeline(WireframePipelineInfo);

		Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
		UpdateConstant(FVector(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));

		Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
		UpdateConstant(WireframeComponent->GetColor());

		Pipeline->SetVertexBuffer(WireframeComponent->GetVertexBuffer(), Stride);
		Pipeline->SetIndexBuffer(WireframeComponent->GetIndexBuffer(), DXGI_FORMAT_R32_UINT);
		Pipeline->DrawIndexed(WireframeComponent->GetNumIndices(), 0, 0);
	}
}

void URenderer::RenderTest()
{
	if (IsShowFlagEnabled(EEngineShowFlags::SF_BillboardText) == false) { return; }

	FRenderState State = FRenderState{ ECullMode::None, EFillMode::Solid };
	Pipeline->UpdatePipeline(CreateTextPipelineInfo(State));

	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	ID3D11ShaderResourceView* Srv = ResourceManager.GetTexture("Asset/Font/Roboto-Medium.dds");
	ID3D11SamplerState* SamplerState = ResourceManager.GetSamplerState(ESamplerType::Text);

	Pipeline->SetTexture(0, false, Srv);
	Pipeline->SetSamplerState(0, false, SamplerState);

	for (UTextComponent* Component : ULevelManager::GetInstance().GetCurrentLevel()->GetTextComponents())
	{
		Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
		UpdateConstant(Component);
		Pipeline->SetVertexBuffer(Component->GetVertexBuffer(), StrideTextVertex);
		Pipeline->SetInstanceBuffer(TextInstanceBuffer, StrideTextInstance);

		TArray<FTextInstance>* InstanceData = Component->GetInstanceData();
		UpdateInstance(InstanceData);

		Pipeline->DrawInstanced(Component->GetNumVertices(), InstanceData->size(), 0, 0);
	}
}

/**
 * @brief 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
 */
void URenderer::RenderEnd() const
{
	GetSwapChain()->Present(0, 0); // 1: VSync 활성화
}

static inline D3D11_CULL_MODE ToD3D11(ECullMode InCull)
{
	switch (InCull) {
	case ECullMode::Back:
		return D3D11_CULL_BACK;
	case ECullMode::Front:
		return D3D11_CULL_FRONT;
	case ECullMode::None:
		return D3D11_CULL_NONE;
	default:
		return D3D11_CULL_BACK;
	}
}

static inline D3D11_FILL_MODE ToD3D11(EFillMode InFill)
{
	switch (InFill) {
	case EFillMode::Solid:
		return D3D11_FILL_SOLID;
	case EFillMode::WireFrame:
		return D3D11_FILL_WIREFRAME;
	default:
		return D3D11_FILL_SOLID;
	}
}

void URenderer::RenderPrimitive(FEditorPrimitive& Primitive, struct FRenderState& InRenderState)
{
	ID3D11DepthStencilState* DepthStencilState =
		Primitive.bShouldAlwaysVisible ? DisabledDepthStencilState : DefaultDepthStencilState;

	ID3D11RasterizerState* RasterizerState =
		GetRasterizerState(InRenderState);

	FPipelineInfo PipelineInfo = {
			DefaultInputLayout,
			DefaultVertexShader,
			RasterizerState,
			DepthStencilState,
			DefaultPixelShader,
			nullptr,
			Primitive.Topology
	};

	Pipeline->UpdatePipeline(PipelineInfo);

	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
	UpdateConstant(Primitive.Location, Primitive.Rotation, Primitive.Scale);

	Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
	UpdateConstant(Primitive.Color);

	Pipeline->SetVertexBuffer(Primitive.Vertexbuffer, Stride);
	Pipeline->Draw(Primitive.NumVertices, 0);
}


/**
 * @brief Index Buffer 생성 함수
 * @param InIndices
 * @param InByteWidth
 * @return
 */
ID3D11Buffer* URenderer::CreateIndexBuffer(const void* InIndices, uint32 InByteWidth) const
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = InByteWidth;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd = {};
	srd.pSysMem = InIndices;

	ID3D11Buffer* buffer = nullptr;
	GetDevice()->CreateBuffer(&desc, &srd, &buffer);
	return buffer;
}

void URenderer::CreateInstanceBuffer()
{
	uint32 InByteWidth = sizeof(FTextInstance)* 100;
	D3D11_BUFFER_DESC InstanceBufferDesc = {};
	InstanceBufferDesc.ByteWidth = InByteWidth;
	InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;


	GetDevice()->CreateBuffer(&InstanceBufferDesc, nullptr, &TextInstanceBuffer);
}

void URenderer::ReleaseInstanceBuffer()
{
	TextInstanceBuffer->Release();
}

void URenderer::OnResize(uint32 InWidth, uint32 InHeight)
{
	if (!DeviceResources || !GetDevice() || !GetDeviceContext() || !GetSwapChain()) return;

	DeviceResources->ReleaseFrameBuffer();
	DeviceResources->ReleaseDepthBuffer();
	GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);

	// ResizeBuffers 호출
	HRESULT hr = GetSwapChain()->ResizeBuffers(2, InWidth, InHeight, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
	{
		UE_LOG("OnResize Failed");
		return;
	}
	DeviceResources->UpdateViewport();

	DeviceResources->CreateFrameBuffer();
	DeviceResources->CreateDepthBuffer();

	auto* rtv = DeviceResources->GetRenderTargetView();
	ID3D11RenderTargetView* rtvs[] = { rtv };  // 배열 생성
	GetDeviceContext()->OMSetRenderTargets(1, rtvs, DeviceResources->GetDepthStencilView());
}

/**
 * @brief Vertex Buffer 소멸 함수
 * @param InVertexBuffer
 */
void URenderer::ReleaseVertexBuffer(ID3D11Buffer* InVertexBuffer)
{
	if (InVertexBuffer) { InVertexBuffer->Release(); }
}

/**
 * @brief 상수 버퍼 생성 함수
 */
void URenderer::CreateConstantBuffer()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	/**
	 * @brief 모델에 사용될 상수 버퍼 생성
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FMatrix) + 0xf & 0xfffffff0;
		// ensure constant buffer size is multiple of 16 bytes
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferModels);
	}

	/**
	 * @brief 색상 수정에 사용할 상수 버퍼
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FVector4) + 0xf & 0xfffffff0;
		// ensure constant buffer size is multiple of 16 bytes
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferColor);
	}

	/**
	 * @brief 카메라에 사용될 상수 버퍼 생성
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FViewProjConstants) + 0xf & 0xfffffff0;
		// ensure constant buffer size is multiple of 16 bytes
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferPerFrame);
	}

	/**
	 * @brief 폰트에 사용될 조회 테이블 상수 버퍼 생성
	 */
	{
		FCharacterInfo* CharTable;

		CharTable = ResourceManager.LoadCharTable();
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = (sizeof(FCharacterInfo)) * 95; //95개 CharacterSet의 UV좌표
		ConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ConstantBufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA CharTableData = {};
		CharTableData.pSysMem = CharTable;
		GetDevice()->CreateBuffer(&ConstantBufferDesc, &CharTableData, &ConstantBufferCharTable);

		Pipeline->SetConstantBuffer(4, true, ConstantBufferCharTable);
	}
}

/**
 * @brief 상수 버퍼 소멸 함수
 */
void URenderer::ReleaseConstantBuffer()
{
	if (ConstantBufferModels)
	{
		ConstantBufferModels->Release();
		ConstantBufferModels = nullptr;
	}

	if (ConstantBufferColor)
	{
		ConstantBufferColor->Release();
		ConstantBufferColor = nullptr;
	}

	if (ConstantBufferPerFrame)
	{
		ConstantBufferPerFrame->Release();
		ConstantBufferPerFrame = nullptr;
	}

	if (ConstantBufferCharTable)
	{
		ConstantBufferCharTable->Release();
		ConstantBufferCharTable = nullptr;
	}
}


void URenderer::UpdateConstant(const UPrimitiveComponent* Primitive)
{
	if (ConstantBufferModels)
	{
		D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

		GetDeviceContext()->Map(ConstantBufferModels, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
		// update constant buffer every frame
		FMatrix* constants = (FMatrix*)constantbufferMSR.pData;
		{
			*constants = Primitive->GetWorldTransformMatrix();
		}
		GetDeviceContext()->Unmap(ConstantBufferModels, 0);
	}
}
/**
 * @brief 상수 버퍼 업데이트 함수
 * @param InOffset
 * @param InScale Ball Size
 */
void URenderer::UpdateConstant(const FVector& InPosition, const FVector& InRotation, const FVector& InScale) const
{
	if (ConstantBufferModels)
	{
		D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

		GetDeviceContext()->Map(ConstantBufferModels, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
		// update constant buffer every frame
		FMatrix* constants = (FMatrix*)constantbufferMSR.pData;
		{
			*constants = FMatrix::GetModelMatrix(InPosition, FVector::GetDegreeToRadian(InRotation), InScale);
		}
		GetDeviceContext()->Unmap(ConstantBufferModels, 0);
	}
}

void URenderer::UpdateConstant(const FViewProjConstants& InViewProjConstants) const
{
	Pipeline->SetConstantBuffer(1, false, ConstantBufferPerFrame);
	Pipeline->SetConstantBuffer(1, true, ConstantBufferPerFrame);

	if (ConstantBufferPerFrame)
	{
		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR = {};

		GetDeviceContext()->Map(ConstantBufferPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		// update constant buffer every frame
		FViewProjConstants* ViewProjectionConstants = (FViewProjConstants*)ConstantBufferMSR.pData;
		{
			ViewProjectionConstants->View = InViewProjConstants.View;
			ViewProjectionConstants->Projection = InViewProjConstants.Projection;
			ViewProjectionConstants->ViewModeIndex = static_cast<uint32>(CurrentViewMode);
		}
		GetDeviceContext()->Unmap(ConstantBufferPerFrame, 0);
	}
}

void URenderer::UpdateConstant(const FVector4& Color) const
{
	Pipeline->SetConstantBuffer(2, false, ConstantBufferColor);

	if (ConstantBufferColor)
	{
		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR = {};

		GetDeviceContext()->Map(ConstantBufferColor, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		// update constant buffer every frame
		FVector4* ColorConstants = (FVector4*)ConstantBufferMSR.pData;
		{
			ColorConstants->X = Color.X;
			ColorConstants->Y = Color.Y;
			ColorConstants->Z = Color.Z;
			ColorConstants->W = Color.W;
		}
		GetDeviceContext()->Unmap(ConstantBufferColor, 0);
	}
}

void URenderer::UpdateInstance(const TArray<FTextInstance>* Instance)
{
	if (TextInstanceBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE InstanceBufferMSR = {};

		GetDeviceContext()->Map(TextInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &InstanceBufferMSR);
		// update constant buffer every frame
		memcpy(InstanceBufferMSR.pData, Instance->data(), sizeof(FTextInstance) * Instance->size());
	
		GetDeviceContext()->Unmap(TextInstanceBuffer, 0);
	}

}

// TODO - 추후 ViewMode가 증가하거나, 바꿔야하는 설정이 많을 경우 별개의 Handler에서 진행하도록 변경
/**
 * @brief ViewMode를 고려한 PipelineInfo 생성
 * @param InRenderState 렌더할 대상의 RenderState
 * @return RenderState와 ViewMode를 고려한 FPipelineInfo
 */
FPipelineInfo URenderer::CreatePipelineInfo(const FRenderState& InRenderState)
{
	FRenderState ModifiedRenderState = InRenderState;

	switch (CurrentViewMode)
	{
	case EViewModeIndex::Wireframe:
		ModifiedRenderState.FillMode = EFillMode::WireFrame;
		ModifiedRenderState.CullMode = ECullMode::None;
		break;
	case EViewModeIndex::Lit:
	case EViewModeIndex::Unlit:
	default:
		break;
	}

	ID3D11RasterizerState* RasterizerState = GetRasterizerState(ModifiedRenderState);
	return FPipelineInfo{DefaultInputLayout, DefaultVertexShader,
		RasterizerState, DefaultDepthStencilState, DefaultPixelShader, nullptr,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	};
}

FPipelineInfo URenderer::CreateTextPipelineInfo(const FRenderState& InRenderState)
{
	FRenderState ModifiedRenderState = InRenderState;

	switch (CurrentViewMode)
	{
	case EViewModeIndex::Wireframe:
		ModifiedRenderState.FillMode = EFillMode::WireFrame;
		ModifiedRenderState.CullMode = ECullMode::None;
		break;
	case EViewModeIndex::Lit:
	case EViewModeIndex::Unlit:
	default:
		break;
	}

	ID3D11RasterizerState* RasterizerState = GetRasterizerState(ModifiedRenderState);
	return FPipelineInfo{ TextInputLayout, TextVertexShader,
		RasterizerState, DefaultDepthStencilState, TextPixelShader, TextBlendState
	};
}

ID3D11RasterizerState* URenderer::GetRasterizerState(const FRenderState& InRenderState)
{
	D3D11_FILL_MODE FillMode = ToD3D11(InRenderState.FillMode);
	D3D11_CULL_MODE CullMode = ToD3D11(InRenderState.CullMode);

	const FRasterKey Key{ FillMode, CullMode };
	if (auto It = RasterCache.find(Key); It != RasterCache.end())
		return It->second;

	ID3D11RasterizerState* RasterizerState = nullptr;
	D3D11_RASTERIZER_DESC RasterizerDesc = {};
	RasterizerDesc.FillMode = FillMode;
	RasterizerDesc.CullMode = CullMode;
	RasterizerDesc.DepthClipEnable = TRUE; // ✅ 근/원거리 평면 클리핑 활성화 (핵심)

	HRESULT Hr = GetDevice()->CreateRasterizerState(&RasterizerDesc, &RasterizerState);

	if (FAILED(Hr)) { return nullptr; }

	RasterCache.emplace(Key, RasterizerState);
	return RasterizerState;
}
