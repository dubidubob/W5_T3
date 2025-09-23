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
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/StaticMeshComponent.h"
#include "Manager/Viewport/ViewportManager.h"
#include "Slate/Viewport.h"
#include "Manager/Input/InputManager.h"

namespace
{
	struct InstanceGPUData
	{
		FMatrix World;
		FVector4 Color;
	};

	struct InstanceDrawConstants
	{
		uint32 bUseInstancing = 0;
		uint32 BaseInstanceOffset = 0;
		uint32 InstanceCount = 0;
		uint32 Padding = 0;
	};

	constexpr D3D11_CULL_MODE ConvertCullMode(ECullMode CullMode)
	{
		switch (CullMode)
		{
		case ECullMode::Back: return D3D11_CULL_BACK;
		case ECullMode::Front: return D3D11_CULL_FRONT;
		case ECullMode::None: return D3D11_CULL_NONE;
		default: return D3D11_CULL_BACK;
		}
	}

	constexpr D3D11_FILL_MODE ConvertFillMode(EFillMode FillMode)
	{
		switch (FillMode)
		{
		case EFillMode::Solid: return D3D11_FILL_SOLID;
		case EFillMode::WireFrame: return D3D11_FILL_WIREFRAME;
		default: return D3D11_FILL_SOLID;
		}
	}
}

IMPLEMENT_CLASS(URenderer, UObject)
IMPLEMENT_SINGLETON(URenderer)

URenderer::URenderer() = default;
URenderer::~URenderer() = default;

// ================== Initialization/Cleanup ==================

void URenderer::Init(HWND WindowHandle)
{
	DeviceResources = new UDeviceResources(WindowHandle);
	Pipeline = new UPipeline(GetDeviceContext());

	InitializeRenderStates();
	InitializeShaders();
	InitializeBuffers();

	ULineBatchRenderer::GetInstance().Init();
}

void URenderer::Release()
{
	ULineBatchRenderer::GetInstance().Release();
	CleanupAll();
	SafeDelete(Pipeline);
	SafeDelete(DeviceResources);
}

void URenderer::InitializeRenderStates()
{
	CreateDepthStencilState(DefaultDepthStencilState, true, D3D11_DEPTH_WRITE_MASK_ALL);
	CreateDepthStencilState(DisabledDepthStencilState, false, D3D11_DEPTH_WRITE_MASK_ALL);
	CreateDepthStencilState(TextDepthStencilState, true, D3D11_DEPTH_WRITE_MASK_ZERO);
	CreateBlendState();
}

void URenderer::InitializeShaders()
{
	CreateShaderSet(L"Data/Shader/SampleShader.hlsl", "MainVS", "MainPS",
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		},
		DefaultVertexShader, DefaultPixelShader, DefaultInputLayout);
	Stride = sizeof(FVertex);

	CreateShaderSet(L"Data/Shader/StaticMeshShader.hlsl", "MainVS", "MainPS",
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0}
		},
		StaticVertexShader, StaticPixelShader, StaticInputLayout);
	StaticStride = sizeof(FNormalVertex);

	CreateShaderSet(L"Data/Shader/TextShader.hlsl", "mainVS", "mainPS",
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1}
		},
		TextVertexShader, TextPixelShader, TextInputLayout);
	StrideTextVertex = sizeof(FTextVertex);
	StrideTextInstance = sizeof(FTextInstance);

	CreateShaderSet(L"Data/Shader/SlateShader.hlsl", "VS_Slate", "PS_Slate",
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
		},
		SlateVertexShader, SlatePixelShader, SlateInputLayout);

	CreateShaderSet(L"Data/Shader/LineInstanced.hlsl", "mainVS", "mainPS",
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
		},
		LineInstancedVertexShader, LineInstancedPixelShader, LineInstancedInputLayout);
}

void URenderer::InitializeBuffers()
{
	CreateConstantBuffer(ConstantBufferModels, sizeof(FMatrix));
	CreateConstantBuffer(ConstantBufferColor, sizeof(FVector4));
	CreateConstantBuffer(ConstantBufferPerFrame, sizeof(FViewProjConstants));
	CreateConstantBuffer(ConstantBufferInstance, sizeof(InstanceDrawConstants));
	CreateConstantBuffer(ConstantBufferMaterialParam, sizeof(FMaterialParamsCB));

	CreateCharacterTableBuffer();
	CreateTextInstanceBuffer();
	CreateSamplerState();

	UpdateInstanceDrawConstants(false, 0, 0);
}

// ================== Core Creation Functions ==================

void URenderer::CreateDepthStencilState(ID3D11DepthStencilState*& State, bool DepthEnable, D3D11_DEPTH_WRITE_MASK WriteMask)
{
	D3D11_DEPTH_STENCIL_DESC Desc = {};
	Desc.DepthEnable = DepthEnable;
	Desc.DepthWriteMask = WriteMask;
	Desc.DepthFunc = D3D11_COMPARISON_LESS;
	Desc.StencilEnable = FALSE;
	Desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	Desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	GetDevice()->CreateDepthStencilState(&Desc, &State);
}

void URenderer::CreateBlendState()
{
	D3D11_BLEND_DESC BlendDesc = {};
	D3D11_RENDER_TARGET_BLEND_DESC& RTBlendDesc = BlendDesc.RenderTarget[0];

	RTBlendDesc.BlendEnable = TRUE;
	RTBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	RTBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	RTBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	RTBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	RTBlendDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	RTBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	RTBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	GetDevice()->CreateBlendState(&BlendDesc, &TextBlendState);
}

void URenderer::CreateShaderSet(const wchar_t* ShaderPath, const char* VSEntry, const char* PSEntry,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& InputElements,
	ID3D11VertexShader*& VertexShader, ID3D11PixelShader*& PixelShader, ID3D11InputLayout*& InputLayout)
{
	ID3DBlob* VSBlob = nullptr;
	ID3DBlob* PSBlob = nullptr;

	// Compile and create vertex shader
	D3DCompileFromFile(ShaderPath, nullptr, nullptr, VSEntry, "vs_5_0", 0, 0, &VSBlob, nullptr);
	GetDevice()->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &VertexShader);

	// Compile and create pixel shader
	D3DCompileFromFile(ShaderPath, nullptr, nullptr, PSEntry, "ps_5_0", 0, 0, &PSBlob, nullptr);
	GetDevice()->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &PixelShader);

	// Create input layout
	GetDevice()->CreateInputLayout(
		InputElements.data(), static_cast<UINT>(InputElements.size()),
		VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &InputLayout);

	SafeRelease(VSBlob);
	SafeRelease(PSBlob);
}

void URenderer::CreateConstantBuffer(ID3D11Buffer*& Buffer, size_t Size)
{
	D3D11_BUFFER_DESC Desc = {};
	Desc.ByteWidth = static_cast<UINT>((Size + 15) & ~15); // 16-byte aligned
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	GetDevice()->CreateBuffer(&Desc, nullptr, &Buffer);
}

void URenderer::CreateCharacterTableBuffer()
{
	const TArray<FCharacterInfo>& CharTable = UResourceManager::GetInstance().GetCharInfos();

	D3D11_BUFFER_DESC Desc = {};
	Desc.ByteWidth = sizeof(FCharacterInfo) * CharTable.Num();
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = CharTable.data();

	GetDevice()->CreateBuffer(&Desc, &InitData, &ConstantBufferCharTable);
	Pipeline->SetConstantBuffer(4, true, ConstantBufferCharTable);
}

void URenderer::CreateTextInstanceBuffer()
{
	constexpr uint32 InitialInstanceCount = 100;

	D3D11_BUFFER_DESC Desc = {};
	Desc.ByteWidth = sizeof(FTextInstance) * InitialInstanceCount;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	GetDevice()->CreateBuffer(&Desc, nullptr, &TextInstanceBuffer);
}

void URenderer::CreateSamplerState()
{
	D3D11_SAMPLER_DESC Desc = {};
	Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	Desc.AddressU = Desc.AddressV = Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	Desc.MinLOD = 0;
	Desc.MaxLOD = D3D11_FLOAT32_MAX;

	GetDevice()->CreateSamplerState(&Desc, &DiffuseSampler);
}

void URenderer::UpdateViewProjConstants(const FViewProjConstants& ViewProj)
{
	Pipeline->SetConstantBuffer(1, false, ConstantBufferPerFrame);
	Pipeline->SetConstantBuffer(1, true, ConstantBufferPerFrame);

	FViewProjConstants Constants = ViewProj;
	Constants.ViewModeIndex = static_cast<uint32>(CurrentRenderMode);

	UpdateBuffer(ConstantBufferPerFrame, ViewProj);
}

// ================== Universal Update Functions ==================
void URenderer::UpdateInstance(const TArray<FTextInstance>* Instances)
{
	if (!TextInstanceBuffer || !Instances || Instances->empty()) return;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	if (SUCCEEDED(GetDeviceContext()->Map(TextInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
	{
		memcpy(MappedResource.pData, Instances->data(), sizeof(FTextInstance) * Instances->size());
		GetDeviceContext()->Unmap(TextInstanceBuffer, 0);
	}
}

void URenderer::UpdateInstanceDrawConstants(bool UseInstancing, uint32 BaseOffset, uint32 InstanceCount) const
{
	Pipeline->SetConstantBuffer(3, true, ConstantBufferInstance);

	InstanceDrawConstants Constants{};
	Constants.bUseInstancing = UseInstancing ? 1u : 0u;
	Constants.BaseInstanceOffset = BaseOffset;
	Constants.InstanceCount = InstanceCount;

	UpdateBuffer(ConstantBufferInstance, Constants);
}
	
// ================== Main Rendering Loop ==================

void URenderer::Update(UEditor* Editor)
{
	RenderBegin();

	GetDeviceContext()->RSSetViewports(1, &DeviceResources->GetViewportInfo());

	if (Editor->GetViewportManager()->GetIsWindowDivided())
	{
		RenderMultiViewport(Editor);
	}
	else
	{
		DeviceResources->UpdateViewport();
		RenderScene(Editor);
	}

	UUIManager::GetInstance().Render();
	RenderEnd();
}

void URenderer::RenderMultiViewport(UEditor* Editor)
{
	long WindowWidth = GetDeviceResources()->GetViewportInfo().Width;
	long WindowHeight = GetDeviceResources()->GetViewportInfo().Height;
	Editor->GetViewportManager()->UpdateViewportRects({ WindowWidth, WindowHeight });

	RenderSlate(Editor);

	for (int Idx = 0; Idx < 4; ++Idx)
	{
		FViewportInfo* ViewportInfo = Editor->GetViewportManager()->GetViewportInfo(Idx);

		GetDeviceContext()->RSSetViewports(1, &ViewportInfo->DxViewport);
		SetViewMode(ViewportInfo->RenderMode);

		// Update view projection constants
		Pipeline->SetConstantBuffer(1, false, ConstantBufferPerFrame);
		Pipeline->SetConstantBuffer(1, true, ConstantBufferPerFrame);
		FViewProjConstants Constants = ViewportInfo->Camera->GetFViewProjConstants();
		Constants.ViewModeIndex = static_cast<uint32>(CurrentRenderMode);
		UpdateBuffer(ConstantBufferPerFrame, Constants);

		DeviceResources->UpdateViewport();

		RenderScene(Editor);
	}
}

void URenderer::RenderScene(UEditor* Editor)
{
	RenderLevel();
	Editor->RenderEditorBatched();
	RenderText(Editor->GetCameraLocation());
}

void URenderer::RenderBegin()
{
	ID3D11RenderTargetView* RenderTargetView = DeviceResources->GetRenderTargetView();
	ID3D11DepthStencilView* DepthStencilView = DeviceResources->GetDepthStencilView();

	GetDeviceContext()->ClearRenderTargetView(RenderTargetView, ClearColor);
	GetDeviceContext()->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	ID3D11RenderTargetView* RenderTargetViews[] = { RenderTargetView };
	GetDeviceContext()->OMSetRenderTargets(1, RenderTargetViews, DepthStencilView);
}

void URenderer::RenderEnd() const
{
	GetSwapChain()->Present(0, 0);
}

// ================== Specific Rendering Functions ==================

void URenderer::RenderLevel()
{
	if (!ULevelManager::GetInstance().GetCurrentLevel() ||
		!IsShowFlagEnabled(EEngineShowFlags::SF_Primitives))
	{
		return;
	}

	const TArray<UPrimitiveComponent*>& PrimitiveComponents =
		ULevelManager::GetInstance().GetCurrentLevel()->GetLevelPrimitiveComponents();

	for (UPrimitiveComponent* Component : PrimitiveComponents)
	{
		RenderStaticMeshComponent(Component);
	}

	DisableInstancing();
}

void URenderer::RenderStaticMeshComponent(UPrimitiveComponent* Component)
{
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
	if (!StaticMeshComponent || !StaticMeshComponent->IsVisible()) return;

	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	if (!StaticMesh) return;

	FStaticMesh* MeshData = StaticMesh->GetStaticMeshAsset();
	if (!MeshData) return;

	SetupStaticMeshRendering(StaticMeshComponent, MeshData);
	RenderStaticMeshSections(MeshData);
}

void URenderer::SetupStaticMeshRendering(UStaticMeshComponent* Component, FStaticMesh* MeshData)
{
	Pipeline->UpdatePipeline(CreatePipelineInfo(Component->GetRenderState()));

	// Set constant buffers
	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
	Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
	Pipeline->SetConstantBuffer(2, false, ConstantBufferColor);

	// Update constants
	UpdateBuffer(ConstantBufferModels, Component->GetWorldTransformMatrix());
	UpdateBuffer(ConstantBufferColor, FVector4(0.f, 0.f, 0.f, 0.f));

	Pipeline->SetConstantBuffer(3, true, ConstantBufferInstance);
	InstanceDrawConstants InstanceConstants{};
	InstanceConstants.bUseInstancing = 0;
	InstanceConstants.BaseInstanceOffset = 0;
	InstanceConstants.InstanceCount = 0;
	UpdateBuffer(ConstantBufferInstance, InstanceConstants);

	// Set buffers and topology
	UINT Offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &MeshData->VertexBuffer, &StaticStride, &Offset);
	GetDeviceContext()->IASetIndexBuffer(MeshData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GetDeviceContext()->PSSetSamplers(0, 1, &DiffuseSampler);
}

void URenderer::RenderStaticMeshSections(FStaticMesh* MeshData)
{
	for (const FStaticMeshSection& Section : MeshData->Sections)
	{
		SetupMaterialForSection(MeshData, Section);
		GetDeviceContext()->DrawIndexed(Section.NumIndices, Section.FirstIndex, 0);
	}
}

void URenderer::SetupMaterialForSection(FStaticMesh* MeshData, const FStaticMeshSection& Section)
{
	ID3D11ShaderResourceView* SRV = nullptr;
	FMaterialParamsCB MaterialParams{};

	if (Section.MaterialIndex >= 0 && Section.MaterialIndex < MeshData->Materials.Num())
	{
		FStaticMaterial& Material = MeshData->Materials[Section.MaterialIndex];
		SRV = Material.TextureSRV;
		MaterialParams.UseTexture = Material.bUseTexture;
		MaterialParams.UVScrollSpeed = FVector2(0.0f, -0.9f); // V 방향으로 스크롤
		MaterialParams.Time = UTimeManager::GetInstance().GetGameTime();     // 누적 시간
	}

	GetDeviceContext()->PSSetShaderResources(1, 1, &SRV);

	Pipeline->SetConstantBuffer(4, false, ConstantBufferMaterialParam);
	//MaterialParams.Padding = FVector(0, 0, 0);
	UpdateBuffer(ConstantBufferMaterialParam, MaterialParams);
}

void URenderer::RenderText(const FVector& CameraLocation)
{
	if (!IsShowFlagEnabled(EEngineShowFlags::SF_BillboardText)) return;

	SetupTextRendering();

	struct TextRenderObject
	{
		UTextComponent* Component;
		float DistanceToCamera;
		bool operator<(const TextRenderObject& Other) const { return DistanceToCamera > Other.DistanceToCamera; }
	};

	TArray<TextRenderObject> RenderList;
	for (UTextComponent* Component : ULevelManager::GetInstance().GetCurrentLevel()->GetTextComponents())
	{
		TextRenderObject Object;
		Object.Component = Component;
		Object.DistanceToCamera = (CameraLocation - Component->GetWorldLocation()).Length();
		RenderList.push_back(Object);
	}
	std::sort(RenderList.begin(), RenderList.end());

	for (const auto& RenderObject : RenderList)
	{
		RenderTextComponent(RenderObject.Component);
	}

	UpdateInstanceDrawConstants(false, 0, 0);
}

void URenderer::SetupTextRendering()
{
	FRenderState State{ ECullMode::None, EFillMode::Solid };
	Pipeline->UpdatePipeline(CreateTextPipelineInfo(State));

	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	ID3D11ShaderResourceView* SRV = ResourceManager.GetTexture("Data/Font/Pretendard-Regular.dds");
	ID3D11SamplerState* SamplerState = ResourceManager.GetSamplerState(ESamplerType::Text);

	Pipeline->SetShaderResourceView(0, false, SRV);
	Pipeline->SetSamplerState(0, false, SamplerState);
}

void URenderer::RenderTextComponent(UTextComponent* Component)
{
	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);

	FVector TextPosition = CalculateTextPosition(Component);
	FMatrix ModelMatrix = FMatrix::GetModelMatrix(TextPosition, FVector::GetDegreeToRadian(FVector()), FVector());
	UpdateBuffer(ConstantBufferModels, ModelMatrix);

	Pipeline->SetVertexBuffer(Component->GetVertexBuffer(), StrideTextVertex);
	Pipeline->SetInstanceBuffer(TextInstanceBuffer, StrideTextInstance);

	TArray<FTextInstance>* InstanceData = Component->GetInstanceData();
	UpdateInstance(InstanceData);

	Pipeline->DrawInstanced(Component->GetVertexNum(), InstanceData->size(), 0, 0);
}

FVector URenderer::CalculateTextPosition(UTextComponent* Component)
{
	USceneComponent* RootComponent = Component->GetOwner()->GetRootComponent();

	if (RootComponent->GetClass()->IsChildOf(UPrimitiveComponent::StaticClass()))
	{
		FAABB AABB = static_cast<UPrimitiveComponent*>(RootComponent)->GetWorldBounds();
		FVector Position = AABB.GetCenter();
		Position.Z = AABB.Max.Z + 1.f;
		return Position;
	}

	return Component->GetWorldLocation() + FVector(0, 0, 2.0f);
}

void URenderer::RenderSlate(UEditor* Editor)
{
	FPipelineInfo PipelineInfo{
		SlateInputLayout, SlateVertexShader,
		GetRasterizerState(FRenderState()), TextDepthStencilState,
		SlatePixelShader, nullptr
	};

	Pipeline->UpdatePipeline(PipelineInfo);

	FVector2 MouseCoord = UInputManager::GetInstance().GetMouseNDCPosition();

	for (SWindow* Window : Editor->GetViewportManager()->GetWindows())
	{
		RenderWindow(Window, MouseCoord);
	}
}

void URenderer::RenderWindow(SWindow* Window, const FVector2& MouseCoord)
{
	FRect Rect;
	FVector4 Color;

	if (!Window->CanRender(Rect, Color, MouseCoord)) return;

	FMatrix TransformMatrix = FMatrix::ScaleMatrix(FVector(Rect.Width, Rect.Height, 1))
		* FMatrix::TranslationMatrix(FVector(Rect.GetCenterX(), Rect.GetCenterY(), 0));

	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);
	UpdateBuffer(ConstantBufferModels, TransformMatrix);
	Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
	UpdateBuffer(ConstantBufferColor, Color);

	static constexpr uint32 SlateStride = sizeof(FVertex);
	Pipeline->SetVertexBuffer(UResourceManager::GetInstance().GetVertexBuffer(EPrimitiveType::Square), SlateStride);
	Pipeline->Draw(6, 0);
}

void URenderer::RenderEditorPrimitive(FEditorPrimitive& Primitive, FRenderState& RenderState)
{
	ID3D11DepthStencilState* DepthStencilState = Primitive.bShouldAlwaysVisible
		? DisabledDepthStencilState : DefaultDepthStencilState;

	FPipelineInfo PipelineInfo{
		DefaultInputLayout, DefaultVertexShader,
		GetRasterizerState(RenderState), DepthStencilState,
		DefaultPixelShader, nullptr, Primitive.Topology
	};

	Pipeline->UpdatePipeline(PipelineInfo);
	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);

	FMatrix ModelMatrix = FMatrix::GetModelMatrix(Primitive.Location, FVector::GetDegreeToRadian(Primitive.Rotation), Primitive.Scale);
	UpdateBuffer(ConstantBufferModels, ModelMatrix);

	Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
	UpdateBuffer(ConstantBufferColor, Primitive.Color);
	Pipeline->SetVertexBuffer(Primitive.Vertexbuffer, Stride);
	Pipeline->Draw(Primitive.NumVertices, 0);
}

// ================== Pipeline Creation ==================

FPipelineInfo URenderer::CreatePipelineInfo(const FRenderState& RenderState)
{
	FRenderState ModifiedState = ApplyViewModeToRenderState(RenderState);

	return FPipelineInfo{
		StaticInputLayout, StaticVertexShader,
		GetRasterizerState(ModifiedState), DefaultDepthStencilState,
		StaticPixelShader, nullptr, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
	};
}

FPipelineInfo URenderer::CreateTextPipelineInfo(const FRenderState& RenderState)
{
	FRenderState ModifiedState = ApplyViewModeToRenderState(RenderState);

	return FPipelineInfo{
		TextInputLayout, TextVertexShader,
		GetRasterizerState(ModifiedState), TextDepthStencilState,
		TextPixelShader, TextBlendState
	};
}

FRenderState URenderer::ApplyViewModeToRenderState(const FRenderState& OriginalState)
{
	FRenderState ModifiedState = OriginalState;

	if (CurrentRenderMode == EViewportRenderMode::Wireframe)
	{
		ModifiedState.FillMode = EFillMode::WireFrame;
		ModifiedState.CullMode = ECullMode::None;
	}

	return ModifiedState;
}

ID3D11RasterizerState* URenderer::GetRasterizerState(const FRenderState& RenderState)
{
	const FRasterKey Key{ ConvertFillMode(RenderState.FillMode), ConvertCullMode(RenderState.CullMode) };

	if (auto It = RasterCache.find(Key); It != RasterCache.end())
	{
		return It->second;
	}

	ID3D11RasterizerState* RasterizerState = nullptr;
	D3D11_RASTERIZER_DESC Desc = {};
	Desc.FillMode = Key.FillMode;
	Desc.CullMode = Key.CullMode;
	Desc.DepthClipEnable = TRUE;
	Desc.FrontCounterClockwise = FALSE;

	if (SUCCEEDED(GetDevice()->CreateRasterizerState(&Desc, &RasterizerState)))
	{
		RasterCache.emplace(Key, RasterizerState);
	}

	return RasterizerState;
}

// ================== Instance Buffer Management ==================

URenderer::FInstanceBufferResource& URenderer::GetOrCreateInstanceBuffer(const FPrimitiveBatchKey& Key)
{
	return PrimitiveInstanceBuffers[Key];
}

void URenderer::EnsureInstanceBufferCapacity(FInstanceBufferResource& Resource, uint32 RequiredCount)
{
	if (RequiredCount == 0 ||
		(Resource.Capacity >= RequiredCount && Resource.Buffer && Resource.ShaderResourceView))
	{
		return;
	}

	uint32 NewCapacity = std::max(Resource.Capacity * 2, std::max(RequiredCount, 64u));

	SafeRelease(Resource.ShaderResourceView);
	SafeRelease(Resource.Buffer);

	CreateStructuredBuffer(Resource, NewCapacity);
	Resource.Capacity = NewCapacity;
}

void URenderer::CreateStructuredBuffer(FInstanceBufferResource& Resource, uint32 Capacity)
{
	D3D11_BUFFER_DESC BufferDesc = {};
	BufferDesc.ByteWidth = sizeof(InstanceGPUData) * Capacity;
	BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	BufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	BufferDesc.StructureByteStride = sizeof(InstanceGPUData);

	if (FAILED(GetDevice()->CreateBuffer(&BufferDesc, nullptr, &Resource.Buffer)))
	{
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	SRVDesc.Buffer.FirstElement = 0;
	SRVDesc.Buffer.NumElements = Capacity;

	if (FAILED(GetDevice()->CreateShaderResourceView(Resource.Buffer, &SRVDesc, &Resource.ShaderResourceView)))
	{
		SafeRelease(Resource.Buffer);
	}
}

void URenderer::UploadInstanceBufferData(FInstanceBufferResource& Resource, const void* Data, uint32 InstanceCount)
{
	if (!Resource.Buffer || InstanceCount == 0) return;

	D3D11_MAPPED_SUBRESOURCE Mapped;
	if (SUCCEEDED(GetDeviceContext()->Map(Resource.Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped)))
	{
		memcpy(Mapped.pData, Data, sizeof(InstanceGPUData) * InstanceCount);
		GetDeviceContext()->Unmap(Resource.Buffer, 0);
	}
}

// ================== Resize Handling ==================

void URenderer::OnResize(uint32 Width, uint32 Height)
{
	if (!DeviceResources || !GetDevice() || !GetDeviceContext() || !GetSwapChain()) return;

	// Release current resources
	DeviceResources->ReleaseFrameBuffer();
	DeviceResources->ReleaseDepthBuffer();
	GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);

	// Resize swap chain buffers
	if (FAILED(GetSwapChain()->ResizeBuffers(2, Width, Height, DXGI_FORMAT_UNKNOWN, 0)))
	{
		UE_LOG("OnResize Failed");
		return;
	}

	// Recreate resources
	DeviceResources->UpdateViewport();
	DeviceResources->CreateFrameBuffer();
	DeviceResources->CreateDepthBuffer();

	// Reset render targets
	ID3D11RenderTargetView* RenderTargetView = DeviceResources->GetRenderTargetView();
	ID3D11RenderTargetView* RenderTargetViews[] = { RenderTargetView };
	GetDeviceContext()->OMSetRenderTargets(1, RenderTargetViews, DeviceResources->GetDepthStencilView());
}

// ================== Utility Functions ==================

void URenderer::ReleaseVertexBuffer(ID3D11Buffer* VertexBuffer)
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
	}
}

void URenderer::DisableInstancing()
{
	UpdateInstanceDrawConstants(false, 0, 0);
	ID3D11ShaderResourceView* NullSRV = nullptr;
	GetDeviceContext()->VSSetShaderResources(0, 1, &NullSRV);
}

// ================== Cleanup Functions ==================

void URenderer::CleanupAll()
{
	CleanupBuffers();
	CleanupShaders();
	CleanupRenderStates();
}

void URenderer::CleanupRenderStates()
{
	for (auto& [Key, State] : RasterCache)
	{
		SafeRelease(State);
	}
	RasterCache.clear();

	SafeRelease(DefaultDepthStencilState);
	SafeRelease(DisabledDepthStencilState);
	SafeRelease(TextDepthStencilState);
	SafeRelease(TextBlendState);
}

void URenderer::CleanupShaders()
{
	ReleaseShaderSet(DefaultVertexShader, DefaultPixelShader, DefaultInputLayout);
	ReleaseShaderSet(StaticVertexShader, StaticPixelShader, StaticInputLayout);
	ReleaseShaderSet(TextVertexShader, TextPixelShader, TextInputLayout);
	ReleaseShaderSet(SlateVertexShader, SlatePixelShader, SlateInputLayout);
	ReleaseShaderSet(LineInstancedVertexShader, LineInstancedPixelShader, LineInstancedInputLayout);
}

void URenderer::CleanupBuffers()
{
	SafeRelease(ConstantBufferModels);
	SafeRelease(ConstantBufferColor);
	SafeRelease(ConstantBufferPerFrame);
	SafeRelease(ConstantBufferInstance);
	SafeRelease(ConstantBufferCharTable);
	SafeRelease(ConstantBufferMaterialParam);
	SafeRelease(TextInstanceBuffer);
	SafeRelease(DiffuseSampler);

	for (auto& [Key, Resource] : PrimitiveInstanceBuffers)
	{
		SafeRelease(Resource.ShaderResourceView);
		SafeRelease(Resource.Buffer);
		Resource.Capacity = 0;
	}
	PrimitiveInstanceBuffers.Empty();
}

void URenderer::ReleaseShaderSet(ID3D11VertexShader*& VS, ID3D11PixelShader*& PS, ID3D11InputLayout*& Layout)
{
	SafeRelease(VS);
	SafeRelease(PS);
	SafeRelease(Layout);
}
