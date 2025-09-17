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

namespace
{
	struct FInstanceGPUData
	{
		FMatrix World;
		FVector4 Color;
	};

	struct FInstanceDrawConstants
	{
		uint32 bUseInstancing = 0;
		uint32 BaseInstanceOffset = 0;
		uint32 InstanceCount = 0;
		uint32 Padding = 0;
	};
}

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
	CreateLineInstancedShader();
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
	ReleaseLineInstancedShader();
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

	D3D11_DEPTH_STENCIL_DESC DescText = {};

	//Text(외 투명한 물체)는 깊이테스트를 하되 쓰지는 않아야 함.
	DescText.DepthEnable = TRUE;
	DescText.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	DescDefault.StencilEnable = FALSE;
	DescDefault.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DescDefault.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	hr = DeviceResources->GetDevice()->CreateDepthStencilState(
		&DescText,
		&TextDepthStencilState
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

	D3DCompileFromFile(L"Asset/Shader/SampleShader.hlsl", nullptr, nullptr, "MainVS", "vs_5_0", 0, 0,
	                   &VertexShaderCSO, nullptr);

	GetDevice()->CreateVertexShader(VertexShaderCSO->GetBufferPointer(),
	                                VertexShaderCSO->GetBufferSize(), nullptr, &DefaultVertexShader);

	D3DCompileFromFile(L"Asset/Shader/SampleShader.hlsl", nullptr, nullptr, "MainPS", "ps_5_0", 0, 0,
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

		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{"TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(),
	                               VertexShaderCSO->GetBufferSize(), &TextInputLayout);

	StrideTextVertex = sizeof(FTextVertex);
	StrideTextInstance = sizeof(FTextInstance);

	VertexShaderCSO->Release();
	PixelShaderCSO->Release();
}

void URenderer::CreateLineInstancedShader()
{
	ID3DBlob* VertexShaderCSO = nullptr;
	ID3DBlob* PixelShaderCSO = nullptr;

	D3DCompileFromFile(L"Asset/Shader/LineInstanced.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0,
	                   &VertexShaderCSO, nullptr);

	GetDevice()->CreateVertexShader(VertexShaderCSO->GetBufferPointer(),
	                                VertexShaderCSO->GetBufferSize(), nullptr, &LineInstancedVertexShader);

	D3DCompileFromFile(L"Asset/Shader/LineInstanced.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0,
	                   &PixelShaderCSO, nullptr);

	GetDevice()->CreatePixelShader(PixelShaderCSO->GetBufferPointer(),
	                               PixelShaderCSO->GetBufferSize(), nullptr, &LineInstancedPixelShader);

	// slot 0: POSITION (per-vertex), slot 1: COLOR (per-instance)
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};

	GetDevice()->CreateInputLayout(layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(),
	                               VertexShaderCSO->GetBufferSize(), &LineInstancedInputLayout);

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

void URenderer::ReleaseLineInstancedShader()
{
	if (LineInstancedInputLayout)
	{
		LineInstancedInputLayout->Release();
		LineInstancedInputLayout = nullptr;
	}
	if (LineInstancedPixelShader)
	{
		LineInstancedPixelShader->Release();
		LineInstancedPixelShader = nullptr;
	}
	if (LineInstancedVertexShader)
	{
		LineInstancedVertexShader->Release();
		LineInstancedVertexShader = nullptr;
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
	// Editor->RenderEditor();
	Editor->RenderEditorBatched();
	RenderTest(Editor->GetCameraLocation());
	//RenderLines();

	UUIManager::GetInstance().Render();

	RenderEnd();
}

/**
 * @brief Render Prepare Step
 */
void URenderer::RenderBegin()
{
	ID3D11RenderTargetView* RenderTargetView = DeviceResources->GetRenderTargetView();
	GetDeviceContext()->ClearRenderTargetView(RenderTargetView, ClearColor);
	ID3D11DepthStencilView* DepthStencilView = DeviceResources->GetDepthStencilView();
	GetDeviceContext()->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	GetDeviceContext()->RSSetViewports(1, &DeviceResources->GetViewportInfo());

	ID3D11RenderTargetView* RenderTargetViews[] = {RenderTargetView}; // 배열 생성

	GetDeviceContext()->OMSetRenderTargets(1, RenderTargetViews, DeviceResources->GetDepthStencilView());
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
	if (!ULevelManager::GetInstance().GetCurrentLevel()) { return; }

	// Check show flags for primitive components
	if (IsShowFlagEnabled(EEngineShowFlags::SF_Primitives) == false) { return; }

	TMap<FPrimitiveBatchKey, TArray<FInstanceGPUData>, FPrimitiveBatchKeyHasher> InstanceBatches;

	const TArray<UPrimitiveComponent*>& PrimitiveComponents =
		ULevelManager::GetInstance().GetCurrentLevel()->GetLevelPrimitiveComponents();

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent) { continue; }
		if (!PrimitiveComponent->IsVisible()) { continue; }

		FPrimitiveBatchKey Key;
		Key.VertexBuffer = PrimitiveComponent->GetReducedVertexBuffer();
		Key.IndexBuffer = PrimitiveComponent->GetIndexBuffer();
		Key.IndexCount = PrimitiveComponent->GetIndexNum();
		Key.RenderState = PrimitiveComponent->GetRenderState();


		if (!Key.VertexBuffer || !Key.IndexBuffer || Key.IndexCount == 0)
		{
			continue;
		}

		InstanceBatches[Key].Emplace(PrimitiveComponent->GetWorldTransformMatrix(), PrimitiveComponent->GetColor());
	}

	if (InstanceBatches.IsEmpty())
	{
		UpdateInstanceDrawConstants(false, 0, 0);
		ID3D11ShaderResourceView* NullSRV = nullptr;
		GetDeviceContext()->VSSetShaderResources(0, 1, &NullSRV);
		return;
	}

	for (auto& Batch : InstanceBatches)
	{
		const FPrimitiveBatchKey& Key = Batch.first;
		TArray<FInstanceGPUData>& Instances = Batch.second;

		if (Instances.IsEmpty())
		{
			continue;
		}

		FInstanceBufferResource& Resource = GetOrCreateInstanceBuffer(Key);
		EnsureInstanceBufferCapacity(Resource, static_cast<uint32>(Instances.Num()));
		if (!Resource.Buffer || !Resource.ShaderResourceView)
		{
			continue;
		}

		UploadInstanceBufferData(Resource, Instances.data(), static_cast<uint32>(Instances.Num()));

		Pipeline->UpdatePipeline(CreatePipelineInfo(Key.RenderState));

		Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
		UpdateConstant(FMatrix::Identity);

		Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
		UpdateConstant(FVector4(0.f, 0.f, 0.f, 0.f));

		Pipeline->SetConstantBuffer(3, true, ConstantBufferInstance);
		UpdateInstanceDrawConstants(true, 0, static_cast<uint32>(Instances.Num()));

		Pipeline->SetVertexBuffer(Key.VertexBuffer, Stride);
		Pipeline->SetIndexBuffer(Key.IndexBuffer, DXGI_FORMAT_R32_UINT);
		Pipeline->SetShaderResourceView(0, true, Resource.ShaderResourceView);

		Pipeline->DrawIndexedInstanced(Key.IndexCount, static_cast<uint32>(Instances.Num()), 0, 0, 0);
	}

	UpdateInstanceDrawConstants(false, 0, 0);
	ID3D11ShaderResourceView* NullSRV = nullptr;
	GetDeviceContext()->VSSetShaderResources(0, 1, &NullSRV);
}

void URenderer::RenderTest(const FVector& CameraLocation)
{
	if (IsShowFlagEnabled(EEngineShowFlags::SF_BillboardText) == false) { return; }

	//shader, rasterizaer state, depth stencil state, input layout 설정
	FRenderState State = FRenderState{ECullMode::None, EFillMode::Solid};
	Pipeline->UpdatePipeline(CreateTextPipelineInfo(State));

	//텍스처, 샘플러 설정
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	ID3D11ShaderResourceView* Srv = ResourceManager.GetTexture("Asset/Font/Pretendard-Regular.dds");
	ID3D11SamplerState* SamplerState = ResourceManager.GetSamplerState(ESamplerType::Text);

	Pipeline->SetShaderResourceView(0, false, Srv);
	Pipeline->SetSamplerState(0, false, SamplerState);

	//text(외 투명한 물체)들은 블랜딩을 적용하기 위해서 zbuffer에 쓰기를 하지 않음, 그래서 뒤에 있는 물체가 앞에 있는 물체 위에 렌더링되는 현상이 벌어짐
	//그래서 zbuffer에 쓰지 않으면서 추가로 카메라로부터 거리순으로 정렬을 해서 멀리 있는 물체부터 그려줘야함.
	struct RenderObject
	{
		UTextComponent* Component;
		float DistanceToCamera;

		bool operator<(const RenderObject& Other) const
		{
			return this->DistanceToCamera > Other.DistanceToCamera;
		}
	};

	TArray<RenderObject> RenderList;

	for (UTextComponent* Component : ULevelManager::GetInstance().GetCurrentLevel()->GetTextComponents())
	{
		RenderObject Object;
		Object.Component = Component;
		Object.DistanceToCamera = (CameraLocation - Component->GetWorldLocation()).Length();
		RenderList.push_back(Object);
	}
	std::sort(RenderList.begin(), RenderList.end());


	//정렬된 리스트 순회하면서 렌더링
	for (RenderObject& Object : RenderList)
	{
		Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);

		USceneComponent* RootComponent = Object.Component->GetOwner()->GetRootComponent();

		//루트컴포넌트가 PrimitiveComponent가 아닌 다른 component가 될 수 있는지는 모르겠지만 일단 예외처리를 함.
		//AABB의 높이값을 이용해서 항상 엑터 위에 텍스트가 출력되도록 함
		if (RootComponent->GetClass()->IsChildOf(UPrimitiveComponent::StaticClass()))
		{
			FAABB AABB = static_cast<UPrimitiveComponent*>(RootComponent)->GetWorldBounds();
			FVector Position = AABB.GetCenter();
			Position.Z = AABB.Max.Z + 1.f;
			UpdateConstant(Position, FVector(0, 0, 0), FVector(0, 0, 0));
		}
		//루트컴포넌트가 primitive가 아니면 그냥 textcomponent의 월드좌표에 z축으로 2 더해서 출력해줌
		else
		{
			UpdateConstant(Object.Component->GetWorldLocation() + FVector(0, 0, 2.0f), FVector(), FVector());
		}

		Pipeline->SetVertexBuffer(Object.Component->GetVertexBuffer(), StrideTextVertex);
		Pipeline->SetInstanceBuffer(TextInstanceBuffer, StrideTextInstance);

		//인스턴스 버퍼 업데이트(텍스트마다 다름)
		TArray<FTextInstance>* InstanceData = Object.Component->GetInstanceData();
		UpdateInstance(InstanceData);

		Pipeline->DrawInstanced(Object.Component->GetVertexNum(), InstanceData->size(), 0, 0);
	}

	UpdateInstanceDrawConstants(false, 0, 0);
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
	switch (InCull)
	{
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
	switch (InFill)
	{
	case EFillMode::Solid:
		return D3D11_FILL_SOLID;
	case EFillMode::WireFrame:
		return D3D11_FILL_WIREFRAME;
	default:
		return D3D11_FILL_SOLID;
	}
}

void URenderer::RenderEditorPrimitive(FEditorPrimitive& Primitive, struct FRenderState& InRenderState)
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


void URenderer::CreateInstanceBuffer()
{
	uint32 InByteWidth = sizeof(FTextInstance) * 100;
	D3D11_BUFFER_DESC InstanceBufferDesc = {};
	InstanceBufferDesc.ByteWidth = InByteWidth;
	InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;


	GetDevice()->CreateBuffer(&InstanceBufferDesc, nullptr, &TextInstanceBuffer);
}

void URenderer::ReleaseInstanceBuffer()
{
	if (TextInstanceBuffer)
	{
		TextInstanceBuffer->Release();
		TextInstanceBuffer = nullptr;
	}

	ReleasePrimitiveInstanceBuffers();
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

	ID3D11RenderTargetView* RenderTargetView = DeviceResources->GetRenderTargetView();
	ID3D11RenderTargetView* RenderTargetViews[] = {RenderTargetView}; // 배열 생성
	GetDeviceContext()->OMSetRenderTargets(1, RenderTargetViews, DeviceResources->GetDepthStencilView());
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
	 * @brief 인스턴싱 드로우 제어용 상수 버퍼 생성
	 */
	{
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FInstanceDrawConstants);
		ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		GetDevice()->CreateBuffer(&ConstantBufferDesc, nullptr, &ConstantBufferInstance);
	}

	/**
	 * @brief 폰트에 사용될 조회 테이블 상수 버퍼 생성
	 */
	{
		const TArray<FCharacterInfo>& CharTable = ResourceManager.GetCharInfos();
		D3D11_BUFFER_DESC ConstantBufferDesc = {};
		ConstantBufferDesc.ByteWidth = sizeof(FCharacterInfo) * CharTable.Num();
		ConstantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ConstantBufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA CharTableData = {};
		CharTableData.pSysMem = CharTable.data();
		GetDevice()->CreateBuffer(&ConstantBufferDesc, &CharTableData, &ConstantBufferCharTable);

		Pipeline->SetConstantBuffer(4, true, ConstantBufferCharTable);
	}

	UpdateInstanceDrawConstants(false, 0, 0);
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

	if (ConstantBufferInstance)
	{
		ConstantBufferInstance->Release();
		ConstantBufferInstance = nullptr;
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

void URenderer::UpdateConstant(const FMatrix& InMatrix) const
{
	if (ConstantBufferModels)
	{
		D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR;
		GetDeviceContext()->Map(ConstantBufferModels, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
		FMatrix* Constants = static_cast<FMatrix*>(ConstantBufferMSR.pData);
		*Constants = InMatrix;
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

void URenderer::UpdateInstanceDrawConstants(bool bUseInstancing, uint32 BaseInstanceOffset, uint32 InstanceCount) const
{
	if (!ConstantBufferInstance)
	{
		return;
	}

	Pipeline->SetConstantBuffer(3, true, ConstantBufferInstance);

	D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR = {};
	HRESULT Hr = GetDeviceContext()->Map(ConstantBufferInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR);
	if (FAILED(Hr))
	{
		UE_LOG("Failed to map instancing constant buffer");
		return;
	}

	auto* Constants = static_cast<FInstanceDrawConstants*>(ConstantBufferMSR.pData);
	Constants->bUseInstancing = bUseInstancing ? 1u : 0u;
	Constants->BaseInstanceOffset = BaseInstanceOffset;
	Constants->InstanceCount = InstanceCount;
	Constants->Padding = 0;

	GetDeviceContext()->Unmap(ConstantBufferInstance, 0);
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
	return FPipelineInfo{
		DefaultInputLayout, DefaultVertexShader,
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
	return FPipelineInfo{
		TextInputLayout, TextVertexShader,
		RasterizerState, TextDepthStencilState, TextPixelShader, TextBlendState
	};
}

URenderer::FInstanceBufferResource& URenderer::GetOrCreateInstanceBuffer(const FPrimitiveBatchKey& InKey)
{
	return PrimitiveInstanceBuffers[InKey];
}

void URenderer::EnsureInstanceBufferCapacity(FInstanceBufferResource& InResource, uint32 InRequiredInstanceCount)
{
	if (InRequiredInstanceCount == 0)
	{
		return;
	}

	if (InResource.Capacity >= InRequiredInstanceCount && InResource.Buffer && InResource.ShaderResourceView)
	{
		return;
	}

	uint32 NewCapacity = InResource.Capacity > 0 ? InResource.Capacity : 64u;
	while (NewCapacity < InRequiredInstanceCount)
	{
		NewCapacity *= 2u;
	}

	if (InResource.ShaderResourceView)
	{
		InResource.ShaderResourceView->Release();
		InResource.ShaderResourceView = nullptr;
	}

	if (InResource.Buffer)
	{
		InResource.Buffer->Release();
		InResource.Buffer = nullptr;
	}

	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(FInstanceGPUData) * NewCapacity;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(FInstanceGPUData);

	ID3D11Device* Device = GetDevice();
	HRESULT Hr = Device->CreateBuffer(&BufferDesc, nullptr, &InResource.Buffer);
	if (FAILED(Hr) || !InResource.Buffer)
	{
		UE_LOG("Failed to create instance structured buffer");
		InResource.Capacity = 0;
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SrvDesc.Buffer.FirstElement = 0;
	SrvDesc.Buffer.NumElements = NewCapacity;

	Hr = Device->CreateShaderResourceView(InResource.Buffer, &SrvDesc, &InResource.ShaderResourceView);
	if (FAILED(Hr) || !InResource.ShaderResourceView)
	{
		UE_LOG("Failed to create instance buffer SRV");
		InResource.Buffer->Release();
		InResource.Buffer = nullptr;
		InResource.Capacity = 0;
		return;
	}

	InResource.Capacity = NewCapacity;
}

void URenderer::UploadInstanceBufferData(FInstanceBufferResource& InResource, const void* InData,
                                         uint32 InInstanceCount)
{
	if (!InResource.Buffer || InInstanceCount == 0)
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE Mapped = {};
	HRESULT Hr = GetDeviceContext()->Map(InResource.Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);
	if (FAILED(Hr))
	{
		UE_LOG("Failed to map instance buffer");
		return;
	}

	const size_t CopySize = sizeof(FInstanceGPUData) * static_cast<size_t>(InInstanceCount);
	memcpy(Mapped.pData, InData, CopySize);
	GetDeviceContext()->Unmap(InResource.Buffer, 0);
}

void URenderer::ReleasePrimitiveInstanceBuffers()
{
	for (auto& Pair : PrimitiveInstanceBuffers)
	{
		if (Pair.second.ShaderResourceView)
		{
			Pair.second.ShaderResourceView->Release();
			Pair.second.ShaderResourceView = nullptr;
		}

		if (Pair.second.Buffer)
		{
			Pair.second.Buffer->Release();
			Pair.second.Buffer = nullptr;
		}

		Pair.second.Capacity = 0;
	}

	PrimitiveInstanceBuffers.Empty();
}

ID3D11RasterizerState* URenderer::GetRasterizerState(const FRenderState& InRenderState)
{
	D3D11_FILL_MODE FillMode = ToD3D11(InRenderState.FillMode);
	D3D11_CULL_MODE CullMode = ToD3D11(InRenderState.CullMode);

	const FRasterKey Key{FillMode, CullMode};
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
