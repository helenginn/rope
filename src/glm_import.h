#ifndef __glm__import__
#define __glm__import__

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#define deg2rad(a) ((a)*M_PI/180)
#define rad2deg(a) ((a) / M_PI * 180)

inline glm::mat3x3 mat3x3_rhbasis(glm::vec3 a, glm::vec3 b)
{
	glm::normalize(a);
	glm::normalize(b);
	glm::vec3 c = cross(a, b);
	glm::normalize(c);

	glm::mat3x3 mat;

	mat[0] = a;
	mat[1] = c;
	mat[2] = b;

	return mat;
}

#include <sstream>
#include <iomanip>

#endif

