#include "../AtomGroup.h"
#include "../Cif2Geometry.h"
#include "../BondTorsion.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

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
	
	if (bad)
	{
		BondSequence *sequence = new BondSequence();
		sequence->addToGraph(glycine.possibleAnchor(0), UINT_MAX);
		int added = sequence->addedAtomsCount();
		
		std::cout << "Added " << added << " atoms." << std::endl;

		for (size_t i = 0; i < sequence->atomGraphCount(); i++)
		{
			std::cout << sequence->atomGraphDesc(i);
		}
		
		delete sequence;

	}
	
	return bad;
}
