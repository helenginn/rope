#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");

	BondSequence *sequence = new BondSequence();
	sequence->setSampleCount(2);
	sequence->addToGraph(anchor);
	sequence->multiplyUpBySampleCount();
	
	int count = sequence->blockCount();
	
	if (count != 22)
	{
		std::cout << "Wrong block count for sample size of 2: " << 
		count << std::endl;
		return 1;
	}

	delete atoms;

	return 0;
}

