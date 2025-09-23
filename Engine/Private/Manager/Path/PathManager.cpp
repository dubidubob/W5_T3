#include "pch.h"
#include "Manager/Path/PathManager.h"

IMPLEMENT_CLASS(UPathManager, UObject)
IMPLEMENT_SINGLETON(UPathManager)

UPathManager::UPathManager()
{
	Init();
}

UPathManager::~UPathManager() = default;

void UPathManager::Init()
{
	InitializeRootPath();
	GetEssentialPath();
	ValidateAndCreateDirectories();

	UE_LOG("PathManager: Initialized Successfully");
	UE_LOG("PathManager: Solution Path: %s", RootPath.c_str());
	UE_LOG("PathManager: Asset Path: %s", DataPath.c_str());
}

/**
 * @brief 프로그램 실행 파일을 기반으로 Root Path 세팅하는 함수
 */
void UPathManager::InitializeRootPath()
{
	// Get Execution File Path
	wchar_t ProgramPath[MAX_PATH];
	GetModuleFileNameW(nullptr, ProgramPath, MAX_PATH);

	// Add Root Path
	RootPath = path(ProgramPath).parent_path();
}

/**
 * @brief 프로그램 실행에 필요한 필수 경로를 추가하는 함수
 */
void UPathManager::GetEssentialPath()
{
	// Add Essential

	DataPath = RootPath / L"Data";
	ShaderPath = DataPath / L"Shader";
	TexturePath = DataPath / "Texture";
	ModelPath = DataPath / "Model";
	AudioPath = DataPath / "Audio";
	WorldPath = DataPath / "World";
	ConfigPath = DataPath / "Config";
	EditorIniPath = ConfigPath / "editor.ini";
	FontPath = DataPath / "Font";
	BinaryPath = DataPath / "Binary";
}

/**
 * @brief 경로 유효성을 확인하고 없는 디렉토리를 생성하는 함수
 */
void UPathManager::ValidateAndCreateDirectories() const
{
	TArray<path> DirectoriesToCreate = {
		DataPath,
		ShaderPath,
		TexturePath,
		ModelPath,
		AudioPath,
		WorldPath,
		ConfigPath,
		FontPath,
		BinaryPath
	};

	for (const auto& Directory : DirectoriesToCreate)
	{
		try
		{
			if (!exists(Directory))
			{
				create_directories(Directory);
				UE_LOG("PathManager: Created Directory: %s", Directory.c_str());
			}
			else
			{
				UE_LOG("PathManager: Directory Exists: %s", Directory.c_str());
			}
		}
		catch (const filesystem::filesystem_error& e)
		{
			UE_LOG("PathManager:  Failed To Create Directory %s: %s", Directory.c_str(), e.what());
			assert(!"Asset 경로 생성 에러 발생");
		}
	}
}
