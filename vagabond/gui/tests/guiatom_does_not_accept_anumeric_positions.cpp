#include "../GuiAtom.h"
#include <vagabond/core/Atom.h>

int main()
{
	glm::vec3 nanvec = glm::vec3(NAN, 0, 0);
	glm::vec3 infvec = glm::vec3(INFINITY, 0, 0);
	const double init_b = 30;
	
	GuiAtom *gui = new GuiAtom();
	
	try
	{
		Atom *atom = new Atom();
		atom->setInitialPosition(nanvec, init_b);
		gui->watchAtom(atom);
	}
	catch (std::runtime_error err)
	{
		delete gui;
		return 0;
	}
	
	try
	{
		Atom *atom = new Atom();
		atom->setInitialPosition(infvec, init_b);
		gui->watchAtom(atom);
	}
	catch (std::runtime_error err)
	{
		delete gui;
		return 0;
	}

	
	delete gui;
	return 1;
}

