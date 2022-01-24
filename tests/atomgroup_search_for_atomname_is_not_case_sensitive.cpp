#include "../src/AtomGroup.h"

int main()
{
	Atom a;
	a.setAtomName("oxt");
	Atom b;
	b.setAtomName("OXT");
	
	AtomGroup ab;
	ab += &a;
	ab += &b;
	
	int count = ab.atomsWithName("OXT").size();
	count += ab.atomsWithName("oxt").size();

	return !(count == 4);
}
