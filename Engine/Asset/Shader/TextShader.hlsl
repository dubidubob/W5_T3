cbuffer Model : register(b0)
{
	row_major float4x4 ModelMatrix;
}

cbuffer PerFrame : register(b1)
{
	row_major float4x4 ViewMatrix; // View Matrix Calculation of MVP Matrix
	row_major float4x4 ProjectionMatrix; // Projection Matrix Calculation of MVP Matrix
	uint ViewModeIndex; // View Mode (0: Lit, 1: Unlit, 2: WireFrame)
	float3 Padding;
};

struct CharUv
{
	float2 UvOffset;
	float2 UvSize;
};

cbuffer CharTable : register(b4)
{
	CharUv UvTable[2446];
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

float3 GetCameraForward();

PS_INPUT mainVS(VS_INPUT Input)
{
	PS_INPUT Output;

	float FontScale = 1 / 3.0f;
	//가로 32픽셀 세로 64픽셀이므로 Y를 2배 스케일(zxy->xyz)
	float3 BasePos = float3(Input.Position.x, Input.Position.y, Input.Position.z) * FontScale;
	Input.Offset.y *= FontScale;
	
	float3 ModelPos = ModelMatrix[3].xyz;
	float3 CameraForward = GetCameraForward();
	
	float3 Forward = CameraForward;
	float3 Right = normalize(cross(float3(0, 0, 1), Forward));
	float3 Up = normalize(cross(Forward, Right));

	float3 FontOffset = Input.Offset.y * Right + Input.Offset.z * Up;

	BasePos = BasePos.y * Right + BasePos.z * Up;
	float4 OutputPos = float4(ModelPos, 1) + float4(FontOffset, 0) + float4(BasePos, 0);

	OutputPos = mul(OutputPos, ViewMatrix);
	
	OutputPos = mul(OutputPos, ProjectionMatrix);
	Output.WorldPos = OutputPos;
	

	Output.UV = UvTable[Input.CharID].UvSize * Input.UV + UvTable[Input.CharID].UvOffset;
	Output.Color = Input.Color;
	return Output;
}

float4 mainPS(PS_INPUT Input) : SV_Target
{
	if (ViewModeIndex == 2) { return float4(1, 1, 1, 1); }
	float4 TextureColor = FontAtlas.Sample(Sampler, Input.UV);
	return TextureColor;
}

float3 GetCameraForward()
{
	float3 Result;

	float3x3 RotationMatrixInverse = transpose(float3x3(ViewMatrix[0].xyz, ViewMatrix[1].xyz, ViewMatrix[2].xyz));

	
	return RotationMatrixInverse[2].xyz;
}
