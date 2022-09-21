#pragma once
#include "Object.h"

// [일감 기술서] 외주 인력들이 뭘 해야할 지 기술. 명령서

enum class SHADER_TYPE : uint8
{
	DEFERRED,
	FORWARD,
	LIGHTING,
	PARTICLE,
	COMPUTE,
	SHADOW,
};

enum class RASTERIZER_TYPE : uint8
{
	CULL_NONE, //컬링하지 않고 모두 연산
	CULL_FRONT, //시계방향을 컬링
	CULL_BACK, //반시계방향을 컬링
	WIREFRAME,
};

enum class DEPTH_STENCIL_TYPE : uint8
{
	LESS,
	LESS_EQUAL,
	GREATER,
	GREATER_EQUAL,
	NO_DEPTH_TEST, // 깊이 테스트(X) + 깊이 기록(O)
	NO_DEPTH_TEST_NO_WRITE, // 깊이 테스트(X) + 깊이 기록(X)
	LESS_NO_WRITE, // 깊이 테스트(O) + 깊이 기록(X)
};

enum class BLEND_TYPE : uint8 //PS에서 계산한 결과물이랑, RT 텍스쳐에 있는 결과물이랑 어떻게 혼합할지.
{
	DEFAULT, 
	ALPHA_BLEND, //알파값(투명도)를 이용한 블렌드
	ONE_TO_ONE_BLEND,
	END,
};

struct ShaderInfo
{
	SHADER_TYPE shaderType = SHADER_TYPE::FORWARD;
	RASTERIZER_TYPE rasterizerType = RASTERIZER_TYPE::CULL_BACK;
	DEPTH_STENCIL_TYPE depthStencilType = DEPTH_STENCIL_TYPE::LESS;
	BLEND_TYPE blendType = BLEND_TYPE::DEFAULT;
	D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct ShaderArg
{
	const string vs = "VS_Main";
	const string hs;
	const string ds;
	const string gs;
	const string ps = "PS_Main";
};

class Shader : public Object
{
public:
	Shader();
	virtual ~Shader();
	void CreateGraphicsShader(const wstring& path, ShaderInfo info = ShaderInfo(), ShaderArg arg = ShaderArg());
	void CreateComputeShader(const wstring& path, const string& name, const string& version);

	void Update();

	SHADER_TYPE GetShaderType() { return _info.shaderType; }

	static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);

private:
	void CreateShader(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob, D3D12_SHADER_BYTECODE& shaderByteCode);
	void CreateVertexShader(const wstring& path, const string& name, const string& version);
	void CreateHullShader(const wstring& path, const string& name, const string& version);
	void CreateDomainShader(const wstring& path, const string& name, const string& version);
	void CreateGeometryShader(const wstring& path, const string& name, const string& version);
	void CreatePixelShader(const wstring& path, const string& name, const string& version);
private:
	ShaderInfo _info;
	ComPtr<ID3D12PipelineState> _pipelineState;

	// GraphicsShader
	ComPtr<ID3DBlob> _vsBlob;
	ComPtr<ID3DBlob> _hsBlob;
	ComPtr<ID3DBlob> _dsBlob;
	ComPtr<ID3DBlob> _gsBlob;
	ComPtr<ID3DBlob> _psBlob;
	ComPtr<ID3DBlob> _errBlob;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC  _graphicsPipelineDesc = {}; //GRAPHICS_PIPELINE

	// ComputeShader
	ComPtr<ID3DBlob> _csBlob;
	D3D12_COMPUTE_PIPELINE_STATE_DESC   _computePipelineDesc = {}; //COMPUTE_PIPELINE

};

