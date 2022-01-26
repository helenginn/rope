#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondSequence.h"

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
	
	glm::vec3 c2_exact = sequence->positionForAtomBlock(c2_idx);

	int child = c2_idx - c1_idx - 1;
	glm::vec3 c2_by_c1 = sequence->positionForNextBlock(c1_idx, child);
	
	double c2diff = glm::length(c2_by_c1 - c2_exact);

	if (c2diff > 1e-6)
	{
		std::cout << "Child " << glm::to_string(c2_by_c1) << 
		" did not match expected location " << glm::to_string(c2_exact)
		<< std::endl;
		return 1;
	}
	
	delete chx;

	return 0;
}

