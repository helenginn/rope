#include "../src/Atom.h"

int main()
{
	glm::vec3 start = glm::vec3(0.5, 0.5, 0.5);
	double init_b = 30;
	Atom *atom = new Atom();
	atom->setInitialPosition(start, init_b);

	glm::vec3 p = atom->derivedPosition();
	float b = atom->derivedBFactor();
	
	float diff = glm::length(p - start) + b - init_b;
	
	if (fabs(diff) > 1e-6)
	{
		throw std::runtime_error("Derived positions don't match.");
	}
	
	return 0;
}
