#pragma once
#include "DeviceResources.h"
#include "Core/Object.h"
#include "Mesh/SceneComponent.h"
#include "Editor/EditorPrimitive.h"

class UPipeline;
class UDeviceResources;
class UPrimitiveComponent;
class AActor;
class AGizmo;
class UEditor;
struct FPipelineInfo;
/**
 * @brief Rendering Pipeline 전반을 처리하는 클래스
 *
 * Direct3D 11 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 포인터들
 * @param Device GPU와 통신하기 위한 Direct3D 장치
 * @param DeviceContext GPU 명령 실행을 담당하는 컨텍스트
 * @param SwapChain 프레임 버퍼를 교체하는 데 사용되는 스왑 체인
 *
 * // 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
 * @param FrameBuffer 화면 출력용 텍스처
 * @param FrameBufferRTV 텍스처를 렌더 타겟으로 사용하는 뷰
 * @param RasterizerState 래스터라이저 상태(컬링, 채우기 모드 등 정의)
 * @param ConstantBuffer 쉐이더에 데이터를 전달하기 위한 상수 버퍼
 *
 * @param ClearColor 화면을 초기화(clear)할 때 사용할 색상 (RGBA)
 * @param ViewportInfo 렌더링 영역을 정의하는 뷰포트 정보
 *
 * @param DefaultVertexShader
 * @param DefaultPixelShader
 * @param DefaultInputLayout
 * @param Stride
 *
 * @param vertexBufferSphere
 * @param numVerticesSphere
 */
class URenderer : public UObject
{
	DECLARE_CLASS(URenderer, UObject)
	DECLARE_SINGLETON(URenderer)

public:
	void Init(HWND InWindowHandle);
	void Release();

	void CreateRasterizerState();
	void CreateDepthStencilState();
	void CreateBlendState();
	void CreateDefaultShader();
	void CreateTextShader();
	void CreateLineInstancedShader();
	void CreateConstantBuffer();

	void ReleaseDefaultShader();
	void ReleaseTextShader();
	void ReleaseLineInstancedShader();
	static void ReleaseVertexBuffer(ID3D11Buffer* InVertexBuffer);
	void ReleaseConstantBuffer();
	void ReleaseRasterizerState();
	void ReleaseResource();
	void ReleaseBlendState();
	void ReleaseInstanceBuffer();

	void Update(UEditor* Editor);
	//void Update();
	void RenderBegin();
	void RenderLevel();
	void RenderTest(const FVector& CameraLocation);
	void RenderEnd() const;
	void RenderEditorPrimitive(FEditorPrimitive& InPrimitive, struct FRenderState& InRenderState);
	void RenderBoundingBox(UPrimitiveComponent* PrimitiveComponent);

	void OnResize(uint32 Inwidth = 0, uint32 InHeight = 0);
	bool GetIsResizing() { return bIsResizing;}
	void SetIsResizing(bool isResizing) { bIsResizing = isResizing; }


	template<typename T>
	ID3D11Buffer* CreateVertexBuffer(TArray<T>& InVertices) const
	{
		D3D11_BUFFER_DESC VertexBufferDesc = {};
		VertexBufferDesc.ByteWidth = InVertices.size() * sizeof(T);
		VertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
		VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA VertexBufferSRD = { InVertices.data()};

		ID3D11Buffer* VertexBuffer;

		GetDevice()->CreateBuffer(&VertexBufferDesc, &VertexBufferSRD, &VertexBuffer);

		return VertexBuffer;
	}

	template<typename T>
	ID3D11Buffer* CreateIndexBuffer(TArray<T>& InIndices) const
	{
		D3D11_BUFFER_DESC IndexBufferDesc = {};
		IndexBufferDesc.ByteWidth = InIndices.size() * sizeof(T);
		IndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated
		IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA IndexBufferSRD = { InIndices.data()};

		ID3D11Buffer* IndexBuffer;

		GetDevice()->CreateBuffer(&IndexBufferDesc, &IndexBufferSRD, &IndexBuffer);

		return IndexBuffer;
	}

	void CreateInstanceBuffer();

	void UpdateConstant(const UPrimitiveComponent* Primitive);
	void UpdateConstant(const FMatrix& InMatrix) const;
	void UpdateConstant(const FVector& InPosition, const FVector& InRotation, const FVector& InScale) const;
	void UpdateConstant(const FViewProjConstants& InViewProjConstants) const;
	void UpdateConstant(const FVector4& Color) const;
	void UpdateInstance(const TArray<FTextInstance>* Instance);
	void UpdateInstanceDrawConstants(bool bUseInstancing, uint32 BaseInstanceOffset, uint32 InstanceCount) const;

	void SetViewMode(EViewModeIndex InViewMode) { CurrentViewMode = InViewMode; }
	EViewModeIndex GetViewMode(EViewModeIndex InViewMode) const { return CurrentViewMode; }

	/** Show Flags management */
	void SetShowFlags(EEngineShowFlags InShowFlags) { CurrentShowFlags = InShowFlags; }
	EEngineShowFlags GetShowFlags() const { return CurrentShowFlags; }
	void ToggleShowFlag(EEngineShowFlags InFlag) { CurrentShowFlags = CurrentShowFlags ^ InFlag; }
	bool IsShowFlagEnabled(EEngineShowFlags InFlag) const { return HasFlag(CurrentShowFlags, InFlag); }

	ID3D11Device* GetDevice() const { return DeviceResources->GetDevice(); }
	ID3D11DeviceContext* GetDeviceContext() const { return DeviceResources->GetDeviceContext(); }
	IDXGISwapChain* GetSwapChain() const { return DeviceResources->GetSwapChain();}
	ID3D11RenderTargetView* GetRenderTargetView() const { return DeviceResources->GetRenderTargetView(); }
	UDeviceResources* GetDeviceResources() const { return DeviceResources; }

	/** LineBatchRenderer에서 사용할 공개 메서드 */
	UPipeline* GetPipeline() const { return Pipeline; }
	ID3D11InputLayout* GetDefaultInputLayout() const { return DefaultInputLayout; }
	ID3D11VertexShader* GetDefaultVertexShader() const { return DefaultVertexShader; }
	ID3D11PixelShader* GetDefaultPixelShader() const { return DefaultPixelShader; }
	ID3D11DepthStencilState* GetDefaultDepthStencilState() const { return DefaultDepthStencilState; }
	ID3D11RasterizerState* GetRasterizerState(const FRenderState& InRenderState);

	/** Instanced line shader accessors */
	ID3D11InputLayout* GetLineInstancedInputLayout() const { return LineInstancedInputLayout; }
	ID3D11VertexShader* GetLineInstancedVertexShader() const { return LineInstancedVertexShader; }
	ID3D11PixelShader* GetLineInstancedPixelShader() const { return LineInstancedPixelShader; }

private:
	UPipeline* Pipeline = nullptr;
	UDeviceResources* DeviceResources = nullptr;
	EViewModeIndex CurrentViewMode = EViewModeIndex::Lit;
	EEngineShowFlags CurrentShowFlags = EEngineShowFlags::SF_Default;
	TArray<UPrimitiveComponent*> PrimitiveComponents;

private:
	ID3D11DepthStencilState* DefaultDepthStencilState = nullptr;
	ID3D11DepthStencilState* DisabledDepthStencilState = nullptr;
	ID3D11DepthStencilState* TextDepthStencilState = nullptr;
	ID3D11BlendState* TextBlendState = nullptr;
	ID3D11Buffer* ConstantBufferModels = nullptr;
	ID3D11Buffer* ConstantBufferPerFrame = nullptr;
	ID3D11Buffer* ConstantBufferColor = nullptr;
	ID3D11Buffer* ConstantBufferCharTable = nullptr;
	ID3D11Buffer* ConstantBufferInstance = nullptr;
	//////////////////////////////////////

	ID3D11Buffer* TextInstanceBuffer = nullptr;
	/////////////////////////////////////
	FLOAT ClearColor[4] = {0.025f, 0.025f, 0.025f, 1.0f};

	ID3D11VertexShader* DefaultVertexShader = nullptr;
	ID3D11PixelShader* DefaultPixelShader = nullptr;
	ID3D11InputLayout* DefaultInputLayout = nullptr;

	ID3D11VertexShader* TextVertexShader = nullptr;
	ID3D11PixelShader* TextPixelShader = nullptr;
	ID3D11InputLayout* TextInputLayout = nullptr;

	ID3D11VertexShader* LineInstancedVertexShader = nullptr;
	ID3D11PixelShader* LineInstancedPixelShader = nullptr;
	ID3D11InputLayout* LineInstancedInputLayout = nullptr;

	uint32 Stride = 0;
	uint32 StrideTextVertex = 0;
	uint32 StrideTextInstance = 0;

	struct FPrimitiveBatchKey
	{
		ID3D11Buffer* VertexBuffer = nullptr;
		ID3D11Buffer* IndexBuffer = nullptr;
		uint32 IndexCount = 0;
		FRenderState RenderState = {};
		bool operator==(const FPrimitiveBatchKey& InRhs) const
		{
			return VertexBuffer == InRhs.VertexBuffer &&
				IndexBuffer == InRhs.IndexBuffer &&
				IndexCount == InRhs.IndexCount &&
				RenderState.CullMode == InRhs.RenderState.CullMode &&
				RenderState.FillMode == InRhs.RenderState.FillMode;
		}
	};

	struct FPrimitiveBatchKeyHasher
	{
		size_t operator()(const FPrimitiveBatchKey& InKey) const noexcept
		{
			auto Mix = [](size_t& H, size_t V)
			{
				H ^= V + 0x9e3779b97f4a7c15ULL + (H << 6) + (H >> 2);
			};

			size_t Hash = 0;
			Mix(Hash, reinterpret_cast<size_t>(InKey.VertexBuffer));
			Mix(Hash, reinterpret_cast<size_t>(InKey.IndexBuffer));
			Mix(Hash, static_cast<size_t>(InKey.IndexCount));
			Mix(Hash, static_cast<size_t>(InKey.RenderState.CullMode));
			Mix(Hash, static_cast<size_t>(InKey.RenderState.FillMode));

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

private:
	struct FRasterKey
	{
		D3D11_FILL_MODE FillMode = {};
		D3D11_CULL_MODE CullMode = {};

		bool operator==(const FRasterKey& InKey) const
		{
			return FillMode == InKey.FillMode && CullMode == InKey.CullMode;
		}
	};

	struct FRasterKeyHasher
	{
		size_t operator()(const FRasterKey& InKey) const noexcept
		{
			auto Mix = [](size_t& H, size_t V)
				{
					H ^= V + 0x9e3779b97f4a7c15ULL + (H << 6) + (H << 2);
				};

			size_t H = 0;
			Mix(H, (size_t)InKey.FillMode);
			Mix(H, (size_t)InKey.CullMode);

			return H;
		}
	};

	TMap<FRasterKey, ID3D11RasterizerState*, FRasterKeyHasher> RasterCache;

	FPipelineInfo CreatePipelineInfo(const FRenderState& InRenderState);
	FPipelineInfo CreateTextPipelineInfo(const FRenderState& InRenderState);
	FInstanceBufferResource& GetOrCreateInstanceBuffer(const FPrimitiveBatchKey& InKey);
	void EnsureInstanceBufferCapacity(FInstanceBufferResource& InResource, uint32 InRequiredInstanceCount);
	void UploadInstanceBufferData(FInstanceBufferResource& InResource, const void* InData, uint32 InInstanceCount);
	void ReleasePrimitiveInstanceBuffers();

	bool bIsResizing = false;

	///////////////////////////////////////////
	// 카메라 VP Matrix 값 전달 받는 용도
	// (차후 리팩터링이 필요합니다)
	FViewProjConstants ViewProjConstants;
	///////////////////////////////////////////
};
