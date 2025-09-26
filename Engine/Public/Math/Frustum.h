#pragma once

#include "Global/Types.h"
#include "Global/Vector.h"
#include "Global/Matrix.h"
#include "Global/CoreTypes.h"

struct FAABB;
struct FAABB_SIMD_Chunk;

void ExtractFrustumPlanes(const FMatrix& View, const FMatrix& Projection, TStaticArray<FVector4, 6>& OutPlanes);
void ExtractFrustumPlanes(const FViewProjConstants& VP, TStaticArray<FVector4, 6>& OutPlanes);
bool TestAABBFrustum(const FAABB& Box, const TStaticArray<FVector4, 6>& Planes);
bool TestAABBFrustum(const FAABB& Box, const TStaticArray<FVector4, 6>& Planes);
__m128 TestAABBFrustum_Chunk_SIMD(const FAABB_SIMD_Chunk& Chunk, const TStaticArray<FVector4, 6>& Planes);
