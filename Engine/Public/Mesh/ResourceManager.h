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

private:

	TMap<EPrimitiveType, ID3D11Buffer*> Vertexbuffers;
	TMap<EPrimitiveType, uint32> NumVertices;
	TMap<EPrimitiveType, TArray<FVertex>*> VertexDatas;
};
