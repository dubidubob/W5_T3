#pragma once
#include "Core/Object.h"
struct FMaterialResource
{
	// 텍스쳐 버퍼 저장하기
	// 추후, 픽셀 셰이더나 필요한 Set 저장 
};

class UMaterial : public UObject
{
	DECLARE_CLASS(UMaterial, UObject)

public:
	UMaterial();
	virtual ~UMaterial();
private:
	FMaterialResource* Resource;
};

