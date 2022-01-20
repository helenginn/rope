#include "../src/GuiAtom.h"

int main()
{
	glm::vec3 nanvec = glm::vec3(NAN, 0, 0);
	glm::vec3 infvec = glm::vec3(INFINITY, 0, 0);
	
	GuiAtom *atom = new GuiAtom();
	
	try
	{
		atom->addPosition(nanvec);
	}
	catch (std::runtime_error err)
	{
		delete atom;
		return 0;
	}
	
	try
	{
		atom->addPosition(infvec);
	}
	catch (std::runtime_error err)
	{
		delete atom;
		return 0;
	}

	
	delete atom;
	return 1;
}

