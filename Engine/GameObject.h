#pragma once
#include "Component.h"
#include "Object.h"

class Transform;
class MeshRenderer;
class Camera;
class Light;
class MonoBehaviour;
class ParticleSystem;
class Terrain;
class BaseCollider;
class Animator;

class GameObject : public Object, public enable_shared_from_this<GameObject> 
{
public:
	GameObject();
	virtual ~GameObject();



	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FinalUpdate();

	shared_ptr<Component> GetFixedComponent(COMPONENT_TYPE type);

	shared_ptr<Transform> GetTransform();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<Camera> GetCamera();
	shared_ptr<Light> GetLight();
	shared_ptr<ParticleSystem> GetParticleSystem();
	shared_ptr<Terrain> GetTerrain();
	shared_ptr<BaseCollider> GetCollider();
	shared_ptr<Animator> GetAnimator();



	void AddComponent(shared_ptr<Component> component);
	bool GetCheckFrustum() { return _checkFrustum; }
	void SetCheckFrustum(bool checkFrustum) { _checkFrustum = checkFrustum; }

	void SetLayerIndex(uint8 layer) { _layerIndex = layer; }
	uint8 GetLayerIndex() { return _layerIndex; }

	void SetStatic(bool flag) { _static = flag; }
	bool IsStatic() { return _static; }
private:
	array<shared_ptr<Component>, FIXED_COMPONENT_COUNT> _components; //각 게임 오브젝트는 중복되는 컴포넌트를 가질 수 없다.
	vector<shared_ptr<MonoBehaviour>> _scripts; //스크립트는 몇개가 들어올지 모르기 때문에 vector

	bool _checkFrustum = true;
	uint8 _layerIndex = 0; //이 게임오브젝트가 어떤 레이어인지 저장하기 위한 인덱스
	bool _static = true;
};

