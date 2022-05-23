#pragma once

class Shader;
class Pipeline
{
public:
	Pipeline();
	~Pipeline();
	virtual Shader* GetShader();

protected:
	Shader* MShader;

};