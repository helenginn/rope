#include <iostream>
#include <vagabond/core/Atom.h>
#include <vagabond/gui/GuiAtom.h>

int main()
{
	glm::vec3 start = glm::vec3(0.5, 0.5, 0.5);
	double init_b = 30;
	Atom *atom = new Atom();
	atom->setInitialPosition(start, init_b);

	GuiAtom *gui = new GuiAtom();
	gui->watchAtom(atom);
	glm::vec3 centre = gui->centroid();
	
	glm::vec3 diff = start - centre;
	double l = glm::length(diff);

	bool ok = (l < 1e-6);
	if (!ok)
	{
		std::cout << "Distance to atom: " << l << std::endl;
	}
	return !ok;
}


