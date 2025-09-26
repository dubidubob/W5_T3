#pragma once

#include "Global/Types.h"
#include "Global/Vector.h"
#include "Global/Matrix.h"
#include "Global/CoreTypes.h"

struct FAABB;

void ExtractFrustumPlanes(const FMatrix& View, const FMatrix& Projection, TStaticArray<FVector4, 6>& OutPlanes);
void ExtractFrustumPlanes(const FViewProjConstants& VP, TStaticArray<FVector4, 6>& OutPlanes);
bool TestAABBFrustum(const FAABB& Box, const TStaticArray<FVector4, 6>& Planes);

