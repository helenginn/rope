#include "../src/Superpose.h"

int main()
{
	glm::vec3 a[4];
	glm::vec3 b[4];
	a[0] = glm::vec3(1., 2., 3.);
	a[1] = glm::vec3(2., 1., 3.);
	a[2] = glm::vec3(5., 1., 4.);
	a[3] = glm::vec3(-3., -3., -2.);

	glm::vec3 axis = normalize(glm::vec3(0.71, 0.21, 0.5));
	glm::mat4x4 rotate = glm::rotate(glm::mat4(1.f), 0.3f, axis);
	
	for (size_t i = 0; i < 4; i++)
	{
		rotate[i].z *= -1;
	}

	glm::mat4x4 form = glm::translate(rotate, glm::vec3(-5, -3, -2));

	Superpose pose;
	pose.forceSameHand(true);

	for (size_t i = 0; i < 4; i++)
	{
		glm::vec3 t = form * glm::vec4(a[i], 1.);
		b[i] = t;
		pose.addPositionPair(a[i], b[i]);
	}

	/* perfect match */
	pose.superpose();

	glm::mat4x4 transform = pose.transformation();
	glm::mat3x3 rotation = pose.rotation();
	glm::vec3 translation = pose.translation();
	
	bool same = true;
	for (size_t i = 0; i < 3; i++)
	{
		glm::vec4 p = transform * glm::vec4(b[i], 1.);
		for (size_t j = 0; j < 3; j++)
		{
			float diff = p[j] - a[i][j];
			if (fabs(diff) > 1e-6)
			{
				same = false;
			}
		}
	}
	
	/* if they superimpose fully, the handedness was erroneously flipped */
	return same;
}
