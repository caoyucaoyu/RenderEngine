#pragma once
#include "MathHelper.h"

struct LightDate
{
	glm::vec3 strength = { 0.5f, 0.5f, 0.5f };//��Դ��ɫ������ͨ�ã�
	float falloffStart = 1.0f;	//���ƺ;۹�ƵĿ�ʼ˥������
	glm::vec3 direction = { 0.0f, 0.0f, -1.0f };//�����;۹�Ƶķ�������
	float falloffEnd = 10.0f;//���;۹�Ƶ�˥����������
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };//���;۹�Ƶ�����
	float spotPower = 64.0f;//�۹�������еĲ���

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

private:
	float sunSpeed = 0.003;//��Ҫ����ʱ�������ݴ���
	float sunPhi = 0;
	float sunThe = MathHelper::Pi / 4;
	glm::vec3 sunDir;


	glm::mat4 SunLightProj;
	float Left = -30;
	float Right = 30;
	float Top = 30;
	float Bottom = -30;
	float Near = -30;
	float Far = 30;
};