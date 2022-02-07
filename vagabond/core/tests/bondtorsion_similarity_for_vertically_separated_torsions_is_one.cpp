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

	Atom e, f, g, h;
	e.setInitialPosition(glm::vec3(2, +1, 0));
	f.setInitialPosition(glm::vec3(2,  0, 0));
	g.setInitialPosition(glm::vec3(4,  0, 0));
	h.setInitialPosition(glm::vec3(4, +1, 0));
	BondTorsion *identical = new BondTorsion(nullptr, &e, &f, &g, &h, 2);

	double similar = torsion->similarityScore(identical);
	std::cout << similar << std::endl;

	return !(fabs(similar - 1) < 1e-6);
}
