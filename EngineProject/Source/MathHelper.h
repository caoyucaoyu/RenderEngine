#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi




class MathHelper
{
public:
	//glm::mat4(1.0f);
	static glm::mat4 GIdentity4x4()
	{
		static glm::mat4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}

	template<typename T>
	static T Clamp(const T& x,const T& low,const T& high)
	{
		return x<low? low:(x>high ? high:x);
	}

	static glm::vec3 SphericalToCartesian(float radius, float theta, float phi)
	{
		return glm::vec3( 
			//-radius * sinf(phi) * sinf(theta),
			//-radius * sinf(phi) * cosf(theta),
			//radius * cosf(phi)
			// 
			//radius * sinf(phi) * cosf(theta),
			//radius * cosf(phi),
			//radius * sinf(phi) * sinf(theta)
			// 
			radius * sinf(theta) * cosf(phi),
			radius * sinf(theta) * sinf(phi),
			radius * cosf(theta)
		);
	}

	static const float Pi;
	static const float PIDIV4;
};

const float MathHelper::Pi = 3.1415926535f;
const float MathHelper::PIDIV4 = 0.785398163;