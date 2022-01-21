#include "../src/matrix_functions.h"
#include <iostream>

int main()
{
	double a = 1;
	double b = 1;
	double c = 1;
	double alpha = 120;
	double beta = 120;
	double gamma = 120;
	
	glm::mat3x3 mat = mat3x3_from_unit_cell(a, b, c, alpha, beta, gamma);

	for (size_t i = 0; i < 3; i++)
	{
		glm::vec3 v = mat[i];
		if (!is_glm_vec_sane(v))
		{
			return 1;
		}
	}
	
	return 0;
}
