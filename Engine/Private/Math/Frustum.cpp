#include "pch.h"
#include "Math/Frustum.h"
#include "Math/AABB.h"

static inline void NormalizePlane(FVector4& P)
{
    float L = std::sqrtf(P.X * P.X + P.Y * P.Y + P.Z * P.Z);
    if (L > 0.0f)
    {
        float R = 1.0f / L;
        P.X *= R; P.Y *= R; P.Z *= R; P.W *= R;
    }
}

void ExtractFrustumPlanes(const FMatrix& View, const FMatrix& Projection, TStaticArray<FVector4, 6>& OutPlanes)
{
    FMatrix VP = View * Projection;

    FVector4 C0(VP.Data[0][0], VP.Data[1][0], VP.Data[2][0], VP.Data[3][0]);
    FVector4 C1(VP.Data[0][1], VP.Data[1][1], VP.Data[2][1], VP.Data[3][1]);
    FVector4 C2(VP.Data[0][2], VP.Data[1][2], VP.Data[2][2], VP.Data[3][2]);
    FVector4 C3(VP.Data[0][3], VP.Data[1][3], VP.Data[2][3], VP.Data[3][3]);

    OutPlanes[0] = C3 + C0;
    OutPlanes[1] = C3 - C0;
    OutPlanes[2] = C3 + C1;
    OutPlanes[3] = C3 - C1;
    OutPlanes[4] = C2;
    OutPlanes[5] = C3 - C2;

    for (auto& P : OutPlanes) NormalizePlane(P);
}

void ExtractFrustumPlanes(const FViewProjConstants& VP, TStaticArray<FVector4, 6>& OutPlanes)
{
    ExtractFrustumPlanes(VP.View, VP.Projection, OutPlanes);
}

bool TestAABBFrustum(const FAABB& Box, const TStaticArray<FVector4, 6>& Planes)
{
    if (!Box.IsValid()) return false;
    for (const auto& P : Planes)
    {
        float x = (P.X >= 0.0f) ? Box.Min.X : Box.Max.X;
        float y = (P.Y >= 0.0f) ? Box.Min.Y : Box.Max.Y;
        float z = (P.Z >= 0.0f) ? Box.Min.Z : Box.Max.Z;
        float d = P.X * x + P.Y * y + P.Z * z + P.W;
        if (d < 0.0f) return false;
    }
    return true;
}

