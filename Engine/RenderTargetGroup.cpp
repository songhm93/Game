#include "pch.h"
#include "RenderTargetGroup.h"
#include "Engine.h"
#include "Device.h"


void RenderTargetGroup::Create(RENDER_TARGET_GROUP_TYPE groupType, vector<RenderTarget>& rtVec, shared_ptr<Texture> dsTexture)
{
	_groupType = groupType;
	_rtVec = rtVec;
	_rtCount = static_cast<uint32>(rtVec.size()); //렌더타겟 갯수
	_dsTexture = dsTexture; //뎁스스텐실텍스처는 어느 그룹에나 들어감

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {}; //데스크립터힙을 vector에 들어있는 렌더타겟들을 한번에 대상으로 잡아서 만듬.
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NumDescriptors = _rtCount;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 0;

	DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeap));

	_rtvHeapSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_rtvHeapBegin = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	_dsvHeapBegin = _dsTexture->GetDSV()->GetCPUDescriptorHandleForHeapStart();

	for (uint32 i = 0; i < _rtCount; i++)
	{
		uint32 destSize = 1;
		D3D12_CPU_DESCRIPTOR_HANDLE destHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_rtvHeapBegin, i * _rtvHeapSize);

		uint32 srcSize = 1;
		ComPtr<ID3D12DescriptorHeap> srcRtvHeapBegin = _rtVec[i].target->GetRTV(); 
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = srcRtvHeapBegin->GetCPUDescriptorHandleForHeapStart();

		DEVICE->CopyDescriptors(1, &destHandle, &destSize, 1, &srcHandle, &srcSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	for (int i = 0; i < _rtCount; ++i)
	{
		_targetToResource[i] = CD3DX12_RESOURCE_BARRIER::Transition(
			_rtVec[i].target->GetTex2D().Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_COMMON);

		_resourceToTarget[i] = CD3DX12_RESOURCE_BARRIER::Transition(
			_rtVec[i].target->GetTex2D().Get(),
			D3D12_RESOURCE_STATE_COMMON, 
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void RenderTargetGroup::OMSetRenderTargets(uint32 count, uint32 offset) //백버퍼 1개 설정
{
	D3D12_VIEWPORT vp = D3D12_VIEWPORT{ 0.f, 0.f, _rtVec[0].target->GetWidth() , _rtVec[0].target->GetHeight(), 0.f, 1.f };
	D3D12_RECT rect = D3D12_RECT{ 0, 0, static_cast<LONG>(_rtVec[0].target->GetWidth()),  static_cast<LONG>(_rtVec[0].target->GetHeight()) };
	//렌더타겟 그룹마다 자기가 그려야할 크기에 따라 세팅
	GRAPHICS_CMD_LIST->RSSetViewports(1, &vp);
	GRAPHICS_CMD_LIST->RSSetScissorRects(1, &rect);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_rtvHeapBegin, offset * _rtvHeapSize);
	GRAPHICS_CMD_LIST->OMSetRenderTargets(count, &rtvHandle, FALSE/*1개*/, &_dsvHeapBegin); //dsv는 두개 셰이더에서 둘다 넘겨줌
}

void RenderTargetGroup::OMSetRenderTargets() //렌더타겟 POSITION, NORMAL, COLOR 3개를 설정
{
	D3D12_VIEWPORT vp = D3D12_VIEWPORT{ 0.f, 0.f, _rtVec[0].target->GetWidth() , _rtVec[0].target->GetHeight(), 0.f, 1.f };
	D3D12_RECT rect = D3D12_RECT{ 0, 0, static_cast<LONG>(_rtVec[0].target->GetWidth()),  static_cast<LONG>(_rtVec[0].target->GetHeight()) };

	GRAPHICS_CMD_LIST->RSSetViewports(1, &vp);
	GRAPHICS_CMD_LIST->RSSetScissorRects(1, &rect);

	//렌더타겟 및 깊이스텐실에 대한 cpu 디스크립터핸들을 설정한다. TRUE이면은 디스크립터의 연속 범위에 대한 포인터임을 의미. 바인딩할 디스크립터 세트가 메모리에서 연속적일 때.
	GRAPHICS_CMD_LIST->OMSetRenderTargets(_rtCount, &_rtvHeapBegin, TRUE/*다중*/, &_dsvHeapBegin); //dsv는 두개 셰이더에서 둘다 넘겨줌
}

void RenderTargetGroup::ClearRenderTargetView(uint32 index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_rtvHeapBegin, index * _rtvHeapSize);
	GRAPHICS_CMD_LIST->ClearRenderTargetView(rtvHandle, _rtVec[index].clearColor, 0, nullptr);

	GRAPHICS_CMD_LIST->ClearDepthStencilView(_dsvHeapBegin, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void RenderTargetGroup::ClearRenderTargetView()
{
	WaitResourceToTarget(); //배리어 처리
	
	for (uint32 i = 0; i < _rtCount; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(_rtvHeapBegin, i * _rtvHeapSize);
		GRAPHICS_CMD_LIST->ClearRenderTargetView(rtvHandle, _rtVec[i].clearColor, 0, nullptr);
	}

	GRAPHICS_CMD_LIST->ClearDepthStencilView(_dsvHeapBegin, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void RenderTargetGroup::WaitTargetToResource()
{
	GRAPHICS_CMD_LIST->ResourceBarrier(_rtCount, _targetToResource);
}

void RenderTargetGroup::WaitResourceToTarget()
{
	GRAPHICS_CMD_LIST->ResourceBarrier(_rtCount, _resourceToTarget);
}