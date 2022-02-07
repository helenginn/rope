#include "../AtomGroup.h"
#include "../Cif2Geometry.h"
#include "../BondTorsion.h"

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	AtomGroup &glycine = *geom.atoms();
	glycine.recalculate();

	bool bad = false;
	for (size_t i = 0; i < glycine.bondTorsionCount(); i++)
	{
		BondTorsion *t = glycine.bondTorsion(i);

		double init = t->measurement(BondTorsion::SourceInitial);
		double derived = t->measurement(BondTorsion::SourceDerived);
		
		if (fabs(init - derived) > 5)
		{
			bad = true;
			std::cout << "Problem for torsion " << t->desc() << ", "
			"init angle " << init << " does not match " <<
			derived << std::endl;
		}
		else
		{
			std::cout << "MATCH for torsion " << t->desc() << ", "
			"init angle " << init << " matches " <<
			derived << std::endl;

		}
	}
	
	return bad;
}
