#pragma once
#include "Texture.h"

enum class RENDER_TARGET_GROUP_TYPE : uint8
{
	SWAP_CHAIN, // BACK_BUFFER, FRONT_BUFFER
	SHADOW, //SHADOW
	G_BUFFER, // POSITION, NORMAL, COLOR. Global Buffer
	LIGHTING, // DIFFUSE_LIGHT, SPECULAR_LIGHT
	
	END,
};

enum
{
	RENDER_TARGET_SHADOW_GROUP_MEMBER_COUNT = 1,
	RENDER_TARGET_G_BUFFER_GROUP_MEMBER_COUNT = 3,
	RENDER_TARGET_LIGHTING_GROUP_MEMBER_COUNT = 2,
	RENDER_TARGET_GROUP_COUNT = static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::END)
};

struct RenderTarget
{
	shared_ptr<Texture> target;
	float clearColor[4];
};

class RenderTargetGroup
{
public:
	void Create(RENDER_TARGET_GROUP_TYPE groupType, vector<RenderTarget>& rtVec, shared_ptr<Texture> dsTexture); //dsTexture:깊이텍스쳐(뎁스스텐실버퍼)

	void OMSetRenderTargets(uint32 count, uint32 offset);
	void OMSetRenderTargets();

	void ClearRenderTargetView(uint32 index);
	void ClearRenderTargetView();

	shared_ptr<Texture> GetRTTexture(uint32 index) { return _rtVec[index].target; }
	shared_ptr<Texture> GetDSTexture() { return _dsTexture; }

	void WaitTargetToResource();
	void WaitResourceToTarget();

private:
	RENDER_TARGET_GROUP_TYPE		_groupType; //스왑체인 용도인지, 지오메트리버퍼 용도(멀티렌더타겟)인지
	vector<RenderTarget>			_rtVec;
	uint32							_rtCount;
	shared_ptr<Texture>				_dsTexture; //depthStencil. 얘는 어떤 그룹에나 똑같이 들어간다. 깊이값이기 때문에.
	ComPtr<ID3D12DescriptorHeap>	_rtvHeap;

private:
	uint32							_rtvHeapSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		_rtvHeapBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE		_dsvHeapBegin;

private:
	D3D12_RESOURCE_BARRIER			_targetToResource[8];
	D3D12_RESOURCE_BARRIER			_resourceToTarget[8];

};

