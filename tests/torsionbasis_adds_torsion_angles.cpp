#include "../src/SimpleBasis.h"
#include "../src/BondTorsion.h"
#include "../src/Atom.h"

int main()
{
	Atom a, b, c, d;
	BondTorsion *angle = new BondTorsion(nullptr, &a, &b, &c, &d, 2);

	SimpleBasis sb;
	sb.addTorsion(angle);
	size_t count = sb.torsionCount();

	return !(count == 1);
}
