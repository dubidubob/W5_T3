#include "pch.h"
#include "Public/Mesh/StaticMesh/ObjImporter.h"
#include "Public/Mesh/StaticMesh/StaticMesh.h"
#include "Public/Manager/Path/PathManager.h"
#include "Mesh/ResourceManager.h"

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
	FString CurrentMaterialName;
	uint32 FaceCount = 0;

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
		else if (Prefix == "mtllib")
		{
			FString MtlFileName;
			Stream >> MtlFileName;
			path MtlFilePath = FilePath.parent_path() / MtlFileName;
			UE_LOG("%s", MtlFilePath.string().c_str());
			ParseMtlFile(MtlFilePath, OutObjInfo.Materials);
		}
		else if (Prefix == "usemtl")
		{
			FString CurrentMaterialNameStr;
			Stream >> CurrentMaterialNameStr;

			auto It = std::find_if(OutObjInfo.Materials.begin(), OutObjInfo.Materials.end(),
				[&](const FObjMaterialInfo& Info) {
					return Info.Name == CurrentMaterialNameStr;
				}
			);

			int32 MatIndex = -1;
			if (It != OutObjInfo.Materials.end())
			{
				MatIndex = std::distance(OutObjInfo.Materials.begin(), It);
			}

			FObjInfo::FMaterialGroup NewGroup;
			NewGroup.MaterialIndex = MatIndex;
			NewGroup.FirstFaceIndex = FaceCount;
			OutObjInfo.MaterialGroups.Add(NewGroup);
		}
		else if (Prefix == "f")
		{
			FString FaceData;
			while (Stream >> FaceData)
			{
				size_t Pos = FaceData.find('/');
				size_t Normal = FaceData.find('/', Pos + 1);

				uint32 PosIndex = 0;
				uint32 UVIndex = 0;
				uint32 NormalIndex = 0;

				PosIndex = std::stoi(FaceData.substr(0, Pos));

				if (Pos != std::string::npos && Normal != std::string::npos && Normal > Pos + 1)
				{
					// v/vt/vn
					UVIndex = std::stoi(FaceData.substr(Pos + 1, Normal - Pos - 1));
					NormalIndex = std::stoi(FaceData.substr(Normal + 1));
				}
				else if (Pos != std::string::npos && Normal == std::string::npos)
				{
					// v/vt
					UVIndex = std::stoi(FaceData.substr(Pos + 1));
				}
				else if (Pos != std::string::npos && Normal != std::string::npos && Normal == Pos + 1)
				{
					// v//vn
					NormalIndex = std::stoi(FaceData.substr(Normal + 1));
					UVIndex = 0;
				}

				if (PosIndex == 0) {
					UE_LOG("Error: Invalid PosIndex 0 found. Skipping.");
					continue;
				}

				OutObjInfo.PositionIndices.Add(PosIndex - 1);
				OutObjInfo.UVIndices.Add(UVIndex > 0 ? UVIndex - 1 : 0);
				OutObjInfo.NormalIndices.Add(NormalIndex > 0 ? NormalIndex - 1 : 0);
				int a = 0;
			}
			FaceCount++;
		}
	}

	return true;
}

bool FObjImporter::ParseMtlFile(const path& FilePath, TArray<FObjMaterialInfo>& OutMaterials)
{
	std::ifstream File(FilePath);
	if (!File.is_open())
	{
		UE_LOG("Failed to open file: %s", FilePath.c_str());
		return false;
	}

	FObjMaterialInfo CurrentMaterial;
	bool bFirstMaterial = true;
	FString Line;
	while (std::getline(File, Line))
	{
		std::stringstream Stream(Line);
		std::string Prefix;
		Stream >> Prefix;

		if (Prefix == "newmtl")
		{
			if (!bFirstMaterial)
			{
				UE_LOG("--- Parsed Material: %s ---", CurrentMaterial.Name.c_str());
				UE_LOG("  - Ka: %.2f %.2f %.2f", CurrentMaterial.AmbientColor.X, CurrentMaterial.AmbientColor.Y, CurrentMaterial.AmbientColor.Z);
				UE_LOG("  - Kd: %.2f %.2f %.2f", CurrentMaterial.DiffuseColor.X, CurrentMaterial.DiffuseColor.Y, CurrentMaterial.DiffuseColor.Z);
				UE_LOG("  - Ks: %.2f %.2f %.2f", CurrentMaterial.SpecularColor.X, CurrentMaterial.SpecularColor.Y, CurrentMaterial.SpecularColor.Z);
				UE_LOG("  - Ns: %.2f", CurrentMaterial.SpecularExponent);
				UE_LOG("  - Alpha: %.2f", CurrentMaterial.Alpha);
				if (!CurrentMaterial.DiffuseTexturePath.empty())
				{
					UE_LOG("  - Diffuse Map: %s", CurrentMaterial.DiffuseTexturePath.c_str());
				}
				if (!CurrentMaterial.NormalTexturePath.empty())
				{
					UE_LOG("  - Normal Map: %s", CurrentMaterial.NormalTexturePath.c_str());
				}
				OutMaterials.Add(CurrentMaterial);
			}
			bFirstMaterial = false;
			CurrentMaterial = FObjMaterialInfo();
			Stream >> CurrentMaterial.Name;
		}
		else if (Prefix == "Ka")
		{
			float R, G, B;
			Stream >> R >> G >> B;
			CurrentMaterial.AmbientColor = FVector(R, G, B);
		}
		else if (Prefix == "Kd")
		{
			float R, G, B;
			Stream >> R >> G >> B;
			CurrentMaterial.DiffuseColor = FVector(R, G, B);
		}
		else if (Prefix == "Ks")
		{
			float R, G, B;
			Stream >> R >> G >> B;
			CurrentMaterial.SpecularColor = FVector(R, G, B);
		}
		else if (Prefix == "Ns")
		{
			float Ns;
			Stream >> Ns;
			CurrentMaterial.SpecularExponent = Ns;
		}
		else if (Prefix == "map_Kd")
		{
			Stream >> CurrentMaterial.DiffuseTexturePath;
		}
	}

	if (!bFirstMaterial)
	{
		UE_LOG("--- Parsed Material: %s ---", CurrentMaterial.Name.c_str());
		UE_LOG("  - Ka: %.2f %.2f %.2f", CurrentMaterial.AmbientColor.X, CurrentMaterial.AmbientColor.Y, CurrentMaterial.AmbientColor.Z);
		UE_LOG("  - Kd: %.2f %.2f %.2f", CurrentMaterial.DiffuseColor.X, CurrentMaterial.DiffuseColor.Y, CurrentMaterial.DiffuseColor.Z);
		UE_LOG("  - Ks: %.2f %.2f %.2f", CurrentMaterial.SpecularColor.X, CurrentMaterial.SpecularColor.Y, CurrentMaterial.SpecularColor.Z);
		UE_LOG("  - Ns: %.2f", CurrentMaterial.SpecularExponent);
		UE_LOG("  - Alpha: %.2f", CurrentMaterial.Alpha);
		if (!CurrentMaterial.DiffuseTexturePath.empty())
		{
			UE_LOG("  - Diffuse Map: %s", CurrentMaterial.DiffuseTexturePath.c_str());
		}
		if (!CurrentMaterial.NormalTexturePath.empty())
		{
			UE_LOG("  - Normal Map: %s", CurrentMaterial.NormalTexturePath.c_str());
		}
		OutMaterials.Add(CurrentMaterial);
	}

	return true;
}

void FObjImporter::ConvertObjToStaticMesh(const FObjInfo& ObjInfo, FStaticMesh& OutStaticMesh)
{
	std::unordered_map<FString, uint32> VertexMap;

	for (size_t Idx = 0; Idx < ObjInfo.PositionIndices.size(); ++Idx)
	{
		uint32 PosIndex = ObjInfo.PositionIndices[Idx];
		uint32 UVIndex = ObjInfo.UVIndices[Idx];
		uint32 NormalIndex = ObjInfo.NormalIndices[Idx];

		FString VertexKey = std::format("{}_{}_{}", PosIndex, UVIndex, NormalIndex);

		uint32 VertexIndex;
		auto It = VertexMap.find(VertexKey);

		if (It != VertexMap.end())
		{
			// Already Vertex Exists
			VertexIndex = It->second;
		}
		else
		{
			FNormalVertex Vertex;
			Vertex.Pos = PositionToUEBasis(ObjInfo.Positions[PosIndex]);

			if (UVIndex < ObjInfo.UVs.size()) { Vertex.Tex = UVToUEBasis(ObjInfo.UVs[UVIndex]); }
			else { Vertex.Tex = FVector2(0.0f, 0.0f); }

			if (NormalIndex < ObjInfo.Normals.size()) { Vertex.Normal = PositionToUEBasis(ObjInfo.Normals[NormalIndex]); }
			else { Vertex.Normal = FVector(0.0f, 0.0f, 1.0f); }

			Vertex.Color = { static_cast<float>(rand()) / RAND_MAX ,
				static_cast<float>(rand()) / RAND_MAX ,
				static_cast<float>(rand()) / RAND_MAX ,
				1.0f };
			VertexIndex = OutStaticMesh.Vertices.size();
			OutStaticMesh.Vertices.Add(Vertex);
			VertexMap[VertexKey] = VertexIndex;
		}

		// 실제 의미있는 인덱스 추가
		OutStaticMesh.Indices.Add(VertexIndex);
	}

	// Material Section 처리 (기존과 동일하지만 주석 추가)
	for (int32 Idx = 0; Idx < ObjInfo.MaterialGroups.Num(); ++Idx)
	{
		const auto& Group = ObjInfo.MaterialGroups[Idx];
		FStaticMeshSection NewSection;
		NewSection.MaterialIndex = Group.MaterialIndex;
		NewSection.FirstIndex = Group.FirstFaceIndex * 3; // 1개 면 = 3개 인덱스

		if (Idx + 1 < ObjInfo.MaterialGroups.Num())
		{
			// 다음 그룹까지의 면 개수 * 3
			NewSection.NumIndices = (ObjInfo.MaterialGroups[Idx + 1].FirstFaceIndex - Group.FirstFaceIndex) * 3;
		}
		else
		{
			// 마지막 그룹 - 끝까지의 면 개수 * 3
			uint32 TotalFaces = ObjInfo.PositionIndices.size() / 3;
			NewSection.NumIndices = (TotalFaces - Group.FirstFaceIndex) * 3;
		}

		OutStaticMesh.Sections.Add(NewSection);
	}

	// Material 정보 복사 (기존과 동일)
	for (const FObjMaterialInfo& ObjMat : ObjInfo.Materials)
	{
		FStaticMaterial StaticMat;
		StaticMat.AmbientColor = ObjMat.AmbientColor;
		StaticMat.DiffuseColor = ObjMat.DiffuseColor;
		StaticMat.SpecularColor = ObjMat.SpecularColor;
		StaticMat.SpecularExponent = ObjMat.SpecularExponent;
		StaticMat.Alpha = ObjMat.Alpha;

		if (!ObjMat.DiffuseTexturePath.empty())
		{
			StaticMat.DiffuseSRV = UResourceManager::GetInstance().GetTexture(ObjMat.DiffuseTexturePath);
		}
		if (!ObjMat.NormalTexturePath.empty())
		{
			StaticMat.NormalSRV = UResourceManager::GetInstance().GetTexture(ObjMat.NormalTexturePath);
		}

		OutStaticMesh.Materials.Add(StaticMat);
	}

	UE_LOG("Vertex optimization: %d indices -> %d unique vertices",	ObjInfo.PositionIndices.size(), OutStaticMesh.Vertices.size());
}
