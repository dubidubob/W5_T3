#include "pch.h"
#include "Render/UI/Window/OutlinerWindow.h"

#include "Render/UI/Widget/ActorListWidget.h"

IMPLEMENT_CLASS(UOutlinerWindow, UUIWindow)

UOutlinerWindow::UOutlinerWindow()
{
	FUIWindowConfig Config;
	Config.WindowTitle = "Outliner";
	Config.DefaultSize = ImVec2(350, 400);
	Config.DefaultPosition = ImVec2(10, 10); // 임시, 동적으로 계산됨
	Config.MinSize = ImVec2(300, 300);
	Config.bResizable = true;
	Config.bMovable = true;
	Config.bCollapsible = true;
	Config.DockDirection = EUIDockDirection::None;

	Config.UpdateWindowFlags();
	SetConfig(Config);

	AddWidget(new UActorListWidget);
}

void UOutlinerWindow::Initialize()
{
	UE_LOG("OutlinerWindow: Successfully Initialized");
}
