#include "stdafx.h"
#include "Input.h"

POINT Input::MousePos;
bool Input::RMouseDown;

Input::Input()
{

}

Input::~Input()
{

}

void Input::OnMouseDown(WPARAM btnState, int x, int y)
{
	MousePos.x = x;
	MousePos.y = y;

	if ((btnState & MK_RBUTTON) != 0)
	{
		RMouseDown=true;
	}
	
}

void Input::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();

	if ((btnState & MK_RBUTTON) == 0)
	{
		RMouseDown = false;
	}

}

void Input::OnMouseMove(WPARAM btnState, int x, int y)
{
	MousePos.x = x;
	MousePos.y = y;

	//Theta += dx;
		//Phi += dy;
		//Phi = MathHelper::Clamp(Phi, 0.1f, MathHelper::Pi - 0.1f);
	//}

	//else if ((btnState & MK_RBUTTON) != 0)
	//{
	//	float dx = 0.005f * static_cast<float>(x - LastMousePos.x);
	//	float dy = 0.005f * static_cast<float>(y - LastMousePos.y);
	//
	//	Radius += (dx - dy) * 25;
	//	Radius = MathHelper::Clamp(Radius, 300.0f, 1000.0f);
	//}
}

void Input::OnKeyboardInput()
{

}

POINT Input::GetMousePose()
{
	return MousePos;
}

bool Input::MouseDown()
{
	return RMouseDown;
}
