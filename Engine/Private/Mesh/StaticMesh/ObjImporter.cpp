#include "pch.h"
#include "Public/Mesh/StaticMesh/ObjImporter.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"
#include "Public/Manager/Path/PathManager.h"

FStaticMesh* FObjImporter::ParseAndConvert(const FString& FileName)
{
	FObjInfo RawData;
	if (!ParseObjFile(FileName, RawData))
	{
		return nullptr;
	}

	FStaticMesh* CookedData = new FStaticMesh();
	ConvertObjToStaticMesh(RawData, *CookedData);

	return CookedData;
}

bool FObjImporter::ParseObjFile(const FString& FileName, FObjInfo& OutObjInfo)
{
	const path FilePath = UPathManager::GetInstance().GetDataPath() / FileName;
	std::ifstream File(FilePath);
	if (!File.is_open())
	{
		UE_LOG("Failed to open file: %s", FilePath.c_str());
		return false;
	}

	FString Line;
	while (std::getline(File, Line))
	{
		std::stringstream Stream(Line);
		FString Prefix;
		Stream >> Prefix;

		if (Prefix == "v")
		{
			float X, Y, Z;
			Stream >> X >> Y >> Z;
			OutObjInfo.Positions.Add(FVector(X, Y, Z));
		}
		else if (Prefix == "vt")
		{
			float U, V;
			Stream >> U >> V;
			OutObjInfo.UVs.Add(FVector2(U, V));
		}
		else if (Prefix == "vn")
		{
			float X, Y, Z;
			Stream >> X >> Y >> Z;
			OutObjInfo.Normals.Add(FVector(X, Y, Z));
		}
		else if (Prefix == "f")
		{
			FString FaceData;
			while (Stream >> FaceData)
			{
				std::replace(FaceData.begin(), FaceData.end(), '/', ' ');
				std::stringstream FaceStream(FaceData);
				uint32 PosIndex, UVIndex, NormalIndex;
				FaceStream >> PosIndex >> UVIndex >> NormalIndex;

				// OBJ 인덱스는 1부터 시작하므로 0기반 인덱스로 변환
				OutObjInfo.PositionIndices.Add(PosIndex - 1);
				OutObjInfo.UVIndices.Add(UVIndex - 1);
				OutObjInfo.NormalIndices.Add(NormalIndex - 1);
			}
		}
	}
	return true;

}

void FObjImporter::ConvertObjToStaticMesh(const FObjInfo& ObjInfo, FStaticMesh& OutStaticMesh)
{
	for (size_t Idx = 0; Idx < ObjInfo.PositionIndices.size(); ++Idx)
	{
		uint32 PosIndex = ObjInfo.PositionIndices[Idx];
		uint32 UVIndex = ObjInfo.UVIndices[Idx];
		uint32 NormalIndex = ObjInfo.NormalIndices[Idx];

		// FNormalVertex 생성
		FNormalVertex Vertex;
		Vertex.Pos = ObjInfo.Positions[PosIndex];
		Vertex.Tex = ObjInfo.UVs[UVIndex];
		Vertex.Normal = ObjInfo.Normals[NormalIndex];
		// OutStaticMesh에 추가
		OutStaticMesh.Vertices.Add(Vertex);
		OutStaticMesh.Indices.Add(Idx);
	}
}
