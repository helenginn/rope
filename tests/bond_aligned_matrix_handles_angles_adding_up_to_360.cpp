#include <iostream>
#include "../src/matrix_functions.h"

int main()
{
	glm::mat4x4 ret = glm::mat4(0.f);
	
	float lengths[3] = {1, 2, 3};
	float angles[3] = {120, 120, 120};

	ret = bond_aligned_matrix(lengths[0], lengths[1], lengths[2],
	                          angles[0], angles[1], angles[2]);

	for (size_t i = 0; i < 3; i++)
	{
		if (!is_glm_vec_sane(ret[i]))
		{
			return 1;
		}
	}

	return 0;
}
