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
	Atom *anchor = atoms->firstAtomWithName("OXT");

	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(anchor);

	if (sequence->blockCount() != 11)
	{
		std::cout << "Incorrect number of blocks calculated" << std::endl;
		return 1;
	}
	
	std::string name = sequence->atomForBlock(2)->atomName();
	if (name != "C")
	{
		std::cout << "Block 1 is not C, instead " << name << std::endl;
		return 1;
	}
	
	name = sequence->atomForBlock(7)->atomName();
	if (name != "N")
	{
		std::cout << "Block 6 is not N, instead " << name << std::endl;
		return 1;
	}

	delete atoms;

	return 0;
}

