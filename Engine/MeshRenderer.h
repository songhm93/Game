#pragma once
#include "Component.h"

// [32][32]
union InstanceID //공용체. uint32가 2개 uint64가1개 여서 16바이트 같지만 8바이트이다. 구조체형태로 사용할 수도 있고, 아래 uint64형태로도 사용할 수 있다.
{
	struct
	{
		uint32 meshID;
		uint32 materialID;
	};
	uint64 id;
};

class Mesh;
class Material;
class InstancingBuffer;


class MeshRenderer : public Component
{
public:
	MeshRenderer();
	virtual ~MeshRenderer();

	shared_ptr<Mesh> GetMesh() { return _mesh; }
	shared_ptr<Material> GetMaterial(uint32 idx = 0) { return _materials[idx]; }

	void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
	void SetMaterial(shared_ptr<Material> material, uint32 idx = 0);
	
	void Render(); //finalupdate에서 좌표계산을 다 해주고 그것을 행렬로 만들어 준 다음에 Render를 할 때에는 그 계산된 최종 결과물을 GPU쪽에다가 올리는 부분이 일어남
	void Render(shared_ptr<InstancingBuffer>& buffer);
	void RenderShadow();

	uint64 GetInstanceID();
private:
	shared_ptr<Mesh> _mesh;
	vector<shared_ptr<Material>> _materials;
};

