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
	Vertexbuffer = ResourceManager.GetVertexbuffer(EPrimitiveType::Quad);
	NumVertices =ResourceManager.GetNumVertices(EPrimitiveType::Quad);

	FString Text = "UUID : "+ 1;
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
	for (int Index = 0; Index < Characters.size(); Index++)
	{
		float OffsetX = Index* 0.5f;
		InstanceData.push_back({ FVector4(1,1,1,1), FVector(OffsetX,0,2.0f), (uint32)Characters[Index] });
	}
}

