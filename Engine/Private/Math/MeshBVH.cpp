#include "pch.h"
#include "Math/MeshBVH.h"
#include "Mesh/StaticMesh/StaticMesh.h"

void FMeshBVH::Clear()
{
	Items.clear();
	Nodes.clear();
	Root = -1;
	LeafMax = 4;
}

void FMeshBVH::Build(const FStaticMesh& Mesh)
{
	// 지금까지 자료구조 지우기
	Clear();
	if (!&Mesh || Mesh.Indices.IsEmpty()) return;

	uint32 NumTriangles = Mesh.Indices.Num() / 3;
	Items.Reserve(NumTriangles);

	for (uint32 i = 0; i < NumTriangles; ++i)
	{
		const FVector& V0 = Mesh.Vertices[Mesh.Indices[i * 3 + 0]].Pos;
		const FVector& V1 = Mesh.Vertices[Mesh.Indices[i * 3 + 1]].Pos;
		const FVector& V2 = Mesh.Vertices[Mesh.Indices[i * 3 + 2]].Pos;

		FMeshBVHItem Item;
		Item.VertexPosX[0] = V0.X; Item.VertexPosX[1] = V1.X; Item.VertexPosX[2] = V2.X;
		Item.VertexPosY[0] = V0.Y; Item.VertexPosY[1] = V1.Y; Item.VertexPosY[2] = V2.Y;
		Item.VertexPosZ[0] = V0.Z; Item.VertexPosZ[1] = V1.Z; Item.VertexPosZ[2] = V2.Z;

		// 삼각형 AABB 계산
		const FVector Min(std::min({ V0.X,V1.X,V2.X }),
			std::min({ V0.Y,V1.Y,V2.Y }),
			std::min({ V0.Z,V1.Z,V2.Z }));
		const FVector Max(std::max({ V0.X,V1.X,V2.X }),
			std::max({ V0.Y,V1.Y,V2.Y }),
			std::max({ V0.Z,V1.Z,V2.Z }));
		Item.Bounds = FAABB(Min, Max);
		Item.Centroid = Item.Bounds.GetCenter();
		Item.OriginIdx = i;
		Items.Add(Item);
	}

	if (Items.Num() == 0) { Root = -1; return; }

	Nodes.Reserve(static_cast<int32>(Items.size()) * 2);
	Root = BuildRange(0, Items.Num(), 0);
}

struct FCand { FMeshBVHItem* Comp; float TNear; };

void FMeshBVH::QueryRayLocalMesh(const FRay& ModelRay, int MaxK, TArray<FMeshBVHItem*>& Out)
{
	Out.clear();

	if (Root < 0) return;

	float TMaxGlobal = FLT_MAX;

	struct FStackItem { int32 NodeIdx; float TNear; };
	TArray<FStackItem> Stack; Stack.reserve(64);
	Stack.push_back({ Root, 0.0f });

	TArray<FCand> Cands; Cands.reserve(MaxK > 0 ? MaxK * 2 : 64);

	while (!Stack.empty())
	{
		const FStackItem It = Stack.back();
		Stack.pop_back();
		const FBVHNode& Node = Nodes[It.NodeIdx];

		// 노드 박스와 레이 교차: front-to-back 순서를 위해 T0(엔트리), T1(출구) 확보
		float N0 = 0.0f, N1 = 0.0f;
		if (!IntersectAABB(ModelRay, Node.Bounds, TMaxGlobal, N0, N1)) continue;

		if (Node.bLeaf)
		{
			// 리프: 각 아이템 AABB 교차 → 후보 수집
			for (int i = 0; i < Node.Count; ++i)
			{
				FMeshBVHItem& Item = Items[Node.First + i];
				float TNearItem;
				if (RayBoxEntry(Item.Bounds, ModelRay, TNearItem))
				{
					if (TNearItem <= TMaxGlobal)
						Cands.push_back({ &Item , TNearItem });
				}
			}
		}
		else
		{
			// 자식들 front-to-back 방문 (가까운 쪽이 먼저 팝되도록 먼 쪽을 먼저 푸시)
			int32 L = Node.Left, R = Node.Right;
			float L0 = 0.f, L1 = 0.f, R0 = 0.f, R1 = 0.f;
			bool HL = false, HR = false;

			if (L >= 0) HL = IntersectAABB(ModelRay, Nodes[L].Bounds, TMaxGlobal, L0, L1);
			if (R >= 0) HR = IntersectAABB(ModelRay, Nodes[R].Bounds, TMaxGlobal, R0, R1);

			if (HL && HR)
			{
				if (L0 > R0) { std::swap(L, R); std::swap(L0, R0); }
				Stack.push_back({ R, R0 });
				Stack.push_back({ L, L0 });
			}
			else if (HL)
			{
				Stack.push_back({ L, L0 });
			}
			else if (HR)
			{
				Stack.push_back({ R, R0 });
			}
		}
	}

	// 거리순 정렬 + K개 컷
	std::sort(Cands.begin(), Cands.end(),
		[](const FCand& A, const FCand& B) { return A.TNear < B.TNear; });

	if (MaxK > 0 && (int)Cands.size() > MaxK)
		Cands.resize(MaxK);

	Out.reserve(Cands.size());
	for (auto& C : Cands)
		Out.push_back(C.Comp);
}

int32 FMeshBVH::BuildRange(int32 First, int32 Last, int32 Depth)
{
	FBVHNode Node;

	// Create new AABB that Include First ~ Last Objects(Vertex)
	FAABB BoundingBox;
	for (int32 i = First; i < Last; ++i) {
		BoundingBox.AddAABB(Items[i].Bounds);
	}
	Node.Bounds = BoundingBox;

	int32 Count = Last - First;
	// Make Leaf Node
	if (Count <= LeafMax || Depth >= MaxDepth)
	{
		Node.bLeaf = true;
		Node.First = First;
		Node.Count = Count;
		int32 Index = static_cast<int32>(Nodes.size());
		Nodes.push_back(Node);
		return Index; // Return to Parent with Their Index
	}

	// Midian Split for Choose Axis
	FAABB CentroidBoundingBox;
	for (int32 i = First; i < Last; ++i)
	{
		CentroidBoundingBox.AddPoint(Items[i].Centroid);
	}
	// Choose Most Longest Axis
	int Axis = ChooseAxis(CentroidBoundingBox);

	int32 Mid = (First + Last) / 2;
	auto ItBeg = Items.begin() + First;
	auto ItMid = Items.begin() + Mid;
	auto ItEnd = Items.begin() + Last;

	// Midian is In ItMid after nth_element Executed, Partial reorder A : Small than Mid, B : Large than Mid
	if (Axis == 0)
	{
		std::nth_element(ItBeg, ItMid, ItEnd, [](const FMeshBVHItem& A, const FMeshBVHItem& B) { return A.Centroid.X < B.Centroid.X; });
	}
	else if (Axis == 1)
	{
		std::nth_element(ItBeg, ItMid, ItEnd, [](const FMeshBVHItem& A, const FMeshBVHItem& B) { return A.Centroid.Y < B.Centroid.Y; });
	}
	else
	{
		std::nth_element(ItBeg, ItMid, ItEnd, [](const FMeshBVHItem& A, const FMeshBVHItem& B) { return A.Centroid.Z < B.Centroid.Z; });
	}

	int32 Index = static_cast<int32>(Nodes.size());
	Nodes.push_back(FBVHNode());

	int32 L = BuildRange(First, Mid, Depth + 1);
	int32 R = BuildRange(Mid, Last, Depth + 1);

	Nodes[Index].Bounds = Nodes[L].Bounds + Nodes[R].Bounds;
	Nodes[Index].Left = L;
	Nodes[Index].Right = R;

	return Index;
}
