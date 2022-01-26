#include <iostream>
#include "../src/matrix_functions.h"

int main()
{
	glm::mat4x4 ret = glm::mat4(0.f);
	
	float lengths[3] = {1, 2, 3};
	float angles[3] = {130, 120, 150};

	insert_three_atoms(ret, lengths, angles);
	
	for (size_t i = 0; i < 3; i++)
	{
		if (!is_glm_vec_sane(ret[i]))
		{
			std::cout << "Insane matrix " << glm::to_string(ret) << std::endl;
			return 1;
		}
	}

	return 0;
}
