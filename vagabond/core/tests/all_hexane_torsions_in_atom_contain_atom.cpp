#include <iostream>
#include "../AtomGroup.h"
#include "../files/CifFile.h"
#include "../BondTorsion.h"

int main()
{
	std::string path = "/assets/geometry/HEX.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup &hexane = *geom.atoms();
	
	for (size_t i = 0; i < hexane.size(); i++)
	{
		Atom *atom = hexane[i];
		
		for (size_t j = 0; j < atom->bondTorsionCount(); j++)
		{
			BondTorsion *torsion = atom->bondTorsion(j);
			bool found = false;
			for (size_t k = 0; k < 4; k++)
			{
				if (torsion->atom(k) == atom)
				{
					found = true;
					break;
				}
			}
			
			if (!found)
			{
				return 1;
			}
		}
	}
	
	delete geom.atoms();
	return 0;
}
