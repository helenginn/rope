#include "../AtomGroup.h"
#include "../Atom.h"

int main()
{
	AtomGroup group;
	Atom a;
	group += &a;
	
	bool exists = group.hasAtom(&a);
	return !exists;
}
