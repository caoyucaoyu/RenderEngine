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

	//–¥Input¿Ô»•
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		sunPhi -= 1.0f * dt;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		sunPhi += 1.0f * dt;
	if (GetAsyncKeyState(VK_UP) & 0x8000)
		sunThe -= 1.0f * dt;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		sunThe += 1.0f * dt;

	sunThe = MathHelper::Clamp(sunThe, 0.01f, MathHelper::Pi * 0.55f);
	sunDir = -MathHelper::SphericalToCartesian(1.0f, sunThe, sunPhi);
	
}

void Light::UpdateSunLightProjection()
{
	SunLightProj = glm::orthoLH_ZO(Left, Right, Bottom, Top, Near, Far);
}
