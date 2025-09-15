#include "pch.h"
#include "Render/UI/Window/DetailWindow.h"

#include "Render/UI/Widget/ActorDetailWidget.h"
#include "Render/UI/Widget/TargetActorTransformWidget.h"
#include "Render/UI/Widget/ActorTerminationWidget.h"

IMPLEMENT_CLASS(UDetailWindow, UUIWindow)

UDetailWindow::UDetailWindow()
{
	FUIWindowConfig Config;
	Config.WindowTitle = "Details";
	Config.DefaultSize = ImVec2(350, 400);
	Config.DefaultPosition = ImVec2(10, 10); // 임시, 동적으로 계산됨
	Config.MinSize = ImVec2(300, 300);
	Config.bResizable = true;
	Config.bMovable = true;
	Config.bCollapsible = true;
	Config.DockDirection = EUIDockDirection::None;

	Config.UpdateWindowFlags();
	SetConfig(Config);

	AddWidget(new UActorDetailWidget);
	AddWidget(new UTargetActorTransformWidget);
	AddWidget(new UActorTerminationWidget);
}

void UDetailWindow::Initialize()
{
	UE_LOG("DetailWindow: Successfully Initialized");
}
