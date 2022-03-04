#include "stdafx.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	SetLens(0.25f * MathHelper::Pi, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{

}

void Camera::SetPosition(float x, float y, float z)
{
	Position = XMFLOAT3(x, y, z);
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

	XMMATRIX P = XMMatrixPerspectiveFovLH(FovY, Aspect, NearZ, FarZ);
	XMStoreFloat4x4(&Proj, P);
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&Position, pos);
	XMStoreFloat3(&Look, L);
	XMStoreFloat3(&Right, R);
	XMStoreFloat3(&Up, U);

	ViewDirty = true;
}

void Camera::Walk(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&Look);
	XMVECTOR p = XMLoadFloat3(&Position);
	XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, l, p));

	ViewDirty = true;
}

void Camera::Strafe(float d)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&Right);
	XMVECTOR p = XMLoadFloat3(&Position);
	XMStoreFloat3(&Position, XMVectorMultiplyAdd(s, r, p));

	ViewDirty = true;
}

void Camera::Pitch(float angle)
{
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&Right), angle);

	XMStoreFloat3(&Up, XMVector3TransformNormal(XMLoadFloat3(&Up), R));
	XMStoreFloat3(&Look, XMVector3TransformNormal(XMLoadFloat3(&Look), R));

	ViewDirty = true;
}

void Camera::RotateZ(float angle)
{

	XMMATRIX R = XMMatrixRotationZ(angle);

	XMStoreFloat3(&Right, XMVector3TransformNormal(XMLoadFloat3(&Right), R));
	XMStoreFloat3(&Up,    XMVector3TransformNormal(XMLoadFloat3(&Up), R));
	XMStoreFloat3(&Look,  XMVector3TransformNormal(XMLoadFloat3(&Look), R));

	ViewDirty = true;
}

void Camera::UpdateViewMatrix()
{
	if (ViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&Right);
		XMVECTOR U = XMLoadFloat3(&Up);
		XMVECTOR L = XMLoadFloat3(&Look);
		XMVECTOR P = XMLoadFloat3(&Position);

		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		R = XMVector3Cross(U, L);

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&Right, R);
		XMStoreFloat3(&Up, U);
		XMStoreFloat3(&Look, L);

		View(0, 0) = Right.x;
		View(1, 0) = Right.y;
		View(2, 0) = Right.z;
		View(3, 0) = x;

		View(0, 1) = Up.x;
		View(1, 1) = Up.y;
		View(2, 1) = Up.z;
		View(3, 1) = y;

		View(0, 2) = Look.x;
		View(1, 2) = Look.y;
		View(2, 2) = Look.z;
		View(3, 2) = z;

		View(0, 3) = 0.0f;
		View(1, 3) = 0.0f;
		View(2, 3) = 0.0f;
		View(3, 3) = 1.0f;

		ViewDirty = false;
	}
}

DirectX::XMMATRIX Camera::GetView()
{
	return XMLoadFloat4x4(&View);
}

DirectX::XMMATRIX Camera::GetProj()
{
	return XMLoadFloat4x4(&Proj);
}
