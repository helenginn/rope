#include "../src/SimpleBasis.h"
#include "../src/BondTorsion.h"
#include "../src/Atom.h"

int main()
{
	Atom a, b, c, d;
	BondTorsion *torsion = new BondTorsion(nullptr, &a, &b, &c, &d, 2);
	torsion->setRefinedAngle(100);

	SimpleBasis sb;
	sb.addTorsion(torsion);
	sb.prepare();
	
	float custom = 0;
	float angle = sb.torsionForVector(0, &custom, 1);

	return !(fabs(angle - 100) <= 1e-6);
}
