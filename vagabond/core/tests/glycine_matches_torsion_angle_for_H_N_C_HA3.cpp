#include <iostream>
#include "../AtomGroup.h"
#include "../CifFile.h"
#include "../BondTorsion.h"

int measure_torsion(AtomGroup &glycine, std::string a1, std::string a2, 
                    std::string a3, std::string a4, double expected)
{
	BondTorsion torsion = BondTorsion(nullptr, glycine[a1], glycine[a2], 
	                                  glycine[a3], glycine[a4], 0);
	double angle = torsion.measurement(BondTorsion::SourceInitial);

	std::cout << "Expected angle: " << expected << std::endl;
	std::cout << "Got: " << angle << std::endl;
	if (fabs(angle - expected) > 0.1)
	{
		return 1;
	}

	return 0;
}

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup &glycine = *geom.atoms();
	
	int bad = false;
	bad |= measure_torsion(glycine, "H", "N", "CA", "HA3", -114.601);
	
	delete &glycine;
	return bad;
}
