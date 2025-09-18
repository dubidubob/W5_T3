#include "pch.h"
#include "Core/ClientApp.h"

#include "Editor/Editor.h"
#include "Core/AppWindow.h"
#include "Manager/Input/InputManager.h"
#include "Manager/Level/LevelManager.h"
#include "Manager/Time/TimeManager.h"

#include "Manager/UI/UIManager.h"
#include "Render/Renderer/Renderer.h"

#include "Render/UI/Window/ConsoleWindow.h"

#include <chrono>

#pragma comment(lib, "winmm.lib")


FClientApp::FClientApp() = default;

FClientApp::~FClientApp() = default;
/**
 * @brief Client Main Runtime Function
 * App 초기화, Main Loop 실행을 통한 전체 Cycle
 *
 * @param InInstanceHandle Process Instance Handle
 * @param InCmdShow Window Display Method
 *
 *
 * @return Program Termination Code
 */
int FClientApp::Run(HINSTANCE InInstanceHandle, int InCmdShow)
{
	// Memory Leak Detection & Report
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetBreakAlloc(0);
	#endif

	// Window Object Initialize
	Window = new FAppWindow(this);
	if (!Window->Init(InInstanceHandle, InCmdShow))
	{
		assert(!"Window Creation Failed");
		return 0;
	}

	// Create Console
	// #ifdef _DEBUG
	// 	Window->InitializeConsole();
	// #endif

	// Keyboard Accelerator Table Setting
	// AcceleratorTable = LoadAccelerators(InInstanceHandle, MAKEINTRESOURCE(IDC_CLIENT));

	// Initialize Base System
	int InitResult = InitializeSystem();
	if (InitResult != S_OK)
	{
		assert(!"Initialize Failed");
		return 0;
	}

	// Execute Main Loop
	MainLoop();

	// Termination Process
	ShutdownSystem();

	return static_cast<int>(MainMessage.wParam);
}

/**
 * @brief Initialize System For Game Execution
 */
int FClientApp::InitializeSystem()
{
	// Initialize By Get Instance
	UTimeManager::GetInstance();
	UInputManager::GetInstance();

	auto& Renderer = URenderer::GetInstance();
	Renderer.Init(Window->GetWindowHandle());

	// UIManager Initialize
	auto& UiManager = UUIManager::GetInstance();
	UiManager.Initialize(Window->GetWindowHandle());
	UUIWindowFactory::CreateDefaultUILayout();

	UResourceManager::GetInstance().Initialize();

	// UE_LOG 테스트
	// UE_LOG("=== Engine Initialization Started ===");
	// UE_LOG("Window Handle: %p", Window->GetWindowHandle());
	// UE_LOG("Renderer initialized successfully");

	// Console Window 테스트
	// cout << "[System] This is cout output test\n";
	// cerr << "[System] This is cerr output test\n";

	// UE_LOG("=== Engine Initialization Completed ===");

	// Create Default Level
	// TODO(KHJ): 나중에 Init에서 처리하도록 하는 게 맞을 듯
	ULevelManager::GetInstance().CreateDefaultLevel();

	// Initialize Editor
	Editor = NewObject<UEditor>();

	return S_OK;
}

/**
 * @brief Update System While Game Processing
 */
void FClientApp::UpdateSystem()
{
	auto& TimeManager = UTimeManager::GetInstance();
	auto& InputManager = UInputManager::GetInstance();
	auto& Renderer = URenderer::GetInstance();
	auto& LevelManager = ULevelManager::GetInstance();
	auto& UiManager = UUIManager::GetInstance();

	Editor->Update();
	TimeManager.Update();
	InputManager.Update(Window);
	LevelManager.Update();
	UiManager.Update();
	Renderer.Update(Editor);
}

/**
 * @brief Execute Main Message Loop
 * 윈도우 메시지 처리 및 게임 시스템 업데이트를 담당
 * 60fps로 프레임 제한
 */
void FClientApp::MainLoop()
{
	// 고정밀 타이머 설정 (1ms 해상도)
	timeBeginPeriod(1);
	
	const double TargetFPS = 60.0;
	const double TargetFrameTime = 1000.0 / TargetFPS; // 16.666... ms
	
	LARGE_INTEGER Frequency, LastTime, CurrentTime;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&LastTime);

	while (true)
	{
		// Process all pending messages
		while (PeekMessage(&MainMessage, nullptr, 0, 0, PM_REMOVE))
		{
			// Process Termination
			if (MainMessage.message == WM_QUIT)
			{
				timeEndPeriod(1); // 타이머 해상도 복구
				return;
			}
			
			// Shortcut Key Processing
			if (!TranslateAccelerator(MainMessage.hwnd, AcceleratorTable, &MainMessage))
			{
				TranslateMessage(&MainMessage);
				DispatchMessage(&MainMessage);
			}
		}
		
		QueryPerformanceCounter(&CurrentTime);
		double ElapsedTime = ((CurrentTime.QuadPart - LastTime.QuadPart) * 1000.0) / Frequency.QuadPart;
		
		// 60fps 제한: 16.67ms마다 업데이트
		if (ElapsedTime >= TargetFrameTime)
		{
			UpdateSystem();
			LastTime = CurrentTime;
		}
		else
		{
			// 남은 시간만큼 정확하게 대기
			double RemainingTime = TargetFrameTime - ElapsedTime;
			if (RemainingTime > 2.0) // 2ms 이상 남았을 때만 Sleep
			{
				Sleep(static_cast<DWORD>(RemainingTime - 1.0));
			}
			// 나머지는 busy wait로 정밀하게 처리
		}
	}
}

/**
 * @brief 시스템 종료 처리
 * 모든 리소스를 안전하게 해제하고 매니저들을 정리합니다.
 */
void FClientApp::ShutdownSystem()
{
	URenderer::GetInstance().Release();
	UUIManager::GetInstance().Shutdown();
	ULevelManager::GetInstance().Shutdown();
	UResourceManager::GetInstance().Release();

	// 레벨 매니저 정리
	// ULevelManager::GetInstance().Release();
	delete Editor;
	delete Window;
}
