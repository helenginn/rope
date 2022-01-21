#include "../src/Atom.h"
#include "../src/AtomGroup.h"

int main()
{
	Atom *atom = new Atom();

	AtomGroup group;
	group += atom;
	group += atom;

	return !(group.size() == 1);
}





