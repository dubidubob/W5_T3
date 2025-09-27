#include "pch.h"
#include "Overlay.h"
#include <sstream>
#include <iomanip>

void OverlayStat::Initialize(IDXGISwapChain* SwapChain)
{
	// 1. Direct2D/DirectWrite 팩토리 생성
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2dFactory);
	if (FAILED(hr)) { /* 에러 처리 */ return; }

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&DwriteFactory));
	if (FAILED(hr)) { /* 에러 처리 */ return; }

	// 2. 텍스트 형식 정의
	hr = DwriteFactory->CreateTextFormat(
		L"Segoe UI", // 폰트 이름
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		24.0f, // 폰트 크기
		L"ko-kr",
		&TextFormat
	);
	if (FAILED(hr)) { /* 에러 처리 */ return; }

	// 텍스트 정렬 설정
	TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	// 3. 렌더 타겟 및 브러시 생성
	CreateDeviceResources(SwapChain);

	// 4. 통계 계산 초기화
	QueryPerformanceFrequency(&QpcFrequency);
	QueryPerformanceCounter(&LastTime);
}

void OverlayStat::Release()
{
	// Direct2D/DirectWrite 리소스 해제
	SafeRelease(TextBrush);
	SafeRelease(D2dRenderTarget);
	SafeRelease(TextFormat);
	SafeRelease(DwriteFactory);
	SafeRelease(D2dFactory);
}

void OverlayStat::CreateDeviceResources(IDXGISwapChain* SwapChain)
{
	// 기존 리소스 해제
	SafeRelease(D2dRenderTarget);
	SafeRelease(TextBrush);

	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(hr)) return;

	// 1. DXGI Surface를 ID2D1Bitmap1로 래핑하여 렌더 타겟 생성
	IDXGISurface* dxgiSurface = nullptr;
	hr = backBuffer->QueryInterface(__uuidof(IDXGISurface), (void**)&dxgiSurface);
	backBuffer->Release(); // 백 버퍼는 더 이상 필요 없으므로 해제
	if (FAILED(hr)) return;

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED) // D3D11과 호환
	);

	hr = D2dFactory->CreateDxgiSurfaceRenderTarget(
		dxgiSurface,
		&props,
		&D2dRenderTarget
	);
	dxgiSurface->Release(); // DXGI Surface는 렌더 타겟 생성 후 해제
	if (FAILED(hr)) return;

	// 2. 텍스트 브러시 생성 (밝은 녹색)
	D2D1_COLOR_F green = { 0.298f, 0.858f, 0.443f, 1.0f }; // 이미지와 유사한 녹색
	hr = D2dRenderTarget->CreateSolidColorBrush(green, &TextBrush);
}

void OverlayStat::OnResize(IDXGISwapChain* SwapChain)
{
	if (D2dRenderTarget)
	{
		// 렌더 타겟을 해제하고 다시 생성해야 합니다.
		// D2D는 새 크기의 백 버퍼와 연결되어야 합니다.
		CreateDeviceResources(SwapChain);
	}
}

void OverlayStat::CalculateStats()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	LONGLONG elapsedTicks = currentTime.QuadPart - LastTime.QuadPart;
	LastTime = currentTime;

	// 초 단위 시간
	double elapsedTimeSeconds = static_cast<double>(elapsedTicks) / QpcFrequency.QuadPart;

	// FPS
	if (elapsedTimeSeconds > 0)
	{
		CurrentFPS = static_cast<float>(1.0 / elapsedTimeSeconds);
	}
}

void OverlayStat::Render(ID3D11DeviceContext* DeviceContext)
{
	if (!D2dRenderTarget) return;

	// 통계 계산 업데이트
	CalculateStats();

	// 1. 문자열 생성
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2);

	// FPS 라인 (47.87 FPS)
	ss << CurrentFPS << L" FPS\n";

	std::wstring statText = ss.str();

	// 2. Direct2D 렌더링 시작
	D2dRenderTarget->BeginDraw();

	// Antialiasing 설정
	D2dRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	// 3. 그림자/외곽선 효과를 위해 텍스트를 오프셋하여 한 번 더 그립니다.
	D2D1_RECT_F layoutRect = D2D1::RectF(10.0f, 10.0f, 300.0f, 100.0f); // 화면 좌상단 영역 (조절 필요)

	// 그림자 그리기 (어둡게, 약간 오프셋)
	TextBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black, 0.8f));
	D2D1_POINT_2F shadowOffset = D2D1::Point2F(1.5f, 1.5f);
	D2dRenderTarget->DrawText(
		statText.c_str(),
		static_cast<UINT32>(statText.length()),
		TextFormat,
		D2D1::RectF(layoutRect.left + shadowOffset.x, layoutRect.top + shadowOffset.y, layoutRect.right + shadowOffset.x, layoutRect.bottom + shadowOffset.y),
		TextBrush
	);

	// 4. 실제 텍스트 그리기 (녹색)
	TextBrush->SetColor(D2D1::ColorF(0.298f, 0.858f, 0.443f, 1.0f)); // 밝은 녹색
	D2dRenderTarget->DrawText(
		statText.c_str(),
		static_cast<UINT32>(statText.length()),
		TextFormat,
		layoutRect,
		TextBrush
	);

	// 5. 렌더링 종료
	HRESULT hr = D2dRenderTarget->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
	{
		// 렌더 타겟 재구성이 필요하면 D3D DeviceContext는 플러시하지 않고 반환
		// 외부에서 OnResize를 호출해야 함
	}
}
