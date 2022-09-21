#pragma once

//스왑 체인(교환 사슬)
/*
[외주 과정]
- 현재 게임 세상에 있는 상황을 묘사
- 어떤 공식으로 어떻게 계산할지 외주한테 던져줌
- GPU가 열심히 계산 (외주)
- 결과물을 외주한테서 받아서 화면에 그려준다.

[외주 결과물]을 어디에 받아야 할까?
- 어떤 종이(Buffer)에 그려서 건네달라고 부탁해보자
- 특수 종이를 만들어서 -> 처음에 외주한테 건네주고 -> 결과물을 해당 종이에 받는다 OK
- 우리 화면에 특수 종이(외주 결과물) 출력해준다.

[BUT]
- 그런데 화면에 현재 결과물 출력하는 와중에, 다음 화면도 외주를 맡겨야함
- 그런데.. 현재 화면 결과물(처음에 건네준 종이)은 이미 화면 출력에 사용중
->> 특수 종이를 2개 만들어서, 하나는 현재 화면을 그려주고, 하나는 외주 맡기고 해야겠네!
- 이것이 Double Buffering!

-[1] [2] 종이 2개
현재 화면 [1] <-> GPU 작업중 [2]( BackBuffer )
-> 스왑
현재 화면 [2] <-> GPU 작업중 [1]( BackBuffer )

얘네는 결국 리소스이다. 이 리소스를 바로 GPU에 넘겨주는 것은 아니고, DescriptorHeap(기안서)를 만들어서 View를 건네줘야 한다.
기안서에서 스왑체인의 렌더 타겟을 배열에 저장함.
*/
class SwapChain
{
public:
	void Init(const WindowInfo& info, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);
	void Present();
	void SwapIndex();

	ComPtr<IDXGISwapChain> GetSwapChain() { return _swapChain; }
	uint8 GetBackBufferIndex() { return _backBufferIndex; }
	//ComPtr<ID3D12Resource> GetRenderTarget(uint32 index) { return _rtvBuffer[index]; }

	//ComPtr<ID3D12Resource> GetBackRTVBuffer() { return _rtvBuffer[_backBufferIndex];  }
	//D3D12_CPU_DESCRIPTOR_HANDLE GetBackRTV() { return _rtvHandle[_backBufferIndex]; }
private:
	void CreateSwapChain(const WindowInfo& info, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);
	//void CreateRTV(ComPtr<ID3D12Device> device);
private:
	ComPtr<IDXGISwapChain> _swapChain;

	//ComPtr<ID3D12Resource> _rtvBuffer[SWAP_CHAIN_BUFFER_COUNT]; //그릴 대상(특수 종이)
	//ComPtr<ID3D12DescriptorHeap> _rtvHeap; //rtv -> Render Target View
	//D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle[SWAP_CHAIN_BUFFER_COUNT]; 렌더타겟은 이제 Texture에서 관리.

	uint32 _backBufferIndex = 0;
};

