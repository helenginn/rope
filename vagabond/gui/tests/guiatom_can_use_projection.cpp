#include <vagabond/gui/GuiAtom.h>
#include <iostream>

int main()
{
	GuiAtom *atom = new GuiAtom();
	glm::mat4x4 not_identity = glm::mat4(2.);
	atom->setProj(not_identity);
	atom->setModel(not_identity);
	
	glm::mat4x4 proj = atom->projection();
	glm::mat4x4 model = atom->model();
	double det1 = glm::determinant(proj);
	double det2 = glm::determinant(model);

	delete atom;
	
	bool bad = (det1 > 0.999 && det1 < 1.001);
	bad |= (det2 > 0.999 && det2 < 1.001);
	
	if (bad)
	{
		std::cout << "Determinant for projection: " << det1 << std::endl;
		std::cout << "Determinant for model: " << det2 << std::endl;
	}
	
	return bad;
}

