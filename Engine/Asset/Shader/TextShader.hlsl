cbuffer PerFrame : register(b0)
{
	row_major float4x4 ModelMatrix;
}

cbuffer PerFrame : register(b1)
{
	row_major float4x4 ViewMatrix; // View Matrix Calculation of MVP Matrix
	row_major float4x4 ProjectionMatrix; // Projection Matrix Calculation of MVP Matrix
};

struct CharUv
{
	float2 UvOffset;
	float2 UvSize;
};

cbuffer CharTable : register(b4)
{
	CharUv UvTable[95];
}

Texture2D FontAtlas : register(t0);

SamplerState Sampler : register(s0);


struct VS_INPUT
{
	float3 Position : POSITION;
	float2 UV : TEXCOORD0;
	
};

struct PS_INPUT
{
	float4 WorldPos : SV_Position;
	float2 UV : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT Input)
{
	PS_INPUT Output;

	float4 Position = float4(Input.Position, 1.0f);
	Output.WorldPos = Position;

	Output.UV = Input.UV;
	return Output;
}

float4 mainPS(PS_INPUT Input) : SV_Target
{
	float4 TextureColor = FontAtlas.Sample(Sampler, Input.UV);

	return TextureColor;
}
