#include "pch.h"
#include "Render/Renderer/Renderer.h"

#include "Level/Level.h"
#include "Manager/Level/LevelManager.h"
#include "Manager/UI/UIManager.h"
#include "Manager/Path/PathManager.h"
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
#include "Math/Frustum.h"

#include "Global/PlatformTime.h"

#if IS_OBJ_VIEWER
#include "Utility/ObjectPreviewScene.h"
#endif

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

	// Create Picking Pixel Shader (uses existing vertex shaders)
	ID3DBlob* PixelShaderBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(
		L"Data/Shader/PickingShader.hlsl",
		nullptr,
		nullptr,
		"mainPS",
		"ps_5_0",
		0,
		0,
		&PixelShaderBlob,
		nullptr
	);

	if (SUCCEEDED(hr))
	{
		hr = GetDevice()->CreatePixelShader(
			PixelShaderBlob->GetBufferPointer(),
			PixelShaderBlob->GetBufferSize(),
			nullptr,
			&PickingPixelShader
		);
		PixelShaderBlob->Release();
	}

	if (FAILED(hr))
	{
		assert(!"Failed to create Picking Pixel Shader");
	}
}

void URenderer::InitializeBuffers()
{
	CreateConstantBuffer(ConstantBufferModels, sizeof(FMatrix));
	CreateConstantBuffer(ConstantBufferColor, sizeof(FVector4));
	CreateConstantBuffer(ConstantBufferPerFrame, sizeof(FViewProjConstants));
	CreateConstantBuffer(ConstantBufferInstance, sizeof(InstanceDrawConstants));
	CreateConstantBuffer(ConstantBufferMaterialParam, sizeof(FMaterialParamsCB));

	// Create picking constant buffer with same structure as PickingShader.hlsl
	struct PickingCB {
		uint32 Pick;
		uint32 ObjectID;
		int32 Padding[2];
	};
	CreateConstantBuffer(ConstantBufferPicking, sizeof(PickingCB));

	CreateCharacterTableBuffer();

	CreateTextInstanceBuffer();
	CreateSamplerState();

	UpdateInstanceDrawConstants(false, 0, 0);
}
#ifdef _DEVELOP
void URenderer::InitializeRenderStateChangeCount()
{
	MaterialChangeCount = 0;
	StaticMeshChangeCount = 0;
	StaticMeshComponentChagneCount = 0;
	MeshSectionDrawCount = 0;
}
#endif
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
    CachedViewProj = ViewProj;
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
	TIME_PROFILE(Update)

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

#if IS_OBJ_VIEWER
	if (Editor->GetObjPreview()->SelectActivated())
		RenderObjectViewer(Editor);
#endif

    // Only perform color picking when conditions are met
    if (ShouldPerformColorPicking())
    {
        RenderColorPicking();
    }

	// Switch back to main render target for UI rendering
	ID3D11RenderTargetView* MainRTV = DeviceResources->GetRenderTargetView();
	ID3D11DepthStencilView* MainDSV = DeviceResources->GetDepthStencilView();
	GetDeviceContext()->OMSetRenderTargets(1, &MainRTV, MainDSV);
	GetDeviceContext()->RSSetViewports(1, &DeviceResources->GetViewportInfo());

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
        CachedViewProj = Constants;

		DeviceResources->UpdateViewport();

		RenderScene(Editor, Idx);
	}

	// Render color picking pass (off-screen) after all viewports
	//RenderColorPicking();
}

void URenderer::RenderScene(UEditor* Editor, int Idx)
{
	RenderLevel();
	Editor->RenderEditorBatched(Idx);
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

// ================== Sorting Batch ==================
void URenderer::CleanUpSortingBatch()
{
	SortingBatchMap.clear();
}
void URenderer::ReSetSortingBatchMap()
{
    if (bSortingBatchMapDirty == false)
    {
        return;
    }

    bSortingBatchMapDirty = false;
    const TArray<UPrimitiveComponent*>& PrimitiveComponents =
        ULevelManager::GetInstance().GetCurrentLevel()->GetLevelPrimitiveComponents();

    CleanUpSortingBatch();

    TArray<UStaticMeshComponent*> AllComps;
    for (auto& Primitive : PrimitiveComponents)
    {
        if (UStaticMeshComponent* S = Cast<UStaticMeshComponent>(Primitive))
        {
            if (S->IsVisible()) AllComps.push_back(S);
        }
    }

    SceneBVH.Build(AllComps);

    for (auto& Primitive : PrimitiveComponents)
    {
        UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Primitive);
        if (StaticMeshComponent != nullptr)
        {
            FStaticMesh* StaticMeshAsset = StaticMeshComponent->GetStaticMesh()->GetStaticMeshAsset();
            int MaterialSize = StaticMeshAsset->Materials.size();
            for (int i = 0; i < MaterialSize; i++)
            {
                FStaticMaterial* pMaterial = &StaticMeshAsset->Materials[i];
				//마테리얼 없으면 추가	
				if (SortingBatchMap.Contains(pMaterial) == false)
				{
					SortingBatchMap[pMaterial] =
					{
						{
							{StaticMeshAsset, {	{StaticMeshComponent, {}}}}
						}
					};
				}
				else
				{
					//스태틱메쉬 없으면 추가
					if (SortingBatchMap[pMaterial].Contains(StaticMeshAsset) == false)
					{
						SortingBatchMap[pMaterial][StaticMeshAsset] = { {StaticMeshComponent, {}} };
					}
					else
					{
						//스태틱 메쉬 컴포넌트 추가
						SortingBatchMap[pMaterial][StaticMeshAsset][StaticMeshComponent] = {};
					}
				}
			}

			TArray<FStaticMeshSection>& Sections = StaticMeshAsset->Sections;
			int SectionSize = StaticMeshAsset->Sections.size();
			for (int i = 0; i < SectionSize; i++)
			{
				FStaticMeshSection* pSection = &Sections[i];
				FStaticMaterial* SectionMaterial = &StaticMeshAsset->Materials[pSection->MaterialIndex];
				SortingBatchMap[SectionMaterial][StaticMeshAsset][StaticMeshComponent].Push(pSection);
			}
		}
	}
}

// ================== Specific Rendering Functions ==================

void URenderer::RenderLevel()
{
	TIME_PROFILE(RenderLevel)
	Pipeline->SetConstantBuffer(2, true, ConstantBufferColor);
	Pipeline->SetConstantBuffer(2, false, ConstantBufferColor);
	
	if (!ULevelManager::GetInstance().GetCurrentLevel() ||
		!IsShowFlagEnabled(EEngineShowFlags::SF_Primitives))
	{
		return;
	}
#ifdef _DEVELOP
	InitializeRenderStateChangeCount();
#endif
	//렌더스테이트 배치
	ReSetSortingBatchMap();
	RenderSortingBatchMap();

	//Legacy
	/*const TArray<UPrimitiveComponent*>& PrimitiveComponents =
		ULevelManager::GetInstance().GetCurrentLevel()->GetLevelPrimitiveComponents();

	for (UPrimitiveComponent* Component : PrimitiveComponents)
	{
		RenderStaticMeshComponent(Component);
	}*/
	
	//DisableInstancing();
}


void URenderer::RenderSortingBatchMap()
{
	TIME_PROFILE(RenderSortingBatchMap);
    SetupStaticMeshCommon();
    TStaticArray<FVector4, 6> Planes;
    ExtractFrustumPlanes(CachedViewProj, Planes);
    
    SceneBVH.QueryFrustum(Planes, Candidates);		
    FrameStamp++;
    uint32 MaxId = 0;
    for (auto* C : Candidates)
    {
        if (!C) continue;
        uint32 Id = static_cast<uint32>(C->GetInternalIndex());
        if (Id > MaxId) MaxId = Id;
    }
    if (VisibleStamp.Num() <= MaxId)
    {
        VisibleStamp.Reserve(MaxId + 1);
        while (VisibleStamp.Num() <= MaxId) VisibleStamp.push_back(0u);
    }
    for (auto* C : Candidates)
    {
        if (!C) continue;
        uint32 Id = static_cast<uint32>(C->GetInternalIndex());
        VisibleStamp[Id] = FrameStamp;
    }
    TArray<FStaticMaterial*> MaterialKeys = SortingBatchMap.GetKeys();

    for (FStaticMaterial* MaterialKey : MaterialKeys)
    {
        SetupMaterial(MaterialKey);
        TMap<FStaticMesh*, TMap<UStaticMeshComponent*, TArray<FStaticMeshSection*>>>& SortingMaterialMap = SortingBatchMap[MaterialKey];
        TArray<FStaticMesh*> StaticMeshKeys = SortingMaterialMap.GetKeys();
        for (FStaticMesh* StaticMeshKey : StaticMeshKeys)
        {
            SetupStaticMeshAsset(StaticMeshKey);
            TMap<UStaticMeshComponent*, TArray<FStaticMeshSection*>>& SortingMeshComponentMap = SortingMaterialMap[StaticMeshKey];
            TArray<UStaticMeshComponent*> MeshComponentKeys = SortingMeshComponentMap.GetKeys(); //병목지점 fix 여부(x)
            TArray<UStaticMeshComponent*> Filtered;
            Filtered.Reserve(MeshComponentKeys.Num());
            for (auto* CompKey : MeshComponentKeys)
            {
                if (!CompKey) continue;
                uint32 Id = static_cast<uint32>(CompKey->GetInternalIndex());
                if (Id < VisibleStamp.Num() && VisibleStamp[Id] == FrameStamp)
                {
                    Filtered.push_back(CompKey);
                }
            }
            MeshComponentKeys = std::move(Filtered);
#if SIMD_LEVEL >= 1
            if (true)
            {
                for (UStaticMeshComponent* MeshComponentKey : MeshComponentKeys)
                {
                    SetupStaticMeshComponent(MeshComponentKey);
                    TArray<FStaticMeshSection*>& SectionArray = SortingMeshComponentMap[MeshComponentKey];
                    for (FStaticMeshSection* Section : SectionArray)
                    {
                        Pipeline->DrawIndexed(Section->NumIndices, Section->FirstIndex, 0);
                    }
                }
            }
            else
            {
			int32 NumComponents = MeshComponentKeys.Num();

			// TArray를 직접 루프 돌지 않고 인덱스로 순회
			for (int32 i = 0; i < NumComponents; i += 4)
			{

				// 1. FAABB 4개 추출
				FAABB chunk_aabb[4];
				int32 chunk_size = std::min(4, NumComponents - i);

				// 2. AOS (chunk_aabb) -> SOA (simd_chunk) 변환
				FAABB_SIMD_Chunk simd_chunk;

				for (int32 j = 0; j < chunk_size; ++j) {

					chunk_aabb[j] = MeshComponentKeys[i + j]->GetWorldBounds(); //병목지점 fix 여부(x)

					simd_chunk.MinX[j] = chunk_aabb[j].Min.X;
					simd_chunk.MinY[j] = chunk_aabb[j].Min.Y;
					simd_chunk.MinZ[j] = chunk_aabb[j].Min.Z;

					simd_chunk.MaxX[j] = chunk_aabb[j].Max.X;
					simd_chunk.MaxY[j] = chunk_aabb[j].Max.Y;
					simd_chunk.MaxZ[j] = chunk_aabb[j].Max.Z;
				}

				// SIMD 안전을 위해 모든 필드를 무효 AABB 값(FLT_MAX)으로 초기화
				for (int32 j = chunk_size; j < 4; j++)
				{
					simd_chunk.MinX[j] = FLT_MAX;
					simd_chunk.MinY[j] = FLT_MAX;
					simd_chunk.MinZ[j] = FLT_MAX;

					simd_chunk.MaxX[j] = -FLT_MAX;
					simd_chunk.MaxY[j] = -FLT_MAX;
					simd_chunk.MaxZ[j] = -FLT_MAX;
				}

				// 3. SIMD 컬링 함수 호출
				__m128 render_mask = TestAABBFrustum_Chunk_SIMD(simd_chunk, Planes);
				int32 mask_int = _mm_movemask_ps(render_mask); // 4비트 정수 마스크로 변환



				// 4. 마스크를 사용하여 개별 액터 처리
				for (int32 j = 0; j < chunk_size; ++j) {
					if ((mask_int >> j) & 1) // j번째 비트가 1이면 렌더링 대상
					{
						UStaticMeshComponent* MeshComponentKey = MeshComponentKeys[i + j];
						SetupStaticMeshComponent(MeshComponentKey); //병목지점 fix 여부(x)
						TArray<FStaticMeshSection*>& SectionArray = SortingMeshComponentMap[MeshComponentKey];  //병목지점 fix 여부(x)
						for (FStaticMeshSection* Section : SectionArray)
						{
							Pipeline->DrawIndexed(Section->NumIndices, Section->FirstIndex, 0);
#ifdef _DEVELOP
							MeshSectionDrawCount++;
#endif
						}

					}
				}

			}
            }
#elif
			if (true)
			{
				for (UStaticMeshComponent* MeshComponentKey : MeshComponentKeys)
				{
					SetupStaticMeshComponent(MeshComponentKey);
					TArray<FStaticMeshSection*>& SectionArray = SortingMeshComponentMap[MeshComponentKey];
					for (FStaticMeshSection* Section : SectionArray)
					{
						Pipeline->DrawIndexed(Section->NumIndices, Section->FirstIndex, 0);
					}
				}
			}
			else
			{
			for (UStaticMeshComponent* MeshComponentKey : MeshComponentKeys)
			{
				FAABB Bounds = MeshComponentKey->GetWorldBounds();
				if (!TestAABBFrustum(Bounds, Planes))
				{
					continue;
				}
				SetupStaticMeshComponent(MeshComponentKey);
				TArray<FStaticMeshSection*>& SectionArray = SortingMeshComponentMap[MeshComponentKey];
				for (FStaticMeshSection* Section : SectionArray)
				{
					Pipeline->DrawIndexed(Section->NumIndices, Section->FirstIndex, 0);
#ifdef _DEVELOP
					MeshSectionDrawCount++;
#endif
				}
			}
			}
#endif
        }
    }
}
void URenderer::SetupStaticMeshCommon()
{
	FRenderState RenderState;
	RenderState.CullMode = ECullMode::Back;
	RenderState.FillMode = EFillMode::Solid;

	Pipeline->UpdatePipeline(CreatePipelineInfo(RenderState));
	GetDeviceContext()->PSSetSamplers(0, 1, &DiffuseSampler);

}
void URenderer::SetupMaterial(FStaticMaterial* Material)
{
	ID3D11ShaderResourceView* SRV = nullptr;
	FMaterialParamsCB MaterialParams{};
	MaterialParams.DiffuseColor = FVector4(Material->DiffuseColor, 1);
	MaterialParams.UseTexture = Material->bUseTexture ? 1 : 0;
	if (MaterialParams.UseTexture) { SRV = Material->TextureSRV; }

	GetDeviceContext()->PSSetShaderResources(1, 1, &SRV);
	Pipeline->SetConstantBuffer(4, false, ConstantBufferMaterialParam);
	UpdateBuffer(ConstantBufferMaterialParam, MaterialParams);
#ifdef _DEVELOP
	MaterialChangeCount++;
#endif
}
void URenderer::SetupStaticMeshAsset(FStaticMesh* StaticMeshAsset)
{
	// Set buffers and topology
	UINT Offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &StaticMeshAsset->VertexBuffer, &StaticStride, &Offset);
	GetDeviceContext()->IASetIndexBuffer(StaticMeshAsset->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
#ifdef _DEVELOP
	StaticMeshChangeCount++;
#endif
}
void URenderer::SetupStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent)
{
TIME_PROFILE(SetupStaticMeshComponent)
	UpdateBuffer(ConstantBufferModels, StaticMeshComponent->GetWorldTransformMatrix());
#ifdef _DEVELOP
	StaticMeshComponentChagneCount++;
#endif
}

bool URenderer::ShouldPerformColorPicking()
{
	// Check if mouse is pressed
	UInputManager& InputManager = UInputManager::GetInstance();
	bool bMousePressed = InputManager.IsKeyDown(EKeyInput::MouseLeft) || InputManager.IsKeyDown(EKeyInput::MouseRight);
	if (!bMousePressed)
	{
		return false;
	}

	// UI위에 마우스 있으면 안그림
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return false;
	}

	return true;
}

void URenderer::RenderColorPicking()
{
	if (!ULevelManager::GetInstance().GetCurrentLevel())
	{
		return;
	}

	// Switch to color picking render target
	ID3D11RenderTargetView* ColorPickingRTV = DeviceResources->GetColorPickingRTV();
	ID3D11DepthStencilView* ColorPickingDSV = DeviceResources->GetColorPickingDSV();

	// Clear color picking render target (0 = no object)
	const FLOAT ClearColorPicking[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetDeviceContext()->ClearRenderTargetView(ColorPickingRTV, ClearColorPicking);
	GetDeviceContext()->ClearDepthStencilView(ColorPickingDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Set render targets and scaled viewport for resolution optimization
	GetDeviceContext()->OMSetRenderTargets(1, &ColorPickingRTV, ColorPickingDSV);

	// Set scaled viewport for color picking
	D3D11_VIEWPORT ColorPickingViewport = DeviceResources->GetColorPickingViewport();
	GetDeviceContext()->RSSetViewports(1, &ColorPickingViewport);

	// Set constant buffers
	Pipeline->SetConstantBuffer(2, true, ConstantBufferPicking);
	Pipeline->SetConstantBuffer(2, false, ConstantBufferPicking);

	const TArray<UPrimitiveComponent*>& PrimitiveComponents =
		ULevelManager::GetInstance().GetCurrentLevel()->GetLevelPrimitiveComponents();

	for (UPrimitiveComponent* Candidate : Candidates)
	{
		RenderStaticMeshComponentForPicking(Candidate);
	}

	DisableInstancing();
}

void URenderer::RenderStaticMeshComponentForPicking(UPrimitiveComponent* Component)
{
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
	if (!StaticMeshComponent || !StaticMeshComponent->IsVisible()) return;

	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	if (!StaticMesh) return;

	FStaticMesh* MeshData = StaticMesh->GetStaticMeshAsset();
	if (!MeshData) return;

	// Set up picking-specific rendering
	SetupPickingMeshRendering(StaticMeshComponent, MeshData);
	RenderStaticMeshSections(StaticMeshComponent, MeshData);
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
	RenderStaticMeshSections(StaticMeshComponent, MeshData);
}

void URenderer::SetupStaticMeshRendering(UStaticMeshComponent* Component, FStaticMesh* MeshData)
{
	Pipeline->UpdatePipeline(CreatePipelineInfo(Component->GetRenderState()));

	// Set constant buffers
	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);

	// Update constants
	UpdateBuffer(ConstantBufferModels, Component->GetWorldTransformMatrix());
	UpdateBuffer(ConstantBufferColor, FVector4(0.f, 0.f, 0.f, 0.f));

	//Pipeline->SetConstantBuffer(3, true, ConstantBufferInstance);
	InstanceDrawConstants InstanceConstants{};
	InstanceConstants.bUseInstancing = 0;
	InstanceConstants.BaseInstanceOffset = 0;
	InstanceConstants.InstanceCount = 0;
	//UpdateBuffer(ConstantBufferInstance, InstanceConstants);

	// Set buffers and topology
	UINT Offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &MeshData->VertexBuffer, &StaticStride, &Offset);
	GetDeviceContext()->IASetIndexBuffer(MeshData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GetDeviceContext()->PSSetSamplers(0, 1, &DiffuseSampler);
}

void URenderer::SetupPickingMeshRendering(UStaticMeshComponent* Component, FStaticMesh* MeshData)
{
	// Create render state for picking (no culling, solid fill)
	FRenderState PickingRenderState = Component->GetRenderState();
	PickingRenderState.CullMode = ECullMode::None;
	PickingRenderState.FillMode = EFillMode::Solid;

	// Create pipeline info with picking pixel shader
	FPipelineInfo PipelineInfo = CreatePipelineInfo(PickingRenderState);
	PipelineInfo.PixelShader = PickingPixelShader;

	Pipeline->UpdatePipeline(PipelineInfo);

	// Set constant buffers
	Pipeline->SetConstantBuffer(0, true, ConstantBufferModels);

	// Update constants
	UpdateBuffer(ConstantBufferModels, Component->GetWorldTransformMatrix());

    // Update picking constant buffer with object ID
    struct PickingConstants
    {
        uint32 Pick = 1;
        uint32 ObjectID = 0;
        int32 Padding[2] = {0, 0};
    };

	PickingConstants PickingCB;
	if (Component->GetOwner())
	{
		bool bIsUUIDPicking = Editor->bUUIDColorPicking;
		bool bIsIndexPicking = Editor->bIndexColorPicking;

		if (bIsUUIDPicking == true && bIsIndexPicking == false)
		{
			PickingCB.ObjectID = Component->GetOwner()->GetUUID();
		}

		if (bIsUUIDPicking == false && bIsIndexPicking == true)
		{
			PickingCB.ObjectID = Component->GetOwner()->GetInternalIndex();
		}
	}
	UpdateBuffer(ConstantBufferPicking, PickingCB);

	// Set buffers and topology
	UINT Offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &MeshData->VertexBuffer, &StaticStride, &Offset);
	GetDeviceContext()->IASetIndexBuffer(MeshData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void URenderer::RenderStaticMeshSections(const UStaticMeshComponent* OwnerComponent, FStaticMesh* MeshData)
{
	for (const FStaticMeshSection& Section : MeshData->Sections)
	{
		SetupMaterialForSection(OwnerComponent, MeshData, Section);
		GetDeviceContext()->DrawIndexed(Section.NumIndices, Section.FirstIndex, 0);
	}
}

void URenderer::SetupMaterialForSection(const UStaticMeshComponent* OwnerComponent, FStaticMesh* MeshData, const FStaticMeshSection& Section)
{
	ID3D11ShaderResourceView* SRV = nullptr;
	FMaterialParamsCB MaterialParams{};

	if (Section.MaterialIndex >= 0 && Section.MaterialIndex < MeshData->Materials.Num())
	{
		FStaticMaterial& Material = MeshData->Materials[Section.MaterialIndex];

		MaterialParams.UseTexture = Material.bUseTexture ? 1 : 0;
		if (MaterialParams.UseTexture) { SRV = Material.TextureSRV; }

		MaterialParams.AmbientColor = FVector4(Material.AmbientColor.X, Material.AmbientColor.Y, Material.AmbientColor.Z, 1.0f);
		MaterialParams.DiffuseColor = FVector4(Material.DiffuseColor.X, Material.DiffuseColor.Y, Material.DiffuseColor.Z, Material.Alpha);
		MaterialParams.SpecularColor = FVector4(Material.SpecularColor.X, Material.SpecularColor.Y, Material.SpecularColor.Z, 1.0f);
		MaterialParams.SpecularExponent = Material.SpecularExponent;

        MaterialParams.UVScrollSpeed = OwnerComponent->GetUVScrollSpeed();
        MaterialParams.Time = OwnerComponent->GetUVScrollTimeForShader();
    }
	GetDeviceContext()->PSSetShaderResources(1, 1, &SRV);

	Pipeline->SetConstantBuffer(4, false, ConstantBufferMaterialParam);
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
	ID3D11ShaderResourceView* SRV = ResourceManager.GetTexture(
		(UPathManager::GetInstance().GetFontPath() / "Pretendard-Regular.dds").string());
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
	Desc.FrontCounterClockwise = TRUE;

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
	CleanUpSortingBatch();
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
	SafeRelease(PickingPixelShader);
}

void URenderer::CleanupBuffers()
{
	SafeRelease(ConstantBufferModels);
	SafeRelease(ConstantBufferColor);
	SafeRelease(ConstantBufferPerFrame);
	SafeRelease(ConstantBufferInstance);
	SafeRelease(ConstantBufferCharTable);
	SafeRelease(ConstantBufferMaterialParam);
	SafeRelease(ConstantBufferPicking);
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

#if IS_OBJ_VIEWER
// jft
void URenderer::RenderObjectViewer(UEditor* Editor)
{
	// 1. Get resources
	ID3D11RenderTargetView* ObjectViewerRTV = DeviceResources->GetObjectViewerRTV();
	ID3D11DepthStencilView* ObjectViewerDSV = DeviceResources->GetObjectViewerDSV();
	UObjectPreviewScene* PreviewScene = Editor->GetObjPreview();

	if (!ObjectViewerRTV || !ObjectViewerDSV || !PreviewScene)
	{
		return;
	}

	const TArray<UPrimitiveComponent*>& PrimitiveComponents = PreviewScene->GetPrimitiveInObjViewer();
	if (PrimitiveComponents.IsEmpty())
	{
		return;
	}

	// 2. Store original targets and viewport
	ID3D11RenderTargetView* OriginalRTV = nullptr;
	ID3D11DepthStencilView* OriginalDSV = nullptr;
	GetDeviceContext()->OMGetRenderTargets(1, &OriginalRTV, &OriginalDSV);

	D3D11_VIEWPORT OriginalViewport;
	UINT NumViewports = 1;
	GetDeviceContext()->RSGetViewports(&NumViewports, &OriginalViewport);

	// 3. Set new render target and viewport for Object Viewer
	GetDeviceContext()->OMSetRenderTargets(1, &ObjectViewerRTV, ObjectViewerDSV);

	D3D11_VIEWPORT ObjectViewerViewport = {};
	ObjectViewerViewport.Width = 1024; // Must match the texture size in DeviceResources
	ObjectViewerViewport.Height = 1024;
	ObjectViewerViewport.MinDepth = 0.0f;
	ObjectViewerViewport.MaxDepth = 1.0f;
	ObjectViewerViewport.TopLeftX = 0;
	ObjectViewerViewport.TopLeftY = 0;
	GetDeviceContext()->RSSetViewports(1, &ObjectViewerViewport);

	// 4. Clear the new render target
	GetDeviceContext()->ClearRenderTargetView(ObjectViewerRTV, ClearColor);
	GetDeviceContext()->ClearDepthStencilView(ObjectViewerDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// 5. Set camera constants (TODO: Use a dedicated preview camera)
	// For now, using the main editor camera as a placeholder
	FViewProjConstants Constants = Editor->GetObjPreview()->GetCamera()->GetFViewProjConstants();
	UpdateViewProjConstants(Constants);

	// 6. Render the component(s)
	const TArray<UPrimitiveComponent*>& Components = Editor->GetObjPreview()->GetPrimitiveInObjViewer();
	if (!Components.IsEmpty())
	{
		for (UPrimitiveComponent* Component : Components)
		{
			RenderStaticMeshComponent(Component);
		}
	}

	// 7. Restore original render target and viewport
	GetDeviceContext()->OMSetRenderTargets(1, &OriginalRTV, OriginalDSV);
	GetDeviceContext()->RSSetViewports(1, &OriginalViewport);

	// Release the COM objects we obtained from OMGetRenderTargets
	SafeRelease(OriginalRTV);
	SafeRelease(OriginalDSV);
}
#endif
