#pragma once

class UDeviceResources
{
public:
	UDeviceResources(HWND InWindowHandle);
	~UDeviceResources();

	void Create(HWND InWindowHandle);
	void Release();

	void CreateDeviceAndSwapChain(HWND InWindowHandle);
	void ReleaseDeviceAndSwapChain();
	void CreateFrameBuffer();
	void ReleaseFrameBuffer();
	void CreateDepthBuffer();
	void ReleaseDepthBuffer();
	void CreateObjectViewerResources();
	void ReleaseObjectViewerResources();
	void CreateColorPickingResources();
	void ReleaseColorPickingResources();

	ID3D11Device* GetDevice() const { return Device; }
	ID3D11DeviceContext* GetDeviceContext() const { return DeviceContext; }
	ID3D11Texture2D* GetFrameBufferTex() const { return FrameBuffer; }
	IDXGISwapChain* GetSwapChain() const { return SwapChain; }
	ID3D11RenderTargetView* GetRenderTargetView() const { return FrameBufferRTV; }
	ID3D11DepthStencilView* GetDepthStencilView() const { return DepthStencilView; }
	const D3D11_VIEWPORT& GetViewportInfo() const { return ViewportInfo; }

	// Object Viewer Getters
	ID3D11RenderTargetView* GetObjectViewerRTV() const { return ObjectViewerRTV; }
	ID3D11DepthStencilView* GetObjectViewerDSV() const { return ObjectViewerDSV; }
	ID3D11ShaderResourceView* GetObjectViewerSRV() const { return ObjectViewerSRV; }

	// Color Picking Getters
	ID3D11RenderTargetView* GetColorPickingRTV() const { return ColorPickingRTV; }
	ID3D11DepthStencilView* GetColorPickingDSV() const { return ColorPickingDSV; }
	ID3D11Texture2D* GetColorPickingTexture() const { return ColorPickingTexture; }

	// Color Picking Utility
	uint32 ReadPixelFromColorPickingTexture(int32 X, int32 Y);

	// Color Picking Resolution Scaling
	void SetColorPickingScale(float InScale);
	float GetColorPickingScale() const { return ColorPickingScale; }
	uint32 GetColorPickingWidth() const { return static_cast<uint32>(Width * ColorPickingScale); }
	uint32 GetColorPickingHeight() const { return static_cast<uint32>(Height * ColorPickingScale); }
	D3D11_VIEWPORT GetColorPickingViewport() const;

	void UpdateViewport();

private:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	IDXGISwapChain* SwapChain = nullptr;

	ID3D11Texture2D* FrameBuffer = nullptr;
	ID3D11RenderTargetView* FrameBufferRTV = nullptr;

	ID3D11Texture2D* DepthBuffer = nullptr;
	ID3D11DepthStencilView* DepthStencilView = nullptr;

	D3D11_VIEWPORT ViewportInfo = {};

	uint32 Width = 0;
	uint32 Height = 0;

	// Resources for Object Viewer Render Target
	ID3D11Texture2D*        ObjectViewerTexture = nullptr;
	ID3D11RenderTargetView* ObjectViewerRTV = nullptr;
	ID3D11ShaderResourceView* ObjectViewerSRV = nullptr;
	ID3D11Texture2D*        ObjectViewerDepthTexture = nullptr;
	ID3D11DepthStencilView* ObjectViewerDSV = nullptr;

	// Resources for Color Picking Render Target
	ID3D11Texture2D*        ColorPickingTexture = nullptr;
	ID3D11RenderTargetView* ColorPickingRTV = nullptr;
	ID3D11Texture2D*        ColorPickingDepthTexture = nullptr;
	ID3D11DepthStencilView* ColorPickingDSV = nullptr;

	// Reusable Staging Texture for Pixel Readback (Optimization)
	//ID3D11Texture2D*        ColorPickingStagingTexture = nullptr;

	// Color Picking Resolution Scale (0.25 = quarter resolution, 1.0 = full resolution)
	float ColorPickingScale = 0.25f;
};
