#include "pch.h"
#include "RootSignature.h"
#include "Engine.h"

void RootSignature::Init()
{
	CreateGraphicsRootSignature();
	CreateComputeRootSignature();
}

void RootSignature::CreateGraphicsRootSignature()
{
	_samplerDesc = CD3DX12_STATIC_SAMPLER_DESC(0); //SAMPLER_DESC 초기화. s0을 사용하겠다. SamplerState g_sam_0 : register(s0);
	
	CD3DX12_DESCRIPTOR_RANGE ranges[] = //리소스를 어떠한 용도로 사용할 건지에 대한 정의
	{
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT - 1, 1), // ContantBufferView, b1~b4를 사용하겠다.
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0), // ShaderResourceView t0~t9를 사용하겠다.
	};

	CD3DX12_ROOT_PARAMETER param[2]; //1개는 전역으로 CBV사용하고, 1개의 디스크립터 테이블
	//param[0].InitAsConstantBufferView(0); // api 바인드 슬롯 0번 슬롯 -> b0 레지스터-> rootCBV 
	//param[1].InitAsConstantBufferView(1); // api 바인드 슬롯 1번 슬롯 -> b1 레지스터-> rootCBV //디스크립터 테이블을 사용하지 않았을 때.
	param[0].InitAsConstantBufferView(static_cast<uint32>(CBV_REGISTER::b0)); //b0를 전역으로 사용할 것임.
	param[1].InitAsDescriptorTable(_countof(ranges), ranges); //_countof : ranges 배열안에 요소 갯수

	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, 1, &_samplerDesc); //D3D12_DEFAULT이게 들어가면 완전 기본상태
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; //입력 조립기(IA) 단계를 사용하도록 허용. 최적화가 미미하지만 단계를 생략할 수 있다.

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_graphicsRootSignature));
}

void RootSignature::CreateComputeRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE ranges[] =
	{
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT, 0), // b0~b4
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0), // t0~t9
		CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UAV_REGISTER_COUNT, 0), // u0~u4
	};

	CD3DX12_ROOT_PARAMETER param[1];
	param[0].InitAsDescriptorTable(_countof(ranges), ranges);

	D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param);
	sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; //IA단계없음(파이프라인따위 없음)

	ComPtr<ID3DBlob> blobSignature;
	ComPtr<ID3DBlob> blobError;
	::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
	DEVICE->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(), IID_PPV_ARGS(&_computeRootSignature));

	COMPUTE_CMD_LIST->SetComputeRootSignature(_computeRootSignature.Get());
}