Texture2D DiffuseTexture : register(t1);
SamplerState DiffuseSampler : register(s0);

cbuffer constants : register(b0)
{
	row_major float4x4 world;
}
cbuffer WorldMatrixArray : register(b5)
{
	row_major float4x4 WorldMatrixArray[1000];
};

cbuffer ModelIdxCBuffer : register(b6)
{
	uint ModelIdx;
	uint3 Padding6;
};
cbuffer PerFrame : register(b1)
{
	row_major float4x4 ViewMatrix;
	row_major float4x4 ProjectionMatrix;
	row_major float4x4 ViewProj;
	uint ViewModeIndex;
	float CamNear;
	float CamFar;
	float Padding;
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

	float4 AmbientColor; // 16바이트
	float4 DiffuseColor; // 16바이트
	float4 SpecularColor; // 16바이트
	float SpecularExponent; // 4바이트
	// --- 패딩 ---
	float3 PADDING;

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

	//if (UseInstancing != 0 && InstanceId < InstanceCount)
	//{
	//	InstanceData Instance = InstanceMatrices[BaseInstanceOffset + InstanceId];
	//	Position = mul(Position, Instance.World);
	//	ShadeColor = lerp(ShadeColor, Instance.Color, Instance.Color.a);
	//}

	Position = mul(Position, WorldMatrixArray[ModelIdx]);
	Position = mul(Position, ViewProj);

	Output.Position = Position;
	Output.Color = ShadeColor;
	Output.Tex = Input.Tex; // pass through UVs to PS
	return Output;
}

float4 MainPS(PS_INPUT Input) : SV_TARGET
{
	return DiffuseTexture.Sample(DiffuseSampler, Input.Tex);
  //  if (UseTexture != 0)
  //  {
		//// UV 스크롤 적용
		//float2 scrolledUV = Input.Tex + UVScrollSpeed * Time;
		//scrolledUV = frac(scrolledUV);

  //      // 텍스처 샘플링 결과만 반환
  //      return DiffuseTexture.Sample(DiffuseSampler, scrolledUV);
  //  }
    
  //  // 텍스처를 사용하지 않을 경우 DiffuseColor의 RGB를 사용하되, 알파는 1.0으로 고정
  //  // (오브젝트 뷰어에서 UI 블렌딩 시 투명해지는 문제 방지)
  //  // 텍스처 미사용 시 Kd가 거의 검정이면 정점색 사용
  //  float3 kd = DiffuseColor.rgb;
  //  bool useVertexColor = all(kd < float3(0.001, 0.001, 0.001));
  //  return float4(useVertexColor ? Input.Color.rgb : kd, 1.0f);
}
