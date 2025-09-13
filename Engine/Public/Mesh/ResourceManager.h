#pragma once
#include "Core/Object.h"

enum class ESamplerType
{
	Text,
};
class UResourceManager : public UObject
{
	DECLARE_CLASS(UResourceManager, UObject)
	DECLARE_SINGLETON(UResourceManager)

public:
	void Initialize();

	void Release();

	TArray<FVertex>* GetVertexData(EPrimitiveType Type);
	ID3D11Buffer* GetVertexbuffer(EPrimitiveType Type);
	uint32 GetNumVertices(EPrimitiveType Type);
	ID3D11ShaderResourceView* LoadTexture(const FString& Path);
	ID3D11ShaderResourceView* GetTexture(const FString& Path);

	ID3D11SamplerState* GetSamplerState(ESamplerType Type);

	FCharacterInfo* LoadCharTable();

private:

	TMap<EPrimitiveType, ID3D11Buffer*> Vertexbuffers;
	TMap<EPrimitiveType, uint32> NumVertices;
	TMap<EPrimitiveType, TArray<FVertex>*> VertexDatas;

	TMap<EPrimitiveType, TArray<FTextVertex>*> TextVertexDatas;

	TMap<FString, ID3D11ShaderResourceView*> ShaderResourceViews;
	TMap<ESamplerType, ID3D11SamplerState*> SamplerStates;

	FCharacterInfo CharTable[95];
};
