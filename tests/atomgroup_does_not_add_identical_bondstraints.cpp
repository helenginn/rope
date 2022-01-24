#include "../src/BondLength.h"
#include "../src/Atom.h"
#include "../src/AtomGroup.h"

int main()
{
	AtomGroup g;
	Atom a, b;
	g += &a;
	g += &b;
	
	new BondLength(&g, &a, &b, 2);
	new BondLength(&g, &b, &a, 2);

	return !(g.bondLengthCount() == 1);
}
