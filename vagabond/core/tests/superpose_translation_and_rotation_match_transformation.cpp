#include "../Superpose.h"

int main()
{
	Superpose pose;
	glm::vec3 a[3], b[3];
	a[0] = glm::vec3(1., 2., 3.);
	a[1] = glm::vec3(2., 1., 3.);
	a[2] = glm::vec3(5., 1., 4.);

	b[0] = glm::vec3(4, 1., -3.);
	b[1] = glm::vec3(1., 4., -3.);
	b[2] = glm::vec3(1., 2., -4.);

	pose.addPositionPair(a[0], b[0]);
	pose.addPositionPair(a[1], b[1]);
	pose.addPositionPair(a[2], b[2]);
	/* imperfect match */
	pose.superpose();

	glm::mat4x4 transform = pose.transformation();
	glm::mat3x3 rotation = pose.rotation();
	glm::vec3 translation = pose.translation();
	
	for (size_t i = 0; i < 3; i++)
	{
		glm::vec4 p = transform * glm::vec4(b[i], 1.);
		glm::vec3 q = rotation * b[i] + translation;
		for (size_t j = 0; j < 3; j++)
		{
			float diff = q[j] - p[j];
			if (fabs(diff) > 1e-6)
			{
				return 1;
			}
		}
	}
}
