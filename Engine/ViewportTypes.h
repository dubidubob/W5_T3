#pragma once
#include <array>

// ViewportUI array Must Sync with Enum Classes
enum class EViewportViewType : uint8_t
{
	Perspective = 0,
	Front,
	Back,
	Top,
	Bottom,
	Left,
	Right,
	Count
};

enum class EViewportRenderMode : uint8_t
{
	Lit = 0,
	Unlit,
	Wireframe,
	Count
};

namespace ViewportUI
{
	inline constexpr std::array<const char*, static_cast<size_t>(EViewportViewType::Count)> ViewTypeLabels = {
		"Perspective", "Front", "Back", "Top", "Bottom", "Left", "Right"
	};

	inline constexpr std::array<const char*, static_cast<size_t>(EViewportRenderMode::Count)> RenderModeLabels = {
		"Lit", "Unlit", "Wireframe"
	};
}
