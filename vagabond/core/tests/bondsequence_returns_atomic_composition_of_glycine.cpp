#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");

	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(anchor);
	
	std::map<std::string, int> expected;
	expected["N"] = 1;
	expected["C"] = 2;
	expected["O"] = 2;
	expected["H"] = 5;

	std::map<std::string, int> obtained = sequence->elementList();
	
	if (obtained != expected)
	{
		std::map<std::string, int>::iterator it;
		
		for (it = obtained.begin(); it != obtained.end(); it++)
		{
			std::cout << it->first << " " << it->second << std::endl;
		}
		
		return 1;
	}

	return 0;
}


