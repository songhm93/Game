#include "pch.h"
#include "Timer.h"

void Timer::Init()
{
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&_frequency)); //현재의 시간을 구함.timeGetTime()으로 구하는 tick보다 1000분의 1 정도 더 정확하다.
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_prevCount)); // CPU 클럭과 관련. Counter를 계산. 타이머임.
}

void Timer::Update()
{
	uint64 currentCount;
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentCount)); // CPU 클럭과 관련. Counter를 계산. 타이머임.

	_deltaTime = (currentCount - _prevCount) / static_cast<float>(_frequency); //초단위로 나옴. 현재카운트 - 지난카운트 / 지난시간?
	_prevCount = currentCount;

	_frameCount++; //fps를 구하기 위해 프레임 수 카운팅
	_frameTime += _deltaTime; // 누적 시간

	if (_frameTime > 1.f) //1초 지나면
	{
		_fps = static_cast<uint32>(_frameCount / _frameTime); //1초에 몇프레임인지 계산

		_frameTime = 0.f; //0으로 초기화
		_frameCount = 0;
	}
}