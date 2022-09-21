#pragma once

#include "GameObject.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

class MeshData;

class Resources
{
	DECLARE_SINGLE(Resources);

public:
	void Init();

	template<typename T>
	shared_ptr<T> Load(const wstring& key, const wstring& path); //타입이 머티리얼, 메시, 셰이더 등등일 수 있어서 템플릿 사용

	template<typename T>
	bool Add(const wstring& key, shared_ptr<T> object);

	template<typename T>
	shared_ptr<T> Get(const wstring& Key);

	template<typename T>
	OBJECT_TYPE GetObjectType();

	shared_ptr<Mesh> LoadPointMesh();
	shared_ptr<Mesh> LoadRectangleMesh();
	shared_ptr<Mesh> LoadCubeMesh();
	shared_ptr<Mesh> LoadSphereMesh();
	shared_ptr<Mesh> LoadTerrainMesh(int32 sizeX = 15, int32 sizeZ = 15);

	shared_ptr<Texture> CreateTexture(const wstring& name, DXGI_FORMAT format, uint32 width, uint32 height,
		const D3D12_HEAP_PROPERTIES& heapProperty, D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE, Vec4 clearColor = Vec4());

	shared_ptr<Texture> CreateTextureFromResource(const wstring& name, ComPtr<ID3D12Resource> tex2D);
	shared_ptr<MeshData> LoadFBX(const wstring& path);

private:
	void CreateDefaultShader();
	void CreateDefaultMaterial();

private:
	using KeyObjMap = std::map<wstring/*key*/, shared_ptr<Object>>;
	array<KeyObjMap, OBJECT_TYPE_COUNT> _resources;
	//std::map<wstring, shared_ptr<Object>> StdKeyMap; 위와 같다. 가독성을 더 편하게 하기 위해 using을 사용한 것 같음.
	//array<std::map<wstring, shared_ptr<Object>>, OBJECT_TYPE_COUNT> _resources;
};

template<typename T>
inline shared_ptr<T> Resources::Load(const wstring& key, const wstring& path)
{
	OBJECT_TYPE objectType = GetObjectType<T>();
	KeyObjMap& keyObjMap = _resources[static_cast<uint8>(objectType)]; //참조로 받음.

	auto findIt = keyObjMap.find(key);
	if (findIt != keyObjMap.end()) //키가 있으면 찾은걸로 반환
		return static_pointer_cast<T>(findIt->second);

	shared_ptr<T> object = make_shared<T>(); //없으면 메모리에 만들고
	object->Load(path); //T가 Texture면은 Texture의 Load함수가 호출됨.
	keyObjMap[key] = object;

	return object;
}

template<typename T>
bool Resources::Add(const wstring& key, shared_ptr<T> object)
{
	OBJECT_TYPE objectType = GetObjectType<T>();
	KeyObjMap& keyObjMap = _resources[static_cast<uint8>(objectType)];

	auto findIt = keyObjMap.find(key);
	if (findIt != keyObjMap.end())
		return false;

	keyObjMap[key] = object;

	return true;
}

template<typename T>
shared_ptr<T> Resources::Get(const wstring& key)
{
	OBJECT_TYPE objectType = GetObjectType<T>();
	KeyObjMap& keyObjMap = _resources[static_cast<uint8>(objectType)];

	auto findIt = keyObjMap.find(key);
	if (findIt != keyObjMap.end())
		return static_pointer_cast<T>(findIt->second);

	return nullptr;
}

template<typename T>
inline OBJECT_TYPE Resources::GetObjectType()
{
	if (std::is_same_v<T, GameObject>) //여기 부분은 컴파일 타임에 연산한다.
		return OBJECT_TYPE::GAMEOBJECT; //객체를 보고 클래스 타입을 리턴하는게 아니라
	else if (std::is_same_v<T, Material>) //클래스 타입만을 보고 해당 타입을 리턴한다.
		return OBJECT_TYPE::MATERIAL; //Object클래스의 GetType함수를 쓸 수 없는 이유이다.
	else if (std::is_same_v<T, Mesh>)
		return OBJECT_TYPE::MESH;
	else if (std::is_same_v<T, Shader>)
		return OBJECT_TYPE::SHADER;
	else if (std::is_same_v<T, Texture>)
		return OBJECT_TYPE::TEXTURE;
	else if (std::is_convertible_v<T, Component>)
		return OBJECT_TYPE::COMPONENT;
	else
		return OBJECT_TYPE::NONE;
}