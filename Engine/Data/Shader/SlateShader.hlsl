cbuffer SlateRectBuffer : register(b0)
{
	row_major float4x4 NDCMatrix;
}

cbuffer SlateColorBuffer : register(b2)
{
	float4 SlateColor;
}

struct VS_INPUT
{
	float3 Position : POSITION; // -0.5 ~ 0.5 Square
	float4 Color : COLOR;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

// 버텍스 셰이더
PS_INPUT VS_Slate(VS_INPUT input)
{
	PS_INPUT output;
    
	float4 worldPos = mul(float4(input.Position, 1.0f), NDCMatrix);
	output.Position = float4(worldPos.xy, 0.0f, 1.0f);
	output.Color = SlateColor;
	return output;
}

float4 PS_Slate(PS_INPUT input) : SV_Target
{
	return input.Color;
}
