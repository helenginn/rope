#include "../src/matrix_functions.h"

int main()
{
	glm::mat4x4 result = glm::mat4(1.f);
	glm::vec4 self = glm::vec4(0.);
	glm::vec4 prev = glm::vec4(-0.7, 0.7, 0., 0);
	glm::vec4 next = glm::vec4(0.7, 0, 0.7, 0.);
	
	torsion_basis(result, self, prev, next);

	return !(glm::determinant(result) > 0);
}
