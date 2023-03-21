#ifndef __glm__import__
#define __glm__import__

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_PURE
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <ostream>

#include "degrad.h"

inline std::ostream &operator<<(std::ostream& stream, const glm::mat4x4& a)
{
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			stream << a[i][j] << " ";
		}
		stream << std::endl;
	}
//	stream << glm::to_string(a);
	return stream;
}

inline std::ostream &operator<<(std::ostream& stream, const glm::mat3x3& a)
{
	stream << glm::to_string(a);
	return stream;
}

inline std::ostream &operator<<(std::ostream& stream, const glm::vec3& a)
{
	stream << glm::to_string(a);
	return stream;
}

inline std::ostream &operator<<(std::ostream& stream, const glm::vec4& a)
{
	stream << glm::to_string(a);
	return stream;
}

#endif

