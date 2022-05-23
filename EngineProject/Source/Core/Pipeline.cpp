#include "stdafx.h"
#include "Pipeline.h"

Pipeline::Pipeline()
{

}

Pipeline::~Pipeline()
{

}

Shader* Pipeline::GetShader()
{
	return MShader;
}
