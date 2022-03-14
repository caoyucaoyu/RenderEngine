#pragma once

struct MeshActor
{
public:
	MeshActor(){};
	MeshActor(MapItem MapMeshItem)
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

		DirectX::XMVECTORF32 g_XMIdentityR3 = { { { R.X, R.Y, R.Z, R.W } } };//auto x=DirectX::XMQuaternionIdentity();
		auto Quat = DirectX::XMMatrixRotationQuaternion(g_XMIdentityR3);

		XMStoreFloat4x4(&Location_Matrix, DirectX::XMMatrixTranslation(L.X, L.Y, L.Z));
		XMStoreFloat4x4(&Rotation_Matrix, Quat);
		XMStoreFloat4x4(&Scale3D_Matrix, DirectX::XMMatrixScaling(S.X, S.Y, S.Z));
		XMStoreFloat4x4(&World_Matrix, Quat * DirectX::XMMatrixScaling(S.X, S.Y, S.Z) * DirectX::XMMatrixTranslation(L.X, L.Y, L.Z));
	}

	Float3 Location = Float3(0, 0, 0);
	Float4 Rotation = Float4(0, 0, 0, 1);
	Float3 Scale3D = Float3(1, 1, 1);

	std::string MeshName;
	std::string MaterialName = "Default";

	DirectX::XMFLOAT4X4 World_Matrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Location_Matrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Rotation_Matrix = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Scale3D_Matrix = MathHelper::Identity4x4();
};

class Scene
{
public:
	Scene();
	~Scene();
	
	void Tick();
	void Init();

	void LoadMapActors();
	void AddMeshActor(MeshActor NewActor);

	void UpdateActorNum();

	std::vector<MeshActor> GetSceneMeshActors();
	Camera& GetMainCamera();

private:
	Camera MainCamera;
	std::vector<MeshActor> SceneMeshActors;
	int MeshActorNum=0;
};