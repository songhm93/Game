#pragma once

class SwapChain;

	//CommandQueue : DX12에 등장
	//외주를 요청할 때, 하나씩 요청하면 비효율적
	//[외주 목록]에 일감을 차곡차곡 기록했다가 한 방에 요청하는 것. 커맨드 패턴과 유사.
//////////////////////////////////////
//GraphicsCommandQueue
/////////////////////////////////////
class GraphicsCommandQueue
{
public:
	~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain);
	void WaitSync(); //fence를 이용해서 기다려주는 함수

	void RenderBegin();
	void RenderEnd();

	void FlushResourceCommandQueue();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCmdList() { return _cmdList; }
	ComPtr<ID3D12GraphicsCommandList> GetResourceCmdList() { return	_resCmdList; }
private:
	
	ComPtr<ID3D12CommandQueue> _cmdQueue;
	ComPtr<ID3D12CommandAllocator> _cmdAlloc; //Allocator와 CommandList는 1:1로 대응시켜야함
	ComPtr<ID3D12GraphicsCommandList> _cmdList;

	ComPtr<ID3D12CommandAllocator>		_resCmdAlloc; //텍스처같은 리소스용
	ComPtr<ID3D12GraphicsCommandList>	_resCmdList; //텍스처같은 리소스용

	//Fence : 울타리(?)
	//CPU/GPU 동기화를 위한 간단한 도구
	ComPtr<ID3D12Fence> _fence;
	uint32 _fenceValue = 0;
	HANDLE _fenceEvent = INVALID_HANDLE_VALUE;

	shared_ptr<SwapChain> _swapChain;
};

// ************************
// ComputeCommandQueue
// ************************

class ComputeCommandQueue
{
public:
	~ComputeCommandQueue();

	void Init(ComPtr<ID3D12Device> device);
	void WaitSync();
	void FlushComputeCommandQueue();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList> GetComputeCmdList() { return _cmdList; }

private:
	ComPtr<ID3D12CommandQueue> _cmdQueue;
	ComPtr<ID3D12CommandAllocator> _cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList> _cmdList;

	ComPtr<ID3D12Fence> _fence;
	uint32 _fenceValue = 0;
	HANDLE _fenceEvent = INVALID_HANDLE_VALUE;
};
