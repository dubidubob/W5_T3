#include "pch.h"
#include "Math/OcclusionCulling.h"
#include "Mesh/StaticMeshComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"

// ====== 내부 유틸 ======
struct FClipVert { float x, y, z, w; };
struct FScreenVert { float x, y, z; };

struct FNormalVertex; // 선언만

struct FMeshView
{
	const FNormalVertex* V = nullptr;
	const uint32* I = nullptr;
	size_t VCount = 0;
	size_t ICount = 0;
	bool IsValid() const { return V && I && VCount > 0 && (ICount % 3 == 0); }
};

static inline FMeshView MakeMeshView(UStaticMeshComponent* Comp)
{
	FMeshView mv{};
	if (!Comp) return mv;
	if (UStaticMesh* sm = Comp->GetStaticMesh())
	{
		if (FStaticMesh* a = sm->GetStaticMeshAsset())
		{
			if (!a->Vertices.empty() && !a->Indices.empty())
			{
				mv.V = a->Vertices.data();
				mv.I = a->Indices.data();
				mv.VCount = a->Vertices.size();
				mv.ICount = a->Indices.size();
			}
		}
	}
	return mv;
}

// row-vector + row-major : (x y z 1) * VP
static inline FClipVert ToClip_RV(const FVector& pWS, const FMatrix& VP)
{
	FClipVert v{};
	v.x = pWS.X * VP.Data[0][0] + pWS.Y * VP.Data[1][0] + pWS.Z * VP.Data[2][0] + VP.Data[3][0];
	v.y = pWS.X * VP.Data[0][1] + pWS.Y * VP.Data[1][1] + pWS.Z * VP.Data[2][1] + VP.Data[3][1];
	v.z = pWS.X * VP.Data[0][2] + pWS.Y * VP.Data[1][2] + pWS.Z * VP.Data[2][2] + VP.Data[3][2];
	v.w = pWS.X * VP.Data[0][3] + pWS.Y * VP.Data[1][3] + pWS.Z * VP.Data[2][3] + VP.Data[3][3];
	return v;
}

static inline FScreenVert ToScreen(const FClipVert& v, int W, int H)
{
	const float invw = 1.f / v.w;
	const float ndcX = v.x * invw;     // [-1,1]
	const float ndcY = v.y * invw;     // [-1,1]
	const float ndcZ = v.z * invw;     // D3D: [0,1]
	FScreenVert s{};
	s.x = (ndcX * 0.5f + 0.5f) * float(W);
	s.y = (1.f - (ndcY * 0.5f + 0.5f)) * float(H);
	s.z = ndcZ;
	return s;
}

static inline float EdgeFunction(float ax, float ay, float bx, float by, float px, float py)
{
	return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

// 윈딩(CCW/CW) 무관 래스터 + depth min-write
static void RasterizeTriDepthMin(const FScreenVert& a,
	const FScreenVert& b,
	const FScreenVert& c,
	float* ZBuf, int ZW, int ZH)
{
	float minx = std::floor(std::min({ a.x,b.x,c.x }) - 0.5f);
	float maxx = std::ceil(std::max({ a.x,b.x,c.x }) + 0.5f);
	float miny = std::floor(std::min({ a.y,b.y,c.y }) - 0.5f);
	float maxy = std::ceil(std::max({ a.y,b.y,c.y }) + 0.5f);

	int x0 = std::max(0, int(minx));
	int x1 = std::min(ZW - 1, int(maxx));
	int y0 = std::max(0, int(miny));
	int y1 = std::min(ZH - 1, int(maxy));
	if (x0 > x1 || y0 > y1) return;

	const float area = EdgeFunction(a.x, a.y, b.x, b.y, c.x, c.y);
	if (area == 0.f) return;

	const float sign = (area > 0.f) ? 1.f : -1.f;
	const float invA = 1.f / area;

	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x)
		{
			const float px = x + 0.5f, py = y + 0.5f;
			float w0 = EdgeFunction(b.x, b.y, c.x, c.y, px, py) * sign;
			float w1 = EdgeFunction(c.x, c.y, a.x, a.y, px, py) * sign;
			float w2 = EdgeFunction(a.x, a.y, b.x, b.y, px, py) * sign;

			if (w0 >= 0.f && w1 >= 0.f && w2 >= 0.f)
			{
				w0 *= invA; w1 *= invA; w2 *= invA;
				const float z = w0 * a.z + w1 * b.z + w2 * c.z; // ndc z
				float& depth = ZBuf[y * ZW + x];
				if (z < depth) depth = z;
			}
		}
}

void OcclusionCulling::UpdateOcclusionCulling(TArray<UStaticMeshComponent*> PreCandidates,
	FVector CameraLocation,
	const FMatrix& ViewProj)
{
	VP = ViewProj;
	CreateZbuffer(ZbufferSize);
	SelectOccluder(PreCandidates, CameraLocation);
	FillOccluderInZbuffer();
	SetOutputArray();
}

void OcclusionCulling::GetOutputArrays(TArray<UStaticMeshComponent*>& Passed,
	TArray<UStaticMeshComponent*>& Failed)
{
	Passed = PassedCandidates;
	Failed = FailedCandidates;
}

void OcclusionCulling::CreateZbuffer(int Size)
{
	ZBuf.SetNum(Size * Size);
	std::fill(ZBuf.begin(), ZBuf.end(), 1.0f); // far 로 초기화
}

void OcclusionCulling::SelectOccluder(TArray<UStaticMeshComponent*> PreCandidates,
	FVector CameraLocation)
{
	NonOccluders.Empty();
	Occluders.empty();

	for (int i = 0; i < PreCandidates.Num(); ++i)
	{
		UStaticMeshComponent* C = PreCandidates[i];
		if (!C || !C->IsVisible()) continue;

		// 거리: 월드 위치 + 제곱거리(루트 회피)
		const FVector d = (CameraLocation - C->GetWorldLocation());
		const float distSq = d.X * d.X + d.Y * d.Y + d.Z * d.Z;

		UStaticMeshIdx node{ C, distSq };
		Occluders.push(node);

		// 큐 사이즈가 OccluderNum 초과하면 top()(가장 먼) 팝 → NonOccluders 로 이동
		if (Occluders.size() > OccluderNum)
		{
			UStaticMeshIdx popped = Occluders.top();
			Occluders.pop(); // priority 특수화에서 top() pop
			NonOccluders.Add(popped);
		}
	}

	// 큐에 남은 N개(가장 가까운 occluder) 이외의 나머지 후보들도 NonOccluders에 합쳐야 전체가 완성됨
	// 방법: 큐를 다 비우며 임시 벡터에 담았다가 occluder 리스트/NonOccluder 리스트로 재분배해도 됨.
	// 여기서는 "큐에 남은 것 = 진짜 occluder"로 쓰고, NonOccluders에는 지금까지 팝된 것만 들어감.
	// ※ 필요하면 여기에 전체 PreCandidates에서 occluder로 선정되지 않은 나머지를 추가하는 로직을 넣어도 됨.
}

void OcclusionCulling::FillOccluderInZbuffer()
{
	// 큐를 잠깐 비우며 보관(그리기 끝난 뒤 되돌려 쓰려면 임시배열 필요)
	TArray<UStaticMeshIdx> temp;
	temp.Reserve(Occluders.size());

	UStaticMeshIdx it;
	while (!Occluders.empty())
	{
		it = Occluders.top();
		Occluders.pop();
		temp.Add(it);
	}

	for (const UStaticMeshIdx& node : temp)
	{
		UStaticMeshComponent* C = node.Comp;
		const FMeshView mv = MakeMeshView(C);
		if (!mv.IsValid()) continue;

		// row-vector: local * (W*VP)
		const FMatrix WVP = (C->GetWorldTransformMatrix() * VP);

		// 한 번만 변환
		TArray<FClipVert> clip;  clip.SetNum(mv.VCount);
		TArray<FScreenVert> scr; scr.SetNum(mv.VCount);

		for (size_t i = 0; i < mv.VCount; ++i)
		{
			const FVector p = mv.V[i].Pos; // 로컬
			FClipVert cv;
			cv.x = p.X * WVP.Data[0][0] + p.Y * WVP.Data[1][0] + p.Z * WVP.Data[2][0] + WVP.Data[3][0];
			cv.y = p.X * WVP.Data[0][1] + p.Y * WVP.Data[1][1] + p.Z * WVP.Data[2][1] + WVP.Data[3][1];
			cv.z = p.X * WVP.Data[0][2] + p.Y * WVP.Data[1][2] + p.Z * WVP.Data[2][2] + WVP.Data[3][2];
			cv.w = p.X * WVP.Data[0][3] + p.Y * WVP.Data[1][3] + p.Z * WVP.Data[2][3] + WVP.Data[3][3];
			clip[i] = cv;

			if (cv.w <= 0.f) { scr[i] = { +INFINITY,+INFINITY,+INFINITY }; continue; }
			scr[i] = ToScreen(cv, ZbufferSize, ZbufferSize);
		}

		// 과격 reject 완화: 셋 다 뒤면 skip, 아니면 시도
		auto AnyInFront = [](const FClipVert& a, const FClipVert& b, const FClipVert& c) {
			return (a.w > 0.f) || (b.w > 0.f) || (c.w > 0.f);
			};

		for (size_t t = 0; t < mv.ICount; t += 3)
		{
			const uint32 i0 = mv.I[t + 0], i1 = mv.I[t + 1], i2 = mv.I[t + 2];
			const FClipVert& c0 = clip[i0];
			const FClipVert& c1 = clip[i1];
			const FClipVert& c2 = clip[i2];
			if (!AnyInFront(c0, c1, c2)) continue;

			const FScreenVert& a = scr[i0];
			const FScreenVert& b = scr[i1];
			const FScreenVert& c = scr[i2];
			if (!std::isfinite(a.x) || !std::isfinite(b.x) || !std::isfinite(c.x)) continue;

			RasterizeTriDepthMin(a, b, c, ZBuf.data(), ZbufferSize, ZbufferSize);
		}
	}

	// 필요하면 Occluders에 temp 재적재
	for (const auto& n : temp) { Occluders.push(n); }
}

bool OcclusionCulling::ZTestAABB(const UStaticMeshIdx& NonOccluder)
{
	UStaticMeshComponent* C = NonOccluder.Comp;
	if (!C) return true;

	const FAABB worldAABB = C->GetWorldBounds();
	if (!worldAABB.IsValid()) return true;

	// 스크린 rect + nearZ
	float minX = +INFINITY, minY = +INFINITY, maxX = -INFINITY, maxY = -INFINITY;
	float nearZ = +INFINITY; int valid = 0;

	for (int i = 0; i < 8; ++i)
	{
		const FVector pWS{
			(i & 4) ? worldAABB.Max.X : worldAABB.Min.X,
			(i & 2) ? worldAABB.Max.Y : worldAABB.Min.Y,
			(i & 1) ? worldAABB.Max.Z : worldAABB.Min.Z
		};

		const FClipVert cv = ToClip_RV(pWS, VP);
		if (cv.w <= 0.f) continue;
		const FScreenVert sv = ToScreen(cv, ZbufferSize, ZbufferSize);

		minX = std::min(minX, sv.x); minY = std::min(minY, sv.y);
		maxX = std::max(maxX, sv.x); maxY = std::max(maxY, sv.y);
		nearZ = std::min(nearZ, sv.z);
		++valid;
	}
	if (valid == 0) return true;

	int x0 = std::max(0, (int)std::floor(minX));
	int y0 = std::max(0, (int)std::floor(minY));
	int x1 = std::min(ZbufferSize - 1, (int)std::ceil(maxX));
	int y1 = std::min(ZbufferSize - 1, (int)std::ceil(maxY));
	if (x0 > x1 || y0 > y1) return true;

	const int w = (x1 - x0 + 1), h = (y1 - y0 + 1);
	const int strideX = std::max(1, w / 8);    // 튜닝 파라미터
	const int strideY = std::max(1, h / 8);
	const float depthBias = 1e-4f;

	for (int y = y0; y <= y1; y += strideY)
	{
		const int yy = std::min(y, y1);
		for (int x = x0; x <= x1; x += strideX)
		{
			const int xx = std::min(x, x1);
			const float zOcc = ZBuf[yy * ZbufferSize + xx];
			if (nearZ <= zOcc + depthBias)
				return true; // 보일 가능성 → 그린다
		}
	}
	return false; // 전부 뒤 → 가려짐
}

void OcclusionCulling::SetOutputArray()
{
	PassedCandidates.Empty();
	FailedCandidates.Empty();

	for (int i = 0; i < NonOccluders.Num(); ++i)
	{
		const UStaticMeshIdx& n = NonOccluders[i];
		if (!n.Comp) continue;

		if (ZTestAABB(n)) PassedCandidates.Add(n.Comp);
		else             FailedCandidates.Add(n.Comp);
	}
}
