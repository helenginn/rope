#include <iostream>
#include "../src/Atom.h"
#include "../src/GuiAtom.h"

int main()
{
	glm::vec3 start = glm::vec3(0.5, 0.5, 0.5);
	double init_b = 30;
	Atom *atom = new Atom();
	atom->setInitialPosition(start, init_b);

	GuiAtom *gui = new GuiAtom();
	gui->watchAtom(atom);
	glm::vec3 old = gui->centroid();
	
	glm::vec3 update = glm::vec3(2.5, 2.5, 2.5);
	atom->setDerivedPosition(update);

	gui->checkAtoms();
	glm::vec3 now = gui->centroid();
	
	glm::vec3 diff = now - old;
	
	for (size_t i = 0; i < 3; i++)
	{
		if (fabs(diff[i] - 2.) > 1e-6)
		{
			return 1;
		}
	}

	return 0;
}


