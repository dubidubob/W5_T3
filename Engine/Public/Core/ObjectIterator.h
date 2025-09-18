#pragma once

template<typename TObject>
class TObjectIterator
{
public:
	TObjectIterator()
	{
		CurrentIndex = 0;
		AdvanceToNextValidObject();
	}

	explicit operator bool() const
	{
		return CurrentObject != nullptr;
	}

	TObject& operator*() const
	{
		return *CurrentObject;
	}

	TObject* operator->() const
	{
		return CurrentObject;
	}

	TObjectIterator& operator++()
	{
		++CurrentIndex;
		AdvanceToNextValidObject();
		return *this;
	}

	TObjectIterator operator++(int)
	{
		TObjectIterator Tmp = *this;
		++(*this);
		return Tmp;
	}

	bool operator==(const TObjectIterator& Other) const
	{
		return CurrentObject == Other.CurrentObject;
	}

	bool operator!=(const TObjectIterator& Other) const
	{
		return CurrentObject != Other.CurrentObject;
	}
private:
	void AdvanceToNextValidObject()
	{
		CurrentObject = nullptr;
		while (CurrentIndex < GUObjectArray.size())
		{
			UObject* Obj = GUObjectArray[CurrentIndex];
			if (Obj && Obj->IsA(TObject::StaticClass()))
			{
				CurrentObject = Cast<TObject>(Obj);
				return;
			}
			++CurrentIndex;
		}
	}

	int32 CurrentIndex;
	TObject* CurrentObject = nullptr;
};
