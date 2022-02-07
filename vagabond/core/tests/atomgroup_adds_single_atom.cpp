#include "../Atom.h"
#include "../AtomGroup.h"

int main()
{
	Atom *atom = new Atom();

	AtomGroup group;
	group += atom;

	return !(group.size() == 1);
}



