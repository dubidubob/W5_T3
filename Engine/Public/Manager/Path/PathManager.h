#pragma once
#include "Core/Object.h"

class UPathManager : public UObject
{
	DECLARE_CLASS(UPathManager, UObject)
	DECLARE_SINGLETON(UPathManager)

public:
	void Init();

	// Base Path
	const path& GetRootPath() const { return RootPath; }
	const path& GetDataPath() const { return DataPath; }

	// Detailed Asset Path
	const path& GetShaderPath() const { return ShaderPath; }
	const path& GetTexturePath() const { return TexturePath; }
	const path& GetModelPath() const { return ModelPath; }
	const path& GetAudioPath() const { return AudioPath; }
	const path& GetWorldPath() const { return WorldPath; }
	const path& GetConfigPath() const { return ConfigPath; }
	const path& GetEditorIniPath() const { return EditorIniPath; }
	const path& GetFontPath() const { return FontPath; }
	const path& GetBinaryPath() const { return BinaryPath; }

private:
	path RootPath;
	path DataPath;
	path ShaderPath;
	path TexturePath;
	path ModelPath;
	path AudioPath;
	path WorldPath;
	path ConfigPath;
	path EditorIniPath;
	path FontPath;
	path BinaryPath;

	void InitializeRootPath();
	void GetEssentialPath();
	void ValidateAndCreateDirectories() const;
};
