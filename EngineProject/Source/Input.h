#pragma once
#include "GameTimer.h"

class Input
{
public:
	Input();
	~Input();

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	virtual void OnKeyboardInput();

	static POINT GetMousePose();
	static bool MouseDown();

private:
	static POINT MousePos;
	static bool RMouseDown;
};



