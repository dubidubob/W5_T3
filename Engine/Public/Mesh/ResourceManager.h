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
	ID3D11Buffer* GetVertexBuffer(EPrimitiveType Type);
	uint32 GetVertexNum(EPrimitiveType Type);

	TArray<FVertex>* GetReducedVertexData(EPrimitiveType Type);
	ID3D11Buffer* GetReducedVertexBuffer(EPrimitiveType Type);
	uint32 GetReducedVertexNum(EPrimitiveType Type);

	TArray<uint32>* GetIndexData(EPrimitiveType Type);
	ID3D11Buffer* GetIndexBuffer(EPrimitiveType Type);
	uint32 GetIndexNum(EPrimitiveType Type);

	//////////////////리팩토링 예정///////////
	ID3D11Buffer* GetTextVertexBuffer() const { return TextVertexBuffer; }
	uint32 GetTextNumVertices() const { return TexVertexNum; }
	/////////////////////////////////////////

	void CreateTextSampler();
	ID3D11ShaderResourceView* LoadTexture(const FString& Path);
	ID3D11ShaderResourceView* GetTexture(const FString& Path);

	ID3D11SamplerState* GetSamplerState(ESamplerType Type);

	int32 GetCharInfoIdx(WCHAR Char);
	const TArray<FCharacterInfo>& GetCharInfos();

private:
	void LoadCharInfoMap();

	TMap<EPrimitiveType, TArray<FVertex>*> VertexData;
	TMap<EPrimitiveType, ID3D11Buffer*> VertexBuffers;
	TMap<EPrimitiveType, uint32> VertexNum;

	TMap<EPrimitiveType, TArray<uint32>> IndexData;
	TMap<EPrimitiveType, ID3D11Buffer*> IndexBuffers;
	TMap<EPrimitiveType, uint32> IndexNum;

	TMap<EPrimitiveType, TArray<FVertex>*> ReducedVertexData;
	TMap<EPrimitiveType, ID3D11Buffer*> ReducedVertexBuffers;
	TMap<EPrimitiveType, uint32> ReducedVertexNum;

	TArray<FTextVertex>* TextVertexData;
	ID3D11Buffer* TextVertexBuffer;
	uint32 TexVertexNum;

	TMap<FString, ID3D11ShaderResourceView*> ShaderResourceViews;
	TMap<ESamplerType, ID3D11SamplerState*> SamplerStates;

	TArray<FCharacterInfo> CharInfos;
	TMap<WCHAR, int32> CharInfoIdxMap;
};
