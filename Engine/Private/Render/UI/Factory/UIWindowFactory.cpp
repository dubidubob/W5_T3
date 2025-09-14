#include "pch.h"
#include "Render/UI/Factory/UIWindowFactory.h"

#include "Manager/UI/UIManager.h"
#include "Render/UI/Window/ConsoleWindow.h"
#include "Render/UI/Window/ControlPanelWindow.h"
#include "Render/UI/Window/DetailWindow.h"
#include "Render/UI/Window/ExperimentalFeatureWindow.h"
#include "Render/UI/Window/OutlinerWindow.h"

UConsoleWindow* UUIWindowFactory::CreateConsoleWindow(EUIDockDirection InDockDirection)
{
	auto& Window = UConsoleWindow::GetInstance();
	Window.GetMutableConfig().DockDirection = InDockDirection;
	return &Window;
}

UControlPanelWindow* UUIWindowFactory::CreateControlPanelWindow(EUIDockDirection InDockDirection)
{
	auto* Window = NewObject<UControlPanelWindow>();
	Window->GetMutableConfig().DockDirection = InDockDirection;
	return Window;
}

UOutlinerWindow* UUIWindowFactory::CreateOutlinerWindow(EUIDockDirection InDockDirection)
{
	auto* Window = NewObject<UOutlinerWindow>();;
	Window->GetMutableConfig().DockDirection = InDockDirection;
	return Window;
}

UDetailWindow* UUIWindowFactory::CreateDetailWindow(EUIDockDirection InDockDirection)
{
	auto* Window = NewObject<UDetailWindow>();;
	Window->GetMutableConfig().DockDirection = InDockDirection;
	return Window;
}

UExperimentalFeatureWindow* UUIWindowFactory::CreateExperimentalFeatureWindow(EUIDockDirection InDockDirection)
{
	auto* Window = NewObject<UExperimentalFeatureWindow>();;
	Window->GetMutableConfig().DockDirection = InDockDirection;
	return Window;
}

void UUIWindowFactory::CreateDefaultUILayout()
{
	auto& UIManager = UUIManager::GetInstance();

	// 기본 레이아웃 생성
	UIManager.RegisterUIWindow(CreateConsoleWindow(EUIDockDirection::Bottom));
	UIManager.RegisterUIWindow(CreateControlPanelWindow(EUIDockDirection::Left));
	UIManager.RegisterUIWindow(CreateOutlinerWindow(EUIDockDirection::Center));
	UIManager.RegisterUIWindow(CreateDetailWindow(EUIDockDirection::Right));
	UIManager.RegisterUIWindow(CreateExperimentalFeatureWindow(EUIDockDirection::Right));
	UE_LOG("UIWindowFactory: 기본적인 UI 생성이 성공적으로 완료되었습니다");
}
