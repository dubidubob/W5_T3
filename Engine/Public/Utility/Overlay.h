#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d2d1_1.h>
#include <dwrite.h>

class OverlayStat 
{
public:
	void Initialize(IDXGISwapChain* SwapChain);
	void Release();

	// 렌더링: 통계 계산 및 Direct2D 그리기 실행
	void Render(ID3D11DeviceContext* DeviceContext);

	// 리사이즈 처리: 스왑 체인 리사이즈 시 Direct2D 렌더 타겟 재구성
	void OnResize(IDXGISwapChain* SwapChain);

private:
	// Direct2D/DirectWrite 리소스
	ID2D1Factory1* D2dFactory = nullptr;
	IDWriteFactory* DwriteFactory = nullptr;
	ID2D1RenderTarget* D2dRenderTarget = nullptr;
	ID2D1SolidColorBrush* TextBrush = nullptr;
	IDWriteTextFormat* TextFormat = nullptr;

	// 통계 데이터
	float CurrentFPS = 0.0f; // float FrameTimeMs = 0.0f;

	// 프레임 시간 계산용
	LARGE_INTEGER QpcFrequency = {};
	LARGE_INTEGER LastTime = {};

	// 헬퍼 함수
	void CreateDeviceResources(IDXGISwapChain* SwapChain);
	void CalculateStats();

	template<typename T>
	void SafeRelease(T*& Ptr)
	{
		if (Ptr)
		{
			Ptr->Release();
			Ptr = nullptr;
		}
	}
};

