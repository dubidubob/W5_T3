#pragma once

struct FPipelineInfo
{
	ID3D11InputLayout* InputLayout;
	ID3D11VertexShader* VertexShader;
	ID3D11RasterizerState* RasterizerState;
	ID3D11DepthStencilState* DepthStencilState;
	ID3D11PixelShader* PixelShader;
	ID3D11BlendState* BlendState;
	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

class UPipeline
{
public:
	UPipeline(ID3D11DeviceContext* InDeviceContext);
	~UPipeline();

	void UpdatePipeline(FPipelineInfo Info);

	void SetVertexBuffer(ID3D11Buffer* VertexBuffer, uint32 Stride);

	void SetInstanceBuffer(ID3D11Buffer* InstanceBuffer, uint32 Stride);

	void SetConstantBuffer(uint32 Slot, bool bIsVS, ID3D11Buffer* ConstantBuffer);

	void SetShaderResourceView(uint32 Slot, bool bIsVS, ID3D11ShaderResourceView* ShaderResourceView);

	void SetSamplerState(uint32 Slot, bool bIsVS, ID3D11SamplerState* SamplerState);

	void Draw(uint32 VertexCount, uint32 StartLocation);

	void DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 VertexStartLocation, uint32 InstanceStartLocation);

	void SetIndexBuffer(ID3D11Buffer* IndexBuffer, DXGI_FORMAT Format);

	void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int32 BaseVertexLocation);
	void DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation);
private:
	ID3D11DeviceContext* DeviceContext;
};
