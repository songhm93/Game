#pragma once
#include "Component.h"

class Transform : public Component
{
public:
	Transform();
	virtual ~Transform();

	virtual void FinalUpdate() override;
	void PushData();

public:
	// Parent 기준
	const Vec3& GetLocalPosition() { return _localPosition; }
	const Vec3& GetLocalRotation() { return _localRotation; }
	const Vec3& GetLocalScale() { return _localScale; }

	//TEMP
	float GetBoundingSphereRadius() { return max(max(_localScale.x, _localScale.y), _localScale.z); }

	const Matrix& GetLocalToWorldMatrix() { return _matWorld; } //월드 행렬 반환
	Vec3 GetWorldPosition() { return _matWorld.Translation(); } //월드 좌표 반환. 월드 행렬을 통해 좌표를 반환함. 월드 행렬 자체가 SRT가 다 들어간 데이터라서 가능.

	Vec3 GetRight() { return _matWorld.Right(); } //월드 기준
	Vec3 GetUp() { return _matWorld.Up(); }
	Vec3 GetLook() { return _matWorld.Backward(); } //언리얼 GetForwardVector와 유사.

	void SetLocalPosition(const Vec3& position) { _localPosition = position; }
	void SetLocalRotation(const Vec3& rotation) { _localRotation = rotation; }
	void SetLocalScale(const Vec3& scale) { _localScale = scale; }

	void LookAt(const Vec3& dir);

	static bool CloseEnough(const float& a, const float& b, const float& epsilon = std::numeric_limits<float>::epsilon());
	static Vec3 DecomposeRotationMatrix(const Matrix& rotation);

public:
	void SetParent(shared_ptr<Transform> parent) { _parent = parent; }
	weak_ptr<Transform> GetParent() { return _parent; }
private:
	// TODO : World위치 관련
	// Parent 기준
	Vec3 _localPosition = {};
	Vec3 _localRotation = {};
	Vec3 _localScale = { 1.f, 1.f, 1.f };

	Matrix _matLocal = {}; //Parent 기준
	Matrix _matWorld = {};

	weak_ptr<Transform> _parent;

};

