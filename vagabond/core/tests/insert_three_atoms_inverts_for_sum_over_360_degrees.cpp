#include <iostream>
#include "../matrix_functions.h"

int main()
{
	glm::mat4x4 ret = glm::mat4(0.f);
	glm::mat4x4 inv = glm::mat4(0.f);
	
	float lengths[3] = {1, 2, 3};
	float angles[3] = {130, 130, 130};
	float inverse[3] = {110, 110, 110};

	insert_three_atoms(ret, lengths, angles);
	insert_three_atoms(inv, lengths, inverse);
	
	for (size_t i = 0; i < 3; i++)
	{
		if (fabs(ret[i].y + inv[i].y) > 1e-6)
		{
			std::cout << "Concave three-atom organisation is not "
			"handled properly" << std::endl;
			std::cout << glm::to_string(ret) << std::endl;
			std::cout << glm::to_string(inv) << std::endl;
			return 1;
		}
	}

	return 0;
}
