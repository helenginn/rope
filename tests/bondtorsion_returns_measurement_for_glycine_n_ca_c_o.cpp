#include <iostream>
#include "../src/AtomGroup.h"
#include "../src/Cif2Geometry.h"
#include "../src/BondTorsion.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup &glycine = *geom.atoms();
	
	BondTorsion *torsion = glycine.findBondTorsion(glycine["N"], glycine["CA"], 
	                                                glycine["C"], glycine["O"]);
	
	double angle = torsion->measurement(BondTorsion::SourceInitial);
	double expected = 63.64;
	
	if (fabs(angle - expected) > 1.0)
	{
		std::cout << "Expected angle: " << expected << std::endl;
		std::cout << "Got: " << angle << std::endl;
		return 1;
	}
	
	delete &glycine;
	return 0;
}
