#pragma once

enum class KEY_TYPE
{
	UP = VK_UP, //VK_ 시리즈는 이미 정의 되어있음
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	
	Q = 'Q',
	E = 'E',
	Z = 'Z',
	C = 'C',

	KEY_1 = '1',
	KEY_2 = '2',
	KEY_3 = '3',
	KEY_4 = '4',

	LBUTTON = VK_LBUTTON,
	RBUTTON = VK_RBUTTON,
};

enum class KEY_STATE
{
	NONE,
	PRESS,
	DOWN,
	UP,
	END
};

enum
{
	KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX + 1),
	KEY_STATE_COUNT = static_cast<int32>(KEY_STATE::END),
};


//사용자로부터 입력을 받는 클래스
class Input
{
	DECLARE_SINGLE(Input);
public:
	void Init(HWND hwnd);
	void Update();

	// 누르고 있을 때
	bool GetButton(KEY_TYPE key) { return GetState(key) == KEY_STATE::PRESS; } //매프레임 누르고 있을 때 움직이게 하려면 이 함수 활용. 누른채로 한 프레임 지나면.
	// 맨처음 눌렀을 때
	bool GetButtonDown(KEY_TYPE key) { return GetState(key) == KEY_STATE::DOWN; } //IE_Pressed와 같음
	// 맨 처음 눌렀다 뗐을 때
	bool GetButtonUp(KEY_TYPE key) { return GetState(key) == KEY_STATE::UP; } //IE_Released와 같음

	const POINT& GetMousePos() { return _mousePos; }
private:
	inline KEY_STATE GetState(KEY_TYPE key) { return _states[static_cast<uint8>(key)]; }

private:
	HWND _hwnd;
	vector<KEY_STATE> _states; //각 키에 해당하는 상태를 벡터로 관리
	POINT _mousePos = {};
};

