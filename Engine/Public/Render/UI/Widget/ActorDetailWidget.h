#pragma once
#include "Widget.h"

class AActor;

class UActorDetailWidget : public UWidget
{
public:
	UActorDetailWidget();
	~UActorDetailWidget() override = default;

	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

private:
	void RenderActorInfo();
	void RenderNameField();

	AActor* SelectedActor = nullptr;
	char ActorNameBuffer[256] = "";
	bool bNameChanged = false;
};
