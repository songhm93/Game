#pragma once

// ************************
// GraphicsDescriptorHeap
// ************************


//GPU로 전달하기 위한 디스크립터 힙. CBV를 직접 가지고 있는 디스크립터 힙을 여기로 Copy함. Flag가 달라진다.
class GraphicsDescriptorHeap
{
public:
	void Init(uint32 count);

	void Clear();
	void SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg);
	void CommitTable();

	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return _descHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(SRV_REGISTER reg);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint8 reg);


private:
	ComPtr<ID3D12DescriptorHeap> _descHeap;
	uint64 _handleSize = 0; //CBV를 직접 가지고 있는 디스크립터 힙(배열)의 요소 크기
	uint64 _groupSize = 0; //GPU에 전달될 디스크립터 힙 하나하나를 그룹이라고 칭할 것임. 그것의 사이즈.
	uint64 _groupCount = 0; //그룹의 갯수. 그룹이 있는 이유는 CPU와 GPU의 동기화때문. 복사는 즉시 일어나지만 GPU에 전달하는건 바로 되지 않음. GPU가 처리가 끝나야 전달됨.
		   
	uint32 _currentGroupIndex = 0; //일종의 커서. 그룹의 인덱스임. 디스크립터 힙 자체의 인덱스가 아님.
};

// ************************
// ComputeDescriptorHeap
// ************************

class ComputeDescriptorHeap
{
public:
	void Init();

	void SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg);
	void SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg);
	void SetUAV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, UAV_REGISTER reg);

	void CommitTable();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(CBV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(SRV_REGISTER reg);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(UAV_REGISTER reg);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint8 reg);

private:

	ComPtr<ID3D12DescriptorHeap> _descHeap;
	uint64 _handleSize = 0;
};

