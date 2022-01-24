#include "../src/BondTorsion.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b, c, d;
	g += &a;
	a.setElementSymbol("H");
	g += &b;
	g += &c;
	g += &d;
	
	BondTorsion *angle = new BondTorsion(&g, &a, &b, &c, &d, 2);
	
	return !(angle->isConstrained());
}
