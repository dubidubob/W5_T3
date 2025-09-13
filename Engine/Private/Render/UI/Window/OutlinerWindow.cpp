#include "pch.h"
#include "Render/UI/Window/OutlinerWindow.h"

#include "Render/UI/Widget/ActorListWidget.h"

UOutlinerWindow::UOutlinerWindow()
{
	FUIWindowConfig Config;
	Config.WindowTitle = "Outliner";
	Config.DefaultSize = ImVec2(350, 600);
	Config.DefaultPosition = ImVec2(1225, 10);
	Config.MinSize = ImVec2(300, 400);
	Config.bResizable = true;
	Config.bMovable = true;
	Config.bCollapsible = true;
	Config.DockDirection = EUIDockDirection::Center;

	Config.UpdateWindowFlags();
	SetConfig(Config);

	AddWidget(new UActorListWidget);
}

void UOutlinerWindow::Initialize()
{
	UE_LOG("OutlinerWindow: Successfully Initialized");
}
