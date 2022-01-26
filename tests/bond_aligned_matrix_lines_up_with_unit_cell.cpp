#include "../src/matrix_functions.h"
#include "../src/Superpose.h"
#include <iostream>

int main()
{
	double a = 1.3;
	double b = 1.2;
	double c = 1.1;
	double alpha = 130;
	double beta = 120;
	double gamma = 110;
	
	glm::mat3x3 aligned = bond_aligned_matrix(a, b, c, alpha, beta, gamma);
	glm::mat3x3 unit_cell = mat3x3_from_unit_cell(a, b, c, alpha, beta, gamma);

	Superpose pose;
	for (size_t i = 0; i < 3; i++)
	{
		pose.addPositionPair(aligned[i], unit_cell[i]);
	}
	
	pose.superpose();
	
	glm::mat4x4 transform = pose.transformation();
	
	for (size_t i = 0; i < 3; i++)
	{
		glm::vec4 q = transform * glm::vec4(unit_cell[i], 1.);
		for (size_t j = 0; j < 3; j++)
		{
			float diff = q[j] - aligned[i][j];
			if (fabs(diff) > 1e-6)
			{
				std::cout << glm::to_string(transform) << std::endl;
				std::cout << glm::to_string(q) << " " << 
				glm::to_string(aligned[i]) << std::endl;
				return 1;
			}
		}
	}

	return 0;

	return 1;
}
