#include "../BondAngle.h"
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
	
	BondAngle *angle = new BondAngle(&g, &a, &b, &c, 2);
	BondAngle *next = new BondAngle(&g, &b, &c, &d, 2);
	
	bool torsion = angle->formsTorsionWith(next);

	if (torsion == false)
	{
		std::cout << "Failed torsion between ABC and BCD" << std::endl;
		exit(1);
	}

	next = new BondAngle(&g, &d, &c, &b, 2);
	torsion = angle->formsTorsionWith(next);

	if (torsion == false)
	{
		std::cout << "Failed torsion between ABC and DCB" << std::endl;
		exit(1);
	}

	angle = new BondAngle(&g, &c, &b, &a, 2);
	torsion = angle->formsTorsionWith(next);

	if (torsion == false)
	{
		std::cout << "Failed torsion between CBA and DCB" << std::endl;
		exit(1);
	}

	next = new BondAngle(&g, &d, &c, &b, 2);
	torsion = angle->formsTorsionWith(next);

	if (torsion == false)
	{
		std::cout << "Failed torsion between CBA and DCB" << std::endl;
		exit(1);
	}

	return 0;
}
