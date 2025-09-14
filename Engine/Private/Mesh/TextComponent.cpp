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
	Vertexbuffer = ResourceManager.GetTextVertexBuffer();
	NumVertices = ResourceManager.GetTextNumVertices();

	FString Text = "UID:14fe42526";
	SetInstanceData(Text);
}

UTextComponent::~UTextComponent()
{

}

void UTextComponent::SetInstanceData(const FString& Characters)
{
	InstanceData.clear();
	const int CellWidth = 32;
	const int BitMapWidth = 512;
	const float Spacing = 1/4.0f;	//자간거리. 너무 작으면 글자가 겹침.
	int NumCharacters = Characters.size();
	for (int Index = 0; Index < NumCharacters; Index++)
	{
		float OffsetY = (Index - NumCharacters/2) * Spacing;	
		InstanceData.push_back({ FVector4(1,1,1,1), FVector(0.0f,OffsetY,2.0f), (uint32)Characters[Index] });
	}
}

