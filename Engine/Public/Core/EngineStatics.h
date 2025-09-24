#pragma once

class UEngineStatics
{
public:
	static uint32 GenUUID()
	{
		return NextUUID++;
	}
	static uint32 GetNextUUID()
	{
		return NextUUID;
	}
	static void SetNextUUID(uint32 InUUID)
	{
		NextUUID = InUUID;
	}
private:
	static uint32 NextUUID;
};
