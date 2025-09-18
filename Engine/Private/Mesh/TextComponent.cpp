#include "pch.h"
#include "Mesh/TextComponent.h"
#include "Mesh/SceneComponent.h"
#include "Render/Renderer/Renderer.h"
#include <string>

UTextComponent::UTextComponent()
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	RenderState.CullMode = ECullMode::None;
	RenderState.FillMode = EFillMode::Solid;
	ComponentType = EComponentType::Text;
	VertexBuffer = ResourceManager.GetTextVertexBuffer();
	VertexNum = ResourceManager.GetTextNumVertices();

	SetText(L"[크래프톤정글게임테크랩] UID:" + std::to_wstring(GetUUID()));
}

UTextComponent::~UTextComponent()
{

}

void UTextComponent::SetInstanceData(const FWstring& Characters)
{
	UResourceManager& ResourceManager = UResourceManager::GetInstance();
	InstanceData.clear();
	int NumCharacters = Characters.size();
	for (int Index = 0; Index < NumCharacters; Index++)
	{
		float OffsetY = (Index - NumCharacters/2);
		FVector4 Color(1, 1, 1, 1);
		FVector Offset(0.0f, OffsetY, 0.0f);
		uint32 CharIdx = ResourceManager.GetCharInfoIdx(Characters[Index]);

		InstanceData.push_back({ Color, Offset, CharIdx });
	}
}

void UTextComponent::SetText(const FWstring& InText)
{
	Text = InText;
	SetInstanceData(Text);
}
