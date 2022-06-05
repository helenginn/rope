#include "../matrix_functions.h"
#include <iostream>

int main()
{
	glm::vec3 a = glm::vec3(0, 0, 0);
	glm::vec3 b = glm::vec3(0, 1, 0);
	glm::vec3 c = glm::vec3(1, 1, 0);
	glm::vec3 d = glm::vec3(1, 1, 1);
	glm::vec3 e = glm::vec3(2, 1, 1);

	float dg = bond_rotation_on_distance_gradient(a, b, c, d);
	float de = bond_rotation_on_distance_gradient(a, b, c, e);
	
	if (fabs(dg - 1) > 1e-6)
	{
		std::cout << "Gradient " << dg << " should be +1" << std::endl;
		return 1;
	}

	if (de >= dg)
	{
		std::cout << "de should be lower than gradient " << dg << std::endl;
		return 1;
	}
	
	return 0;
}


