#include "../src/GuiAtom.h"

int main()
{
	GuiAtom *atom = new GuiAtom();
	size_t v = atom->verticesPerAtom();
	
	glm::vec3 pos = glm::vec3(0.5, 0.5, 0.5);
	atom->addPosition(pos);
	glm::vec3 centre = atom->centroid();
	
	glm::vec3 diff = centre - pos;
	double length = glm::length(diff);
	
	bool ok = (length < 1e-6);
	return !ok;
}

