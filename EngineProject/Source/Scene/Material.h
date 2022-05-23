#pragma once
#include "Texture.h"

class Material
{
public:
	Material(){};
	~Material(){};

	void AddTexure(std::string TextureName,Texture* InTexture);
	
	std::string Name = "Default";

	glm::vec4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };//材质反照率
	glm::vec3 FresnelR0 = glm::vec3(0.05f, 0.05f, 0.05f);//RF(0)值，即材质的反射属性
	float Roughness = 0.2f;//材质的粗糙度

	std::unordered_map<std::string,Texture*> Textures;

	glm::vec4 BaseColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	float Metallic = 0.05f;
	float Specular = 0.0f;
};
