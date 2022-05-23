#include "stdafx.h"
#include "Material.h"


void Material::AddTexure(std::string TextureName, Texture* InTexture)
{
	Textures.insert({ TextureName,InTexture });
}
