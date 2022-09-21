#include "pch.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "Engine.h"

//////////////////////////////////////
//GraphicsCommandQueue
/////////////////////////////////////

GraphicsCommandQueue::~GraphicsCommandQueue()
{
	::CloseHandle(_fenceEvent);
}

void GraphicsCommandQueue::Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain)
{
	_swapChain = swapChain;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_cmdQueue)); //_cmdQueue가 만들어짐

	//-D3D12_COMMAND_LIST_TYPE_DIRECT : GPU가 직접 실행하는 명령 목록
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAlloc)); //_cmdAlloc가 만들어짐

	//GPU가 하나인 시스템에서는 0으로
	//DIRECT or BUNDLE
	//CommandList를 생성하기 위해서는 Allocator 인터페이스 포인터가 필요하다.(2번째 인자)
	//초기 상태(그리기 명령은 nullptr 지정)
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList)); //_cmdList가 만들어짐

	//CommandList는 Close/Open 상태가 있는데
	//Open상태에서 Command를 넣다가 Close한 다음 제출하는 개념
	_cmdList->Close();


	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_resCmdAlloc)); // _resCmdAlloc가 만들어짐
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _resCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_resCmdList)); //_resCmdList가 만들어짐

	//CreateFence
	//-CPU와 GPU의 동기화 수단으로 쓰인다.
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)); //_fence가 만들어짐
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr); //이벤트를 생성. 

}

void GraphicsCommandQueue::WaitSync() //CPU가 GPU의 일감이 끝날 때까지 대기하는 방식.
{
	_fenceValue++;

	//명령 대기열에 명령을 추가하여 새 펜스 포인트를 설정하십시오.
	//GPU 타임라인에 있으므로 GPU가 이 Signal() 이전의 모든 명령 처리를 완료할 때까지 새 펜스 포인트가 설정되지 않습니다.
	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	//GPU가 이 펜스 지점까지 명령을 완료할 때까지 기다리십시오.
	if (_fence->GetCompletedValue() < _fenceValue)
	{
		//GPU가 현재 펜스에 도달하면 이벤트가 발생합니다.
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);

		//GPU가 현재 펜스 이벤트에 도달할 때까지 기다립니다.
		::WaitForSingleObject(_fenceEvent, INFINITE); //위에서 생성한 이벤트 핸들이 TRUE가 될때까지 무한히 기다림.
	}
}

void GraphicsCommandQueue::RenderBegin()
{
	_cmdAlloc->Reset();							//벡터의 clear와 비슷한 느낌. 내용물은 날리고 capacity은 그대로.
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);	//벡터의 clear와 비슷한 느낌. 내용물은 날리고 capacity은 그대로. cmdlist는 reset해줘야 open이 된다.

	int8 backIndex = _swapChain->GetBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition( //C로 시작하는 d3dx12들은 헬퍼 클래스
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(), //스왑체인도 그룹으로 관리하고, 텍스쳐로 저장해놨으니 이렇게 가져옴
		D3D12_RESOURCE_STATE_PRESENT,			//화면 출력 before
		D3D12_RESOURCE_STATE_RENDER_TARGET);	//외주 결과물 after
	//여기가 스왑체인에서 현재화면 [0] <-> [1] GPU작업중 을 [0] <-> [0]으로 바꿔주는 셈
	//현재 백버퍼 리소스를 가져와서 gpu 작업 용도로 활용하겠다. 라고 요청을 만들어주는 것.


	_cmdList->SetGraphicsRootSignature(GRAPHICS_ROOT_SIGNATURE.Get()); //서명
	//GEngine->GetCB()->Clear(); //상수버퍼배열 인덱스 위치를 맨처음으로., 원래 한개만 쓰다가 머티리얼 추가하면서 상수버퍼를 2개 써야해서 주석. 아래에 추가.
	GEngine->GetConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM)->Clear();
	GEngine->GetConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL)->Clear();

	GEngine->GetGraphicsDescHeap()->Clear(); //GPU에 전달할 디스크립터힙 그룹 인덱스 위치를 맨처음으로.

	ID3D12DescriptorHeap* descHeap = GEngine->GetGraphicsDescHeap()->GetDescriptorHeap().Get(); //GPU에 전달할 디스크립터힙의 포인터를 반환
	_cmdList->SetDescriptorHeaps(1, &descHeap); //한 프레임에 한번만 호출되어야 함. 
	//이걸 호출해줘야 디스크립터힙 테이블에서 호출하는 SetGraphicsRootDescriptorTable 함수가 정상적으로 호출된다. 두 개는 세트로 움직이는 느낌이다.


	_cmdList->ResourceBarrier(1, &barrier); //현재 리소스가 어떤 상태인지 상태를 바꿔줌
	
	//뷰포트와 ScissorRect를 설정합니다. 이것은 명령 목록이 재설정될 때마다 재설정되어야 합니다.
	/*_cmdList->RSSetViewports(1, vp);
	_cmdList->RSSetScissorRects(1, rect);*/

	//렌더링할 버퍼를 지정합니다. 이코드도 자리를 옮김
	//D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = _swapChain->GetBackRTV(); //백버퍼뷰(gpu가 작업할 것)를 꺼내와서
	//_cmdList->ClearRenderTargetView(backBufferView, Colors::Black, 0, nullptr);  //백버퍼를 비워준다.

	//D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = GEngine->GetDepthStencilBuffer()->GetDSVCpuHandle();
	//_cmdList->OMSetRenderTargets(1, &backBufferView, FALSE, &depthStencilView); //최종적으로 셰이더가 완료된다음 OM단계에서 출력할 대상을 세팅
	//_cmdList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr); //깊이버퍼를 비워준다.
}

void GraphicsCommandQueue::RenderEnd()
{
	int8 backIndex = _swapChain->GetBackBufferIndex();
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(	
		GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->GetRTTexture(backIndex)->GetTex2D().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,									//외주 결과물. 위 함수와 다르게 여기 순서가 변경됨.
		D3D12_RESOURCE_STATE_PRESENT);										//화면 출력
	//여기가 스왑체인에서 현재화면 [0] <-> [1] GPU작업중 을 [1] <-> [1]으로 바꿔주는 셈

	_cmdList->ResourceBarrier(1, &barrier);
	_cmdList->Close();


	//커맨드 리스트 수행
	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr); //실행 요청.

	_swapChain->Present(); //프레임을 그림

	//프레임 명령이 완료될 때까지 기다리십시오.
	//이 대기는 비효율적이며 단순성을 위해 수행됩니다.
	//나중에 프레임당 기다릴 필요가 없도록 렌더링 코드를 구성하는 방법을 보여줄 것입니다.
	WaitSync(); //cpu 대기

	_swapChain->SwapIndex(); //백버퍼 인덱스가 0번이면 1번, 1번이면 0번으로 스왑. 
}

void GraphicsCommandQueue::FlushResourceCommandQueue() //리소스 커맨드 리스트에 추가한 명령 제출
{
	_resCmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _resCmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	_resCmdAlloc->Reset();
	_resCmdList->Reset(_resCmdAlloc.Get(), nullptr);
}

// ************************
// ComputeCommandQueue
// ************************

ComputeCommandQueue::~ComputeCommandQueue()
{
	::CloseHandle(_fenceEvent);
}

void ComputeCommandQueue::Init(ComPtr<ID3D12Device> device)
{
	D3D12_COMMAND_QUEUE_DESC computeQueueDesc = {};
	computeQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE; //타입이 다름.
	computeQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	device->CreateCommandQueue(&computeQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&_cmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	// CreateFence
	// - CPU와 GPU의 동기화 수단으로 쓰인다.
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void ComputeCommandQueue::WaitSync()
{
	_fenceValue++;

	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	if (_fence->GetCompletedValue() < _fenceValue)
	{
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void ComputeCommandQueue::FlushComputeCommandQueue()
{
	_cmdList->Close();

	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	auto t = _countof(cmdListArr);
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	WaitSync();

	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	COMPUTE_CMD_LIST->SetComputeRootSignature(COMPUTE_ROOT_SIGNATURE.Get());
}