#include "../src/matrix_functions.h"

int main()
{
	double a = 1.3;
	double b = 1.2;
	double c = 1.1;
	double alpha = 130;
	double beta = 120;
	double gamma = 110;
	
	glm::mat3x3 aligned = bond_aligned_matrix(a, b, c, alpha, beta, gamma);
	
	for (size_t i = 0; i < 2; i++)
	{
		if (fabs(aligned[0][i]) > 1e-6)
		{
			return 1;
		}
	}

	return 0;
}

