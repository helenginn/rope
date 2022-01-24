#include "../src/AtomGroup.h"
#include "../src/Atom.h"

int main()
{
	AtomGroup group;
	Atom a;
	group += &a;
	
	bool exists = group.hasAtom(&a);
	return !exists;
}
