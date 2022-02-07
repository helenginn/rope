#include <iostream>
#include "../BondTorsion.h"
#include "../Atom.h"

int main()
{
	Atom a, b, c, d;
	a.setInitialPosition(glm::vec3(-1, +1, 0));
	b.setInitialPosition(glm::vec3(-1,  0, 0));
	c.setInitialPosition(glm::vec3(+1,  0, 0));
	d.setInitialPosition(glm::vec3(+1, +1, 0));
	BondTorsion *torsion = new BondTorsion(nullptr, &a, &b, &c, &d, 2);
	BondTorsion *identical = new BondTorsion(nullptr, &a, &b, &c, &d, 2);

	double similar = torsion->similarityScore(identical);
	std::cout << "similar = " << similar << std::endl;

	return !(fabs(similar - 1) < 1e-6);
}
