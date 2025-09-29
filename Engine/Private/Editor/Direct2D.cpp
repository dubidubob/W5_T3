#include "pch.h"
#include "Editor/Direct2D.h"

IMPLEMENT_CLASS(UDirect2D, UObject)

void UDirect2D::Init(ID3D11Texture2D* RenderTargetTexture)
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &Factory);

	D2D1_PIXEL_FORMAT PixelFormat = D2D1::PixelFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
	IDXGISurface* DxgiSurface = nullptr;
	RenderTargetTexture->QueryInterface(__uuidof(IDXGISurface), (void**)&DxgiSurface);

	D2D1_RENDER_TARGET_PROPERTIES Props = D2D1::RenderTargetProperties();
	Props.pixelFormat = PixelFormat;
	HRESULT hr = Factory->CreateDxgiSurfaceRenderTarget(DxgiSurface, &Props, &D2DRenderTarget);
	if (FAILED(hr))
	{
		int a = 0;
	}

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&TextFactory));

	InitOverlay();
}
void UDirect2D::InitOverlay()
{
	FPSBackGroundRect.left = 100;
	FPSBackGroundRect.top = 100;
	FPSBackGroundRect.bottom = 200;
	FPSBackGroundRect.right = 500;
	FPSTextRect.left = 110;
	FPSTextRect.top = 110;
	FPSTextRect.bottom = 190;
	FPSTextRect.right = 490;
	D2DRenderTarget->CreateSolidColorBrush(ColorF(0.8f, 0.8f, 0.8f, 0.8f), &GrayBrush);

	HRESULT hr = TextFactory->CreateTextFormat(L"Verdana", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"en-us", &TextFormat);
	if (FAILED(hr))
	{
		int a = 0;
	}
	TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

}
void UDirect2D::DrawOverlay()
{
	D2DRenderTarget->BeginDraw();
	D2DRenderTarget->DrawRectangle(FPSBackGroundRect, GrayBrush);

	wchar_t buffer[64];
	swprintf(buffer, 64, L"FPS %d", 60);
	FWstring WString = FWstring(buffer);
	D2DRenderTarget->DrawTextW(WString.c_str(), WString.size(), TextFormat, FPSTextRect, GrayBrush);
	D2DRenderTarget->EndDraw();
}
UDirect2D::~UDirect2D()
{
	TextFormat->Release();
	GrayBrush->Release();
	D2DRenderTarget->Release();
	Factory->Release();
	CoUninitialize();
}
