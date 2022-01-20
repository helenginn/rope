#ifndef __matrix__functions__
#define __matrix__functions__

#include "matrix_functions.h"

glm::mat3x3 mat3x3_rhbasis(glm::vec3 a, glm::vec3 b)
{
	glm::normalize(a);
	glm::normalize(b);
	glm::vec3 c = cross(a, b);
	glm::normalize(c);
	c *= -1;

	glm::mat3x3 mat;

	mat[0] = a;
	mat[1] = c;
	mat[2] = b;

	return mat;
}

#endif
