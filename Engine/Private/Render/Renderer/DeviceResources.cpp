#include "pch.h"
#include "Render/Renderer/DeviceResources.h"

UDeviceResources::UDeviceResources(HWND InWindowHandle)
{
	Create(InWindowHandle);
}

UDeviceResources::~UDeviceResources()
{
	Release();
}

void UDeviceResources::Create(HWND InWindowHandle)
{
	RECT ClientRect;
	GetClientRect(InWindowHandle, &ClientRect);
	Width = ClientRect.right - ClientRect.left;
	Height = ClientRect.bottom - ClientRect.top;

	CreateDeviceAndSwapChain(InWindowHandle);
	CreateFrameBuffer();
	CreateDepthBuffer();
	CreateObjectViewerResources();
}

void UDeviceResources::Release()
{
	ReleaseObjectViewerResources();
	ReleaseFrameBuffer();
	ReleaseDepthBuffer();
	ReleaseDeviceAndSwapChain();
}

/**
 * @brief Direct3D 장치 및 스왑 체인을 생성하는 함수
 * @param InWindowHandle
 */
void UDeviceResources::CreateDeviceAndSwapChain(HWND InWindowHandle)
{
	// 지원하는 Direct3D 기능 레벨을 정의
	D3D_FEATURE_LEVEL featurelevels[] = {D3D_FEATURE_LEVEL_11_0};

	// 스왑 체인 설정 구조체 초기화
	DXGI_SWAP_CHAIN_DESC SwapChainDescription = {};
	SwapChainDescription.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
	SwapChainDescription.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
	SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
	SwapChainDescription.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
	SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
	SwapChainDescription.BufferCount = 2; // 더블 버퍼링
	SwapChainDescription.OutputWindow = InWindowHandle; // 렌더링할 창 핸들
	SwapChainDescription.Windowed = TRUE; // 창 모드
	SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

	// Direct3D 장치와 스왑 체인을 생성
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
	                                           D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
	                                           featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
	                                           &SwapChainDescription, &SwapChain, &Device, nullptr, &DeviceContext);

	if (FAILED(hr))
	{
		assert(!"Failed To Create SwapChain");
	}

	// 생성된 스왑 체인의 정보 가져오기
	SwapChain->GetDesc(&SwapChainDescription);

	// Viewport Info 업데이트
	ViewportInfo = {
		0.0f, 0.0f, static_cast<float>(SwapChainDescription.BufferDesc.Width),
		static_cast<float>(SwapChainDescription.BufferDesc.Height), 0.0f, 1.0f
	};
}

/**
 * @brief Direct3D 장치 및 스왑 체인을 해제하는 함수
 */
void UDeviceResources::ReleaseDeviceAndSwapChain()
{
	if (DeviceContext)
	{
		// 남아있는 GPU 명령 실행
		DeviceContext->Flush();
	}

	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}

	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}
}

/**
 * @brief FrameBuffer 생성 함수
 */
void UDeviceResources::CreateFrameBuffer()
{
	// 스왑 체인으로부터 백 버퍼 텍스처 가져오기
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	// 렌더 타겟 뷰 생성
	D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
	framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
	framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

	Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
	
}

/**
 * @brief 프레임 버퍼를 해제하는 함수
 */
void UDeviceResources::ReleaseFrameBuffer()
{
	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}

	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}
}

void UDeviceResources::CreateDepthBuffer()
{
	D3D11_TEXTURE2D_DESC dsDesc = {};

	dsDesc.Width = Width;
	dsDesc.Height = Height;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;

	Device->CreateTexture2D(&dsDesc, nullptr, &DepthBuffer);
	Device->CreateDepthStencilView(DepthBuffer, nullptr, &DepthStencilView);

	
}

void UDeviceResources::ReleaseDepthBuffer()
{
	if (DepthStencilView)
	{
		DepthStencilView->Release();
		DepthStencilView = nullptr;
	}
	if (DepthBuffer)
	{
		DepthBuffer->Release();
		DepthBuffer = nullptr;
	}
}

void UDeviceResources::UpdateViewport()
{
	DXGI_SWAP_CHAIN_DESC SwapChainDescription = {};
	SwapChain->GetDesc(&SwapChainDescription);

	ViewportInfo = {
		0.0f, 0.0f, static_cast<float>(SwapChainDescription.BufferDesc.Width),
		static_cast<float>(SwapChainDescription.BufferDesc.Height), 0.0f, 1.0f
	};
	Width = SwapChainDescription.BufferDesc.Width;
	Height = SwapChainDescription.BufferDesc.Height;
}

void UDeviceResources::CreateObjectViewerResources()
{
	// Define the size of the Object Viewer texture
	const int textureWidth = 1024;
	const int textureHeight = 1024;

	// Create the render target texture
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = Device->CreateTexture2D(&textureDesc, nullptr, &ObjectViewerTexture);
	if(FAILED(hr)) assert(!"Failed to create Object Viewer Texture");

	// Create the render target view (RTV)
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = Device->CreateRenderTargetView(ObjectViewerTexture, &rtvDesc, &ObjectViewerRTV);
	if(FAILED(hr)) assert(!"Failed to create Object Viewer RTV");

	// Create the shader resource view (SRV)
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	hr = Device->CreateShaderResourceView(ObjectViewerTexture, &srvDesc, &ObjectViewerSRV);
	if(FAILED(hr)) assert(!"Failed to create Object Viewer SRV");

	// Create the depth stencil texture
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = textureWidth;
	depthDesc.Height = textureHeight;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	hr = Device->CreateTexture2D(&depthDesc, nullptr, &ObjectViewerDepthTexture);
	if(FAILED(hr)) assert(!"Failed to create Object Viewer Depth Texture");

	// Create the depth stencil view (DSV)
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = Device->CreateDepthStencilView(ObjectViewerDepthTexture, &dsvDesc, &ObjectViewerDSV);
	if(FAILED(hr)) assert(!"Failed to create Object Viewer DSV");
}

void UDeviceResources::ReleaseObjectViewerResources()
{
	if (ObjectViewerSRV)
	{
		ObjectViewerSRV->Release();
		ObjectViewerSRV = nullptr;
	}
	if (ObjectViewerDSV)
	{
		ObjectViewerDSV->Release();
		ObjectViewerDSV = nullptr;
	}
	if (ObjectViewerDepthTexture)
	{
		ObjectViewerDepthTexture->Release();
		ObjectViewerDepthTexture = nullptr;
	}
	if (ObjectViewerRTV)
	{
		ObjectViewerRTV->Release();
		ObjectViewerRTV = nullptr;

	}
	if (ObjectViewerTexture)
	{
		ObjectViewerTexture->Release();
		ObjectViewerTexture = nullptr;
	}
}
