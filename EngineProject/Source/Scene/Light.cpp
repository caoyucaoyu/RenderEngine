#include "stdafx.h"
#include "Light.h"

Light::Light()
{
	UpdateSunLightProjection();
}

Light::~Light()
{

}

void Light::Update()
{
	UpdateSunLight(sunSpeed);
}

void Light::UpdateSunLight(float dt)
{
	//const float dt = Engine::Get()->GetTimer()->DeltaTime();

	//写Input里去
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		sunPhi -= 1.0f * dt;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		sunPhi += 1.0f * dt;
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		sunThe -= 1.0f * dt;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		sunThe += 1.0f * dt;

	sunThe = MathHelper::Clamp(sunThe, 0.01f, MathHelper::Pi * 0.5f);	

	//更新位置方向
	Position = MathHelper::SphericalToCartesian(1.0f, sunThe, sunPhi);
	sunDir = -Position;

	SunLightView = glm::lookAtLH(Position, Position + sunDir, sunUp);

	SunLightPositionMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));

}

void Light::UpdateSunLightProjection()
{
	SunLightProj = glm::orthoLH_ZO(Left, Right, Bottom, Top, Near, Far);
}

glm::mat4 Light::GetVP_N()
{
	glm::mat4 T = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	glm::mat4 VP = T * SunLightProj * SunLightView;

	return VP;

}
