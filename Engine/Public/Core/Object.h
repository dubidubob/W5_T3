#pragma once
#include "Class.h"

class ULevel;

UCLASS()
class UObject
{
	GENERATED_BODY()
	DECLARE_CLASS(UObject, UObject)

public:
	// Special Member Function
	UObject();
	virtual ~UObject();

	// Getter & Setter
	uint32 GetInternalIndex() const { return InternalIndex; }
	FString GetName() const { return Name.ToString(); }
	FString GetBaseName() const { return Name.ToBaseNameString(); }
	const UObject* GetOuter() const { return Outer; }
	uint32 GetUUID() const { return UUID; }

	void SetName(const FName& InName) { Name = InName; }
	void SetOuter(UObject* InObject);
	void SetUUID(uint32 InUUID);

	void AddMemoryUsage(uint64 InBytes, uint32 InCount = 1);
	void RemoveMemoryUsage(uint64 InBytes, uint32 InCount = 1);

	uint64 GetAllocatedBytes() const { return AllocatedBytes; }
	uint32 GetAllocatedCount() const { return AllocatedCounts; }

	bool IsA(const UClass* InClass) const;

	virtual void DuplicateSubObjects(ULevel* InLevel);
	virtual UObject* Duplicate(ULevel* InLevel);

private:
	uint32 UUID = -1;
	uint32 InternalIndex = -1;
	FName Name;
	UObject* Outer;

	uint64 AllocatedBytes = 0;
	uint32 AllocatedCounts = 0;
};

extern TArray<UObject*> GUObjectArray;

template <typename T>
T* NewObject()
{
	T* NewObject = new T();
	NewObject->SetName(FNameTable::GetInstance().GetUniqueName(NewObject->GetClass()->GetName()));

	return NewObject;
}

// UObject Cast
template<typename T>
T* Cast(UObject* Object)
{
	if (!Object)
		return nullptr;

	if (Object->IsA(T::StaticClass()))
		return static_cast<T*>(Object);

	return nullptr;
}

inline bool IsValid(UObject* Object)
{
	if(Object && Object->GetInternalIndex() >= GUObjectArray.Num())
	{
		return false;
	}

	return (GUObjectArray[Object->GetInternalIndex()] == Object);
}
