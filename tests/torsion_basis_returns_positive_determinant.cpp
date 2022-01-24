#include "../src/matrix_functions.h"

int main()
{
	glm::mat4x4 basis = glm::mat4(1.f);
	glm::vec4 prev = glm::vec4(-0.7, 0.7, 0., 0);
	glm::vec4 next = glm::vec4(0.7, 0, 0.7, 0.);
	
	glm::mat4x4 result = torsion_basis(basis, prev, next);

	return !(glm::determinant(result) > 0);
}
