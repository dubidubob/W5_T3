// ColorPicking Debug Shader - Visualizes picking IDs as colors

Texture2D<uint> ColorPickingTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

// Vertex Shader - Simple fullscreen quad
PS_INPUT VS_ColorPickingDebug(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = float4(input.Position, 1.0f);
    output.TexCoord = input.TexCoord;
    return output;
}

// Convert object ID to a visible color
float3 IDToColor(uint objectID)
{
    if (objectID == 0)
    {
        return float3(0.0f, 0.0f, 0.0f); // Black for background/no object
    }

    // Use simple hash function to generate pseudo-random colors
    uint hash = objectID;
    hash = hash * 2654435761u; // Knuth's multiplicative hash

    float r = ((hash >> 16) & 0xFF) / 255.0f;
    float g = ((hash >> 8) & 0xFF) / 255.0f;
    float b = (hash & 0xFF) / 255.0f;

    // Ensure minimum brightness for visibility
    float3 color = float3(r, g, b);
    float luminance = dot(color, float3(0.299f, 0.587f, 0.114f));
    if (luminance < 0.3f)
    {
        color = normalize(color) * 0.5f; // Make darker colors more visible
    }

    return color;
}

// Pixel Shader - Sample picking texture and convert ID to color
float4 PS_ColorPickingDebug(PS_INPUT input) : SV_Target
{
    // Get texture dimensions
    uint width, height;
    ColorPickingTexture.GetDimensions(width, height);

    // Sample the picking texture at current pixel
    int2 texCoord = int2(input.TexCoord * float2(width, height));
    uint objectID = ColorPickingTexture.Load(int3(texCoord, 0));

    // Convert ID to visible color
    float3 color = IDToColor(objectID);

    return float4(color, 0.7f); // Use some transparency to overlay on scene
}