#include "../src/BondTorsion.h"
#include "../src/AtomGroup.h"
#include "../src/Atom.h"

int main()
{
	AtomGroup g;
	Atom a, b, c, d;
	a.setAtomName("A");
	b.setAtomName("B");
	c.setAtomName("C");
	d.setAtomName("D");
	g += &a;
	g += &b;
	g += &c;
	g += &d;
	
	BondTorsion *torsion = new BondTorsion(&g, &a, &b, &c, &d, 2);
	
	std::string description = torsion->desc();
	std::string compare = "A-B-C-D";
	
	return !(compare == description);
}
