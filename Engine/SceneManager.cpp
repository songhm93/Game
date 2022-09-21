#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"

#include "Engine.h"
#include "Material.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "Transform.h"
#include "Light.h"

#include "TestCameraScript.h"
#include "Resources.h"
#include "ParticleSystem.h"
#include "Terrain.h"
#include "SphereCollider.h"
#include "MeshData.h"
#include "TestDragon.h"

void SceneManager::Update()
{
	if (nullptr == _activeScene)
		return;

	_activeScene->Update(); //안쓰이고있음
	_activeScene->LateUpdate();
	_activeScene->FinalUpdate();
}

//Temp
void SceneManager::Render()
{
	if (_activeScene)
		_activeScene->Render();
}


void SceneManager::LoadScene(wstring sceneName)
{
	// TODO : 기존 Scene 정리
	// TODO : 파일에서 Scene 정보 로드

	_activeScene = LoadTestScene();

	_activeScene->Awake(); //얘네 아직 쓸모없음.
	_activeScene->Start();
}

void SceneManager::SetLayerName(uint8 index, const wstring& name)
{
	// 기존 데이터 삭제
	const wstring& prevName = _layerNames[index];
	_layerIndex.erase(prevName);

	_layerNames[index] = name;
	_layerIndex[name] = index;
}

uint8 SceneManager::LayerNameToIndex(const wstring& name)
{
	auto findIt = _layerIndex.find(name);
	if (findIt == _layerIndex.end())
		return 0;

	return findIt->second;
}

shared_ptr<GameObject> SceneManager::Pick(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GetActiveScene()->GetMainCamera();

	float width = static_cast<float>(GEngine->GetWindow().width);
	float height = static_cast<float>(GEngine->GetWindow().height);

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	// ViewSpace에서 Picking 진행
	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	auto& gameObjects = GET_SINGLE(SceneManager)->GetActiveScene()->GetGameObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetCollider() == nullptr)
			continue;

		// ViewSpace에서의 Ray 정의
		Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);

		// WorldSpace에서의 Ray 정의
		rayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
		rayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
		rayDir.Normalize();

		// WorldSpace에서의 연산
		float distance = 0.f;
		if (gameObject->GetCollider()->Intersects(rayOrigin, rayDir, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	return picked;
}

shared_ptr<Scene> SceneManager::LoadTestScene()
{
#pragma region LayerMask
	SetLayerName(0, L"Default"); //0번 레이어를 기본
	SetLayerName(1, L"UI");      //1번 레이어를 UI로 사용한다.
#pragma endregion

	shared_ptr<Scene> scene = make_shared<Scene>();

//#pragma region TestObject
//	shared_ptr<GameObject> gameObject = make_shared<GameObject>();
//
//	vector<Vertex> vec(4); //버텍스버퍼.      인덱스 버퍼를 사용하니까 접점을 줄임. 사각형.
//	vec[0].pos = Vec3(-0.5f, 0.5f, 0.5f);
//	//vec[0].color = Vec4(1.f, 0.f, 0.f, 1.f); //r,g,b,a
//	vec[0].uv = Vec2(0.f, 0.f);
//
//	vec[1].pos = Vec3(0.5f, 0.5f, 0.5f);
//	//vec[1].color = Vec4(0.f, 1.f, 0.f, 1.f);
//	vec[1].uv = Vec2(1.f, 0.f);
//
//	vec[2].pos = Vec3(0.5f, -0.5f, 0.5f);
//	//vec[2].color = Vec4(0.f, 0.f, 1.f, 1.f);
//	vec[2].uv = Vec2(1.f, 1.f);
//
//	vec[3].pos = Vec3(-0.5f, -0.5f, 0.5f);
//	//vec[3].color = Vec4(0.f, 1.f, 0.f, 1.f);
//	vec[3].uv = Vec2(0.f, 1.f);
//
//	vector<uint32> indexVec; //인덱스 버퍼.
//	{
//		indexVec.push_back(0);
//		indexVec.push_back(1);
//		indexVec.push_back(2);
//	}
//	{
//		indexVec.push_back(0);
//		indexVec.push_back(2);
//		indexVec.push_back(3);
//	}
//
//	//기본 Transform 추가해줌
//	gameObject->AddComponent(make_shared<Transform>()); //빈 게임 오브젝트를 생성했을 때 Transform은 가지고 있게.
//	shared_ptr<Transform> transform = gameObject->GetTransform();
//	transform->SetLocalPosition(Vec3(0.f, 100.f, 200.f));
//	transform->SetLocalScale(Vec3(100.f, 100.f, 1.f));
//
//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
//	{
//		shared_ptr<Mesh> mesh = make_shared<Mesh>();
//		mesh->Init(vec, indexVec);
//		meshRenderer->SetMesh(mesh);
//	}
//	{
//		shared_ptr<Shader> shader = make_shared<Shader>();
//		shared_ptr<Texture> texture = make_shared<Texture>();
//		shader->Init(L"..\\Resources\\Shader\\default.fx");
//		texture->Init(L"..\\Resources\\Texture\\crossworld.jpg");
//
//		shared_ptr<Material> material = make_shared<Material>();
//		material->SetShader(shader); //머티리얼에 셰이더랑 텍스처를 묶어준 것.
//		material->SetFloat(0, 0.1f); //머티리얼 파라미터 세팅
//		material->SetFloat(1, 0.2f);
//		material->SetFloat(2, 0.3f);
//		material->SetTexture(0, texture); //0번째, 즉 레지스터 t0에 해당하는 애한테 로드한 텍스처를 세팅
//		//mesh->SetMaterial(material); //머티리얼을 메시에 연결
//		meshRenderer->SetMaterial(material);
//	}
//
//	gameObject->AddComponent(meshRenderer);
//	scene->AddGameObject(gameObject);
//#pragma endregion

#pragma region ComputeShader
	{
		shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"ComputeShader");

		// UAV용 Texture 생성
		shared_ptr<Texture> texture = GET_SINGLE(Resources)->CreateTexture(L"UAVTexture",
			DXGI_FORMAT_R8G8B8A8_UNORM, 1024, 1024,
			CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS); //ALLOW_UNORDERED_ACCESS -> UAV에 들어갈 텍스쳐니까 UnorderedAccess로 세팅

		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"ComputeShader");
		material->SetShader(shader);
		material->SetInt(0, 1);
		GEngine->GetComputeDescHeap()->SetUAV(texture->GetUAVHandle(), UAV_REGISTER::u0);

		// 쓰레드 그룹 (1 * 1024 * 1)
		material->Dispatch(1, 1024, 1); //컴퓨트 셰이더한테 일 시킬때 사용하는 함수.스레드 그룹에서 명령 목록을 실행한다.
	}
#pragma endregion

#pragma region Main_Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->SetName(L"Main_Camera");
		camera->AddComponent(make_shared<Transform>());
		camera->AddComponent(make_shared<Camera>()); //기본 Near = 1, Far = 1000, FOV = 45도
		camera->AddComponent(make_shared<TestCameraScript>());
		camera->GetCamera()->SetFar(10000.f);
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		uint8 layerIndex = GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, true); //UI는 안 찍음. -> 0000 0000 0000 0010 앞에 4바이트만 표시하면 1번째 인덱스 비트만 켜짐.
		scene->AddGameObject(camera);
	}
#pragma endregion

#pragma region UI_Camera
	{
		shared_ptr<GameObject> camera = make_shared<GameObject>();
		camera->SetName(L"Orthographic_Camera");
		camera->AddComponent(make_shared<Transform>());
		camera->AddComponent(make_shared<Camera>()); // Near=1, Far=1000, 800*600
		camera->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
		camera->GetCamera()->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC); //이 카메라는 직교투영. Camera클래스에서 FinalUpdate부분에서 직교나 원근이냐에따라 달라짐.
		uint8 layerIndex = GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI");
		camera->GetCamera()->SetCullingMaskAll(); // 다 끄고
		camera->GetCamera()->SetCullingMaskLayerOnOff(layerIndex, false); // UI만 찍음. -> 1111 1111 1111 1101 8바이트중에 다 1로 켜져있고 1번째 인덱스 비트만 꺼짐.
		scene->AddGameObject(camera);
	}
#pragma endregion

//#pragma region Sphere
//	{
//		shared_ptr<GameObject> sphere = make_shared<GameObject>();
//		sphere->AddComponent(make_shared<Transform>());
//		sphere->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
//		sphere->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 150.f));
//		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
//		{
//			shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
//			meshRenderer->SetMesh(sphereMesh);
//		}
//		{
//			shared_ptr<Shader> shader = make_shared<Shader>();
//			shared_ptr<Texture> texture = make_shared<Texture>();
//			shader->Init(L"..\\Resources\\Shader\\default.fx");
//			texture->Init(L"..\\Resources\\Texture\\crossworld.jpg");
//			shared_ptr<Material> material = make_shared<Material>();
//			material->SetShader(shader);
//			material->SetTexture(0, texture);
//			meshRenderer->SetMaterial(material);
//		}
//		sphere->AddComponent(meshRenderer);
//		scene->AddGameObject(sphere);
//	}
//#pragma endregion

#pragma region SkyBox
	{
		shared_ptr<GameObject> skybox = make_shared<GameObject>();
		skybox->AddComponent(make_shared<Transform>());
		skybox->SetCheckFrustum(false);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
			meshRenderer->SetMesh(sphereMesh);
		}
		{
			//shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Skybox"); //Map으로 관리되고 있던거. 키값으로 가져오는것임. 밸류는 공유포인터.
			shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Sky01", L"..\\Resources\\Texture\\Sky01.jpg");
			//shader->Init(L"..\\Resources\\Shader\\skybox.fx",
			//	{ RASTERIZER_TYPE::CULL_NONE, DEPTH_STENCIL_TYPE::LESS_EQUAL }); //c++11에서 추가된 문법인데 Init을 Resources에서 해주고 있어서 주석.
			
			shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"Skybox");
			//material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		skybox->AddComponent(meshRenderer);
		scene->AddGameObject(skybox);
	}
#pragma endregion

#pragma region Object
	//{
	//	shared_ptr<GameObject> obj = make_shared<GameObject>();
	//	obj->SetName(L"OBJ");
	//	obj->AddComponent(make_shared<Transform>());
	//	obj->AddComponent(make_shared<SphereCollider>());
	//	obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
	//	obj->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 500.f));
	//	obj->SetStatic(false); //static으로 안해서 그림자적용을 받는다. 현 프로젝트는 static이면 그림자표현안하게 했음.
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	{
	//		shared_ptr<Mesh> sphereMesh = GET_SINGLE(Resources)->LoadSphereMesh();
	//		meshRenderer->SetMesh(sphereMesh);
	//	}
	//	{
	//		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
	//		//material->SetInt(0, 1); //인스턴싱 적용 g_int_0 = 1
	//		//meshRenderer->SetMaterial(material);
	//		
	//		//material->SetInt(0, 0);
	//		meshRenderer->SetMaterial(material->Clone());
	//	}
	//	dynamic_pointer_cast<SphereCollider>(obj->GetCollider())->SetRadius(0.5f);
	//	dynamic_pointer_cast<SphereCollider>(obj->GetCollider())->SetCenter(Vec3(0.f, 0.f, 0.f));
	//	obj->AddComponent(meshRenderer);
	//	scene->AddGameObject(obj);
	//}
#pragma endregion

#pragma region Terrain
	/*{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->AddComponent(make_shared<Transform>());
		obj->AddComponent(make_shared<Terrain>());
		obj->AddComponent(make_shared<MeshRenderer>());

		obj->GetTransform()->SetLocalScale(Vec3(50.f, 250.f, 50.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-100.f, -200.f, 300.f));
		obj->SetStatic(true);
		obj->GetTerrain()->Init(64, 64);
		obj->SetCheckFrustum(false);

		scene->AddGameObject(obj);
	}*/
#pragma endregion

#pragma region UI_Test
	for (int32 i = 0; i < 6; i++)
	{
		shared_ptr<GameObject> obj = make_shared<GameObject>();
		obj->SetLayerIndex(GET_SINGLE(SceneManager)->LayerNameToIndex(L"UI")); // UI. 1번인덱스 레이어. UI 카메라만 얘를 찍을것임.
		obj->AddComponent(make_shared<Transform>());
		obj->GetTransform()->SetLocalScale(Vec3(100.f, 100.f, 100.f));
		obj->GetTransform()->SetLocalPosition(Vec3(-350.f + (i * 120), 250.f, 500.f)); //near=1, far=1000으로 세팅했으니 이 사이값으로 z를 설정해줬음.
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		{
			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh(); //ui로 보일것이기 때문에 큐브형태로 만들기보다 그냥 사각형으로.
			meshRenderer->SetMesh(mesh);
		}
		{
			shared_ptr<Shader> shader = GET_SINGLE(Resources)->Get<Shader>(L"Texture");
			//shared_ptr<Texture> texture = GET_SINGLE(Resources)->Load<Texture>(L"Leather", L"..\\Resources\\Texture\\Leather.jpg");
			shared_ptr<Texture> texture;
			if (i < 3)
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::G_BUFFER)->GetRTTexture(i);
			else if (i < 5)
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::LIGHTING)->GetRTTexture(i - 3);
			else
				texture = GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SHADOW)->GetRTTexture(0);
			shared_ptr<Material> material = make_shared<Material>();
			material->SetShader(shader);
			material->SetTexture(0, texture);
			meshRenderer->SetMaterial(material);
		}
		obj->AddComponent(meshRenderer);
		scene->AddGameObject(obj);
	}
#pragma endregion


#pragma region Directional Light
	{
		shared_ptr<GameObject> light = make_shared<GameObject>();
		light->AddComponent(make_shared<Transform>());
		light->GetTransform()->SetLocalPosition(Vec3(0.f, 1000.f, 150.f));
		light->AddComponent(make_shared<Light>());
		light->GetLight()->SetLightDirection(Vec3(0, -1.f, 1.f));//y가 -1이면 위에서 아래로. z가 1이면 forwardvector. 좌표계가 언리얼로 치면
		//언리얼의 x가 여기는 z, 언리얼의 y가 여기는 x, 언리얼의 z가 여기는 y
		light->GetLight()->SetLightType(LIGHT_TYPE::DIRECTIONAL_LIGHT);
		light->GetLight()->SetDiffuse(Vec3(1.f, 1.f, 1.f));
		light->GetLight()->SetAmbient(Vec3(0.1f, 0.1f, 0.1f));
		light->GetLight()->SetSpecular(Vec3(0.1f, 0.1f, 0.1f));

		scene->AddGameObject(light);
	}
#pragma endregion

//#pragma region ParticleSystem
//	{
//		shared_ptr<GameObject> particle = make_shared<GameObject>();
//		particle->AddComponent(make_shared<Transform>());
//		particle->AddComponent(make_shared<ParticleSystem>());
//		particle->SetCheckFrustum(false);
//		particle->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 100.f));
//		scene->AddGameObject(particle);
//	}
//#pragma endregion

//#pragma region Point Light
//	{
//		shared_ptr<GameObject> light = make_shared<GameObject>();
//		light->AddComponent(make_shared<Transform>());
//		light->GetTransform()->SetLocalPosition(Vec3(0.f, 100.f, 150.f)); //로컬 포지션 설정
//		light->AddComponent(make_shared<Light>());
//		//light->GetLight()->SetLightDirection(Vec3(-1.f, -1.f, 0));
//		light->GetLight()->SetLightType(LIGHT_TYPE::POINT_LIGHT);
//		light->GetLight()->SetDiffuse(Vec3(0.0f, 0.5f, 0.0f));
//		light->GetLight()->SetAmbient(Vec3(0.0f, 0.3f, 0.0f));
//		light->GetLight()->SetSpecular(Vec3(0.0f, 0.3f, 0.0f));
//		light->GetLight()->SetLightRange(200.f); //범위
//
//		scene->AddGameObject(light);
//	}
//#pragma endregion
//
//#pragma region Spot Light
//	{
//		shared_ptr<GameObject> light = make_shared<GameObject>();
//		light->AddComponent(make_shared<Transform>());
//		light->GetTransform()->SetLocalPosition(Vec3(75.f, 0.f, 150.f));
//		light->AddComponent(make_shared<Light>());
//		light->GetLight()->SetLightDirection(Vec3(-1.f, 0, 0)); //좌측으로 쏨
//		light->GetLight()->SetLightType(LIGHT_TYPE::SPOT_LIGHT);
//		light->GetLight()->SetDiffuse(Vec3(0.0f, 0.f, 0.5f));
//		light->GetLight()->SetAmbient(Vec3(0.0f, 0.0f, 0.1f));
//		light->GetLight()->SetSpecular(Vec3(0.0f, 0.0f, 0.1f));
//		light->GetLight()->SetLightRange(200.f);
//		light->GetLight()->SetLightAngle(3.14f / 2); //90도
//
//		scene->AddGameObject(light);
//	}
//#pragma endregion

//#pragma region Tessellation Test
//	{
//		shared_ptr<GameObject> gameObject = make_shared<GameObject>();
//		gameObject->AddComponent(make_shared<Transform>());
//		gameObject->GetTransform()->SetLocalPosition(Vec3(0, 0, 300));
//		gameObject->GetTransform()->SetLocalScale(Vec3(100, 100, 100));
//		gameObject->GetTransform()->SetLocalRotation(Vec3(0, 0, 0));
//
//		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
//		{
//			shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleMesh();
//			meshRenderer->SetMesh(mesh);
//			meshRenderer->SetMaterial(GET_SINGLE(Resources)->Get<Material>(L"Tessellation"));
//		}
//		gameObject->AddComponent(meshRenderer);
//
//		scene->AddGameObject(gameObject);
//	}
//#pragma endregion

#pragma region FBX
	{
		shared_ptr<MeshData> meshData = GET_SINGLE(Resources)->LoadFBX(L"..\\Resources\\FBX\\Dragon.fbx");

		vector<shared_ptr<GameObject>> gameObjects = meshData->Instantiate();

		for (auto& gameObject : gameObjects)
		{
			gameObject->SetName(L"Dragon");
			gameObject->SetCheckFrustum(false);
			gameObject->GetTransform()->SetLocalPosition(Vec3(0.f, 0.f, 300.f));
			gameObject->GetTransform()->SetLocalScale(Vec3(1.f, 1.f, 1.f));
			scene->AddGameObject(gameObject);
			gameObject->AddComponent(make_shared<TestDragon>());
		}
	}
#pragma endregion

	return scene;
}