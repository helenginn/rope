#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/CHX.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	for (size_t i = 0; i < atoms->possibleAnchorCount(); i++)
	{
		BondSequence *sequence = new BondSequence();
		sequence->addToGraph(atoms->possibleAnchor(i), UINT_MAX);

		int added = sequence->atomGraphCount();

		for (size_t i = 0; i < added; i++)
		{
			bool result = sequence->checkAtomGraph(i);
			
			if (result != 0)
			{
				std::cout << "Problem with block " << i << std::endl;
				return result;
			}
		}
	}
	
	delete atoms;

	return 0;
}

