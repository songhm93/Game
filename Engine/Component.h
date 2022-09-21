#pragma once
#include "Object.h"

enum class COMPONENT_TYPE : uint8
{
	TRANSFORM,
	MESH_RENDERER,
	CAMERA,
	LIGHT,
	PARTICLE_SYSTEM,
	TERRAIN,
	COLLIDER,
	ANIMATOR,
	// ...
	MONO_BEHAVIOUR, //얘는 마지막에 넣어줘야함. 스크립터에서 상속받는 클래스.
	END,
};

enum
{
	FIXED_COMPONENT_COUNT = static_cast<uint8>(COMPONENT_TYPE::END) - 1
};

class GameObject;
class Transform;
class MeshRenderer;
class Animator;

class Component : public Object
{
public:
	Component(COMPONENT_TYPE type);
	virtual ~Component();

public:
	virtual void Awake() { } //start이전에 초기화
	virtual void Start() { } //beginplay
	virtual void Update() { } //tick. 현재 안쓰이고 있음
	virtual void LateUpdate() { } //update 다음에 한번 더 호출되는 함수. Behaaviour에서 사용중.
	virtual void FinalUpdate() { } //진짜 마지막에 호출되는 함수. 행렬 변환 관련

public:
	COMPONENT_TYPE GetType() { return _type; }
	bool IsValid() { return _gameObject.expired() == false; } //expired -> 소멸되었는지

	shared_ptr<GameObject> GetGameObject();
	shared_ptr<Transform> GetTransform();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<Animator> GetAnimator();

private:
	friend class GameObject; //게임오브젝트에서만 setgameobject를 사용하게 하기위해 private 하고 friend해줌
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }

protected:
	COMPONENT_TYPE _type;
	weak_ptr<GameObject> _gameObject; //게임오브젝트에서도 컴포넌트 포인터를 가져야할 수 있어서 약포인터 사용
};

