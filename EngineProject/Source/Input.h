#pragma once
#include "GameTimer.h"

struct IPOINT 
{
	float x;
	float y;
};

enum class KeyState
{
	None = 0,
	BtnDown = 1,
	BtnUp = 2,
	BtnHold = 4,
};

enum class Key
{
	RM,
	W,
	S,
	A,
	D,
};


class Input
{
public:
	static Input* CreateInput();

	virtual void Update()=0;
	virtual void OnMouseDown(Key key, int x, int y)=0;
	virtual void OnMouseUp(Key key, int x, int y)=0;
	virtual void OnMouseMove(int x, int y)=0;
	

	static IPOINT GetMousePose();
	static KeyState GetKeyState(Key k);
protected:
	static IPOINT MousePos;
	static std::array<KeyState,5> AllKeyState;
};



class InputWin32 : public Input
{
public:
	InputWin32();
	~InputWin32();

	virtual void Update()override;
	virtual void OnMouseDown(Key key, int x, int y)override;
	virtual void OnMouseUp(Key key, int x, int y)override;
	virtual void OnMouseMove(int x, int y) override;
	
};

