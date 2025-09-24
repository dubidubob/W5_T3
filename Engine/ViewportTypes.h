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

	End
};

enum class EViewportRenderMode : uint8_t
{
	Lit = 0,
	Unlit,
	Wireframe,

	End
};

namespace ViewportUI
{
	inline constexpr std::array<const char*, static_cast<size_t>(EViewportViewType::End)> ViewTypeLabels = {
		"Perspective", "Front", "Back", "Top", "Bottom", "Left", "Right"
	};

	inline constexpr std::array<const char*, static_cast<size_t>(EViewportRenderMode::End)> RenderModeLabels = {
		"Lit", "Unlit", "Wireframe"
	};
}
