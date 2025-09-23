Texture2D DiffuseTexture : register(t1);
SamplerState DiffuseSampler : register(s0);

cbuffer constants : register(b0)
{
	row_major float4x4 world;
}

cbuffer PerFrame : register(b1)
{
	row_major float4x4 ViewMatrix;
	row_major float4x4 ProjectionMatrix;
	uint ViewModeIndex;
	float3 Padding;
};

cbuffer PerDrawColor : register(b2)
{
	float4 totalColor;
};

cbuffer InstanceParams : register(b3)
{
	uint UseInstancing;
	uint BaseInstanceOffset;
	uint InstanceCount;
	uint Padding0;
};

cbuffer MaterialParams : register(b4)
{
	uint UseTexture; // 4바이트
	float2 UVScrollSpeed; // 8바이트
	float Time; // 4바이트
};

struct InstanceData
{
	row_major float4x4 World;
	float4 Color;
};

StructuredBuffer<InstanceData> InstanceMatrices : register(t0);

struct VS_INPUT
{
	float3 Position : POSITION; // matches R32G32B32 in input layout
	float3 Normal : NORMAL;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

PS_INPUT MainVS(VS_INPUT Input, uint InstanceId : SV_InstanceID)
{
	PS_INPUT Output;

	float4 Position = float4(Input.Position, 1.0f);
	float4 ShadeColor = Input.Color;

	if (UseInstancing != 0 && InstanceId < InstanceCount)
	{
		InstanceData Instance = InstanceMatrices[BaseInstanceOffset + InstanceId];
		Position = mul(Position, Instance.World);
		ShadeColor = lerp(ShadeColor, Instance.Color, Instance.Color.a);
	}

	Position = mul(Position, world);
	Position = mul(Position, ViewMatrix);
	Position = mul(Position, ProjectionMatrix);

	Output.Position = Position;
	Output.Color = ShadeColor;
	Output.Tex = Input.Tex; // pass through UVs to PS
	return Output;
}

float4 MainPS(PS_INPUT Input) : SV_TARGET
{
	if (UseTexture != 0)
	{
       // UV 스크롤 적용
		float2 scrolledUV = Input.Tex + UVScrollSpeed * Time;

        // 반복되도록 0~1 범위로 잘라주기
		scrolledUV = frac(scrolledUV);

		return DiffuseTexture.Sample(DiffuseSampler, scrolledUV);
	}
        // 정점 색상(또는 totalColor)만 사용
	return Input.Color;
}
