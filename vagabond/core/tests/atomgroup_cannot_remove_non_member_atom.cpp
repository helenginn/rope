#include "../Atom.h"
#include "../AtomGroup.h"

int main()
{
	Atom *atom = new Atom();
	Atom *other = new Atom();

	AtomGroup group;
	group += atom;
	
	group -= other;

	return !(group.size() == 1);
}




