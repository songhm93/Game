#pragma once
#include "Object.h"

class Texture : public Object //텍스처를 로딩하고 관리하는 클래스. 경로를 통해 이미지를 불러와서 텍스처로 만드는것뿐 아니라 코드로 만들어주는 텍스처도 만들수 있게 함.
{
public:
	Texture();
	virtual ~Texture();
	virtual void Load(const wstring& path) override;

public:
	void Create(DXGI_FORMAT format, uint32 width, uint32 height,
		const D3D12_HEAP_PROPERTIES& heapProperty, D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_FLAGS resFlags, Vec4 clearColor = Vec4()); //아예 0부터 텍스쳐를 만드는 함수

	void CreateFromResource(ComPtr<ID3D12Resource> tex2D); //이미 리소스(버퍼)가 있는 상태에서 텍스쳐를 만드는 함수

public:
	ComPtr<ID3D12Resource> GetTex2D() { return _tex2D; }
	ComPtr<ID3D12DescriptorHeap> GetSRV() { return _srvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetRTV() { return _rtvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetDSV() { return _dsvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetUAV() { return _uavHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle() { return _srvHeapBegin; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle() { return _uavHeapBegin; }
	
	float GetWidth() { return static_cast<float>(_desc.Width); }
	float GetHeight() { return static_cast<float>(_desc.Height); }

public:

private:
	ScratchImage _image; //이미지를 로드하고 텍스처로 만드는 부분
	D3D12_RESOURCE_DESC _desc; //리소스 설명서
	ComPtr<ID3D12Resource> _tex2D; //이미지를 로드하고 텍스처로 만드는 부분

	ComPtr<ID3D12DescriptorHeap> _srvHeap; //셰이더리소스뷰를 담는 힙을 만드는 부분
	ComPtr<ID3D12DescriptorHeap> _rtvHeap; //렌더타겟뷰를 담는 힙을 만드는 부분
	ComPtr<ID3D12DescriptorHeap> _dsvHeap; //뎁스스텐실뷰를 담는 힙을 만드는 부분
	ComPtr<ID3D12DescriptorHeap> _uavHeap; //UnorederedAccessView를 담는 힙을 만드는 부분

private:
	D3D12_CPU_DESCRIPTOR_HANDLE _srvHeapBegin = {};
	D3D12_CPU_DESCRIPTOR_HANDLE _uavHeapBegin = {};
};

