#include "../BondLength.h"
#include "../AtomGroup.h"
#include "../Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b;
	g += &a;
	g += &b;
	
	new BondLength(&g, &a, &b, 2);
	
	if (a.bondLengthCount() == 0)
	{
		std::cout << "no bond lengths made!" << std::endl;
		return 1;
	}
	if (g.bondLengthCount() == 0)
	{
		return 1;
	}

	return 0;
}


