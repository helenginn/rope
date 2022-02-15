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

	glm::mat3x3 rot = glm::mat3x3(glm::rotate(glm::mat4(1.f), (float)deg2rad(60),
	                                          glm::vec3(0., 1., 0.)));

	Atom e, f, g, h;
	e.setInitialPosition(rot * glm::vec3(-1, +1, 0));
	f.setInitialPosition(rot * glm::vec3(-1,  0, 0));
	g.setInitialPosition(rot * glm::vec3(+1,  0, 0));
	h.setInitialPosition(rot * glm::vec3(+1, +1, 0));
	BondTorsion *identical = new BondTorsion(nullptr, &e, &f, &g, &h, 2);

	double similar = torsion->similarityScore(identical);
	std::cout << similar << std::endl;

	return !(fabs(similar + 0.5) < 1e-6);
}
