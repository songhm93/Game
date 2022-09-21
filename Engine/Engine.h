#pragma once

#include "Device.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "RootSignature.h"
#include "Mesh.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "TableDescriptorHeap.h"
#include "Texture.h"
#include "RenderTargetGroup.h"


class Engine
{
public:
	void Init(const WindowInfo& window);
	void Update();

	
public:
	void Render();
	void RenderBegin(); //커맨드 큐에 요청ㅡ사항을 넣는
	void RenderEnd(); //커맨드 큐에 쌓은 일감들을 gpu에 외주를 맡겨서 모든 일감을 그리는 작업이 실행되는
	
	void ResizeWindow(int32 width, int32 height);

private:
	void ShowFPS();
	void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);
	void CreateRenderTargetGroups();
public:
	const WindowInfo& GetWindow() { return _window; }
	shared_ptr<Device> GetDevice() { return _device; }
	shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return _graphicsCmdQueue; }
	shared_ptr<ComputeCommandQueue> GetComputeCmdQueue() { return _computeCmdQueue; }
	shared_ptr<SwapChain> GetSwapChain() { return  _swapChain; }
	shared_ptr<RootSignature> GetRootSignature() { return  _rootSignature; }
	shared_ptr<GraphicsDescriptorHeap> GetGraphicsDescHeap() { return _graphicsDescHeap; }
	shared_ptr<ComputeDescriptorHeap> GetComputeDescHeap() { return _computeDescHeap; }

	shared_ptr<ConstantBuffer> GetConstantBuffer(CONSTANT_BUFFER_TYPE type) { return _constantBuffers[static_cast<uint8>(type)]; }
	shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return _rtGroups[static_cast<uint8>(type)]; }
private:
	//아래 3개는 그려진 화면 크기 관련
	WindowInfo _window;
	D3D12_VIEWPORT _viewport = {};
	D3D12_RECT _scissorRect = {};

	shared_ptr<Device> _device = make_shared<Device>();//얘네 4개 클래스가 핵심인데, 이 선언된 클래스들은 진짜 핵심 d3d 기능들을 담는 사용자 정의 클래스이다.
	shared_ptr<GraphicsCommandQueue> _graphicsCmdQueue = make_shared <GraphicsCommandQueue>(); //그래서 쉐어드포인터로 선언한거고, 진짜 device 객체나 commandqueue 객체는 이 클래스들 안에서 생성한다.
	shared_ptr<ComputeCommandQueue> _computeCmdQueue = make_shared <ComputeCommandQueue>(); 
	shared_ptr<SwapChain> _swapChain = make_shared <SwapChain>(); //얘네를 객체화하고, 그 안에서 진짜를 객체화해서 기능들을 사용하는 것
	shared_ptr<RootSignature> _rootSignature = make_shared<RootSignature>();
	shared_ptr<GraphicsDescriptorHeap> _graphicsDescHeap = make_shared<GraphicsDescriptorHeap>();
	shared_ptr<ComputeDescriptorHeap> _computeDescHeap = make_shared<ComputeDescriptorHeap>();

	vector<shared_ptr<ConstantBuffer>> _constantBuffers;
	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> _rtGroups;
};

