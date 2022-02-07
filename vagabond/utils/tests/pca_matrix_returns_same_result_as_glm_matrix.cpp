#include "../svd/PCA.h"
#include "../glm_import.h"

using namespace PCA;

int main()
{
	glm::mat4x4 m = glm::rotate(glm::mat4(1.f), 0.4f, glm::vec3(1., 0., 0));
	m = glm::rotate(m, 0.4f, glm::vec3(0., 1., 0));
	m = glm::rotate(m, 0.4f, glm::vec3(0., 0., 1));
	
	glm::vec4 v = glm::vec4(2., -1., 3., 1.);

	Matrix mat;
	setupMatrix(&mat, 4, 4);
	
	for (size_t i = 0; i < 4; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			mat.ptrs[i][j] = m[i][j];
		}
	}

	double vect[4] = {v[0], v[1], v[2], v[3]};

	v = m * v;
	multMatrix(mat, vect);
	
	for (size_t i = 0; i < 4; i++)
	{
		if (fabs(vect[i] - v[i]) > 1e-6)
		{
			return 1;
		}
	}
	
	return 0;
}
