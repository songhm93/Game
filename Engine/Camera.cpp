#include "pch.h"
#include "Camera.h"
#include "Transform.h"
#include "Scene.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Engine.h"
#include "Material.h"
#include "Shader.h"
#include "ParticleSystem.h"
#include "InstancingManager.h"

Matrix Camera::S_MatView; //static 변수 구현부. cpp에 만들어줘야 함
Matrix Camera::S_MatProjection;

Camera::Camera() : Component(COMPONENT_TYPE::CAMERA)
{
	_width = static_cast<float>(GEngine->GetWindow().width);
	_height = static_cast<float>(GEngine->GetWindow().height);
}

Camera::~Camera()
{
}

void Camera::FinalUpdate() //행렬 계산
{
	_matView = GetTransform()->GetLocalToWorldMatrix().Invert(); //월드행렬을 가져와서 역행렬을 구함. -> 뷰 행렬


	if (_type == PROJECTION_TYPE::PERSPECTIVE)
		_matProjection = ::XMMatrixPerspectiveFovLH(_fov, _width / _height, _near, _far); // width / height=> ratio(종횡비), 원근 투영 행렬을 구하는 함수.
	else //직교 투영이면은
		_matProjection = ::XMMatrixOrthographicLH(_width * _scale, _height * _scale, _near, _far); //직교 투영. width, height, near, far를 받음.


	_frustum.FinalUpdate();
}

void Camera::SortGameObject()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	const vector<shared_ptr<GameObject>>& gameObjects = scene->GetGameObjects();

	_vecForward.clear(); //이전 프레임의 물체들은 clear
	_vecDeferred.clear();
	_vecParticle.clear();

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetMeshRenderer() == nullptr && gameObject->GetParticleSystem() == nullptr)
			continue;

		if (IsCulled(gameObject->GetLayerIndex()))
			continue;

		if (gameObject->GetCheckFrustum())
		{
			if (_frustum.ContainsSphere(
				gameObject->GetTransform()->GetWorldPosition(),
				gameObject->GetTransform()->GetBoundingSphereRadius()) == false)
			{
				continue;
			}
		}
		//위에까지는 카메라 레이어에 따라 컬링, 카메라 절두체에 따라 컬링하는 부분. 아래에서 실질적으로 그려질 물체들을 벡터에 임시저장한다.
		
		if (gameObject->GetMeshRenderer())
		{
			SHADER_TYPE shaderType = gameObject->GetMeshRenderer()->GetMaterial()->GetShader()->GetShaderType();
			switch (shaderType)
			{
			case SHADER_TYPE::DEFERRED:
				_vecDeferred.push_back(gameObject);
				break;
			case SHADER_TYPE::FORWARD:
				_vecForward.push_back(gameObject);
				break;
			}
		}
		else
		{
			_vecParticle.push_back(gameObject);
		}
		
	}
}

void Camera::SortShadowObject()
{
	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
	const vector<shared_ptr<GameObject>>& gameObjects = scene->GetGameObjects();

	_vecShadow.clear();

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetMeshRenderer() == nullptr)
			continue;

		if (gameObject->IsStatic())
			continue;

		if (IsCulled(gameObject->GetLayerIndex()))
			continue;

		if (gameObject->GetCheckFrustum())
		{
			if (_frustum.ContainsSphere(
				gameObject->GetTransform()->GetWorldPosition(),
				gameObject->GetTransform()->GetBoundingSphereRadius()) == false)
			{
				continue;
			}
		}

		_vecShadow.push_back(gameObject);
	}
}

void Camera::Render_Deferred()
{
	S_MatView = _matView;
	S_MatProjection = _matProjection;

	GET_SINGLE(InstancingManager)->Render(_vecDeferred);
}

void Camera::Render_Forward()
{
	S_MatView = _matView;
	S_MatProjection = _matProjection;


	GET_SINGLE(InstancingManager)->Render(_vecForward);

	for (auto& gameObject : _vecParticle)
	{
		gameObject->GetParticleSystem()->Render();
	}
}
void Camera::Render_Shadow()
{
	S_MatView = _matView;
	S_MatProjection = _matProjection;

	for (auto& gameObject : _vecShadow)
	{
		gameObject->GetMeshRenderer()->RenderShadow();
	}
}
//void Camera::Render()
//{
//	S_MatView = _matView; //임시저장. 
//	S_MatProjection = _matProjection; //직교투영이냐, 원근투영이냐에 따라 투영행렬이 달라짐.
//
//	shared_ptr<Scene> scene = GET_SINGLE(SceneManager)->GetActiveScene();
//
//	// TODO : Layer 구분
//	const vector<shared_ptr<GameObject>>& gameObjects = scene->GetGameObjects();
//
//	for (auto& gameObject : gameObjects)
//	{
//		if (gameObject->GetMeshRenderer() == nullptr)
//			continue;
//
//		if (IsCulled(gameObject->GetLayerIndex())) //이 카메라에서 어떠한 게임오브젝트를 컬링해야하는지.
//			continue;
//
//		if (gameObject->GetCheckFrustum())
//		{
//			if (!_frustum.ContainsSphere(gameObject->GetTransform()->GetWorldPosition(), gameObject->GetTransform()->GetBoundingSphereRadius()))
//				continue;
//		}
//		//윗부분에서 절두체에서 벗어난 애들을 체크해주고 continue를 해줌으로써 아래 Render가 일어나지 않는다. IA단계 조차도 들어가지 않음.
//		gameObject->GetMeshRenderer()->Render();
//	}
//}