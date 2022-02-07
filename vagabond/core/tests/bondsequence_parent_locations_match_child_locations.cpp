#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/CHX.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *chx = geom.atoms();
	Atom *c1 = chx->firstAtomWithName("C1");
	Atom *c2 = chx->firstAtomWithName("C2");
	
	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(c1, UINT_MAX);
	
	sequence->calculate();
	
	int c1_idx = sequence->firstBlockForAtom(c1);
	int c2_idx = sequence->firstBlockForAtom(c2);
	
	glm::vec3 c1_exact = sequence->positionForAtomBlock(c1_idx);

	glm::vec3 c1_by_c2 = sequence->positionForPreviousBlock(c2_idx);
	
	double c1diff = glm::length(c1_by_c2 - c1_exact);
	if (c1diff > 1e-6)
	{
		std::cout << "Parent " << glm::to_string(c1_by_c2) << 
		" did not match expected location " << glm::to_string(c1_exact)
		<< std::endl;
		return 1;
	}
	
	delete chx;

	return 0;
}

