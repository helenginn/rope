#include "../src/matrix_functions.h"
#include <iostream>

int main()
{
	glm::vec3 v1 = glm::vec3(0.5, 1, 0);
	glm::vec3 v2 = glm::vec3(1.0, 0.5, 0);
	
	glm::mat3x3 m = mat3x3_rhbasis(v1, v2);
	float det = glm::determinant(m);
	
	bool bad = (det < 0);
	
	if (bad)
	{
		std::cout << "Determinant: " << det << std::endl;
	}

	return bad;
}
