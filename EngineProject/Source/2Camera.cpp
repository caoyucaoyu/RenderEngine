#include "stdafx.h"
#include "Camera.h"
#include "Input.h"
using namespace DirectX;

Camera::Camera()
{
	SetLens(0.25f * MathHelper::Pi, 16.0f/9.0f, 1.0f, 10000.0f);
}

Camera::~Camera()
{

}

void Camera::Update()
{	
	static float Lastx = 0;
	static float Lasty = 0;

	if (Input::GetKeyState(Key::RM)==KeyState::BtnHold)
	{
		float dx = glm::radians(0.15f * static_cast<float>(Input::GetMousePose().x - Lastx));
		float dy = glm::radians(0.15f * static_cast<float>(Input::GetMousePose().y - Lasty));
		Pitch(dy);
		RotateZ(dx);
	}

	Lastx= Input::GetMousePose().x;
	Lasty= Input::GetMousePose().y;


	if(Input::GetKeyState(Key::W)==KeyState::BtnHold)
		Walk(Speed);

	if (Input::GetKeyState(Key::S) == KeyState::BtnHold)
		Walk(-Speed);

	if (Input::GetKeyState(Key::A) == KeyState::BtnHold)
		Strafe(-Speed);

	if (Input::GetKeyState(Key::D) == KeyState::BtnHold)
		Strafe(Speed);


	UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
	Position = glm::vec3(x, y, z);
	ViewDirty = true;
}

//--------------------------
void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	FovY = fovY;
	Aspect = aspect;
	NearZ = zn;
	FarZ = zf;

	NearWindowHeight = 2.0f * NearZ * tanf(0.5f * FovY);
	FarWindowHeight = 2.0f * FarZ * tanf(0.5f * FovY);

	Proj = glm::perspectiveFovLH(FovY,1.f,1.f,zn,zf);/////////////////////////

}

void Camera::LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp)
{
	glm::vec3 L = glm::normalize(target-pos);//XMVectorSubtract
	glm::vec3 R = glm::normalize(glm::cross(worldUp, L));
	glm::vec3 U = glm::cross(L, R);

	Position = pos;
	Look = L;
	Right = R;
	Up = U;

	ViewDirty = true;
}

void Camera::Walk(float d)
{
	glm::vec3 s = glm::vec3(d,d,d);//XMVectorReplicate(d);
	glm::vec3 l = Look;
	glm::vec3 p = Position;
	Position = s*l+p;//XMVectorMultiplyAdd(s, l, p));

	ViewDirty = true;
}

void Camera::Strafe(float d)
{
	glm::vec3 s = glm::vec3(d, d, d);//XMVectorReplicate(d);
	glm::vec3 r = Right;
	glm::vec3 p = Position;
	Position = s * r + p;//XMVectorMultiplyAdd(s, l, p));

	ViewDirty = true;
}
glm::vec3 VectorMultiplyAdd(glm::vec3 MultiplyV1, glm::vec3 MultiplyV2, glm::vec3 addV) {
	glm::vec3 result;
	result.x = MultiplyV1.x * MultiplyV2.x + addV.x;
	result.y = MultiplyV1.y * MultiplyV2.y + addV.y;
	result.z = MultiplyV1.z * MultiplyV2.z + addV.z;

	return result;
}
glm::vec3 Transform(glm::mat4x4 m, glm::vec3 v) {
	glm::vec4 Z = { v.z,v.z,v.z,v.z };
	glm::vec4 Y = { v.y,v.y,v.y,v.y };
	glm::vec4 X = { v.x,v.x,v.x,v.x };

	glm::vec3 Result = { Z.x * m[2].x,Z.y * m[2].y,Z.z * m[2].z };

	Result = VectorMultiplyAdd(Y, m[1], Result);
	Result = VectorMultiplyAdd(X, m[0], Result);

	return Result;
}

void Camera::Pitch(float angle)
{
	glm::mat4 R = glm::mat4(1.0f);//XMMatrixRotationAxis(XMLoadFloat3(&Right), angle);

	R = glm::rotate(R,angle,m)£»

	//Up = XMVector3TransformNormal(XMLoadFloat3(&Up), R));
	Up = glm::normalize(Transform(R, Up));

	Look=glm::normalize(Transform(R,Look));


	ViewDirty = true;
}

void Camera::RotateZ(float angle)
{

	XMMATRIX R = XMMatrixRotationZ(angle);

	Right = XMVector3TransformNormal(XMLoadFloat3(&Right), R));
	Up = XMVector3TransformNormal(XMLoadFloat3(&Up), R));
	Look =  XMVector3TransformNormal(XMLoadFloat3(&Look), R));

	ViewDirty = true;
}

void Camera::UpdateViewMatrix()
{
	
	if (ViewDirty)
	{
		glm::vec3 R = Right;
		glm::vec3 U = Up;
		glm::vec3 L = Look;
		glm::vec3 P = Position;

		L = glm::normalize(L);
		U = glm::normalize(glm::cross(L, R));
		R = glm::cross(U, L);


		float x = -glm::dot(P, R);
		float y = -glm::dot(P, U);
		float z = -glm::dot(P, L);
		
		float x = -glm::dot(P, R);
		float y = -glm::dot(P, U);
		float z = -glm::dot(P, L);

		Right = R;
		Up = U;
		Look = L;

		View[0][0] = Right.x;
		View[1][0] = Right.y;

		View[1][0] = Right.y;
		View[2][0] = Right.z;
		View[3][0] = x;

		View[0][1] = Up.x;
		View[1][1] = Up.y;
		View[2][1] = Up.z;
		View[3][1] = y;

		View[0][2] = Look.x;
		View[1][2] = Look.y;
		View[2][2] = Look.z;
		View[3][2] = z;

		View[0][3] = 0.0f;
		View[1][3] = 0.0f;
		View[2][3] = 0.0f;
		View[3][3] = 1.0f;

		ViewDirty = false;
	}
}

glm::mat4 Camera::GetView()
{
	return View;
}

glm::mat4 Camera::GetProj()
{
	return Proj;
}
