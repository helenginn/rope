#include "../src/BondLength.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b;
	g += &a;
	g += &b;
	
	BondLength *length = new BondLength(&g, &a, &b, 2);
	
	if (a.bondLengthCount() == 0)
	{
		std::cout << "no bond lengths made!" << std::endl;
		return 1;
	}
	if (a.bondLength(0) != length || b.bondLength(0) != length)
	{
		return 1;
	}

	return 0;
}

