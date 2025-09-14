#pragma once
#include "Core/Object.h"
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

	//////////////////리팩토링 예정///////////
	ID3D11Buffer* GetTextVertexBuffer() const { return TextVertexBuffer; }
	uint32 GetTextNumVertices() const { return TextNumVertices; }
	/////////////////////////////////////////

	void CreateTextSampler();
	ID3D11ShaderResourceView* LoadTexture(const FString& Path);
	ID3D11ShaderResourceView* GetTexture(const FString& Path);

	ID3D11SamplerState* GetSamplerState(ESamplerType Type);

	FCharacterInfo* LoadCharTable();

private:

	TMap<EPrimitiveType, ID3D11Buffer*> Vertexbuffers;
	TMap<EPrimitiveType, uint32> NumVertices;
	TMap<EPrimitiveType, TArray<FVertex>*> VertexDatas;

	TArray<FTextVertex>* TextVertexData;
	ID3D11Buffer* TextVertexBuffer;
	uint32 TextNumVertices;

	TMap<FString, ID3D11ShaderResourceView*> ShaderResourceViews;
	TMap<ESamplerType, ID3D11SamplerState*> SamplerStates;

	FCharacterInfo CharTable[95];
};
