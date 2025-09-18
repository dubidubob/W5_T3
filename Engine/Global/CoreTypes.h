#pragma once
#include "Global/Vector.h"
#include "Global/Matrix.h"
#include <cstdint>

struct FViewProjConstants
{
	FViewProjConstants()
	{
		View = FMatrix::Identity;
		Projection = FMatrix::Identity;
		ViewModeIndex = 0;
	}

	FMatrix View;
	FMatrix Projection;
	uint32 ViewModeIndex;
};


struct FVertex
{
    FVector Position;
    FVector4 Color;

    // 동등 비교 연산자
    bool operator==(const FVertex& Other) const
    {
        return Position == Other.Position && Color == Other.Color;
    }
};

namespace std
{
	template<>
	struct hash<FVertex>
	{
		size_t operator()(const FVertex& V) const noexcept
		{
			auto HashCombine = [](size_t Seed, size_t Value) -> size_t
			{
				Seed ^= Value + 0x9e3779b97f4a7c15ULL + (Seed << 6) + (Seed >> 2);
				return Seed;
			};

			size_t H = 0;
			H = HashCombine(H, std::hash<float>{}(V.Position.X));
			H = HashCombine(H, std::hash<float>{}(V.Position.Y));
			H = HashCombine(H, std::hash<float>{}(V.Position.Z));
			H = HashCombine(H, std::hash<float>{}(V.Color.X));
			H = HashCombine(H, std::hash<float>{}(V.Color.Y));
			H = HashCombine(H, std::hash<float>{}(V.Color.Z));
			H = HashCombine(H, std::hash<float>{}(V.Color.W));
			return H;
		}
	};
}

struct FTextVertex
{
	FVector Position;
	float BaseUvX;
	float BaseUvY;
};

struct FTextInstance
{
	FVector4 Color;
	FVector Offset;
	uint32 CharId;
};

struct FCharacterInfo
{
	float U;
	float V;
	float Width;
	float Height;
};

//TMap<char, FCharacterInfo> CharInfoMap;

struct FRay
{
	FVector4 Origin;
	FVector4 Direction;
};

/**
 * @brief Component Type Enum
 */
enum class EComponentType : uint8_t
{
	None = 0,

	Actor,
		//ActorComponent Dervied Type

	Scene,
		//SceneComponent Dervied Type

	Primitive,

	Text,
		//PrimitiveComponent Derived Type

	End = 0xFF
};

/**
 * @brief UObject Primitive Type Enum
 */
enum class EPrimitiveType : uint8_t
{
	None = 0,
	Sphere,
	Cube,
	Triangle,
	Square,
	Torus,
	Arrow,
	CubeArrow,
	Ring,
	Line,

	End = 0xFF
};

/**
 * @brief RasterizerState Enum
 */
enum class ECullMode : uint8_t
{
	Back,
	Front,
	None,

	End = 0xFF
};

enum class EFillMode : uint8_t
{
	WireFrame,
	Solid,

	End = 0xFF
};

/**
 * @brief View Mode State
 */
enum class EViewModeIndex : uint32
{
	Lit,
	Unlit,
	Wireframe,

	End
};

enum class ESamplerType
{
	Text,
};

/**
 * @brief Engine Show Flags for controlling visibility of various elements
 */
enum class EEngineShowFlags : uint64
{
	SF_None = 0,
	SF_Primitives = 1 << 0,      // Show primitive objects
	SF_BillboardText = 1 << 1,   // Show billboard text
	SF_Grid = 1 << 2,            // Show grid
	SF_Bounds = 1 << 3,          // Show bounding boxes

	// Default flags (everything visible)
	SF_Default = SF_Primitives | SF_BillboardText | SF_Grid
};

// Bitwise operators for EEngineShowFlags
inline EEngineShowFlags operator|(EEngineShowFlags a, EEngineShowFlags b)
{
	return static_cast<EEngineShowFlags>(static_cast<uint64>(a) | static_cast<uint64>(b));
}

inline EEngineShowFlags operator&(EEngineShowFlags a, EEngineShowFlags b)
{
	return static_cast<EEngineShowFlags>(static_cast<uint64>(a) & static_cast<uint64>(b));
}

inline EEngineShowFlags operator^(EEngineShowFlags a, EEngineShowFlags b)
{
	return static_cast<EEngineShowFlags>(static_cast<uint64>(a) ^ static_cast<uint64>(b));
}

inline EEngineShowFlags operator~(EEngineShowFlags a)
{
	return static_cast<EEngineShowFlags>(~static_cast<uint64>(a));
}

inline EEngineShowFlags& operator|=(EEngineShowFlags& a, EEngineShowFlags b)
{
	return a = a | b;
}

inline EEngineShowFlags& operator&=(EEngineShowFlags& a, EEngineShowFlags b)
{
	return a = a & b;
}

inline EEngineShowFlags& operator^=(EEngineShowFlags& a, EEngineShowFlags b)
{
	return a = a ^ b;
}

inline bool HasFlag(EEngineShowFlags flags, EEngineShowFlags flag)
{
	return (flags & flag) == flag;
}

/**
 * @brief Render State Settings for Actor's Component
 */
struct FRenderState
{
	ECullMode CullMode = ECullMode::None;
	EFillMode FillMode = EFillMode::Solid;
};
