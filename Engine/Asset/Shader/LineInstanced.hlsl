cbuffer PerFrame : register(b1)
{
    row_major float4x4 View;
    row_major float4x4 Projection;
    uint ViewModeIndex; // padding follows implicitly
    float3 _Pad0;
}

StructuredBuffer<float4x4> gWorlds : register(t0);

struct VS_INPUT
{
    float3 Position : POSITION;        // slot 0, per-vertex
    float4 InstColor : COLOR;          // slot 1, per-instance
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float4 Color : COLOR;
};

PS_INPUT mainVS(VS_INPUT Input, uint InstanceID : SV_InstanceID)
{
    PS_INPUT Output;
    float4 pos = float4(Input.Position, 1.0f);
    float4x4 World = gWorlds[InstanceID];
	World = transpose(World);

    pos = mul(pos, World);
    pos = mul(pos, View);
    pos = mul(pos, Projection);

    Output.Position = pos;
    Output.Color = Input.InstColor;
    return Output;
}

float4 mainPS(PS_INPUT Input) : SV_Target
{
    return Input.Color;
}

