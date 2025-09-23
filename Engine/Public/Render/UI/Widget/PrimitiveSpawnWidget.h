#pragma once
#include "Widget.h"

class UPrimitiveSpawnWidget : public UWidget
{
	DECLARE_CLASS(UPrimitiveSpawnWidget, UWidget)
public:
	// 새로운 OBJ 파일은 여기에 이름만 써주면 된다.
	static inline TArray<const char*> PrimitiveTypes { "Cube" ,"Sphere" ,"Cone" ,"Cylinder","Torus" ,"Demon" ,"Car"};
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
