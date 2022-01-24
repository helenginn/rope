#include "../src/BondAngle.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b, c;
	g += &a;
	g += &b;
	g += &c;
	
	new BondAngle(&g, &a, &b, &c, 2);
	
	if (a.bondAngleCount() == 0)
	{
		std::cout << "no bond angles made!" << std::endl;
		return 1;
	}
	if (g.bondAngleCount() == 0)
	{
		std::cout << "no bond angles added!" << std::endl;
		return 1;
	}

	return 0;
}


