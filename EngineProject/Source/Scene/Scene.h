#pragma once

struct MeshActor
{
public:
	MeshActor(){};
	MeshActor(MeshRead MapMeshItem)
	{
		MeshName = MapMeshItem.MeshName;
		Location = MapMeshItem.Location;
		Rotation = MapMeshItem.Rotation;
		Scale3D = MapMeshItem.Scale3D;
		UpdateMatrix();
	}

	void UpdateMatrix() 
	{
		Float3 S = Scale3D; Float3 L = Location; Float4 R = Rotation;

		//DirectX::XMVECTORF32 g_XMIdentityR3 = { { { R.X, R.Y, R.Z, R.W } } };//auto x=DirectX::XMQuaternionIdentity();
		//auto Quat = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);//四元数得旋转矩阵
		//
		//XMStoreFloat4x4(&Location_Matrix, DirectX::XMMatrixTranslation(L.X, L.Y, L.Z));
		//XMStoreFloat4x4(&Rotation_Matrix, Quat);
		//XMStoreFloat4x4(&Scale3D_Matrix, DirectX::XMMatrixScaling(S.X, S.Y, S.Z));
		//XMStoreFloat4x4(&World_Matrix, Quat * DirectX::XMMatrixScaling(S.X, S.Y, S.Z) * DirectX::XMMatrixTranslation(L.X, L.Y, L.Z));

		glm::mat4 M = glm::mat4(1.0f);
		glm::vec3 l = { L.X,L.Y,L.Z };
		glm::vec3 s = { S.X,S.Y,S.Z };

		Location_Matrix=glm::translate(M,l);

		glm::qua<float> qrotation;
		qrotation.x = R.X;
		qrotation.y = R.Y;
		qrotation.z = R.Z;
		qrotation.w = R.W;
		Rotation_Matrix=glm::toMat4(qrotation);

		Scale3D_Matrix=glm::scale(M, s);
		World_Matrix= Location_Matrix * Rotation_Matrix * Scale3D_Matrix;
	}

public:
	std::string MeshName;
	std::string MaterialName = "Default";
	Float3 Location = Float3(0, 0, 0);
	Float4 Rotation = Float4(0, 0, 0, 1);
	Float3 Scale3D = Float3(1, 1, 1);

	glm::mat4 World_Matrix = MathHelper::GIdentity4x4();
	glm::mat4 Location_Matrix = MathHelper::GIdentity4x4();
	glm::mat4 Rotation_Matrix = MathHelper::GIdentity4x4();
	glm::mat4 Scale3D_Matrix = MathHelper::GIdentity4x4();
};

class Scene
{
public:
	Scene();
	~Scene();

	void Init();	
	void Tick();

	void LoadMapActors();
	void AddMeshActor(MeshActor NewActor);

	int GetMeshActorNum();

	std::vector<MeshActor> GetSceneMeshActors();
	Camera& GetMainCamera();

private:
	Camera MainCamera;
	std::vector<MeshActor> SceneMeshActors;
};