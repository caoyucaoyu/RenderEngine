#pragma once
#include "Texture.h"

class Material
{
public:
	Material(){};
	~Material(){};

	void AddTexure(std::string TextureName,Texture* InTexture);
	
	std::string Name = "Default";

	glm::vec4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };//���ʷ�����
	glm::vec3 FresnelR0 = glm::vec3(0.05f, 0.05f, 0.05f);//RF(0)ֵ�������ʵķ�������
	float Roughness = 0.2f;//���ʵĴֲڶ�

	std::unordered_map<std::string,Texture*> Textures;

	glm::vec4 BaseColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	float Metallic = 0.05f;
	float Specular = 0.0f;
};
