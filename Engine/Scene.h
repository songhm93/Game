#pragma once

class GameObject;
class Camera;
class Light;

class Scene
{
public:
	void Awake();
	void Start();
	void Update();
	void LateUpdate();
	void FinalUpdate();

	shared_ptr<Camera> GetMainCamera();

	void Render();

	void ClearRTV();

	void RenderShadow();
	void RenderDeferred();
	void RenderLights();
	void RenderFinal(); //최종 결과물 합치는

	void RenderForward();

private:
	void PushLightData();
public:

	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);
	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }

private:
	vector<shared_ptr<GameObject>> _gameObjects;
	vector<shared_ptr<Camera>> _cameras;
	vector<shared_ptr<Light>> _lights;
};

