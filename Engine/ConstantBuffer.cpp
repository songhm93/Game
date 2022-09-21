#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine.h"

ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::~ConstantBuffer()
{
	if (_cbvBuffer)
	{
		if (_cbvBuffer != nullptr)
			_cbvBuffer->Unmap(0, nullptr);

		_cbvBuffer = nullptr;
	}
}


void ConstantBuffer::Init(CBV_REGISTER reg, uint32 size, uint32 count)
{
	_reg = reg; //어떤 레지스터를 사용하는지.

	// 상수 버퍼는 256 바이트 배수로 만들어야 한다.
	// 0 256 512 768
	_elementSize = (size + 255) & ~255; // 255는 8개비트가 다 켜진상태고 ~255는 반대로 8개비트를 다 끈 상태. 다 끈거를 & 하니까 거기 8개를 다 꺼버리는거임. 
	//비트 9개째부턴 256배수이기 때문. 0000 0010 1100 0011(707) 이라는 숫자가 들어왔다고 치면 뒤에 8개 다 꺼져서 0000 0010 0000 0000이 되어서 512가 되는 것임.
	//근데 앞에 +255를 먼저 해주니까 707+255=962 -> 0000 0011 1100 0010(962) 여기서 뒤에 8개를 다 꺼서 0000 0011 0000 0000이 되어서 768이 됨
	// 0 256 512 768 (962)-> 들어온 숫자+255에서 왼쪽에 해당하는 값. 256이 아닌 255를 더해준 이유는 만약 256을 하고싶어서 256을 넣었는데 256을 더하면 512가 된다.
	// 256을 넣었는데 255를 더하면 511이니까 0 256 (511) 512 768 이니까 왼쪽에 해당하는 값인 256이 선택된다.
	_elementCount = count;

	CreateBuffer();
	CreateView();
}

void ConstantBuffer::CreateBuffer()
{
	uint32 bufferSize = _elementSize * _elementCount;
	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	DEVICE->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_cbvBuffer));

	_cbvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&_mappedBuffer));
	// 리소스를 다 사용할때까지 UnMap할 필요 없다. 그러나 GPU에서 사용 중인 리소스에 write해서는 안됨.접근하면 안된다.(따라서 동기화 기술을 사용해야 함).
	// 이 강의에선 fence를 이용해서 cpu가 gpu를 기다리는 waitsync함수로 동기화 해주는 중
}

void ConstantBuffer::CreateView()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
	cbvDesc.NumDescriptors = _elementCount; //256개. 즉 배열 크기가 256임
	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&_cbvHeap)); //디스크립터 힙 생성

	_cpuHandleBegin = _cbvHeap->GetCPUDescriptorHandleForHeapStart(); //디스크립터 힙 시작주소
	_handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); //힙요소 증가사이즈. 주소로 접근할 때 요소 이동하기 위함.
	//버퍼요소사이즈와 무관하다.


	for (uint32 i = 0; i < _elementCount; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = _cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(_elementSize) * i;
		cbvDesc.SizeInBytes = _elementSize;   // 상수 버퍼 크기는 256바이트의 배수로 정렬되어야 한다.

		DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
	}//정리하면 디스크립터 힙(배열)을 먼저 만들어놓고 시작 주소부터 요소크기로 인덱스마다 들르면서 cbvHandle에다가 상수버퍼 뷰를 만들어서 넣어준다.
}

void ConstantBuffer::Clear()
{
	_currentIndex = 0;
}

void ConstantBuffer::PushGraphicsData(void* buffer, uint32 size) //buffer = params. 
{
	assert(_currentIndex < _elementCount); //만족하지 못하면 assert
	assert(_elementSize == ((size + 255) & ~255));

	::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size); //받아온 params를 mappedBuffer에 copy. mappedBuffer가 CPU 가상메모리.

	//D3D12_GPU_VIRTUAL_ADDRESS address = GetGpuVirtualAddress(_currentIndex);
	//CMD_LIST->SetGraphicsRootConstantBufferView(rootParamIndex, address); //나중에 레지스터에 데이터를 넣을것임. 디스크립터 힙 테이블 추가하면서 주석.

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex); //힙의 인덱스만큼 이동한 주소값.

	GEngine->GetGraphicsDescHeap()->SetCBV(cpuHandle, _reg);//엔진이 들고있는 힙을 들고와서 cpuHandle이 가리키는 데이터?버퍼?뷰?를 상수버퍼 reg의 Handle에 copy

	_currentIndex++;

}

void ConstantBuffer::SetGraphicsGlobalData(void* buffer, uint32 size) //무조건 0번째. b0에 세팅함
{
	assert(_elementSize == ((size + 255) & ~255));
	::memcpy(&_mappedBuffer[0], buffer, size);
	GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(0, GetGpuVirtualAddress(0)); 
}

void ConstantBuffer::PushComputeData(void* buffer, uint32 size)
{
	assert(_currentIndex < _elementCount);
	assert(_elementSize == ((size + 255) & ~255));

	::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);
	GEngine->GetComputeDescHeap()->SetCBV(cpuHandle, _reg);

	_currentIndex++;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGpuVirtualAddress(uint32 index)
{
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = _cbvBuffer->GetGPUVirtualAddress();
	objCBAddress += index * _elementSize;
	return objCBAddress;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetCpuHandle(uint32 index)//인덱스로 디스크립터 힙의 요소에 접근
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(_cpuHandleBegin, index * _handleIncrementSize);
}
