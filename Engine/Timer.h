#pragma once


class Timer
{
	DECLARE_SINGLE(Timer);
public:
	void Init();
	void Update();

	uint32 GetFPS() { return _fps; }
	float GetDeltaTime() { return _deltaTime; }

private:
	uint64	_frequency = 0;
	uint64	_prevCount = 0;
	float	_deltaTime = 0.f; //이전 프레임으로부터 경과된 시간

private: //프레임을 계산하기 위한 용도
	uint32	_frameCount = 0;
	float	_frameTime = 0.f;
	uint32	_fps = 0; //frame per second. 초당 프레임. 초마다 계산
};


