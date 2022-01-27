#include "../src/Chirality.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	Atom centre;
	centre.setAtomName("CN");
	centre.setCode("TST");

	Atom a, b, c;
	a.setAtomName("A");
	a.setCode("TST");

	b.setAtomName("B");
	b.setCode("TST");

	c.setAtomName("C");
	c.setCode("TST");
	
	Chirality ch(nullptr, &centre, &a, &b, &c);
	Chirality other(nullptr, &centre, &b, &c, &a);
	
	return !(ch == other);
}
