#pragma once

struct InstancingParams //버퍼에다 넘겨줄 각 인스턴스마다의 물체의 정보
{
	Matrix matWorld;
	Matrix matWV;
	Matrix matWVP;
};

class InstancingBuffer
{
public:
	InstancingBuffer();
	~InstancingBuffer();

	void Init(uint32 maxCount = 10);

	void Clear();
	void AddData(InstancingParams& params);
	void PushData();

public:
	uint32 GetCount() { return static_cast<uint32>(_data.size()); }
	ComPtr<ID3D12Resource> GetBuffer() { return _buffer; }
	D3D12_VERTEX_BUFFER_VIEW GetBufferView() { return _bufferView; }

	void SetID(uint64 instanceId) { _instanceId = instanceId; }
	uint64 GetID() { return _instanceId; }

private:
	uint64 _instanceId = 0;
	ComPtr<ID3D12Resource> _buffer;
	D3D12_VERTEX_BUFFER_VIEW _bufferView;

	uint32 _maxCount = 0;
	vector<InstancingParams> _data;
};