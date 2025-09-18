#include "pch.h"
#include "Render/Renderer/Pipeline.h"

/// @brief 그래픽 파이프라인을 관리하는 클래스
UPipeline::UPipeline(ID3D11DeviceContext* InDeviceContext)
	: DeviceContext(InDeviceContext)
{
}

UPipeline::~UPipeline()
{
	/** Device Context는 Device Resource에서 제거 */
}


/// @brief 파이프라인 상태를 업데이트
void UPipeline::UpdatePipeline(FPipelineInfo Info)
{
	DeviceContext->IASetPrimitiveTopology(Info.Topology);
	if (Info.InputLayout)
		DeviceContext->IASetInputLayout(Info.InputLayout);
	if (Info.VertexShader)
		DeviceContext->VSSetShader(Info.VertexShader, nullptr, 0);
	if (Info.RasterizerState)
		DeviceContext->RSSetState(Info.RasterizerState);
	if (Info.DepthStencilState)
		DeviceContext->OMSetDepthStencilState(Info.DepthStencilState, 0);
	if (Info.PixelShader)
		DeviceContext->PSSetShader(Info.PixelShader, nullptr, 0);
	if (Info.BlendState)
		DeviceContext->OMSetBlendState(Info.BlendState, nullptr, 0xffffffff);
}

/// @brief 정점 버퍼를 바인딩
void UPipeline::SetVertexBuffer(ID3D11Buffer* VertexBuffer, uint32 Stride)
{
	uint32 Offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
}

void UPipeline::SetInstanceBuffer(ID3D11Buffer* InstanceBuffer, uint32 Stride)
{
	uint32 Offset = 0;
	DeviceContext->IASetVertexBuffers(1, 1, &InstanceBuffer, &Stride, &Offset);
}

/// @brief 상수 버퍼를 설정
void UPipeline::SetConstantBuffer(uint32 Slot, bool bIsVS, ID3D11Buffer* ConstantBuffer)
{
	if (ConstantBuffer)
	{
		if (bIsVS)
			DeviceContext->VSSetConstantBuffers(Slot, 1, &ConstantBuffer);
		else
			DeviceContext->PSSetConstantBuffers(Slot, 1, &ConstantBuffer);
	}
}

/// @brief 셰이더 리소스 뷰 설정
void UPipeline::SetShaderResourceView(uint32 Slot, bool bIsVS, ID3D11ShaderResourceView* ShaderResourceView)
{
		if (bIsVS)
			DeviceContext->VSSetShaderResources(Slot, 1, &ShaderResourceView);
		else
			DeviceContext->PSSetShaderResources(Slot, 1, &ShaderResourceView);
}

/// @brief 샘플러 상태를 설정
void UPipeline::SetSamplerState(uint32 Slot, bool bIsVS, ID3D11SamplerState* SamplerState)
{
		if (bIsVS)
			DeviceContext->VSSetSamplers(Slot, 1, &SamplerState);
		else
			DeviceContext->PSSetSamplers(Slot, 1, &SamplerState);
}

/// @brief 정점 개수를 기반으로 드로우 호출
void UPipeline::Draw(uint32 VertexCount, uint32 StartLocation)
{
	DeviceContext->Draw(VertexCount, StartLocation);
}

void UPipeline::DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 VertexStartLocation, uint32 InstanceStartLocation)
{
	DeviceContext->DrawInstanced(VertexCountPerInstance, InstanceCount, VertexStartLocation, InstanceStartLocation);
}

/// @brief 인덱스 버퍼를 설정
void UPipeline::SetIndexBuffer(ID3D11Buffer* IndexBuffer, DXGI_FORMAT Format)
{
	DeviceContext->IASetIndexBuffer(IndexBuffer, Format, 0);
}

/// @brief 인덱스 버퍼를 사용한 드로우 호출
void UPipeline::DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int32 BaseVertexLocation)
{
	DeviceContext->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
}

void UPipeline::DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation)
{
	DeviceContext->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}
