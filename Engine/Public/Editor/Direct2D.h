#pragma once
#include "Global/Types.h"
#include "Global/CoreTypes.h"
#include "Core/Object.h"
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "D2D1.lib")

using namespace D2D1;
class UDirect2D
{
public:
	UDirect2D() = default;
	void Init(ID3D11Texture2D* RenderTargetTexture);
	void Release();
	~UDirect2D();
	void InitOverlay();
	void DrawOverlay();
private:
public:
private:
	D2D1_RECT_F FPSBackGroundRect;
	IDXGISurface* DxgiSurface = nullptr;
	ID2D1RenderTarget* D2DRenderTarget = nullptr;
	ID2D1Factory* Factory = nullptr;
	ID2D1SolidColorBrush* GrayBrush = nullptr;
	ID2D1SolidColorBrush* WhiteBrush = nullptr;

	D2D1_RECT_F FPSTextRect;
	IDWriteFactory* TextFactory = nullptr;
	IDWriteTextFormat* TextFormat = nullptr;

};
