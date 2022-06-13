#pragma once
#include "MathHelper.h"

struct LightDate
{
	glm::vec3 strength = { 0.5f, 0.5f, 0.5f };//光源颜色（三光通用）
	float falloffStart = 1.0f;	//点光灯和聚光灯的开始衰减距离
	glm::vec3 direction = { 0.0f, 0.0f, -1.0f };//方向光和聚光灯的方向向量
	float falloffEnd = 10.0f;//点光和聚光灯的衰减结束距离
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };//点光和聚光灯的坐标
	float spotPower = 64.0f;//聚光灯因子中的参数

};


class Light
{
public:
	Light();
	~Light();

	void Update();
	void UpdateSunLight(float dt);
	void UpdateSunLightProjection();

	glm::vec3 GetSunLightDir() { return sunDir; }
	glm::mat4 GetProj() { return SunLightProj; }
	glm::mat4 GetView() { return SunLightView; }
	glm::mat4 GetPosM() { return SunLightPositionMatrix; }
	glm::vec3 GetPos() { return Position; }
	glm::mat4 GetVP_N();

private:
	float sunSpeed = 0.003;//需要引擎时间类数据代替
	float sunPhi = 0;
	float sunThe = MathHelper::Pi / 4;

	glm::vec3 sunUp = { 0.0f, 0.0f, 1.0f };
	glm::vec3 sunDir = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };

	glm::mat4 SunLightView;
	glm::mat4 SunLightProj;
	glm::mat4 SunLightPositionMatrix;

	float Left = -2300;
	float Right = 2300;
	float Top = 2300;
	float Bottom = -2300;
	float Near = -2300;
	float Far = 2300;
};