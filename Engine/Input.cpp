#include "pch.h"
#include "Input.h"
#include "Engine.h"


void Input::Init(HWND hwnd)
{
	_hwnd = hwnd;
	_states.resize(KEY_TYPE_COUNT, KEY_STATE::NONE); //키타입 갯수만큼 NONE으로 초기화
}

void Input::Update() //매 프레임 호출됨.
{
	HWND hwnd = ::GetActiveWindow(); //윈도우 핸들을 받는 이유. A 프로그램을 선택했을 때 B 프로그램에도 입력이 들어가면 안되서.
	if (_hwnd != hwnd)
	{
		for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
			_states[key] = KEY_STATE::NONE;

		return;
	}

	BYTE asciiKeys[KEY_TYPE_COUNT] = {}; //256개짜리 바이트 배열
	if (::GetKeyboardState(asciiKeys) == false) return;

	for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
	{
		// 키가 눌려있으면 true
		if (asciiKeys[key] & 0x80) //윈도우api관련. 키가 눌려있는지 확인
		{
			KEY_STATE& state = _states[key];

			// 이전 프레임에 키를 누른 상태면 PRESS
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
				state = KEY_STATE::PRESS;
			else //이제서야 처음 누른거면 DOWN
				state = KEY_STATE::DOWN;
		}
		else //키가 안눌려져 있으면
		{
			KEY_STATE& state = _states[key];

			// 이전 프레임에 키를 누른 상태면 이제 뗐다고 하는 UP
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
				state = KEY_STATE::UP;
			else //안 누른 상태였으면 NONE
				state = KEY_STATE::NONE;
		}
	}
	::GetCursorPos(&_mousePos);
	::ScreenToClient(GEngine->GetWindow().hwnd, &_mousePos);
}


