#pragma once
#include "MathHelper.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Update();

	void SetPosition(float x, float y, float z);

	void SetLens(float fovY, float aspect, float zn, float zf);

	void Camera::LookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp);

	void Walk(float d);
	void Strafe(float d);

	void Pitch(float angle);
	void RotateZ(float angle);

	void UpdateViewMatrix();

	glm::mat4  GetView();
	glm::mat4  GetProj();

private:
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Right = { -1.0f, 0.0f, 0.0f };
	glm::vec3 Up = { 0.0f, 0.0f, 1.0f };
	glm::vec3 Look = { 0.0f, 1.0f, 0.0f };


	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float Aspect = 0.0f;
	float FovY = 0.0f;
	float NearWindowHeight = 0.0f;
	float FarWindowHeight = 0.0f;

	bool ViewDirty = true;

	glm::mat4 View;
	glm::mat4 Proj;

	float Speed=8;
};


