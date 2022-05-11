#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"


struct Float2
{
	Float2() {};
	Float2(float x, float y)
	{
		X = x;
		Y = y;
	}
	float X = 0;
	float Y = 0;
};
struct Float3
{
	Float3() {};
	Float3(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}
	float X = 0;
	float Y = 0;
	float Z = 0;
};
struct Float4
{
	Float4() {};
	Float4(float x, float y, float z, float w)
	{
		X = x;
		Y = y;
		Z = z;
		W = w;
	}
	float X = 0;
	float Y = 0;
	float Z = 0;
	float W = 0;
};



struct Vertex
{
	glm::vec3 Pos;
	glm::vec4 Color;
	glm::vec4 Normal;
	glm::vec2 UV;
};

//��ȡ�ļ����õ���������
struct MeshRead
{
	std::string MeshName;
	Float3 Location;
	Float4 Rotation;
	Float3 Scale3D;
	std::vector<Vertex> Vertices;
	std::vector<uint16_t> Indices;
};


struct ObjectConstants
{
	//glm::mat4 World_M = MathHelper::GIdentity4x4();
	glm::mat4 Location_M = MathHelper::GIdentity4x4();
	glm::mat4 Rotation_M = MathHelper::GIdentity4x4();
	glm::mat4 Scale3D_M = MathHelper::GIdentity4x4();
};

struct PassConstants
{
	glm::mat4 ViewProj_M = MathHelper::GIdentity4x4();
	float Time = 0;
};

struct MaterialConstants
{
	glm::vec4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
	float Roughness = 0.25f;
	glm::mat4 MatTransform = glm::mat4(1.0f);
};