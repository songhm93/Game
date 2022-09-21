#include "pch.h"
#include "TableDescriptorHeap.h"
#include "Engine.h"

// ************************
// GraphicsDescriptorHeap
// ************************


void GraphicsDescriptorHeap::Init(uint32 count) //count는 그룹을 몇개를 만들것인가.
{
	_groupCount = count;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = count * (CBV_SRV_REGISTER_COUNT - 1); //디스크립터 갯수 세팅. b0는 전역이라서 -1
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //이것이 달라진다. 상수버퍼 뷰를 직접 가지고 있는 디스크립터 힙은 flag가 NONE이였음.
	//Flag_shader_visible로 만들어줘야 gpu 메모리에 상주. 그래야만 gpu에 전달해줄 수 있음.
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; //타입은 cbv,srv,uav

	DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap)); //디스크립터 힙 생성

	_handleSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_groupSize = _handleSize * (CBV_SRV_REGISTER_COUNT - 1); //b0는 전역이라서 -1
}

void GraphicsDescriptorHeap::Clear()
{
	_currentGroupIndex = 0;
}

void GraphicsDescriptorHeap::SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg) //CBV를 가지고 있는 디스크립터힙의 내용(cbv)을 GPU로 전달 할 디스크립터 힙으로 복사해주는 함수.
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); //소스에서 대상으로 디스크립터(뷰)를 복사
	//cpu 핸들 2개를 받아서 copy해주는 함수. srcHandle을 destHandle로 copy. destHandle이 reg
}

void GraphicsDescriptorHeap::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::CommitTable() //복사한 디스크립터 힙,현재 그룹 인덱스에 해당하는 힙을 GPU에 전달하는 함수. 그룹 단위임.
{
	D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap->GetGPUDescriptorHandleForHeapStart(); //gpu에서 보이는 주소
	handle.ptr += _currentGroupIndex * _groupSize;
	GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(1, handle); //GPU에 제출할 디스크립터 힙들 중 어떤 것을 제출할지 설정.
	
	_currentGroupIndex++; //해당 그룹은 넘겨줬으니 커서위치를 옮기는 것.(그룹 단위)
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(CBV_REGISTER reg) //이 예제에서는 b0~b4
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(SRV_REGISTER reg)
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(uint8 reg)
{
	assert(reg > 0);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = _descHeap->GetCPUDescriptorHandleForHeapStart(); //시작 핸들을 꺼냄
	handle.ptr += _currentGroupIndex * _groupSize; //몇동. 그룹단위 이동
	handle.ptr += (reg - 1) * _handleSize; //몇호. 그룹내에서 인덱스 이동. reg - 1은 b0는 여기서 활용을 안하기 때문에 b1에 해당하는 reg 1이 들어오면 첫번째 데이터니까 -1해줌.
	return handle; //해당 인덱스 핸들을 반환
}

// ************************
// ComputeDescriptorHeap
// ************************

void ComputeDescriptorHeap::Init()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = TOTAL_REGISTER_COUNT; //Total 갯수. UAV까지 포함된 갯수
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap));

	_handleSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::SetUAV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, UAV_REGISTER reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

	uint32 destRange = 1;
	uint32 srcRange = 1;
	DEVICE->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// TODO : 리소스 상태 변경
}

void ComputeDescriptorHeap::CommitTable()
{
	ID3D12DescriptorHeap* descHeap = _descHeap.Get();
	COMPUTE_CMD_LIST->SetDescriptorHeaps(1, &descHeap); //CommandList와 연결된 현재 바인딩된 디스크립터 힙을 변경한다. 
	//D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER 유형의 디스크립터 힙만 바인딩할 수 있다. (루트서명에 들어가는 리소스들)
	//각 유형의 디스크립터 힙은 한 번에 하나만 설정할 수 있다. 즉, 한 번에 최대 2개의 힙(샘플러 1개, CBV/SRV/UAV 1개)을 설정할 수 있다.

	D3D12_GPU_DESCRIPTOR_HANDLE handle = descHeap->GetGPUDescriptorHandleForHeapStart(); 
	//D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap->GetGPUDescriptorHandleForHeapStart(); //위와 같음.
	COMPUTE_CMD_LIST->SetComputeRootDescriptorTable(0, handle); //테이블을 컴퓨트루트서명으로 설정
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(CBV_REGISTER reg)
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(SRV_REGISTER reg)
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(UAV_REGISTER reg)
{
	return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(uint8 reg)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = _descHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += reg * _handleSize;
	return handle;
}