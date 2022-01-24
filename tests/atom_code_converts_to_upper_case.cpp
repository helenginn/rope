#include "../src/Atom.h"

int main()
{
	Atom *atom = new Atom();
	std::string test = "asp";
	
	atom->setCode(test);
	
	std::string result = atom->code();
	
	return !(result == "ASP");
}


