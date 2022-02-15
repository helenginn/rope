#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *oAnchor = atoms->firstAtomWithName("OXT");
	Atom *nAnchor = atoms->firstAtomWithName("N");

	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(oAnchor);
	sequence->addToGraph(nAnchor);

	if (sequence->blockCount() != 22)
	{
		std::cout << "Incorrect number of blocks calculated" << std::endl;
		std::cout << "Created " << sequence->blockCount() << std::endl;
		return 1;
	}

	delete atoms;

	return 0;
}

