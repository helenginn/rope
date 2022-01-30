#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondSequence.h"
#include "../src/BondSequenceHandler.h"
#include "../src/ConcertedBasis.h"
#include "../src/BondCalculator.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setTorsionBasisType(TorsionBasis::TypeConcerted);
	calculator.setup();

	const TorsionBasis &basis = *calculator.sequenceHandler()->torsionBasis();
	int count = basis.torsionCount();
	std::cout << count << std::endl;
	
	for (size_t i = 0; i < count; i++)
	{
		const BondTorsion *t = basis.torsion(i);
		std::cout << t->desc() << std::endl;
	}
	
	return !(count == 2);
}

