#pragma once
#include "MathHelper.h"

struct Light
{
	glm::vec3 strength = { 0.5f, 0.5f, 0.5f };//��Դ��ɫ������ͨ�ã�
	float falloffStart = 1.0f;	//���ƺ;۹�ƵĿ�ʼ˥������
	glm::vec3 direction = { 0.0f, 0.0f, -1.0f };//�����;۹�Ƶķ�������
	float falloffEnd = 10.0f;//���;۹�Ƶ�˥����������
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };//���;۹�Ƶ�����
	float spotPower = 64.0f;//�۹�������еĲ���

};

//class Light 
//{
//public:
//	Light();
//	~Light();
//
//
//	glm::vec3 position = { 0.0f, 0.0f, 0.0f };//���;۹�Ƶ�����
//	float falloffStart = 1.0f;	//���ƺ;۹�ƵĿ�ʼ˥������
//	glm::vec3 direction = { 0.0f, -1.0f, 0.0f };//�����;۹�Ƶķ�������
//	float falloffEnd = 10.0f;//���;۹�Ƶ�˥����������
//	glm::vec3 strength = { 0.5f, 0.5f, 0.5f };//��Դ��ɫ
//	float spotPower = 64.0f;//�۹�������еĲ���
//};