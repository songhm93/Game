#pragma once
#include "InstancingBuffer.h"

class GameObject;

class InstancingManager //인스턴스 버퍼를 관리하는 매니저. 물체를 10개 만들어야한다면 인스턴스 버퍼도 10개가 생겨야함.
{
	DECLARE_SINGLE(InstancingManager);

public:
	void Render(vector<shared_ptr<GameObject>>& gameObjects);

	void ClearBuffer();
	void Clear() { _buffers.clear(); }

private:
	void AddParam(uint64 instanceId, InstancingParams& data);

private:
	map<uint64/*instanceId*/, shared_ptr<InstancingBuffer>> _buffers;
};

