#pragma once

class Texture
{
public:
	Texture(){};
	virtual ~Texture(){};

public:
	std::string Name;
	std::wstring Filename;
};

