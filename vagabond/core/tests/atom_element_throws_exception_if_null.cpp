#include "../Atom.h"

int main()
{
	Atom *atom = new Atom();
	std::string test = "";
	
	try
	{
		atom->setElementSymbol(test);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}
