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

	//InstanceData
	float4 Color : COLOR;
	float3 Offset : OFFSET;
	uint CharID : TEXCOORD1;
};

struct PS_INPUT
{
	float4 WorldPos : SV_Position;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT Input)
{
	PS_INPUT Output;

	
	float3 Pos = float3(Input.Position.x / 2, Input.Position.y, Input.Position.z);
	Pos = Pos + Input.Offset;
	float4 OutputPos = mul(float4(Pos, 1.0f), ModelMatrix);
	OutputPos = mul(OutputPos, ViewMatrix);
	OutputPos = mul(OutputPos, ProjectionMatrix);
	Output.WorldPos = OutputPos;
	

	Output.UV = UvTable[Input.CharID - 32].UvSize * Input.UV + UvTable[Input.CharID - 32].UvOffset;
	Output.Color = Input.Color;
	return Output;
}

float4 mainPS(PS_INPUT Input) : SV_Target
{
	float4 TextureColor = FontAtlas.Sample(Sampler, Input.UV);

	return TextureColor;
}
