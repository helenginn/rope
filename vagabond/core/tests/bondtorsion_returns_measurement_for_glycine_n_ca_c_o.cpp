#include <iostream>
#include "../AtomGroup.h"
#include "../Cif2Geometry.h"
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

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup &glycine = *geom.atoms();
	
	int bad = false;
	bad |= measure_torsion(glycine, "N", "CA", "C", "O", 63.64);
	bad |= measure_torsion(glycine, "O", "C", "CA", "N", 63.64);
	bad |= measure_torsion(glycine, "N", "CA", "C", "OXT", -116.35);
	
	delete &glycine;
	return bad;
}
