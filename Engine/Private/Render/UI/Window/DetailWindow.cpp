#include "pch.h"
#include "Render/UI/Window/DetailWindow.h"

#include "Render/UI/Widget/ActorDetailWidget.h"
#include "Render/UI/Widget/TargetActorTransformWidget.h"
#include "Render/UI/Widget/ActorTerminationWidget.h"

UDetailWindow::UDetailWindow()
{
	FUIWindowConfig Config;
	Config.WindowTitle = "Details";
	Config.DefaultSize = ImVec2(400, 500);
	Config.DefaultPosition = ImVec2(1225, 650);
	Config.MinSize = ImVec2(350, 300);
	Config.bResizable = true;
	Config.bMovable = true;
	Config.bCollapsible = true;
	Config.DockDirection = EUIDockDirection::Center;

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
