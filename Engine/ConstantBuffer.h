#pragma once


enum class CONSTANT_BUFFER_TYPE : uint8
{
	GLOBAL,//전역으로 활용(light 관련)
	TRANSFORM,
	MATERIAL,
	END
};

enum
{
	CONSTANT_BUFFER_COUNT = static_cast<uint8>(CONSTANT_BUFFER_TYPE::END)
};


class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	void Init(CBV_REGISTER reg, uint32 size, uint32 count);

	void Clear();
	void PushGraphicsData(void* buffer, uint32 size);
	void SetGraphicsGlobalData(void* buffer, uint32 size); //글로벌 데이터로 활용할, b0 용도로 활용할 함수
	void PushComputeData(void* buffer, uint32 size);

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(uint32 index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32 index);
private:
	void CreateBuffer();
	void CreateView();

private:
	ComPtr<ID3D12Resource>	_cbvBuffer;
	BYTE* _mappedBuffer = nullptr;
	uint32	_elementSize = 0;
	uint32	_elementCount = 0;

	ComPtr<ID3D12DescriptorHeap> _cbvHeap; //디스크립터 힙. 상수버퍼 뷰를 넣을 것임.
	D3D12_CPU_DESCRIPTOR_HANDLE	_cpuHandleBegin = {}; //디스크립터 힙의 시작 주소
	uint32 _handleIncrementSize = 0; //시작주소에서 다음 요소로 넘어가기 위함.


	uint32	_currentIndex = 0;
	CBV_REGISTER _reg = {};
};

