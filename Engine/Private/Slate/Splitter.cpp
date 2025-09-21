#include "pch.h"
#include "Slate/Splitter.h"
#include "Manager/Path/PathManager.h"

bool SSplitter::CanRender(FRect& OutRect, FVector4& OutColor, const FVector2& MouseCoord) const
{
	OutRect = GetRect();
	OutColor = FVector4(0.1f, 0.1f, 0.1f, 1.0f);
	if (IsHover(MouseCoord) || bIsDragging) { OutColor *= 4; }
	return true;
}

void SSplitter::Drag(FVector2 MouseCoord)
{
	bIsDragging = true;
	if (!GetLabel().empty()) { SaveDragInfo(MouseCoord); }
}

void SSplitter::SaveDragInfo(const FVector2& MouseCoord)
{
	const path ConfigFilePath = UPathManager::GetInstance().GetEditorIniPath();
	WritePrivateProfileStringA(
		"Splitter",
		(SplitterLabel + "X").c_str(),
		std::to_string(MouseCoord.X).c_str(),
		ConfigFilePath.string().c_str()
	);
	WritePrivateProfileStringA(
		"Splitter",
		(SplitterLabel + "Y").c_str(),
		std::to_string(MouseCoord.Y).c_str(),
		ConfigFilePath.string().c_str()
	);
}

bool SSplitter::TryLoadDragInfo(FVector2& OutMouseCoord)
{
	const path ConfigFilePath = UPathManager::GetInstance().GetEditorIniPath();
	char X_buffer[1024];
	char Y_buffer[1024];

	const char* DefaultValue = "";

	DWORD X_Length = GetPrivateProfileStringA(
		"Splitter",
		(SplitterLabel + "X").c_str(),
		DefaultValue,
		X_buffer,
		sizeof(X_buffer),
		ConfigFilePath.string().c_str()
	);

	DWORD Y_Length = GetPrivateProfileStringA(
		"Splitter",
		(SplitterLabel + "Y").c_str(),
		DefaultValue,
		Y_buffer,
		sizeof(Y_buffer),
		ConfigFilePath.string().c_str()
	);

	if (X_Length > 0 && Y_Length > 0)
	{
		OutMouseCoord.X = std::stof(X_buffer);
		OutMouseCoord.Y = std::stof(Y_buffer);
		return true;
	}

	return false;

}
