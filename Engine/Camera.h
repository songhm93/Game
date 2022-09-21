#pragma once
#include "Component.h"
#include "Frustum.h"


enum class PROJECTION_TYPE
{
	PERSPECTIVE, //원근 투영
	ORTHOGRAPHIC, //직교 투영
};


class Camera : public Component
{
public:
	Camera();
	virtual ~Camera();

	virtual void FinalUpdate() override;

	void SetProjectionType(PROJECTION_TYPE type) { _type = type; }
	PROJECTION_TYPE GetProjectionType() { return _type; }

	void SortGameObject();
	void SortShadowObject();

	void Render_Deferred();
	void Render_Forward();
	void Render_Shadow();

	void SetCullingMaskLayerOnOff(uint8 layer, bool on) //특정 비트만 설정해서 끄거나 키는 함수. _cullingMask는 uint32고 여기서 받는 인자타입은 uint8임.
	{                                                   //내가 비트플래그에서 헷갈리는 점은_cullingMask의 비트를 직접 움직이는게 아니라, 특정 비트(layer)를 받아서
		if (on) //킴                                    //그걸 가지고 비트연산을 해서 _cullingMask의 비트가 바뀌는것이다. 헷갈리지 말도록....
			_cullingMask |= (1 << layer);               //그리고 layer는 index임.
		else //끔
			_cullingMask &= ~(1 << layer);
	}

	void SetCullingMaskAll() { SetCullingMask(UINT32_MAX); } //아무것도 찍지 않겠다. 모든 비트를 1로 채워버림.
	void SetCullingMask(uint32 mask) { _cullingMask = mask; }
	bool IsCulled(uint8 layer) { return (_cullingMask & (1 << layer)) != 0; } //해당하는 물체를 컬링해야할지 말아야할지 레이어로 판단.
	//인자인 layer는 게임오브젝트에서 설정한 LayerIndex임. 

	void SetNear(float value) { _near = value; }
	void SetFar(float value) { _far = value; }
	void SetFOV(float value) { _fov = value; }
	void SetScale(float value) { _scale = value; }
	void SetWidth(float value) { _width = value; }
	void SetHeight(float value) { _height = value; }

	Matrix& GetViewMatrix() { return _matView; }
	Matrix& GetProjectionMatrix() { return _matProjection; }
private:
	PROJECTION_TYPE _type = PROJECTION_TYPE::PERSPECTIVE;

	float _near = 1.f;
	float _far = 1000.f;
	float _fov = XM_PI / 4.f; //Field Of View
	float _scale = 1.f;
	float _width = 0.f;
	float _height = 0.f;
	
	Matrix _matView = {}; //뷰행렬
	Matrix _matProjection = {};

	Frustum _frustum;
	uint32 _cullingMask = 0;

private: //렌더하는 물체들을 임시저장.
	vector<shared_ptr<GameObject>>	_vecDeferred;
	vector<shared_ptr<GameObject>>	_vecForward;
	vector<shared_ptr<GameObject>>	_vecParticle;
	vector<shared_ptr<GameObject>>	_vecShadow;


public:
	// TEMP
	static Matrix S_MatView;
	static Matrix S_MatProjection;
};

