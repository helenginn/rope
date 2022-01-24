#include "../src/BondAngle.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b, c;
	g += &a;
	g += &b;
	g += &c;
	
	BondAngle *angle = new BondAngle(&g, &a, &b, &c, 2);
	BondAngle *reversed = new BondAngle(&g, &b, &c, &a, 2);
	
	return !(*angle != *reversed);
}

