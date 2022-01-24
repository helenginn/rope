#include "../src/BondAngle.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b, c;
	a.setAtomName("A");
	b.setAtomName("B");
	c.setAtomName("C");
	g += &a;
	g += &b;
	g += &c;
	
	BondAngle *angle = new BondAngle(&g, &a, &b, &c, 2);
	
	std::string description = angle->desc();
	std::string compare = "A-B-C";
	
	return !(compare == description);
}

