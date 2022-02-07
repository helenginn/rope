#include "../Atom.h"

int main()
{
	Atom *atom = new Atom();
	
	try
	{
		atom->setAtomNum(-5);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}

