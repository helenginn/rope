#include "../src/BondLength.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b, c;
	g += &a;
	g += &b;
	g += &c;
	
	BondLength *length = new BondLength(&g, &a, &b, 2);
	
	Atom *other = length->otherAtom(&b);

	return !(other == &a);
}

