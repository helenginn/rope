#include "../SimpleBasis.h"
#include "../BondTorsion.h"
#include "../Atom.h"

int main()
{
	Atom a, b, c, d;
	BondTorsion *torsion = new BondTorsion(nullptr, &a, &b, &c, &d, 2);
	torsion->setRefinedAngle(100);

	SimpleBasis sb;
	sb.addTorsion(torsion);
	sb.prepare();
	
	float custom = 10;
	float angle = sb.torsionForVector(0, &custom, 1);

	return !(fabs(angle - 110) <= 1e-6);
}
