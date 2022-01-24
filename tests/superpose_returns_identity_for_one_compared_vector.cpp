#include "../src/Superpose.h"

int main()
{
	Superpose pose;
	glm::vec3 a = glm::vec3(1., 2., 3.);
	glm::vec3 b = glm::vec3(1., 2., 3.);
	pose.addPositionPair(a, b);
	pose.superpose();
	
	glm::mat3x3 rot = pose.rotation();
	glm::mat3x3 id = glm::mat3(1.);
	
	for (size_t i = 0; i < 3; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			if (fabs(id[i][j] - rot[i][j]) > 1e-6)
			{
				return 1;
			}
		}
	}
	
	glm::vec3 trans = pose.translation();
	for (size_t i = 0; i < 3; i++)
	{
		if (trans[i] > 1e-6)
		{
			return 1;
		}
	}

	return 0;
}
