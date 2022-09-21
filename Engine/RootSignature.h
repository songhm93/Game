#pragma once

// [ 계약서 / 결재 ] 로 비유
// CPU	[          ] -> 외주 GPU		[          ]
// 한국	[          ] -> 외주 베트남	[          ]
// 어떤 정책을 활용할건지 

class RootSignature
{
public:
	void Init();
	
	ComPtr<ID3D12RootSignature> GetGraphicsRootSignature() { return _graphicsRootSignature; }
	ComPtr<ID3D12RootSignature> GetComputeRootSignature() { return _computeRootSignature; }

private:
	void CreateGraphicsRootSignature();
	void CreateComputeRootSignature();

private:
	ComPtr<ID3D12RootSignature>	_graphicsRootSignature;
	D3D12_STATIC_SAMPLER_DESC _samplerDesc;

	ComPtr<ID3D12RootSignature>	_computeRootSignature;


};

