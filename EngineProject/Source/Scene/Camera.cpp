#include "stdafx.h"
#include "Camera.h"
#include "Input.h"

//using namespace DirectX;

Camera::Camera()
{
	SetLens(0.25f * MathHelper::Pi, 16.0f/9.0f, 1.0f, 50000.0f);
}

Camera::~Camera()
{

}

void Camera::Update()
{	

	static float Lastx = 0;
	static float Lasty = 0;

	if (Input::GetKeyState(Key::RM) == KeyState::BtnHold|| Input::GetKeyState(Key::RM) == KeyState::BtnDown)
	{
		float dx = glm::radians(0.15f * static_cast<float>(Input::GetMousePose().x - Lastx));
		float dy = glm::radians(0.15f * static_cast<float>(Input::GetMousePose().y - Lasty));
		Pitch(dy);
		RotateZ(dx);
	}

	Lastx = Input::GetMousePose().x;
	Lasty = Input::GetMousePose().y;

	if (Input::GetKeyState(Key::W) == KeyState::BtnHold || Input::GetKeyState(Key::W) == KeyState::BtnDown)
		Walk(Speed);

	if (Input::GetKeyState(Key::S) == KeyState::BtnHold || Input::GetKeyState(Key::S) == KeyState::BtnDown)
		Walk(-Speed);

	if (Input::GetKeyState(Key::A) == KeyState::BtnHold || Input::GetKeyState(Key::A) == KeyState::BtnDown)
		Strafe(-Speed);

	if (Input::GetKeyState(Key::D) == KeyState::BtnHold || Input::GetKeyState(Key::D) == KeyState::BtnDown)
		Strafe(Speed);

	UpdatePositionMatrix();
	UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
	Position = glm::vec3(x,y,z);
	ViewDirty = true;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	FovY = fovY;
	Aspect = aspect;
	NearZ = zn;
	FarZ = zf;

	NearWindowHeight = 2.0f * NearZ * tanf(0.5f * FovY);
	FarWindowHeight = 2.0f * FarZ * tanf(0.5f * FovY);

	Proj= glm::perspectiveLH_ZO(FovY, Aspect, NearZ, FarZ);
}

void Camera::LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp)
{
	Look =glm::normalize(target - pos) ;
	Right = glm::normalize(glm::cross(worldUp , Look));
	Up = glm::cross(Look,Right);
	Position=pos;

	ViewDirty = true;
}

void Camera::Walk(float d)
{
	glm::vec3 s = { d,d,d };
	glm::vec3 l = Look;
	glm::vec3 p = Position;
	Position = s * l + p;

	ViewDirty = true;
}

void Camera::Strafe(float d)
{
	glm::vec3 s = glm::vec3(d, d, d);
	glm::vec3 r = Right;
	glm::vec3 p = Position;
	Position = s * r + p;

	ViewDirty = true;
}

void Camera::Pitch(float angle)
{
	auto R = glm::mat4(1.0f);
	R = glm::rotate(R, angle, Right);

	Up = glm::normalize(Transform(R, Up));
	Look = glm::normalize(Transform(R, Look));
	Right = glm::normalize(Transform(R, Right));

	ViewDirty = true;
}

void Camera::RotateZ(float angle)
{ 
	auto R = glm::mat4(1.0f);
	R = glm::rotate(R, angle, glm::vec3(0,0,1));

	Right = glm::normalize(Transform(R, Right));
	Up = glm::normalize(Transform(R, Up));
	Look = glm::normalize(Transform(R, Look));

	ViewDirty = true;
}



void Camera::UpdateViewMatrix()
{
	if (ViewDirty) 
	{	
		View=glm::lookAtLH(Position, Position+Look,Up);

		ViewDirty = false;
	}
}


void Camera::UpdatePositionMatrix()
{
	glm::mat4 M = glm::mat4(1.0f);
	glm::vec3 l = { Position.x,Position.y,Position.z };

	Position_Matrix = glm::translate(M, l);
}

glm::mat4 Camera::GetView()
{
	return View;
}

glm::mat4 Camera::GetProj()
{
	return Proj;
}


glm::mat4 Camera::GetPosM()
{
	return Position_Matrix;
}

glm::vec3 VectorMultiplyAdd(glm::vec3 MultiplyV1, glm::vec3 MultiplyV2, glm::vec3 addV) {
	glm::vec3 result;
	result.x = MultiplyV1.x * MultiplyV2.x + addV.x;
	result.y = MultiplyV1.y * MultiplyV2.y + addV.y;
	result.z = MultiplyV1.z * MultiplyV2.z + addV.z;

	return result;
}

glm::vec3 Camera::Transform(glm::mat4x4 m, glm::vec3 v) {
	glm::vec4 Z = { v.z,v.z,v.z,v.z };
	glm::vec4 Y = { v.y,v.y,v.y,v.y };
	glm::vec4 X = { v.x,v.x,v.x,v.x };

	glm::vec3 Result = { Z.x * m[2].x,Z.y * m[2].y,Z.z * m[2].z };

	Result = VectorMultiplyAdd(Y, m[1], Result);
	Result = VectorMultiplyAdd(X, m[0], Result);

	return Result;
}