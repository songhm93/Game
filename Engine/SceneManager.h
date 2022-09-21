#pragma once

class Scene;
class GameObject;

enum
{
	MAX_LAYER = 32 //레이어를 둬서 카메라마다 찍을 레이어를 구분한다.
};

class SceneManager
{
//private:
//	SceneManager() {}
//	~SceneManager() {}
//	//static SceneManager* _instance;
//
//public:
//	//static SceneManager* GetInstance() //싱글톤 패턴
//	//{
//	//	if (nullptr == _instance)
//	//		_instance = new SceneManager();
//	//	return _instance;
//	//}
//
//	//static SceneManager* GetInstance2() //이것도 싱글톤이라 볼 수 있음.
//	//{
//	//	static SceneManager instance;
//	//	
//	//	return &instance;
//	//} 편리하게 사용하기 위해 아래처럼 매크로로 만듬.

	DECLARE_SINGLE(SceneManager);
public:
	void Update();
	void Render();
	void LoadScene(wstring sceneName);

	void SetLayerName(uint8 index, const wstring& name);
	const wstring& IndexToLayerName(uint8 index) { return _layerNames[index]; }
	uint8 LayerNameToIndex(const wstring& name);

	shared_ptr<GameObject> Pick(int32 screenX, int32 screenY);
public:
	shared_ptr<Scene> GetActiveScene() { return _activeScene; }
private:
	shared_ptr<Scene> LoadTestScene();
private:
	shared_ptr<Scene> _activeScene;

	array<wstring, MAX_LAYER> _layerNames;
	map<wstring, uint8> _layerIndex;

};

