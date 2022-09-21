#include "pch.h"
#include "ParticleSystem.h"
#include "StructuredBuffer.h"
#include "Mesh.h"
#include "Resources.h"
#include "Transform.h"
#include "Timer.h"

ParticleSystem::ParticleSystem() : Component(COMPONENT_TYPE::PARTICLE_SYSTEM)
{
	_particleBuffer = make_shared<StructuredBuffer>();
	_particleBuffer->Init(sizeof(ParticleInfo), _maxParticle);

	_computeSharedBuffer = make_shared<StructuredBuffer>(); //공용버퍼. addCount를 여러 스레드에서 동시에 참조함.
	_computeSharedBuffer->Init(sizeof(ComputeSharedInfo), 1);

	_mesh = GET_SINGLE(Resources)->LoadPointMesh();
	_material = GET_SINGLE(Resources)->Get<Material>(L"Particle");
	shared_ptr<Texture> tex = GET_SINGLE(Resources)->Load<Texture>(
		L"Bubbles", L"..\\Resources\\Texture\\Particle\\bubble.png");

	_material->SetTexture(0, tex);

	_computeMaterial = GET_SINGLE(Resources)->Get<Material>(L"ComputeParticle");
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::FinalUpdate() //구조화버퍼를 사용하는 대략적인 순서가 딱 여기부터 Render 코드까지임.
{
	_accTime += DELTA_TIME;

	int32 add = 0;
	if (_createInterval < _accTime)
	{
		_accTime = _accTime - _createInterval;
		add = 1;
	}

	_particleBuffer->PushComputeUAVData(UAV_REGISTER::u0);
	_computeSharedBuffer->PushComputeUAVData(UAV_REGISTER::u1);

	//여기서 SetInt, SetVec하는것이 머리티얼 Params에 값을 세팅하는거.
	_computeMaterial->SetInt(0, _maxParticle); // g_int_0  : Particle Max Count
	_computeMaterial->SetInt(1, add);		   // g_int_1  : AddCount

	_computeMaterial->SetVec2(1, Vec2(DELTA_TIME, _accTime));
	_computeMaterial->SetVec4(0, Vec4(_minLifeTime, _maxLifeTime, _minSpeed, _maxSpeed));

	_computeMaterial->Dispatch(1, 1, 1);//컴퓨트 셰이더 실행. CS_Main. 여기서 연산한다.
}

void ParticleSystem::Render()
{
	GetTransform()->PushData();

	_particleBuffer->PushGraphicsData(SRV_REGISTER::t9); 
	_material->SetFloat(0, _startScale);
	_material->SetFloat(1, _endScale);
	_material->PushGraphicsData(); //이 함수에서 셰이더 업데이트(파이프라인세팅)

	_mesh->Render(_maxParticle); //u0, u1레지스터로 연산한 것을 토대로 t9에서
	//GPU가 데이터를 꺼내서 GS->PS 해서 그린다.
}
