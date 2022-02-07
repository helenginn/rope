#include "../BondTorsion.h"
#include "../AtomGroup.h"
#include "../Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b, c, d;
	g += &a;
	g += &b;
	g += &c;
	g += &d;
	
	BondTorsion *angle = new BondTorsion(&g, &a, &b, &c, &d, 2);
	BondTorsion *permuted = new BondTorsion(&g, &d, &b, &c, &a, 2);
	
	return !(*angle != *permuted);
}

