#include "pch.h"
#include "SwapChain.h"


void SwapChain::Init(const WindowInfo& info, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue)
{
	CreateSwapChain(info, dxgi, cmdQueue);
	//CreateRTV(device);
}

void SwapChain::Present()
{
	//프레임을 보여준다.(그린다.)
	_swapChain->Present(0, 0);
}

void SwapChain::SwapIndex() //인덱스 스왑. 0이였으면 1, 1이였으면 0.
{
	_backBufferIndex = (_backBufferIndex + 1) % SWAP_CHAIN_BUFFER_COUNT;
}


void SwapChain::CreateSwapChain(const WindowInfo& info, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue)
{
	//이전에 만든 정보를 날린다.
	_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = static_cast<uint32>(info.width); //버퍼의 해상도 너비
	sd.BufferDesc.Height = static_cast<uint32>(info.height); //버퍼의 해상도 높이
	sd.BufferDesc.RefreshRate.Numerator = 60; //화면 갱신 비율
	sd.BufferDesc.RefreshRate.Denominator = 1; //화면 갱신 비율
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //버퍼의 디스플레이 형식.RGBA 8비트씩 총 32비트
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1; //멀티 샘플링 OFF
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //후면 버퍼에 렌더링할 것
	sd.BufferCount = SWAP_CHAIN_BUFFER_COUNT; //전면+후면 버퍼. 2가 들어감.
	sd.OutputWindow = info.hwnd;
	sd.Windowed = info.windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //전면 후면 버퍼 교체 시 이전 프레임 정보(버퍼) 버림(버퍼를 유지하면 비용이 많이 듬)
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	dxgi->CreateSwapChain(cmdQueue.Get(), &sd, &_swapChain); //스왑 체인을 만듬
	
	/*for (int32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++)
	{
		_swapChain->GetBuffer(i, IID_PPV_ARGS(&_rtvBuffer[i]));
	}*/
}

//void SwapChain::CreateRTV(ComPtr<ID3D12Device> device)
//{
//	/*
//	Descriptor (DX12) = View (~DX11)
//	[서술자 힙]으로 RTV 생성
//	DX11의 RTV(RenderTargetView), DSV(DepthStencilView),
//	CBV(ConstantBufferView), SRV(ShaderResourceView), UAV(UnorderedAccessView)를 전부 관리
//	*/
//
//	int32 _rtvHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//
//	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc; //Description을 만들건데 RTV에 대한 걸 만들거니까 이름을 rtv로 지은것이고.
//	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //Description의 타입. RTV이다.
//	rtvDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT; //2개
//	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvDesc.NodeMask = 0;
//
//	/*
//	DescriptorHeap은 일종의 배열
//	같은 종류의 데이터끼리 배열로 관리
//	RTV 목록 : [] []
//	*/
//	device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&_rtvHeap)); //DescriptorHeap을 만든다. 위 배열처럼 만들어질것.
//
//	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapBegin = _rtvHeap->GetCPUDescriptorHandleForHeapStart(); //배열의 시작주소를 가리키고 있다고 보면 된다.
//
//	for (int i = 0; i < SWAP_CHAIN_BUFFER_COUNT; i++) //위에서 배열을 만들고 여기에서 배열에 내용을 채움
//	{
//		//d3dx12.h에 있는 헬퍼 클래스의 생성자이고, 호출하면 어떤 함수 호출됨. 시작주소부터 사이즈만큼 이동해서 거기의 주소를 반환.
//		_rtvHandle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeapBegin, i * _rtvHeapSize); //배열이니까 배열 한칸의 크기만큼 이동해서 다음 인덱스를 반환한거라고 보면 된다.
//		device->CreateRenderTargetView(_rtvBuffer[i].Get(), nullptr, _rtvHandle[i]); //_rtvHandle[i]를 swapChain->GetRenderTarget(i).Get() 정보로 채워라
//	}
//
//
//}