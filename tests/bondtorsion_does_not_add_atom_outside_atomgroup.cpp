#include "../src/BondTorsion.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b, c, d;
	g += &a;
	g += &c;
	g += &d;
	
	try
	{
		new BondTorsion(&g, &a, &b, &c, &d, 2);
	}
	catch (std::runtime_error err)
	{
		return 0;
	}
	
	return 1;
}

