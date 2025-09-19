#pragma once
#include "Core/Object.h"
struct FMaterialResource
{

};

class UMaterial : public UObject
{
	DECLARE_CLASS(UMaterial, UObject)

public:
	UMaterial();
	virtual ~UMaterial();
private:
	FMaterialResource Resource;
};

