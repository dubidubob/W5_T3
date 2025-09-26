#pragma once
#include "DeviceResources.h"
#include "Core/Object.h"
#include "Mesh/SceneComponent.h"
#include "Editor/EditorPrimitive.h"
#include "Editor/Camera.h"
#include "ViewportTypes.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Global/CoreTypes.h"
#include "Math/BVH.h"

class UPipeline;
class UDeviceResources;
class UPrimitiveComponent;
class AActor;
class AGizmo;
class UEditor;
class UTextComponent;
class SWindow;
struct FPipelineInfo;
struct FStaticMesh;
class UStaticMeshComponent;

/**
 * @brief Rendering Pipeline 전반을 처리하는 클래스
 *
 * DirectX 11 기반의 렌더링 파이프라인을 관리하며,
 * 셰이더, 버퍼, 렌더 상태 등을 통합적으로 처리합니다.
 */
class URenderer : public UObject
{
	DECLARE_CLASS(URenderer, UObject)
	DECLARE_SINGLETON(URenderer)

public:
	// ================== Core Lifecycle ==================
	void Init(HWND WindowHandle);
	void Release();
	void Update(UEditor* Editor);
	void OnResize(uint32 Width = 0, uint32 Height = 0);

	// ================== Rendering Functions ==================
	void RenderBegin();
	void RenderEnd() const;
	void RenderLevel();
	void RenderColorPicking();
	bool ShouldPerformColorPicking();
	void RenderText(const FVector& CameraLocation);
	void RenderSlate(UEditor* Editor);
	void RenderEditorPrimitive(FEditorPrimitive& Primitive, FRenderState& RenderState);
	void RenderSortingBatchMap();

	// ================== Buffer Creation Templates ==================
	template<typename T>
	ID3D11Buffer* CreateVertexBuffer(const TArray<T>& Vertices) const
	{
		D3D11_BUFFER_DESC Desc = {};
		Desc.ByteWidth = static_cast<UINT>(Vertices.size() * sizeof(T));
		Desc.Usage = D3D11_USAGE_IMMUTABLE;
		Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA InitData = { Vertices.data() };

		ID3D11Buffer* Buffer = nullptr;
		GetDevice()->CreateBuffer(&Desc, &InitData, &Buffer);
		return Buffer;
	}

	template<typename T>
	ID3D11Buffer* CreateIndexBuffer(const TArray<T>& Indices) const
	{
		D3D11_BUFFER_DESC Desc = {};
		Desc.ByteWidth = static_cast<UINT>(Indices.size() * sizeof(T));
		Desc.Usage = D3D11_USAGE_IMMUTABLE;
		Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA InitData = { Indices.data() };

		ID3D11Buffer* Buffer = nullptr;
		GetDevice()->CreateBuffer(&Desc, &InitData, &Buffer);
		return Buffer;
	}

	// ================== Static Buffer Utility ==================
	static void ReleaseVertexBuffer(ID3D11Buffer* VertexBuffer);

	// ================== Buffer Updates ==================
	template<typename T>
	void UpdateBuffer(ID3D11Buffer* Buffer, const T& Data) const
	{
		if (!Buffer) return;

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		if (SUCCEEDED(GetDeviceContext()->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource)))
		{
			memcpy(MappedResource.pData, &Data, sizeof(T));
			GetDeviceContext()->Unmap(Buffer, 0);
		}
	}

	void UpdateViewProjConstants(const FViewProjConstants& ViewProj);
	void UpdateInstance(const TArray<FTextInstance>* Instances);
	void UpdateInstanceDrawConstants(bool UseInstancing, uint32 BaseOffset, uint32 InstanceCount) const;

	// ================== Sorting Batch ==================
	void SetSortingBatchMapDirty()
	{
		bSortingBatchMapDirty = true;
	}
	void ReSetSortingBatchMap();
	void CleanUpSortingBatch();

	// ================== View Mode Management ==================
	void SetViewMode(EViewportRenderMode ViewMode) { CurrentRenderMode = ViewMode; }
	EViewportRenderMode GetViewMode() const { return CurrentRenderMode; }

	// ================== Show Flags Management ==================
	void SetShowFlags(EEngineShowFlags ShowFlags) { CurrentShowFlags = ShowFlags; }
	EEngineShowFlags GetShowFlags() const { return CurrentShowFlags; }
	void ToggleShowFlag(EEngineShowFlags Flag) { CurrentShowFlags = CurrentShowFlags ^ Flag; }
	bool IsShowFlagEnabled(EEngineShowFlags Flag) const { return HasFlag(CurrentShowFlags, Flag); }

	// ================== Window Management ==================
	bool GetIsResizing() const { return bIsResizing; }
	void SetIsResizing(bool IsResizing) { bIsResizing = IsResizing; }

	// ================== Device Access ==================
	ID3D11Device* GetDevice() const { return DeviceResources->GetDevice(); }
	ID3D11DeviceContext* GetDeviceContext() const { return DeviceResources->GetDeviceContext(); }
	IDXGISwapChain* GetSwapChain() const { return DeviceResources->GetSwapChain(); }
	ID3D11RenderTargetView* GetRenderTargetView() const { return DeviceResources->GetRenderTargetView(); }
	UDeviceResources* GetDeviceResources() const { return DeviceResources; }

	// ================== Pipeline Access ==================
	UPipeline* GetPipeline() const { return Pipeline; }

	// Shader Access
	ID3D11InputLayout* GetDefaultInputLayout() const { return DefaultInputLayout; }
	ID3D11InputLayout* GetStaticInputLayout() const { return StaticInputLayout; }
	ID3D11InputLayout* GetLineInstancedInputLayout() const { return LineInstancedInputLayout; }

	ID3D11VertexShader* GetDefaultVertexShader() const { return DefaultVertexShader; }
	ID3D11VertexShader* GetStaticVertexShader() const { return StaticVertexShader; }
	ID3D11VertexShader* GetLineInstancedVertexShader() const { return LineInstancedVertexShader; }

	ID3D11PixelShader* GetDefaultPixelShader() const { return DefaultPixelShader; }
	ID3D11PixelShader* GetStaticPixelShader() const { return StaticPixelShader; }
	ID3D11PixelShader* GetLineInstancedPixelShader() const { return LineInstancedPixelShader; }
	ID3D11PixelShader* GetPickingPixelShader() const { return PickingPixelShader; }

	// State Access
	ID3D11DepthStencilState* GetDefaultDepthStencilState() const { return DefaultDepthStencilState; }
	ID3D11RasterizerState* GetRasterizerState(const FRenderState& RenderState);

	UEditor* GetEditor() { return Editor; }
	void SetEditor(UEditor* InEditor) { Editor = InEditor; }

#ifdef _DEVELOP
	const uint32 GetMaterialChangeCount() const { return MaterialChangeCount; }
	const uint32 GetStaticMeshChangeCount() const { return StaticMeshChangeCount; }
	const uint32 GetStaticMeshComponentChagneCount() const { return StaticMeshComponentChagneCount; }
	const uint32 GetMeshSectionDrawCount() const { return MeshSectionDrawCount; }
#endif

private:
	UEditor* Editor = nullptr;
	bool bSortingBatchMapDirty = true;
	TMap<FStaticMaterial*, TMap<FStaticMesh*, TMap<UStaticMeshComponent*, TArray<FStaticMeshSection*>>>> SortingBatchMap;

#ifdef _DEVELOP
	uint32 MaterialChangeCount = 0;
	uint32 StaticMeshChangeCount = 0;
	uint32 StaticMeshComponentChagneCount = 0;
	uint32 MeshSectionDrawCount = 0;
#endif

		// ================== Core Components ==================
	UPipeline* Pipeline = nullptr;
	UDeviceResources* DeviceResources = nullptr;

	// ================== Render Settings ==================
	EViewportRenderMode CurrentRenderMode = EViewportRenderMode::Lit;
	EEngineShowFlags CurrentShowFlags = EEngineShowFlags::SF_Default;
	bool bIsResizing = false;

	// ================== Clear Color ==================
	FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

	// ================== Render States ==================
	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	ID3D11DepthStencilState* DisabledDepthStencilState = nullptr;
	ID3D11DepthStencilState* TextDepthStencilState = nullptr;
	ID3D11BlendState* TextBlendState = nullptr;

	// ================== Constant Buffers ==================
	ID3D11Buffer* ConstantBufferModels = nullptr;
	ID3D11Buffer* ConstantBufferPerFrame = nullptr;
	ID3D11Buffer* ConstantBufferColor = nullptr;
	ID3D11Buffer* ConstantBufferCharTable = nullptr;
	ID3D11Buffer* ConstantBufferInstance = nullptr;
	ID3D11Buffer* ConstantBufferMaterialParam = nullptr;
	ID3D11Buffer* ConstantBufferPicking = nullptr;

	// ================== Instance Buffers ==================
	ID3D11Buffer* TextInstanceBuffer = nullptr;

	// ================== Samplers ==================
	ID3D11SamplerState* DiffuseSampler = nullptr;

	// ================== Default Shader Set ==================
	ID3D11VertexShader* DefaultVertexShader = nullptr;
	ID3D11PixelShader* DefaultPixelShader = nullptr;
	ID3D11InputLayout* DefaultInputLayout = nullptr;

	// ================== Static Mesh Shader Set ==================
	ID3D11VertexShader* StaticVertexShader = nullptr;
	ID3D11PixelShader* StaticPixelShader = nullptr;
	ID3D11InputLayout* StaticInputLayout = nullptr;

	// ================== Text Shader Set ==================
	ID3D11VertexShader* TextVertexShader = nullptr;
	ID3D11PixelShader* TextPixelShader = nullptr;
	ID3D11InputLayout* TextInputLayout = nullptr;

	// ================== Slate Shader Set ==================
	ID3D11VertexShader* SlateVertexShader = nullptr;
	ID3D11PixelShader* SlatePixelShader = nullptr;
	ID3D11InputLayout* SlateInputLayout = nullptr;

	// ================== Line Instanced Shader Set ==================
	ID3D11VertexShader* LineInstancedVertexShader = nullptr;
	ID3D11PixelShader* LineInstancedPixelShader = nullptr;
	ID3D11InputLayout* LineInstancedInputLayout = nullptr;

    FBVH SceneBVH;
	// ================== Picking Shader Set ==================
	ID3D11PixelShader* PickingPixelShader = nullptr;

	// ================== Vertex Strides ==================
	uint32 Stride = 0;
	uint32 StaticStride = 0;
	FViewProjConstants CachedViewProj{};
	uint32 StrideTextVertex = 0;
	uint32 StrideTextInstance = 0;

	// Frame-stamp visibility marking to avoid per-frame TSet construction
	TArray<uint32> VisibleStamp;
	uint32 FrameStamp = 1;

	// ================== Batching Structures ==================
	struct FPrimitiveBatchKey
	{
		ID3D11Buffer* VertexBuffer = nullptr;
		ID3D11Buffer* IndexBuffer = nullptr;
		uint32 IndexCount = 0;
		FRenderState RenderState = {};

		bool operator==(const FPrimitiveBatchKey& Other) const
		{
			return VertexBuffer == Other.VertexBuffer &&
				IndexBuffer == Other.IndexBuffer &&
				IndexCount == Other.IndexCount &&
				RenderState.CullMode == Other.RenderState.CullMode &&
				RenderState.FillMode == Other.RenderState.FillMode;
		}
	};

	struct FPrimitiveBatchKeyHasher
	{
		size_t operator()(const FPrimitiveBatchKey& Key) const noexcept
		{
			auto Mix = [](size_t& Hash, size_t Value)
				{
					Hash ^= Value + 0x9e3779b97f4a7c15ULL + (Hash << 6) + (Hash >> 2);
				};

			size_t Hash = 0;
			Mix(Hash, reinterpret_cast<size_t>(Key.VertexBuffer));
			Mix(Hash, reinterpret_cast<size_t>(Key.IndexBuffer));
			Mix(Hash, static_cast<size_t>(Key.IndexCount));
			Mix(Hash, static_cast<size_t>(Key.RenderState.CullMode));
			Mix(Hash, static_cast<size_t>(Key.RenderState.FillMode));
			return Hash;
		}
	};

	struct FInstanceBufferResource
	{
		ID3D11Buffer* Buffer = nullptr;
		ID3D11ShaderResourceView* ShaderResourceView = nullptr;
		uint32 Capacity = 0;
	};

	TMap<FPrimitiveBatchKey, FInstanceBufferResource, FPrimitiveBatchKeyHasher> PrimitiveInstanceBuffers;

	// ================== Rasterizer State Caching ==================
	struct FRasterKey
	{
		D3D11_FILL_MODE FillMode = {};
		D3D11_CULL_MODE CullMode = {};

		bool operator==(const FRasterKey& Other) const
		{
			return FillMode == Other.FillMode && CullMode == Other.CullMode;
		}
	};

	struct FRasterKeyHasher
	{
		size_t operator()(const FRasterKey& Key) const noexcept
		{
			auto Mix = [](size_t& Hash, size_t Value)
				{
					Hash ^= Value + 0x9e3779b97f4a7c15ULL + (Hash << 6) + (Hash >> 2);
				};

			size_t Hash = 0;
			Mix(Hash, static_cast<size_t>(Key.FillMode));
			Mix(Hash, static_cast<size_t>(Key.CullMode));
			return Hash;
		}
	};

	TMap<FRasterKey, ID3D11RasterizerState*, FRasterKeyHasher> RasterCache;

	// ================== Render Object for Text Sorting ==================
	struct TextRenderObject
	{
		UTextComponent* Component;
		float DistanceToCamera;
		bool operator<(const TextRenderObject& Other) const
		{
			return DistanceToCamera > Other.DistanceToCamera;
		}
	};

private:
	// ================== Initialization Functions ==================
	void InitializeRenderStates();
	void InitializeShaders();
	void InitializeBuffers();
#ifdef _DEVELOP
	void InitializeRenderStateChangeCount();
#endif

	// ================== Creation Functions ==================
	void CreateDepthStencilState(ID3D11DepthStencilState*& State, bool DepthEnable, D3D11_DEPTH_WRITE_MASK WriteMask);
	void CreateBlendState();
	void CreateShaderSet(const wchar_t* ShaderPath, const char* VSEntry, const char* PSEntry,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& InputElements,
		ID3D11VertexShader*& VertexShader, ID3D11PixelShader*& PixelShader,
		ID3D11InputLayout*& InputLayout);
	void CreateConstantBuffer(ID3D11Buffer*& Buffer, size_t Size);
	void CreateCharacterTableBuffer();
	void CreateTextInstanceBuffer();
	void CreateSamplerState();

	// ================== Rendering Functions ==================
	void SetupStaticMeshCommon();
	void SetupMaterial(FStaticMaterial* Material);
	void SetupStaticMeshAsset(FStaticMesh* StaticMeshAsset);
	void SetupStaticMeshComponent(UStaticMeshComponent* StaticMeshComponent);
	
	void RenderMultiViewport(UEditor* Editor);
	void RenderScene(UEditor* Editor, int Idx = 0);
	void RenderStaticMeshComponent(UPrimitiveComponent* Component);
	void RenderStaticMeshComponentForPicking(UPrimitiveComponent* Component);
	void SetupStaticMeshRendering(UStaticMeshComponent* Component, FStaticMesh* MeshData);
	void SetupPickingMeshRendering(UStaticMeshComponent* Component, FStaticMesh* MeshData);
	void RenderStaticMeshSections(const UStaticMeshComponent* OwnerComponent, FStaticMesh* MeshData);
	void SetupMaterialForSection(const UStaticMeshComponent* OwnerComponent, FStaticMesh* MeshData, const struct FStaticMeshSection& Section);
	void SetupTextRendering();
	void RenderTextComponent(UTextComponent* Component);

#if IS_OBJ_VIEWER
	void RenderObjectViewer(UEditor* Editor);
#endif

	FVector CalculateTextPosition(UTextComponent* Component);
	void RenderWindow(SWindow* Window, const FVector2& MouseCoord);

	// ================== Pipeline Creation ==================
	FPipelineInfo CreatePipelineInfo(const FRenderState& RenderState);
	FPipelineInfo CreateTextPipelineInfo(const FRenderState& RenderState);
	FRenderState ApplyViewModeToRenderState(const FRenderState& OriginalState);

	// ================== Instance Buffer Management ==================
	FInstanceBufferResource& GetOrCreateInstanceBuffer(const FPrimitiveBatchKey& Key);
	void EnsureInstanceBufferCapacity(FInstanceBufferResource& Resource, uint32 RequiredCount);
	void CreateStructuredBuffer(FInstanceBufferResource& Resource, uint32 Capacity);
	void UploadInstanceBufferData(FInstanceBufferResource& Resource, const void* Data, uint32 InstanceCount);

	// ================== Utility Functions ==================
	void DisableInstancing();
	template<typename T>
	void SafeRelease(T*& Ptr)
	{
		if (Ptr)
		{
			Ptr->Release();
			Ptr = nullptr;
		}
	}

	// ================== Cleanup Functions ==================
	void CleanupAll();
	void CleanupRenderStates();
	void CleanupShaders();
	void CleanupBuffers();
	void ReleaseShaderSet(ID3D11VertexShader*& VS, ID3D11PixelShader*& PS, ID3D11InputLayout*& Layout);
};
