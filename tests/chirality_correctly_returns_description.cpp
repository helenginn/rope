#include "../src/Chirality.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
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

	g += &centre;
	g += &a;
	g += &b;
	g += &c;
	
	Chirality *ch = new Chirality(&g, &centre, &a, &b, &c, 1);
	std::string desc = ch->desc();
	
	if (desc != "CN:A-B-C")
	{
		std::cout << "Desc incorrect, " << desc << std::endl;
		return 1;
	}

	return 0;
}
