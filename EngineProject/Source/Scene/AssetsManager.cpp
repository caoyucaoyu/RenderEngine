#include "stdafx.h"
#include "AssetsManager.h"

AssetsManager::AssetsManager()
{
	Material* DefaultMaterial = new Material;
	DefaultMaterial->Name = "Default";
	AddMaterial("Default", DefaultMaterial);
}

AssetsManager::~AssetsManager()
{
	for (auto mat : MaterialAssets)
	{
		if (mat.second != nullptr)
		{
			delete mat.second;
			mat.second = nullptr;
		}
	}
	for (auto tex : TextureAssets)
	{
		if (tex.second != nullptr)
		{
			delete tex.second;
			tex.second = nullptr;
		}
	}
}

void AssetsManager::LoadMap(const std::string& filePath)
{
	ReadMapFile(filePath, MeshReadData);

	for (auto MeshItem : MeshReadData)
	{	
		if (StaticMeshAssets.count(MeshItem.MeshName))
			continue;
	
		StaticMesh AMesh(MeshItem);
		StaticMeshAssets.insert(std::make_pair(MeshItem.MeshName,AMesh));
	}
}


void AssetsManager::LoadTexture(std::string Name, const std::wstring& filePath)
{
	Texture* Tex=new Texture();
	Tex->Name = Name;
	Tex->Filename = filePath;

	AddTexture(Name,Tex);
}


void AssetsManager::CreateMaterial()
{
	Material* HeadMaterial = new Material;
	HeadMaterial->Name = "Head";
	Texture* ATexture = GetTexture("Head_diff");
	if (ATexture != nullptr)
	{
		HeadMaterial->AddTexure("Head_diff", ATexture);
		OutputDebugStringA("SS Add Head_diff Succeed\n");
	}
	ATexture = GetTexture("Head_norm");
	if (ATexture != nullptr)
	{
		HeadMaterial->AddTexure("Head_norm", ATexture);
		OutputDebugStringA("SS Add Head_norm Succeed\n");
	}
	AddMaterial("Head", HeadMaterial);
}


const std::vector<MeshRead>& AssetsManager::GetMeshReadData()
{
	return MeshReadData;
}


bool AssetsManager::FindMesh(std::string Name, StaticMesh& NMesh)
{
	if (!StaticMeshAssets.count(Name))
	{
		return false;
	}
	else
	{
		NMesh = StaticMeshAssets.at(Name);
		return true;
	}
}



Texture* AssetsManager::GetTexture(std::string Name)
{
	Texture* RTexture;
	if (!TextureAssets.count(Name))
	{
		RTexture = nullptr;
	}
	else
	{
		RTexture = TextureAssets.at(Name);
	}
	return RTexture;
}

Material* AssetsManager::GetMaterial(std::string Name)
{
	Material* RMaterial;
	if (!MaterialAssets.count(Name))
	{
		RMaterial = nullptr;
	}
	else
	{
		RMaterial = MaterialAssets.at(Name);
	}
	return RMaterial;
}

Material* AssetsManager::GetDefaultMaterial()
{
	return MaterialAssets["Default"];
}

void AssetsManager::AddTexture(std::string Name, Texture* NTexture)
{
	if (TextureAssets.count(Name) != 0)
		return;
	TextureAssets.insert({Name,NTexture});
}
void AssetsManager::AddMaterial(std::string Name, Material* NMaterial)
{
	if(MaterialAssets.count(Name)!=0)
		return;
	MaterialAssets.insert({Name,NMaterial});
}

void AssetsManager::ReadMapFile(const std::string& filePath, std::vector<MeshRead>& MeshData)
{
	std::ifstream readFile;
	readFile.open(filePath, std::ios::in | std::ios::binary);
	if (!readFile)
	{
		OutputDebugStringA("SS Error opening file When ReadFile \n");
		return;
	}
	OutputDebugStringA("SS Opening file When ReadFile \n");


	int Num;
	readFile.read((char*)&Num, sizeof(int));

	//??????Mesh????
	std::stringstream ss;
	ss << Num << "\n";

	for (int j = 0; j < Num; j++)
	{
		MeshRead Mesh;
		int32_t NameLength;
		readFile.read((char*)&NameLength, sizeof(int32_t));

		Mesh.MeshName.resize(NameLength - 1);
		readFile.read((char*)Mesh.MeshName.data(), sizeof(char) * NameLength - 1);

		char temp;
		readFile.read((char*)&temp, sizeof(char));
		ss << Mesh.MeshName.c_str() << "\n";

		//Transform
		readFile.read((char*)&Mesh.Location, sizeof(Float3));
		readFile.read((char*)&Mesh.Rotation, sizeof(Float4));
		readFile.read((char*)&Mesh.Scale3D, sizeof(Float3));
		ss << Mesh.Rotation.X << "   " << Mesh.Rotation.Y << "   " << Mesh.Rotation.Z << "   " << Mesh.Rotation.W << "\n";

		//Vertex
		int VCount;
		readFile.read((char*)&VCount, sizeof(int));

		for (int i = 0; i < VCount; i++)
		{
			Float3 VertexPosition;
			readFile.read((char*)&VertexPosition, sizeof(Float3));
			Float4 Normal;
			readFile.read((char*)&Normal, sizeof(Float4));
			Float2 Auv;
			readFile.read((char*)&Auv, sizeof(Float2));

			Vertex Vert;

			Vert.Pos.x = VertexPosition.X; Vert.Pos.y = VertexPosition.Y; Vert.Pos.z = VertexPosition.Z;
			Vert.Normal.x = Normal.X; Vert.Normal.y = Normal.Y; Vert.Normal.z = Normal.Z; Vert.Normal.w = Normal.W;
			Vert.UV.x = Auv.X; Vert.UV.y = Auv.Y;

			//ss << Vert.UV.x <<" "<< Vert.UV.y;
			//Vert.Color = DirectX::XMFLOAT4(float(i + 1) / VCount * 2, float(i + 1) / VCount, 0, 1.0f);				
			Vert.Color = glm::vec4(Normal.X, Normal.Y, Normal.Z, 1.0f);

			Mesh.Vertices.push_back(Vert);
		}
		ss << "\n";

		//Index
		int ICount;
		readFile.read((char*)&ICount, sizeof(int));
		for (int i = 0; i < ICount; i++)
		{
			int Ind;
			readFile.read((char*)&Ind, sizeof(int));
			uint16_t in = Ind;
			Mesh.Indices.push_back(in);
			//ss<<in<<" ";
		}

		MeshData.push_back(Mesh);
	}

	//OutputDebugStringA(ss.str().c_str());
	readFile.close();

}
