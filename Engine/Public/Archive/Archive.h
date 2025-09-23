// FArchive.h
#pragma once
class FArchive
{
public:
	virtual ~FArchive() {}
	virtual FArchive& Serialize(void* V, size_t Length) = 0;
	virtual bool IsLoading() const = 0;

	// 기본 타입 직렬화
	FArchive& operator<<(bool& Value) { Serialize(&Value, sizeof(bool)); return *this; }
	FArchive& operator<<(uint32& Value) { Serialize(&Value, sizeof(uint32)); return *this; }
	FArchive& operator<<(int32& Value) { Serialize(&Value, sizeof(int32)); return *this; }
	FArchive& operator<<(float& Value) { Serialize(&Value, sizeof(float)); return *this; }
	FArchive& operator<<(double& Value) { Serialize(&Value, sizeof(double)); return *this; }
	FArchive& operator<<(size_t& Value) { Serialize(&Value, sizeof(size_t)); return *this; }

	FArchive& operator<<(FVector& Vector) {	*this << Vector.X; *this << Vector.Y; *this << Vector.Z; return *this; }
	FArchive& operator<<(FVector2& Vector) { *this << Vector.X; *this << Vector.Y; return *this; }
	FArchive& operator<<(FVector4& Vector) { *this << Vector.X; *this << Vector.Y; *this << Vector.Z; *this << Vector.W; return *this; }

	FArchive& operator<<(FString& String)
	{
		if (IsLoading()) // Loading
		{
			size_t Length = 0;
			*this << Length;
			String.resize(Length);

			if (Length > 0)
			{
				Serialize(&String[0], Length);
			}
		}
		else // Saving
		{
			size_t Length = String.length();
			*this << Length;
			if (Length > 0)
			{
				Serialize(&String[0], Length);
			}
		}

		return *this;
	}

	template<typename T>
	FArchive& operator<<(TArray<T>& Array)
	{
		size_t Count = Array.size();
		*this << Count;
		if (IsLoading()) { Array.SetNum(Count); }

		for (size_t Idx = 0; Idx < Count; ++Idx)
		{
			*this << Array[Idx];
		}

		return *this;
	}
};

class FWindowsBinWriter : public FArchive
{
private:
	std::ofstream FileStream;

public:
	FWindowsBinWriter(const path& FilePath);
	~FWindowsBinWriter();

	virtual FArchive& Serialize(void* V, size_t Length) override;
	virtual bool IsLoading() const override { return false; }
};


class FWindowsBinReader : public FArchive
{
private:
	std::ifstream FileStream;

public:
	FWindowsBinReader(const path& FilePath);
	~FWindowsBinReader();

	virtual FArchive& Serialize(void* V, size_t Length) override;
	virtual bool IsLoading() const override { return true; }
};
