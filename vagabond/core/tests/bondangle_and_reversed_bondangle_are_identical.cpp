#include "../BondAngle.h"
#include "../AtomGroup.h"
#include "../Atom.h"
#include <iostream>

int main()
{
	AtomGroup g;
	Atom a, b, c;
	g += &a;
	g += &b;
	g += &c;
	
	BondAngle *angle = new BondAngle(&g, &a, &b, &c, 2);
	BondAngle *reversed = new BondAngle(&g, &c, &b, &a, 2);
	
	return !(*angle == *reversed);
}



