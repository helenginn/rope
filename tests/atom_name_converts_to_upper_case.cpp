#include "../src/Atom.h"

int main()
{
	Atom *atom = new Atom();
	std::string test = "cg1";
	
	atom->setAtomName(test);
	
	std::string result = atom->atomName();
	
	return !(result == "CG1");
}


