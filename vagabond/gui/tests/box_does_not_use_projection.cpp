#include "../Box.h"
#include <iostream>

int main()
{
	Box *box = new Box();
	glm::mat4x4 not_identity = glm::mat4(2.);
	box->setProj(not_identity);
	box->setModel(not_identity);
	
	glm::mat4x4 proj = box->projection();
	glm::mat4x4 model = box->model();
	double det1 = glm::determinant(proj);
	double det2 = glm::determinant(model);

	delete box;
	
	bool bad = !(det1 > 0.999 && det1 < 1.001);
	bad |= !(det2 > 0.999 && det2 < 1.001);
	
	if (bad)
	{
		std::cout << "Determinant for projection: " << det1 << std::endl;
		std::cout << "Determinant for model: " << det2 << std::endl;
	}
	
	return bad;
}
