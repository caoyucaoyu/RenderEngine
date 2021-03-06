#include "stdafx.h"
#include "Input.h"


IPOINT Input::MousePos;
std::array<KeyState, (int)Key::count> Input::AllKeyState;


Input* Input::CreateInput()
{
	Input* NInput = nullptr;
#if PLATFORM_WINDOWS
	NInput = new InputWin32;
#elif PLATFORM_IOS
	
#elif PLATFORM_ANDROID

#endif
	return NInput;
}



IPOINT Input::GetMousePose()
{
	return MousePos;
}


KeyState Input::GetKeyState(Key k)
{
	return AllKeyState[(int)k];
}







InputWin32::InputWin32()
{

}

InputWin32::~InputWin32()
{

}

void InputWin32::Update()
{
	if(AllKeyState[(int)Key::LM] == KeyState::BtnDown)
		AllKeyState[(int)Key::LM] = KeyState::BtnHold;


	if (AllKeyState[(int)Key::RM] == KeyState::BtnDown)
		AllKeyState[(int)Key::RM] = KeyState::BtnHold;


	if (GetAsyncKeyState('W') & 0x0001)
		AllKeyState[(int)Key::W] = KeyState::BtnDown;
	else if (GetAsyncKeyState('W') & 0x8000)
		AllKeyState[(int)Key::W] = KeyState::BtnHold;
	else
		AllKeyState[(int)Key::W] = KeyState::BtnUp;


	if (GetAsyncKeyState('S') & 0x0001)
		AllKeyState[(int)Key::S] = KeyState::BtnDown;
	else if (GetAsyncKeyState('S') & 0x8000)
		AllKeyState[(int)Key::S] = KeyState::BtnHold;
	else
		AllKeyState[(int)Key::S] = KeyState::BtnUp;


	if (GetAsyncKeyState('A') & 0x0001)
		AllKeyState[(int)Key::A] = KeyState::BtnDown;
	else if (GetAsyncKeyState('A') & 0x8000)
		AllKeyState[(int)Key::A] = KeyState::BtnHold;
	else
		AllKeyState[(int)Key::A] = KeyState::BtnUp;


	if (GetAsyncKeyState('D') & 0x0001)
		AllKeyState[(int)Key::D] = KeyState::BtnDown;
	else if (GetAsyncKeyState('D') & 0x8000)
		AllKeyState[(int)Key::D] = KeyState::BtnHold;
	else
		AllKeyState[(int)Key::D] = KeyState::BtnUp;
}

void InputWin32::OnMouseDown(Key key, int x, int y)
{
	MousePos.x = x;
	MousePos.y = y;
	
	AllKeyState[(int)key] = KeyState::BtnDown;
}

void InputWin32::OnMouseUp(Key key, int x, int y)
{
	AllKeyState[(int)key] = KeyState::BtnUp;
}

void InputWin32::OnMouseMove(int x, int y)
{
	MousePos.x = x;
	MousePos.y = y;
}

