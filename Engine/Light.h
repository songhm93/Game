#pragma once
#include "Component.h"

enum class LIGHT_TYPE : uint8
{
	DIRECTIONAL_LIGHT,
	POINT_LIGHT,
	SPOT_LIGHT,
};

struct LightColor
{
	Vec4 diffuse;
	Vec4 ambient;
	Vec4 specular;
};

struct LightInfo //조명 정보
{
	LightColor color;
	Vec4 position; //포인트라이트, 스팟라이트꺼
	Vec4 direction; //디렉셔널, 스팟라이트꺼. 빛 방향
	int32 lightType;
	float range; // 빛 최대 범위
	float angle; // 빛이 쏘는 각도. 스팟라이트꺼
	int32 paddig; //아무 의미 없는 값
};

struct LightParams //셰이더한테 넘겨줄 파라미터
{
	uint32 lightCount; //빛이 몇개가 있는지
	Vec3 padding; //의미 없는 값
	LightInfo lights[50]; //라이트는 하나로 묶어서 통으로 셰이더한테 넘겨줌.
};

class Mesh;
class Material;

class Light : public Component
{
public:
	Light();
	virtual ~Light();

	virtual void FinalUpdate() override;
	void Render();
	void RenderShadow();

public:
	LIGHT_TYPE GetLightType() { return static_cast<LIGHT_TYPE>(_lightInfo.lightType); }

	const LightInfo& GetLightInfo() { return _lightInfo; }

	void SetLightDirection(Vec3 direction);

	void SetDiffuse(const Vec3& diffuse) { _lightInfo.color.diffuse = diffuse; }
	void SetAmbient(const Vec3& ambient) { _lightInfo.color.ambient = ambient; }
	void SetSpecular(const Vec3& specular) { _lightInfo.color.specular = specular; }

	void SetLightType(LIGHT_TYPE type);
	void SetLightRange(float range) { _lightInfo.range = range; }
	void SetLightAngle(float angle) { _lightInfo.angle = angle; }

	void SetLightIndex(int8 index) { _lightIndex = index; }

private:
	LightInfo _lightInfo = {};

	int8 _lightIndex = -1;
	shared_ptr<Mesh> _volumeMesh;
	shared_ptr<Material> _lightMaterial;

	shared_ptr<GameObject> _shadowCamera; //빛 위치,방향과 동일해야함. 
};

