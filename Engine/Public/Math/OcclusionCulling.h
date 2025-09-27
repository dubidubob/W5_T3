#pragma once
#include "Global/Types.h"
#include "Global/CoreTypes.h"
#include <queue>

class UStaticMeshComponent;

struct UStaticMeshIdx
{
	UStaticMeshComponent* Comp;
	float Dist;

	bool operator<(const UStaticMeshIdx& Other) const
	{
		return Dist < Other.Dist;
	}
};

class OcclusionCulling
{
public:
	void UpdateOcclusionCulling(TArray<UStaticMeshComponent*> PreCandidates,
								FVector CameraLocation,
								const FMatrix& ViewProj);
	void GetOutputArrays(TArray<UStaticMeshComponent*>& Passed,
						TArray<UStaticMeshComponent*>& Failed);
private:
	std::priority_queue<UStaticMeshIdx,
		std::vector<UStaticMeshIdx>,
		std::less<UStaticMeshIdx>> Occluders;
	TArray<UStaticMeshIdx> NonOccluders;

	int OccluderNum = 1000;
	int ZbufferSize = 256;
	TArray<float> ZBuf;

	FMatrix VP;

	TArray<UStaticMeshComponent*> PassedCandidates;
	TArray<UStaticMeshComponent*> FailedCandidates;

	// Frustum 안 애들에 대해 순회해서 값을 매겨 Priority Queue에 담는다.
	// N개가 넘으면 Pop 하기
	void SelectOccluder(TArray<UStaticMeshComponent*> PreCandidates,
						FVector CameraLocation);

	// CPU 256x256 Z buffer을 구성한다.
	void CreateZbuffer(int Size);
	
	// 상위 1000개를 래스터화 하여, Z buffer를 채운다.
	void FillOccluderInZbuffer();

	// 나머지 애들의 AABB Box를 Z Test를 하고, 가려졌다면 해당 Box를 특정 Array에 PushBack하고
	// 끝까지 남았다면 Draw 넣을 Array에 넣어 반환한다.
	void SetOutputArray();
	bool ZTestAABB(const UStaticMeshIdx& NonOccluder);
};

