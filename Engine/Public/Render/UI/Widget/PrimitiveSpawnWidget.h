#pragma once
#include "Widget.h"

class UPrimitiveSpawnWidget : public UWidget
{
	DECLARE_CLASS(UPrimitiveSpawnWidget, UWidget)
public:
	//inline static const char* PrimitiveTypes[] = {
	//"Cube",
	//"Sphere",
	//"Triangle",
	//"Square",
	//"Demon",
	//"Car"
	//};
	static inline TArray<const char*> PrimitiveTypes { "Cube" ,"Sphere" ,"Triangle" ,"Square" ,"Demon" ,"Car" };
	void Initialize() override;
	void Update() override;
	void RenderWidget() override;
	void SpawnActors() const;

	// Special Member Function
	UPrimitiveSpawnWidget();
	~UPrimitiveSpawnWidget() override;

private:
	int32 SelectedPrimitiveType = 0;
	int32 NumberOfSpawn = 1;
	float SpawnRangeMin = -5.0f;
	float SpawnRangeMax = 5.0f;
};
